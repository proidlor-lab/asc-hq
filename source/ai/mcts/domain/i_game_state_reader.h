/***************************************************************************
 * i_game_state_reader.h - Interface for reading game state (DI)
 *
 * Purpose: Abstract interface to decouple MCTS from legacy GameMap
 *          Enables dependency injection and testing with mock implementations
 *
 * Part of: ASC MCTS AI (Phase 0.1 - Game State Cloning)
 ***************************************************************************/

#ifndef MCTS_I_GAME_STATE_READER_H
#define MCTS_I_GAME_STATE_READER_H

#include "types.h"
#include "unit_snapshot.h"
#include <vector>
#include <memory>

// Forward declarations
class Vehicle;
class Building;
class MapField;
class TerrainType;
class GameMap;

namespace asc {
namespace mcts {

// Forward declarations
class GameStateSnapshot;

/**
 * Interface for reading game state data
 *
 * Design Rationale (Dependency Injection):
 * - Decouples MCTS code from legacy GameMap implementation
 * - Allows testing with mock implementations
 * - Provides clean, minimal API for snapshot creation
 * - Hides complexity of legacy pointer hierarchies
 *
 * Usage:
 *   unique_ptr<IGameStateReader> reader = createGameStateReader(gameMap);
 *   auto snapshot = reader->createTacticalSnapshot(unitIDs, center, radius);
 */
class IGameStateReader {
  public:
   virtual ~IGameStateReader() = default;

   // ========== Map Dimensions ==========

   /**
    * Get map dimensions
    */
   virtual int getMapWidth() const = 0;
   virtual int getMapHeight() const = 0;

   // ========== Current Player ==========

   /**
    * Get current player's turn
    */
   virtual PlayerID getCurrentPlayer() const = 0;

   // ========== Unit Queries ==========

   /**
    * Get all units belonging to a player
    *
    * @param player Player ID (0-7, 8=neutral)
    * @return Vector of Vehicle pointers (DO NOT delete - owned by GameMap)
    */
   virtual std::vector<const Vehicle*> getPlayerUnits(PlayerID player) const = 0;

   /**
    * Get unit by network ID
    *
    * @param networkID Unique unit identifier
    * @return Vehicle pointer or nullptr if not found
    */
   virtual const Vehicle* getUnitByID(UnitID networkID) const = 0;

   /**
    * Get unit at specific map coordinate
    *
    * @param pos Map coordinate
    * @return Vehicle pointer or nullptr if no unit at position
    */
   virtual const Vehicle* getUnitAt(const MapCoordinate& pos) const = 0;

   /**
    * Get all units within range of a center point
    *
    * @param center Center coordinate
    * @param range Radius in hexes
    * @return Vector of units within range
    */
   virtual std::vector<const Vehicle*> getUnitsInRange(const MapCoordinate& center,
                                                       int range) const = 0;

   // ========== Field/Terrain Queries ==========

   /**
    * Get terrain type at coordinate
    *
    * @param pos Map coordinate
    * @return Terrain type pointer (immutable, shared)
    */
   virtual const TerrainType* getTerrainAt(const MapCoordinate& pos) const = 0;

   /**
    * Check if field is visible to player
    *
    * @param pos Map coordinate
    * @param observer Player ID
    * @return true if player can see this field
    */
   virtual bool isFieldVisible(const MapCoordinate& pos, PlayerID observer) const = 0;

   /**
    * Get visibility bitmask at position
    *
    * @param pos Map coordinate
    * @return Bitmask (bit N = player N can see)
    */
   virtual uint16_t getVisibilityMask(const MapCoordinate& pos) const = 0;

   // ========== Resource Queries ==========

   /**
    * Get player's current resources
    *
    * @param player Player ID
    * @return Resource snapshot
    */
   virtual ResourceSnapshot getPlayerResources(PlayerID player) const = 0;

   // ========== Snapshot Creation (Main API) ==========

   /**
    * Create full game state snapshot
    *
    * @param perspective Player perspective (for fog-of-war filtering)
    * @return Unique pointer to snapshot (caller owns)
    */
   virtual std::unique_ptr<GameStateSnapshot> createFullSnapshot(PlayerID perspective) const = 0;

   /**
    * Create tactical snapshot (local area only)
    *
    * For MCTS tactical layer: only includes units and terrain in local area
    * Much smaller and faster than full snapshot
    *
    * @param unitIDs Units to include in snapshot
    * @param center Center of tactical area
    * @param radius Radius around center (in hexes)
    * @return Unique pointer to snapshot (caller owns)
    */
   virtual std::unique_ptr<GameStateSnapshot>
   createTacticalSnapshot(const std::vector<UnitID>& unitIDs, const MapCoordinate& center,
                          int radius) const = 0;

   // ========== Utility ==========

   /**
    * Check if coordinate is valid on map
    */
   virtual bool isValidCoordinate(const MapCoordinate& pos) const = 0;

   /**
    * Get underlying GameMap (for direct access if needed)
    * Note: Use with caution - prefer interface methods
    */
   virtual const GameMap* getGameMap() const = 0;
};

/**
 * Factory function to create reader from GameMap
 *
 * @param gameMap Pointer to ASC GameMap
 * @return Reader implementation
 */
std::unique_ptr<IGameStateReader> createGameStateReader(const GameMap* gameMap);

}  // namespace mcts
}  // namespace asc

#endif  // MCTS_I_GAME_STATE_READER_H
