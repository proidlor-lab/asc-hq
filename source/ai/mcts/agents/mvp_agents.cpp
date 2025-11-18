/***************************************************************************
 * mvp_agents.cpp - Implementations of MVP tactical agents
 ***************************************************************************/

#include "mvp_agents.h"
#include "reaction_fire_detector.h"

#include "../domain/combat_calculator_adapter.h"
#include "../domain/i_combat_calculator.h"
#include "../domain/game_state_snapshot.h"
#include "../../../vehicletype.h"

#include <algorithm>
#include <cmath>

namespace asc {
namespace mcts {

namespace {

constexpr float clampUtility(float value) {
   return std::clamp(value, -1.0f, 1.0f);
}

constexpr float normalizeHP(const UnitSnapshot& unit) {
   return static_cast<float>(unit.getHPPercent()) / 100.0f;
}

const UnitSnapshot* getUnitForAction(const GameStateSnapshot& state, const Action& action) {
   UnitID id = std::visit(overloaded{[](const MoveAction& m) { return m.unitID; },
                                     [](const AttackAction& a) { return a.attackerID; },
                                     [](const WaitAction& w) { return w.unitID; }},
                          action);
   return state.findUnit(id);
}

}  // namespace

// === LegalMoveAgent =======================================================

LegalMoveAgent::LegalMoveAgent() : name_("LegalMove"), weight_(1.0f) {}

AgentScore LegalMoveAgent::evaluate(const Action& action, const AgentContext& context) const {
   AgentScore score{};

   const auto validateDestination = [&](const MapCoordinate& coordinate) {
      if (!context.state.isValidCoordinate(coordinate)) {
         score.isVeto = true;
         score.reasoning = "Destination out of bounds";
         return;
      }

      if (const auto* occupant = context.state.getUnitAt(coordinate)) {
         if (occupant->owner == context.playerId) {
            score.isVeto = true;
            score.reasoning = "Destination occupied by friendly unit";
         }
      }
   };

   std::visit(overloaded{[&](const MoveAction& move) {
                            if (!context.actingUnit) {
                               score.isVeto = true;
                               score.reasoning = "Missing acting unit";
                               return;
                            }

                            if (move.unitID != context.actingUnit->networkID) {
                               score.isVeto = true;
                               score.reasoning = "Unit mismatch";
                               return;
                            }

                            if (!context.actingUnit->canMove()) {
                               score.isVeto = true;
                               score.reasoning = "No movement points";
                               return;
                            }

                            validateDestination(move.destination);
                         },
                         [&](const AttackAction& attack) {
                            if (!context.actingUnit ||
                                attack.attackerID != context.actingUnit->networkID) {
                               score.isVeto = true;
                               score.reasoning = "Invalid attacker";
                               return;
                            }

                            if (!context.state.isValidCoordinate(attack.target)) {
                               score.isVeto = true;
                               score.reasoning = "Attack target out of bounds";
                               return;
                            }

                            if (!context.actingUnit->canAttack()) {
                               score.isVeto = true;
                               score.reasoning = "No attack capability";
                            }
                         },
                         [&](const WaitAction&) { score.utility = 0.0f; }},
              action);

   return score;
}

// === ReactionFireAgent ====================================================

ReactionFireAgent::ReactionFireAgent() : name_("ReactionFire"), weight_(3.0f) {}

AgentScore ReactionFireAgent::evaluate(const Action& action, const AgentContext& context) const {
   AgentScore score{};

   if (!context.actingUnit || !context.rfDetector) {
      return score;
   }

   std::visit(overloaded{[&](const MoveAction& move) {
                            const auto& threats =
                               context.rfDetector->getThreatsAt(move.destination);
                            if (threats.empty()) {
                               score.utility = 0.0f;
                               return;
                            }

                            float totalDamage = 0.0f;
                            for (const auto& threat : threats) {
                               totalDamage +=
                                  computeExpectedDamage(threat, context, move.destination);
                            }

                            const float hp = static_cast<float>(context.actingUnit->getHPPercent());
                            if (hp > 0.0f) {
                               score.utility = clampUtility(-totalDamage / hp);
                            } else {
                               score.utility = -1.0f;
                            }
                         },
                         [&](const AttackAction&) {
                            // Attack actions do not trigger RF directly (handled elsewhere)
                            score.utility = 0.0f;
                         },
                         [&](const WaitAction&) { score.utility = 0.0f; }},
              action);

   return score;
}

float ReactionFireAgent::computeExpectedDamage(const ReactionFireThreat& threat,
                                               const AgentContext& context,
                                               const MapCoordinate& destination) const {
   if (!context.combatCalculator || threat.attacker == nullptr || context.actingUnit == nullptr) {
      return 25.0f;  // fallback penalty
   }

   UnitSnapshot defenderCopy = *context.actingUnit;
   defenderCopy.x = destination.x;
   defenderCopy.y = destination.y;

   return context.combatCalculator->calculateExpectedDamage(*threat.attacker, defenderCopy,
                                                            threat.attackPosition);
}

// === AggressivenessAgent ==================================================

AggressivenessAgent::AggressivenessAgent() : name_("Aggressiveness"), weight_(1.5f) {}

AgentScore AggressivenessAgent::evaluate(const Action& action, const AgentContext& context) const {
   AgentScore score{};
   if (!context.actingUnit) {
      return score;
   }

   const UnitRole role = UnitRoleClassifier::detectRole(*context.actingUnit);

   std::visit(overloaded{[&](const AttackAction&) {
                            float utility = 1.0f;
                            if (role == UnitRole::SERVICE_PRIMARY) {
                               utility = -0.5f;
                            }
                            score.utility = clampUtility(utility);
                         },
                         [&](const MoveAction&) {
                            score.utility = (role == UnitRole::SERVICE_PRIMARY) ? -0.1f : 0.1f;
                         },
                         [&](const WaitAction&) { score.utility = -0.3f; }},
              action);

   return score;
}

// === TargetPriorityAgent ==================================================

TargetPriorityAgent::TargetPriorityAgent() : name_("TargetPriority"), weight_(1.2f) {}

AgentScore TargetPriorityAgent::evaluate(const Action& action, const AgentContext& context) const {
   AgentScore score{};

   if (!context.actingUnit) {
      return score;
   }

   std::visit(overloaded{[&](const AttackAction& attack) {
                            const auto* targetUnit = context.state.getUnitAt(attack.target);
                            if (!targetUnit) {
                               score.utility = -0.2f;  // discourage wasting shots
                               return;
                            }

                            if (targetUnit->owner == context.playerId) {
                               score.isVeto = true;
                               score.reasoning = "Would attack friendly unit";
                               return;
                            }

                            const float hpFactor = 1.0f - normalizeHP(*targetUnit);
                            float valueFactor = 0.5f;
                            if (targetUnit->type) {
                               valueFactor = std::min(
                                  1.0f, static_cast<float>(targetUnit->type->armor) / 200.0f);
                            }

                            const float combined = 0.7f * hpFactor + 0.3f * valueFactor;
                            score.utility = clampUtility(combined);
                         },
                         [&](const MoveAction&) { score.utility = 0.0f; },
                         [&](const WaitAction&) { score.utility = -0.1f; }},
              action);

   return score;
}

// === ServiceUtilityAgent ==================================================

ServiceUtilityAgent::ServiceUtilityAgent() : name_("ServiceUtility"), weight_(2.5f) {}

AgentScore ServiceUtilityAgent::evaluate(const Action& action, const AgentContext& context) const {
   AgentScore score{};
   if (!context.actingUnit) {
      return score;
   }

   const UnitRole role = UnitRoleClassifier::detectRole(*context.actingUnit);
   const float hpPercent = normalizeHP(*context.actingUnit);

   std::visit(overloaded{[&](const AttackAction&) {
                            if (role == UnitRole::SERVICE_PRIMARY) {
                               score.isVeto = true;
                               score.reasoning = "Service unit should not attack";
                            } else {
                               score.utility = -0.1f;
                            }
                         },
                         [&](const MoveAction&) {
                            score.utility = (role == UnitRole::SERVICE_PRIMARY) ? 0.2f : 0.0f;
                         },
                         [&](const WaitAction&) {
                            if (hpPercent < 0.4f) {
                               score.utility = 0.8f;  // Rest to avoid destruction
                            } else {
                               score.utility = 0.2f;
                            }
                         }},
              action);

   return score;
}

}  // namespace mcts
}  // namespace asc
