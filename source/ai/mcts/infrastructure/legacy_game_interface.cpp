/***************************************************************************
 *                   legacy_game_interface.cpp  -  description
 *                             -------------------
 *    begin                : 2025-11-08
 *    copyright            : (C) 2025
 *    email                : asc-hq.org
 ***************************************************************************/

#include "legacy_game_interface.h"
#include "../../../gamemap.h"
#include "../../../vehicle.h"
#include "../../../player.h"
#include "../../../mapfield.h"
#include "../../../actions/moveunitcommand.h"
#include "../../../actions/attackcommand.h"
#include "../../../actions/context.h"
#include <memory>
#include <stdexcept>

namespace asc {
namespace mcts {

// ===== LegacyGameInterface Implementation =====

LegacyGameInterface::LegacyGameInterface(GameMap* gameMap)
    : gameMap(gameMap)
{
    if (!gameMap) {
        throw std::invalid_argument("LegacyGameInterface: gameMap cannot be null");
    }
}

bool LegacyGameInterface::executeMove(UnitID unitID, 
                                     ::MapCoordinate destination,
                                     MapDisplayInterface* display)
{
    // LEGACY CODE INTEGRATION:
    // - Get Vehicle pointer from network ID
    // - Create MoveUnitCommand with new (manual memory management)
    // - Execute command
    // - Command ownership passes to GameMap on success
    // - Must manually delete on failure
    
    Vehicle* unit = getVehicle(unitID);
    if (!unit) {
        return false;  // Unit not found
    }
    
    // Check if move is available
    if (!MoveUnitCommand::avail(unit)) {
        return false;  // Unit cannot move (no movement left, etc.)
    }
    
    // Create command (LEGACY: must use new, manual ownership)
    std::unique_ptr<MoveUnitCommand> command(new MoveUnitCommand(unit));
    
    // Search reachable fields
    ActionResult searchResult = command->searchFields();
    if (!searchResult.successful()) {
        return false;  // Cannot calculate reachable fields
    }
    
    // Check if destination is reachable
    if (!command->isFieldReachable(destination, true)) {
        return false;  // Destination not reachable this turn
    }
    
    // Set destination and calculate path
    command->setDestination(destination);
    command->calcPath();
    
    // Execute command
    // LEGACY CODE ISSUE: Context is created here, should ideally be passed in
    Context context;
    context.gamemap = gameMap;
    context.display = display;
    context.actingPlayer = &gameMap->player[unit->getOwner()];
    
    ActionResult result = command->execute(context);
    
    if (result.successful()) {
        // LEGACY: Command ownership transfers to GameMap
        command.release();
        return true;
    } else {
        // LEGACY: unique_ptr will delete command automatically
        return false;
    }
}

bool LegacyGameInterface::executeAttack(UnitID attackerID,
                                       UnitID targetID,
                                       MapDisplayInterface* display)
{
    // LEGACY CODE INTEGRATION:
    // Similar pattern to executeMove
    
    Vehicle* attacker = getVehicle(attackerID);
    if (!attacker) {
        return false;
    }
    
    Vehicle* target = getVehicle(targetID);
    if (!target) {
        return false;
    }
    
    // Check if attack is available
    if (!AttackCommand::avail(attacker)) {
        return false;  // Unit cannot attack
    }
    
    // Create command (LEGACY: manual memory management)
    std::unique_ptr<AttackCommand> command(new AttackCommand(attacker));
    
    // Search targets
    ActionResult searchResult = command->searchTargets();
    if (!searchResult.successful()) {
        return false;
    }
    
    // Check if target is attackable
    const auto& attackableUnits = command->getAttackableUnits();
    ::MapCoordinate targetPos(target->xpos, target->ypos);
    
    if (attackableUnits.find(targetPos) == attackableUnits.end()) {
        return false;  // Target not in range
    }
    
    // Set target
    command->setTarget(targetPos);
    
    // Execute command
    Context context;
    context.gamemap = gameMap;
    context.display = display;
    context.actingPlayer = &gameMap->player[attacker->getOwner()];
    
    ActionResult result = command->execute(context);
    
    if (result.successful()) {
        command.release();  // Ownership to GameMap
        return true;
    } else {
        return false;
    }
}

bool LegacyGameInterface::executeWait(UnitID unitID)
{
    // LEGACY CODE INTEGRATION:
    // ASC doesn't have an explicit "Wait" command
    // Waiting is implicit - just don't move/attack the unit
    // Mark unit as having attacked (which prevents further actions)
    
    Vehicle* unit = getVehicle(unitID);
    if (!unit) {
        return false;
    }
    
    // LEGACY CODE ISSUE: Direct state mutation (not through Command pattern)
    // This should ideally be a WaitCommand, but ASC doesn't have one
    // FUTURE: Create WaitCommand class
    unit->setAttacked();
    unit->setMovement(0);  // Set movement to zero
    
    return true;
}

bool LegacyGameInterface::canMove(UnitID unitID, ::MapCoordinate destination) const
{
    // LEGACY CODE ISSUE: Must cast away const to use ASC's non-const API
    // FUTURE: Make GameMap query methods const-correct
    
    Vehicle* unit = const_cast<LegacyGameInterface*>(this)->getVehicle(unitID);
    if (!unit) {
        return false;
    }
    
    // Check if unit can move at all
    if (!MoveUnitCommand::avail(unit)) {
        return false;
    }
    
    // LEGACY CODE ISSUE: Must create command to check reachability
    // This is expensive - creates command, searches fields, then discards
    // FUTURE: Cache reachable fields or provide cheaper query API
    
    std::unique_ptr<MoveUnitCommand> command(new MoveUnitCommand(unit));
    
    ActionResult searchResult = command->searchFields();
    if (!searchResult.successful()) {
        return false;
    }
    
    return command->isFieldReachable(destination, true);
}

bool LegacyGameInterface::canAttack(UnitID attackerID, UnitID targetID) const
{
    // LEGACY CODE ISSUE: Same const-correctness issues as canMove
    
    Vehicle* attacker = const_cast<LegacyGameInterface*>(this)->getVehicle(attackerID);
    if (!attacker) {
        return false;
    }
    
    Vehicle* target = const_cast<LegacyGameInterface*>(this)->getVehicle(targetID);
    if (!target) {
        return false;
    }
    
    // Check if attacker can attack at all
    if (!AttackCommand::avail(attacker)) {
        return false;
    }
    
    // LEGACY CODE ISSUE: Expensive check - creates command, searches targets
    std::unique_ptr<AttackCommand> command(new AttackCommand(attacker));
    
    ActionResult searchResult = command->searchTargets();
    if (!searchResult.successful()) {
        return false;
    }
    
    // Check if target is in attackable list
    ::MapCoordinate targetPos(target->xpos, target->ypos);
    const auto& attackableUnits = command->getAttackableUnits();
    
    return (attackableUnits.find(targetPos) != attackableUnits.end());
}

std::vector<UnitID> LegacyGameInterface::getPlayerUnits(PlayerID playerID) const
{
    std::vector<UnitID> units;
    
    // Validate player ID
    if (playerID < 0 || playerID >= 8) {
        return units;  // Empty vector
    }
    
    // LEGACY CODE: Iterate raw pointer list
    // FUTURE: Player should have getUnits() returning const vector<unique_ptr<Vehicle>>
    
    const Player& player = gameMap->player[playerID];
    
    for (auto it = player.vehicleList.begin(); it != player.vehicleList.end(); ++it) {
        Vehicle* unit = *it;
        if (unit) {
            units.push_back(unit->networkid);
        }
    }
    
    return units;
}

UnitID LegacyGameInterface::getUnitAt(::MapCoordinate position) const
{
    // LEGACY CODE: Get field, check for vehicle
    // FUTURE: GameMap should have getUnitAt(MapCoordinate) method
    
    // Validate coordinates
    if (position.x < 0 || position.x >= gameMap->xsize ||
        position.y < 0 || position.y >= gameMap->ysize) {
        return -1;  // Invalid position
    }
    
    // Get field (LEGACY: uses array indexing)
    MapField* field = gameMap->getField(position.x, position.y);
    if (!field) {
        return -1;
    }
    
    // Check for vehicle on field
    if (field->vehicle) {
        return field->vehicle->networkid;
    }
    
    return -1;  // No unit at position
}

bool LegacyGameInterface::hasUnitFinishedTurn(UnitID unitID) const
{
    Vehicle* unit = const_cast<LegacyGameInterface*>(this)->getVehicle(unitID);
    if (!unit) {
        return true;  // Invalid unit = consider finished
    }
    
    // LEGACY CODE: Check multiple flags to determine if unit finished
    // A unit is finished if:
    // - It has attacked (attacked flag set)
    // - It has no movement left
    // - Cannot move and cannot attack
    
    if (unit->attacked) {
        return true;
    }
    
    if (unit->getMovement() <= 0) {
        return true;
    }
    
    // Check if any actions are available
    if (!MoveUnitCommand::avail(unit) && !AttackCommand::avail(unit)) {
        return true;
    }
    
    return false;
}

Vehicle* LegacyGameInterface::getVehicle(UnitID unitID) const
{
    // LEGACY CODE: GameMap::getUnit() is not const-correct
    // Must cast away const
    
    return const_cast<GameMap*>(gameMap)->getUnit(unitID, false);
}

// ===== Factory Function =====

std::unique_ptr<ILegacyGameInterface> createLegacyGameInterface(GameMap* gameMap)
{
    return std::make_unique<LegacyGameInterface>(gameMap);
}

} // namespace mcts
} // namespace asc
