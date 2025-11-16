/***************************************************************************
 *                   legacy_game_interface.h  -  description
 *                             -------------------
 *    begin                : 2025-11-08
 *    copyright            : (C) 2025
 *    email                : asc-hq.org
 ***************************************************************************/

/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef LEGACY_GAME_INTERFACE_H
#define LEGACY_GAME_INTERFACE_H

#include <memory>
#include <vector>
#include "../../../typen.h"  // For ASC's MapCoordinate and other types
#include "../domain/types.h"

// Forward declarations to avoid including legacy headers
class GameMap;
class Vehicle;
class MapDisplayInterface;

namespace asc {
namespace mcts {

/**
 * @brief Interface to execute actions on the real game (not simulation)
 * 
 * This interface isolates the MCTS AI from legacy ASC code that must be
 * used to execute actions on the real GameMap. It provides a clean,
 * modern C++ interface while internally dealing with legacy pointers,
 * manual memory management, and ASC's Command system.
 * 
 * DESIGN RATIONALE:
 * - Isolate legacy code interactions to this adapter
 * - Provide testable interface (can mock for unit tests)
 * - Enable future migration to modern C++ without touching MCTS core
 * 
 * LEGACY CODE ISSUES (see implementation notes):
 * 1. GameMap uses raw pointers everywhere (Vehicle*, Building*, etc.)
 * 2. Command system uses manual memory management (new/delete)
 * 3. No const-correctness in legacy API
 * 4. Global state and side effects everywhere
 * 5. Thread-unsafe (assumes single-threaded execution)
 */
class ILegacyGameInterface {
public:
    virtual ~ILegacyGameInterface() = default;
    
    /**
     * @brief Execute a move action on the real game map
     * 
     * @param unitID Network ID of the unit to move
     * @param destination Target hex coordinate
     * @param display Optional display interface for visualization
     * @return true if move executed successfully
     * 
     * LEGACY INTEGRATION:
     * - Must use MoveUnitCommand from commands.h
     * - Must handle pathfinding through legacy system
     * - Must trigger all side effects (view updates, events, etc.)
     */
    virtual bool executeMove(UnitID unitID, 
                            ::MapCoordinate destination,
                            MapDisplayInterface* display = nullptr) = 0;
    
    /**
     * @brief Execute an attack action on the real game map
     * 
     * @param attackerID Network ID of the attacking unit
     * @param targetID Network ID of the target unit
     * @param display Optional display interface for visualization
     * @return true if attack executed successfully
     * 
     * LEGACY INTEGRATION:
     * - Must use AttackCommand from commands.h
     * - Must handle combat resolution through legacy system
     * - Must trigger all side effects (destruction, XP, etc.)
     */
    virtual bool executeAttack(UnitID attackerID,
                              UnitID targetID,
                              MapDisplayInterface* display = nullptr) = 0;
    
    /**
     * @brief Execute a wait/end-turn action for a unit
     * 
     * @param unitID Network ID of the unit
     * @return true if wait executed successfully
     */
    virtual bool executeWait(UnitID unitID) = 0;
    
    /**
     * @brief Check if a unit can move to a destination
     * 
     * @param unitID Network ID of the unit
     * @param destination Target hex coordinate
     * @return true if move is legal
     * 
     * LEGACY INTEGRATION:
     * - Must use legacy pathfinding system
     * - Must check terrain, fuel, other units, etc.
     */
    virtual bool canMove(UnitID unitID, ::MapCoordinate destination) const = 0;
    
    /**
     * @brief Check if a unit can attack a target
     * 
     * @param attackerID Network ID of the attacking unit
     * @param targetID Network ID of the target
     * @return true if attack is legal
     * 
     * LEGACY INTEGRATION:
     * - Must check weapon ranges, ammo, etc.
     */
    virtual bool canAttack(UnitID attackerID, UnitID targetID) const = 0;
    
    /**
     * @brief Get all units owned by a player
     * 
     * @param playerID Player ID (0-7)
     * @return Vector of unit network IDs
     * 
     * LEGACY INTEGRATION:
     * - Must iterate Player::vehicleList (raw pointers)
     * - Must extract network IDs safely
     */
    virtual std::vector<UnitID> getPlayerUnits(PlayerID playerID) const = 0;
    
    /**
     * @brief Get unit at a specific position
     * 
     * @param position Hex coordinate
     * @return Unit ID, or -1 if no unit at position
     */
    virtual UnitID getUnitAt(::MapCoordinate position) const = 0;
    
    /**
     * @brief Check if a unit has finished its turn
     * 
     * @param unitID Network ID of the unit
     * @return true if unit has no more actions this turn
     */
    virtual bool hasUnitFinishedTurn(UnitID unitID) const = 0;
};

/**
 * @brief Concrete implementation of ILegacyGameInterface
 * 
 * This class contains all the ugly legacy code interactions.
 * It should be the ONLY place where MCTS code directly touches
 * ASC's legacy GameMap, Vehicle, Command classes, etc.
 * 
 * FUTURE MIGRATION PATH:
 * When ASC code is modernized:
 * 1. Update implementation of this class
 * 2. MCTS core code requires no changes
 * 3. Interface remains stable
 */
class LegacyGameInterface : public ILegacyGameInterface {
public:
    /**
     * @brief Construct interface for a game map
     * 
     * @param gameMap Pointer to legacy GameMap (not owned)
     * 
     * LEGACY CODE ISSUE:
     * - Takes raw pointer (GameMap has no copy/move constructors)
     * - Assumes pointer remains valid for lifetime of interface
     * - No ownership semantics
     * 
     * FUTURE: GameMap should use std::shared_ptr
     */
    explicit LegacyGameInterface(GameMap* gameMap);
    
    ~LegacyGameInterface() override = default;
    
    // Interface implementation
    bool executeMove(UnitID unitID, 
                    ::MapCoordinate destination,
                    MapDisplayInterface* display = nullptr) override;
    
    bool executeAttack(UnitID attackerID,
                      UnitID targetID,
                      MapDisplayInterface* display = nullptr) override;
    
    bool executeWait(UnitID unitID) override;
    
    bool canMove(UnitID unitID, ::MapCoordinate destination) const override;
    
    bool canAttack(UnitID attackerID, UnitID targetID) const override;
    
    std::vector<UnitID> getPlayerUnits(PlayerID playerID) const override;
    
    UnitID getUnitAt(::MapCoordinate position) const override;
    
    bool hasUnitFinishedTurn(UnitID unitID) const override;

private:
    GameMap* gameMap;  // Not owned, raw pointer to legacy code
    
    /**
     * @brief Get Vehicle pointer from network ID
     * 
     * LEGACY CODE ISSUE:
     * - GameMap::getUnit() returns raw pointer (Vehicle*)
     * - Pointer can become invalid if unit destroyed
     * - No reference counting or lifetime management
     * 
     * FUTURE: Should return std::shared_ptr<Vehicle>
     */
    Vehicle* getVehicle(UnitID unitID) const;
};

/**
 * @brief Factory for creating legacy game interface
 * 
 * @param gameMap Pointer to legacy GameMap
 * @return Unique pointer to interface
 */
std::unique_ptr<ILegacyGameInterface> createLegacyGameInterface(GameMap* gameMap);

} // namespace mcts
} // namespace asc

#endif // LEGACY_GAME_INTERFACE_H
