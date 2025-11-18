/***************************************************************************
 * pathfinding_adapter.h - Bridge between MCTS snapshots and legacy A* pathfinding
 *
 * Purpose: Integrate ASC's proven AStar3D pathfinding into MCTS action generation
 *
 * Design: Adapter pattern to temporarily sync snapshot state to legacy GameMap
 *         for pathfinding calculations, then extract results as clean data
 *
 * Part of: ASC MCTS AI (Phase 1.3 - Full Pathfinding Integration)
 ***************************************************************************/

#ifndef MCTS_PATHFINDING_ADAPTER_H
#define MCTS_PATHFINDING_ADAPTER_H

#include <vector>
#include "../domain/types.h"
#include "../domain/i_game_state.h"
#include "../domain/unit_snapshot.h"

// Feature toggle: enable legacy A* pathfinding only when linked with libasc
#ifndef MCTS_ENABLE_LEGACY_PATHFINDING
#define MCTS_ENABLE_LEGACY_PATHFINDING 0
#endif

// Forward declarations
class GameMap;
class Vehicle;
class AStar3D;  // Global ASC class

namespace asc {
namespace mcts {

/**
 * Reachable position found by pathfinding
 */
struct ReachablePosition {
   MapCoordinate position;  // Target hex coordinates
   int movementCost;        // Movement points to reach (scaled)
   int8_t height;           // Height level when reaching position
   bool hasAttacked;        // Whether move triggers reaction fire
   bool canStop;            // Whether unit can stop at this position

   ReachablePosition(MapCoordinate pos, int cost, int8_t h, bool attacked, bool stop)
      : position(pos), movementCost(cost), height(h), hasAttacked(attacked), canStop(stop) {}
};

/**
 * Adapter for ASC's AStar3D pathfinding algorithm
 *
 * DESIGN RATIONALE:
 * - Reuses battle-tested AStar3D (600+ lines, handles all edge cases)
 * - Isolates legacy code interaction to this single adapter
 * - Minimal state sync (only units in tactical radius)
 * - Clean interface for MCTS (pure snapshot-based API)
 *
 * USAGE:
 *   PathfindingAdapter adapter;
 *   auto reachable = adapter.findReachablePositions(state, unit, legacyMap);
 *   // Use reachable positions for action generation
 *
 * PERFORMANCE:
 * - AStar3D typical: 5-15ms per unit with 100 movement points
 * - State sync overhead: ~1-2ms (only nearby units)
 * - Total: ~7-17ms per pathfinding call (acceptable for turn-based game)
 */
class PathfindingAdapter {
  public:
   PathfindingAdapter() = default;
   ~PathfindingAdapter() = default;

   /**
    * Find all positions reachable by a unit within its movement budget
    *
    * Uses ASC's AStar3D pathfinding to explore all hexes the unit can reach
    * this turn, accounting for:
    * - Terrain movement costs
    * - Height changes (aircraft ascending/descending)
    * - Container entry/exit (buildings, transports)
    * - Wind effects (for aircraft)
    * - Reaction fire zones
    *
    * @param state Current game state snapshot (for unit context)
    * @param unit Unit to pathfind for
    * @param legacyMap Legacy GameMap pointer (for terrain/pathfinding)
    * @return Vector of all reachable positions with costs
    *
    * LEGACY INTEGRATION:
    * - Temporarily creates Vehicle object for A* input
    * - Syncs nearby unit positions to map (for blocking/RF)
    * - Cleans up temp state after pathfinding
    * - No permanent changes to legacyMap
    */
   [[nodiscard]] std::vector<ReachablePosition> findReachablePositions(const IGameState& state,
                                                                       const UnitSnapshot& unit,
                                                                       GameMap* legacyMap) const;

  private:
   /**
    * Create temporary Vehicle for pathfinding
    *
    * AStar3D requires a Vehicle*, so we create a minimal one with:
    * - networkid, xpos, ypos, movement, fuel, attacked
    * - typ (VehicleType* from snapshot)
    * - height, direction
    *
    * IMPORTANT: Caller must delete returned Vehicle!
    *
    * @param unit Unit snapshot to convert
    * @param map GameMap to attach vehicle to
    * @return Heap-allocated Vehicle (caller owns)
    */
   Vehicle* createTempVehicle(const UnitSnapshot& unit, GameMap* map) const;

   /**
    * Clean up temporary vehicle created for pathfinding
    *
    * LEGACY CLEANUP:
    * - Remove vehicle from map's unit lists (if added)
    * - Delete Vehicle object
    * - Restore map state
    *
    * @param vehicle Vehicle to clean up
    * @param map GameMap to clean from
    */
   void cleanupTempVehicle(Vehicle* vehicle, GameMap* map) const;

   /**
    * Sync unit positions from snapshot to legacy map
    *
    * Only syncs units within tactical radius (~10 hexes) of pathfinding unit
    * for performance. Distant units don't affect pathfinding.
    *
    * Synced data:
    * - Unit positions (for blocking checks)
    * - Owner (for reaction fire checks)
    *
    * NOT synced (not needed for pathfinding):
    * - Unit damage, ammo, cargo, etc.
    *
    * @param state Game state snapshot
    * @param center Center position (pathfinding unit)
    * @param radius Radius to sync (hexes)
    * @param map Map to sync to
    */
   void syncNearbyUnits(const IGameState& state, MapCoordinate center, int radius,
                        GameMap* map) const;

   /**
    * Convert AStar3D visited nodes to ReachablePosition list
    *
    * @param pathfinder AStar3D instance after findAllAccessibleFields()
    * @return Vector of reachable positions
    */
   [[nodiscard]] std::vector<ReachablePosition>
   extractReachablePositions(AStar3D& pathfinder) const;
};

}  // namespace mcts
}  // namespace asc

#endif  // MCTS_PATHFINDING_ADAPTER_H
