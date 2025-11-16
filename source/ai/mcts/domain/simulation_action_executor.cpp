/***************************************************************************
 * simulation_action_executor.cpp - Implementation of simulation executor
 *
 * Part of: ASC MCTS AI (Phase 0.2 - Action Execution Interface)
 * Updated: Phase 1.2 - Capability-based action generation
 ***************************************************************************/

#include "simulation_action_executor.h"
#include "abilities/ability_registry.h"
#include "combat_calculator.h"
#include <algorithm>
#include <cmath>
#include <sstream>
#include <stdexcept>

// Forward declarations from ASC
#include "../../../vehicletype.h"

namespace asc {
namespace mcts {

namespace {

std::unique_ptr<GameStateSnapshot> cloneSnapshot(const IGameState& state) {
   auto cloned = state.clone();
   auto* concrete = dynamic_cast<GameStateSnapshot*>(cloned.release());
   if (!concrete) {
      throw std::runtime_error("Clone produced incompatible game state type");
   }
   return std::unique_ptr<GameStateSnapshot>(concrete);
}

}  // unnamed namespace

// ========== Constructor ==========

SimulationActionExecutor::SimulationActionExecutor(std::unique_ptr<GameStateSnapshot> initialState,
                                                   GameMap* legacyMap)
   : state_(std::move(initialState)), legacyMap_(legacyMap) {
   if (!state_) {
      throw std::invalid_argument("SimulationActionExecutor: initialState cannot be null");
   }
}

// ========== IActionExecutor Interface ==========

ActionResult SimulationActionExecutor::execute(const Action& action,
                                               const ExecutionContext& context) {
   // Save state for undo (if undo stack not full)
   if (undoStack_.size() < MAX_UNDO_DEPTH) {
      saveUndoState();
   }

   // Execute action based on type
   ActionResult result = std::visit(
      overloaded{[this, &context](const MoveAction& m) { return executeMove(m, context); },
                 [this, &context](const AttackAction& a) { return executeAttack(a, context); },
                 [this, &context](const WaitAction& w) { return executeWait(w, context); }},
      action);

   // Log action if enabled
   if (context.enableLogging && result.isSuccess()) {
      context.log("Executed: " + actionToString(action));
   }

   // Track action history
   if (result.isSuccess()) {
      actionHistory_.push_back(action);

      // Callback
      if (context.onActionExecuted) {
         context.onActionExecuted(action);
      }
   }

   return result;
}

ActionResult SimulationActionExecutor::isLegal(const Action& action) const {
   return std::visit(overloaded{[this](const MoveAction& m) { return checkMoveLegal(m); },
                                [this](const AttackAction& a) { return checkAttackLegal(a); },
                                [this](const WaitAction& w) { return checkWaitLegal(w); }},
                     action);
}

std::vector<Action> SimulationActionExecutor::generateLegalActions(UnitID unitID,
                                                                   bool includeWait) const {
   // UPDATED (Phase 1.3): Use ability-based action generation with pathfinding context
   // If legacyMap available, use full pathfinding; otherwise use adjacent-only fallback

   std::vector<Action> actions;

   if (legacyMap_) {
      // Full pathfinding with legacy map context
      actions = AbilityActionGenerator::generateAllActionsWithContext(*state_, unitID, legacyMap_);
   } else {
      // Fallback: adjacent-only generation (no pathfinding)
      actions = AbilityActionGenerator::generateAllActions(*state_, unitID);
   }

   // Filter wait action if not requested
   if (!includeWait) {
      actions.erase(
         std::remove_if(actions.begin(), actions.end(),
                        [](const Action& a) { return std::holds_alternative<WaitAction>(a); }),
         actions.end());
   }

   return actions;
}

bool SimulationActionExecutor::undo() {
   if (undoStack_.empty()) {
      return false;
   }

   // Restore previous state
   state_ = std::move(undoStack_.back());
   undoStack_.pop_back();

   // Remove last action from history
   if (!actionHistory_.empty()) {
      actionHistory_.pop_back();
   }

   return true;
}

// ========== Cloning ==========

std::unique_ptr<SimulationActionExecutor> SimulationActionExecutor::clone() const {
   auto clonedState = cloneSnapshot(*state_);
   return std::make_unique<SimulationActionExecutor>(std::move(clonedState), legacyMap_);
}

void SimulationActionExecutor::reset(std::unique_ptr<GameStateSnapshot> newState) {
   state_ = std::move(newState);
   actionHistory_.clear();
   undoStack_.clear();
}

// ========== Move Execution ==========

ActionResult SimulationActionExecutor::executeMove(const MoveAction& action,
                                                   const ExecutionContext& context) {
   // Legality check
   auto legalResult = checkMoveLegal(action);
   if (legalResult.isFailure()) {
      return legalResult;
   }

   // Find unit
   UnitSnapshot* unit = state_->findUnitMutable(action.unitID);
   if (!unit) {
      return ActionResult::illegalAction("Unit not found");
   }

   MapCoordinate from = unit->getPosition();
   MapCoordinate to = action.destination;

   // Calculate movement cost
   int moveCost = calculateMovementCost(*unit, from, to);
   if (moveCost < 0) {
      return ActionResult(ActionResultCode::Blocked, "Path blocked or unreachable");
   }

   if (moveCost > unit->movement) {
      return ActionResult(ActionResultCode::InsufficientMovement, "Not enough movement points");
   }

   // Simplified pathfinding for MVP (straight line approximation)
   std::vector<MapCoordinate> path = {from, to};

   // Simulate reaction fire (if enabled)
   if (context.enableReactionFire) {
      bool survived = simulateReactionFire(*unit, path, context);
      if (!survived) {
         return ActionResult(ActionResultCode::ReactionFire, "Unit destroyed by reaction fire");
      }
   }

   // Execute move
   unit->x = to.x;
   unit->y = to.y;

   // Consume movement points
   unit->movement -= static_cast<int16_t>(moveCost);

   // Consume fuel (if enabled)
   int fuelCost = moveCost / 10;  // Simplified: 1 fuel per 10 movement points
   if (context.enableFuelConsumption && fuelCost > 0) {
      unit->fuel = static_cast<uint16_t>(std::max(0, static_cast<int>(unit->fuel) - fuelCost));
   }

   ActionResult result = ActionResult::success();
   result.movementConsumed = moveCost;
   result.fuelConsumed = fuelCost;

   return result;
}

// ========== Attack Execution ==========

ActionResult SimulationActionExecutor::executeAttack(const AttackAction& action,
                                                     const ExecutionContext& context) {
   // Legality check
   auto legalResult = checkAttackLegal(action);
   if (legalResult.isFailure()) {
      return legalResult;
   }

   // Find attacker
   UnitSnapshot* attacker = state_->findUnitMutable(action.attackerID);
   if (!attacker) {
      return ActionResult::illegalAction("Attacker not found");
   }

   // Find defender
   const UnitSnapshot* defender = state_->getUnitAt(action.target);
   if (!defender) {
      return ActionResult::illegalAction("No target at position");
   }

   // Get mutable defender (we need to modify it)
   UnitSnapshot* defenderMut = state_->findUnitMutable(defender->networkID);
   if (!defenderMut) {
      return ActionResult::illegalAction("Defender not found");
   }

   // Calculate damage
   int damage = calculateDamage(*attacker, *defenderMut, action.weaponIndex);

   // Apply damage
   defenderMut->damage =
      static_cast<uint8_t>(std::min(100, static_cast<int>(defenderMut->damage) + damage));

   // Mark attacker as having attacked
   attacker->attacked = true;

   // Consume ammo (simplified for MVP)
   if (context.enableAmmoConsumption && action.weaponIndex >= 0) {
      // Clear ammo bit for this weapon
      attacker->ammoMask &= ~(1 << action.weaponIndex);
   }

   // Remove destroyed unit (if HP <= 0)
   if (defenderMut->isDestroyed()) {
      // Remove from units vector
      auto it = std::remove_if(
         state_->units.begin(), state_->units.end(),
         [defenderMut](const UnitSnapshot& u) { return u.networkID == defenderMut->networkID; });
      state_->units.erase(it, state_->units.end());
   }

   ActionResult result = ActionResult::success();
   result.damageDealt = damage;

   return result;
}

// ========== Wait Execution ==========

ActionResult SimulationActionExecutor::executeWait(const WaitAction& action,
                                                   const ExecutionContext& context) {
   // Legality check
   auto legalResult = checkWaitLegal(action);
   if (legalResult.isFailure()) {
      return legalResult;
   }

   // Find unit
   UnitSnapshot* unit = state_->findUnitMutable(action.unitID);
   if (!unit) {
      return ActionResult::illegalAction("Unit not found");
   }

   // Mark unit as done for this turn (consume all movement)
   unit->movement = 0;

   context.log("Unit " + std::to_string(action.unitID) + " waits");

   return ActionResult::success();
}

// ========== Legality Checks ==========

ActionResult SimulationActionExecutor::checkMoveLegal(const MoveAction& action) const {
   // Find unit
   const UnitSnapshot* unit = state_->findUnit(action.unitID);
   if (!unit) {
      return ActionResult(ActionResultCode::UnitNotFound, "Unit not found");
   }

   // Check if unit can move
   if (!unit->canMove()) {
      return ActionResult(ActionResultCode::InsufficientMovement, "Unit cannot move");
   }

   // Check destination is valid
   if (!state_->isValidCoordinate(action.destination)) {
      return ActionResult(ActionResultCode::IllegalAction, "Invalid destination");
   }

   // Check destination is not occupied
   const UnitSnapshot* occupant = state_->getUnitAt(action.destination);
   if (occupant && occupant->networkID != action.unitID) {
      return ActionResult(ActionResultCode::Blocked, "Destination occupied");
   }

   return ActionResult::success();
}

ActionResult SimulationActionExecutor::checkAttackLegal(const AttackAction& action) const {
   // Find attacker
   const UnitSnapshot* attacker = state_->findUnit(action.attackerID);
   if (!attacker) {
      return ActionResult(ActionResultCode::UnitNotFound, "Attacker not found");
   }

   // Check if unit can attack
   if (!attacker->canAttack()) {
      if (attacker->attacked) {
         return ActionResult(ActionResultCode::IllegalAction, "Already attacked this turn");
      }
      if (attacker->ammoMask == 0) {
         return ActionResult(ActionResultCode::InsufficientAmmo, "No ammo");
      }
      return ActionResult(ActionResultCode::IllegalAction, "Cannot attack");
   }

   // Check target exists
   const UnitSnapshot* defender = state_->getUnitAt(action.target);
   if (!defender) {
      return ActionResult(ActionResultCode::TargetNotFound, "No target at position");
   }

   // Check target is enemy
   if (defender->owner == attacker->owner) {
      return ActionResult(ActionResultCode::IllegalAction, "Cannot attack own unit");
   }

   // Check range
   int weaponIdx = canHitTarget(*attacker, action.target, action.weaponIndex);
   if (weaponIdx < 0) {
      return ActionResult(ActionResultCode::OutOfRange, "Target out of range");
   }

   return ActionResult::success();
}

ActionResult SimulationActionExecutor::checkWaitLegal(const WaitAction& action) const {
   // Find unit
   const UnitSnapshot* unit = state_->findUnit(action.unitID);
   if (!unit) {
      return ActionResult(ActionResultCode::UnitNotFound, "Unit not found");
   }

   // Wait is always legal if unit exists
   return ActionResult::success();
}

// ========== Action Generation ==========

std::vector<MoveAction> SimulationActionExecutor::generateMoveActions(UnitID unitID) const {
   std::vector<MoveAction> actions;

   const UnitSnapshot* unit = state_->findUnit(unitID);
   if (!unit || !unit->canMove()) {
      return actions;
   }

   MapCoordinate currentPos = unit->getPosition();

   // Simplified move generation: neighbors only (MVP)
   // Post-MVP: Full A* pathfinding with movement budget
   auto neighbors = getNeighbors(currentPos);

   for (const auto& neighbor : neighbors) {
      MoveAction action{unitID, neighbor, unit->height};

      // Check if legal
      if (checkMoveLegal(action).isSuccess()) {
         actions.push_back(action);
      }
   }

   return actions;
}

std::vector<AttackAction> SimulationActionExecutor::generateAttackActions(UnitID unitID) const {
   std::vector<AttackAction> actions;

   const UnitSnapshot* unit = state_->findUnit(unitID);
   if (!unit || !unit->canAttack()) {
      return actions;
   }

   // Find all enemy units in range
   for (const auto& potentialTarget : state_->units) {
      // Skip own units
      if (potentialTarget.owner == unit->owner) {
         continue;
      }

      // Check if in range
      int weaponIdx = canHitTarget(*unit, potentialTarget.getPosition(), -1);
      if (weaponIdx >= 0) {
         AttackAction action{unitID, potentialTarget.getPosition(), weaponIdx};
         actions.push_back(action);
      }
   }

   return actions;
}

// ========== Helper Methods ==========

int SimulationActionExecutor::calculateMovementCost(const UnitSnapshot& unit,
                                                    const MapCoordinate& from,
                                                    const MapCoordinate& to) const {
   // Simplified for MVP: hex distance * 100 (ASC uses 100 per field typically)
   // Post-MVP: Use actual terrain costs from VehicleType
   int distance = hexDistance(from, to);
   return distance * 100;
}

bool SimulationActionExecutor::isReachable(const UnitSnapshot& unit,
                                           const MapCoordinate& dest) const {
   int cost = calculateMovementCost(unit, unit.getPosition(), dest);
   return cost >= 0 && cost <= unit.movement;
}

int SimulationActionExecutor::calculateDamage(const UnitSnapshot& attacker,
                                              const UnitSnapshot& defender, int weaponIndex) const {
   // Use CombatCalculator which implements ASC's actual combat formula

   // Calculate distance
   int dx = std::abs(defender.x - attacker.x);
   int dy = std::abs(defender.y - attacker.y);
   int distance = std::max(dx, dy);

   // TODO: Get terrain defense bonus from actual terrain
   // For now, use 0 (no terrain bonus)
   int terrainDefenseBonus = 0;

   // TODO: Calculate hemming factor (flanking bonus)
   // For now, use 1.0 (no hemming)
   float hemmingFactor = 1.0f;

   // Use the actual ASC combat formula
   return CombatCalculator::calculateDamage(attacker, defender, weaponIndex, distance,
                                            terrainDefenseBonus, hemmingFactor);
}

int SimulationActionExecutor::canHitTarget(const UnitSnapshot& attacker,
                                           const MapCoordinate& target, int weaponIndex) const {
   // Use actual weapon ranges from VehicleType and check target compatibility

   int distance = hexDistance(attacker.getPosition(), target);

   // Minimum range (can't attack own position)
   if (distance == 0) {
      return -1;
   }

   // Find target unit at this position (for type checking)
   const UnitSnapshot* targetUnit = state_->getUnitAt(target);

   // Fallback for testing/legacy: If no type data, use reasonable defaults
   if (attacker.type == nullptr || attacker.type->weapons.count == 0) {
      // Default fallback: range 1-10, weapon index 0
      // Only used when VehicleType is not available (e.g., unit tests)
      const int DEFAULT_MIN_RANGE = 1;
      const int DEFAULT_MAX_RANGE = 10;

      if (distance >= DEFAULT_MIN_RANGE && distance <= DEFAULT_MAX_RANGE) {
         // Check ammo for weapon 0
         if (weaponIndex >= 0) {
            if (weaponIndex >= 16 || !(attacker.ammoMask & (1 << weaponIndex))) {
               return -1;
            }
            return weaponIndex;
         } else {
            // Auto-select first weapon with ammo
            for (int i = 0; i < 16; ++i) {
               if (attacker.ammoMask & (1 << i)) {
                  return i;
               }
            }
         }
      }
      return -1;
   }

   // If specific weapon requested, check if it can reach and target
   if (weaponIndex >= 0) {
      if (weaponIndex >= 16 || weaponIndex >= attacker.type->weapons.count) {
         return -1;  // Weapon doesn't exist
      }

      // Check ammo
      if (!(attacker.ammoMask & (1 << weaponIndex))) {
         return -1;  // No ammo
      }

      const auto& weapon = attacker.type->weapons.weapon[weaponIndex];

      // Check range (weapon ranges are stored as multiples of 10: 10 = 1 hex, 100 = 10 hexes)
      int minRange = (weapon.mindistance + 9) / 10;  // Round up
      int maxRange = weapon.maxdistance / 10;

      if (distance < minRange || distance > maxRange) {
         return -1;  // Out of range
      }

      // Check if weapon can target this unit type
      if (targetUnit != nullptr && targetUnit->type != nullptr) {
         auto targetCheck = CombatCalculator::canWeaponTarget(
            weapon, attacker.height, targetUnit->height, targetUnit->type->movemalustyp);
         if (!targetCheck.canTarget) {
            return -1;  // Cannot target this unit type/height
         }
      }

      return weaponIndex;
   }

   // Auto-select first available weapon that can reach and target
   for (int i = 0; i < attacker.type->weapons.count && i < 16; ++i) {
      const auto& weapon = attacker.type->weapons.weapon[i];

      // Convert weapon ranges (stored as multiples of 10)
      int minRange = (weapon.mindistance + 9) / 10;
      int maxRange = weapon.maxdistance / 10;

      // Check range and ammo
      if (!(attacker.ammoMask & (1 << i)) || distance < minRange || distance > maxRange) {
         continue;
      }

      // Check if weapon can target this unit type
      if (targetUnit != nullptr && targetUnit->type != nullptr) {
         auto targetCheck = CombatCalculator::canWeaponTarget(
            weapon, attacker.height, targetUnit->height, targetUnit->type->movemalustyp);
         if (!targetCheck.canTarget) {
            continue;  // Try next weapon
         }
      }

      return i;  // Found valid weapon
   }

   return -1;  // No weapon can reach target
}

bool SimulationActionExecutor::simulateReactionFire(UnitSnapshot& movingUnit,
                                                    const std::vector<MapCoordinate>& path,
                                                    const ExecutionContext& context) {
   // Simplified reaction fire for MVP
   // Post-MVP: Full ASC reaction fire simulation

   // Find enemy units that might reaction fire
   for (const auto& enemy : state_->units) {
      // Skip own units
      if (enemy.owner == movingUnit.owner) {
         continue;
      }

      // Check if enemy can attack
      if (!enemy.canAttack()) {
         continue;
      }

      // Check if any point in path is in range
      for (const auto& pathPoint : path) {
         int weaponIdx = canHitTarget(enemy, pathPoint, -1);
         if (weaponIdx >= 0) {
            // Enemy can reaction fire!
            // Use actual weapon damage calculation
            // Note: RF hit chance is simplified (deterministic for now)

            int damage = calculateDamage(enemy, movingUnit, weaponIdx);
            movingUnit.damage =
               static_cast<uint8_t>(std::min(100, static_cast<int>(movingUnit.damage) + damage));

            context.log("Reaction fire: Unit " + std::to_string(enemy.networkID) + " hits unit " +
                        std::to_string(movingUnit.networkID) + " for " + std::to_string(damage) +
                        " damage");

            // Check if unit destroyed
            if (movingUnit.isDestroyed()) {
               return false;
            }

            // Only one reaction fire per move (simplified)
            break;
         }
      }
   }

   return true;
}

std::vector<MapCoordinate> SimulationActionExecutor::getNeighbors(const MapCoordinate& pos) const {
   // Hex grid neighbors (6 directions)
   // Offset coordinates (even-r horizontal layout)
   std::vector<MapCoordinate> neighbors;

   // Even row
   if (pos.y % 2 == 0) {
      neighbors = {
         MapCoordinate(pos.x + 1, pos.y),      // E
         MapCoordinate(pos.x - 1, pos.y),      // W
         MapCoordinate(pos.x, pos.y - 1),      // NE
         MapCoordinate(pos.x - 1, pos.y - 1),  // NW
         MapCoordinate(pos.x, pos.y + 1),      // SE
         MapCoordinate(pos.x - 1, pos.y + 1)   // SW
      };
   } else {
      // Odd row
      neighbors = {
         MapCoordinate(pos.x + 1, pos.y),      // E
         MapCoordinate(pos.x - 1, pos.y),      // W
         MapCoordinate(pos.x + 1, pos.y - 1),  // NE
         MapCoordinate(pos.x, pos.y - 1),      // NW
         MapCoordinate(pos.x + 1, pos.y + 1),  // SE
         MapCoordinate(pos.x, pos.y + 1)       // SW
      };
   }

   // Filter out invalid coordinates
   neighbors.erase(
      std::remove_if(neighbors.begin(), neighbors.end(),
                     [this](const MapCoordinate& c) { return !state_->isValidCoordinate(c); }),
      neighbors.end());

   return neighbors;
}

void SimulationActionExecutor::saveUndoState() {
   undoStack_.push_back(cloneSnapshot(*state_));

   // Limit undo stack size
   while (undoStack_.size() > MAX_UNDO_DEPTH) {
      undoStack_.pop_front();
   }
}

}  // namespace mcts
}  // namespace asc
