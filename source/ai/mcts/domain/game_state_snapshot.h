/***************************************************************************
 * game_state_snapshot.h - Lightweight game state for MCTS simulations
 * 
 * Purpose: Compact, fast-to-copy game state representation
 *          Target: <20 KB for 20 units + local terrain
 * 
 * Part of: ASC MCTS AI (Phase 0.1 - Game State Cloning)
 ***************************************************************************/

#ifndef MCTS_GAME_STATE_SNAPSHOT_H
#define MCTS_GAME_STATE_SNAPSHOT_H

#include "types.h"
#include "unit_snapshot.h"
#include <vector>
#include <map>
#include <memory>
#include <cstddef>
#include <unordered_map>

// Forward declarations
class TerrainType;

namespace asc {
namespace mcts {

/**
 * Sparse terrain data for a single field
 * 
 * Only stores terrain that differs from default or is tactically relevant
 */
struct FieldSnapshot {
    const TerrainType* terrain;      // 8 bytes - Pointer to immutable terrain data
    uint16_t visibilityMask;         // 2 bytes - Which players can see this field
    UnitID unitID;                   // 4 bytes - Unit on field (-1 if empty)
    bool hasMine;                    // 1 byte  - Mine present (simplified for MVP)
    uint8_t _padding[3];             // 3 bytes - Alignment
    
    FieldSnapshot() 
        : terrain(nullptr), visibilityMask(0), unitID(-1), 
          hasMine(false), _padding{0, 0, 0}
    {}
};

/**
 * Lightweight game state snapshot for MCTS
 * 
 * Design Principles:
 * - Selective copying: Only tactically-relevant data
 * - Sparse storage: Only non-default terrain
 * - Shared pointers: Immutable type data (TerrainType, VehicleType)
 * - Fast cloning: std::vector copy is efficient for small datasets
 * 
 * Size Target: <20 KB for tactical snapshot (20 units, 10×10 terrain)
 * - Units: 20 × 32 bytes = 640 bytes
 * - Terrain: 100 × 16 bytes = 1600 bytes (if fully stored)
 * - Metadata: ~100 bytes
 * - Total: ~2.4 KB (well within target!)
 */
class GameStateSnapshot {
public:
    // ========== Metadata ==========
    
    int mapWidth;
    int mapHeight;
    PlayerID currentPlayer;
    PlayerID perspective;  // Which player's perspective (for fog-of-war)
    
    // ========== Unit Data ==========
    
    std::vector<UnitSnapshot> units;
    
    // ========== Terrain Data (Sparse) ==========
    
    // Only store terrain for tactically-relevant area
    // Key: (x, y) coordinate, Value: Field data
    std::map<MapCoordinate, FieldSnapshot> terrain;
    
    // Alternative: For dense tactical areas, could use flat array
    // std::vector<FieldSnapshot> terrainGrid;  // size = radius × radius
    // MapCoordinate terrainOrigin;             // top-left of grid
    
    // ========== Resources (if needed for tactical decisions) ==========
    
    ResourceSnapshot playerResources[8];  // Per-player resources
    
    // ========== Constructors ==========
    
    GameStateSnapshot() 
        : mapWidth(0),
          mapHeight(0),
          currentPlayer(0),
          perspective(0),
          cachedUnitCount(0),
          unitIndexesDirty(true),
          positionIndexDirty(true)
    {
        for (int i = 0; i < 8; ++i) {
            playerResources[i] = ResourceSnapshot();
        }
    }

    /**
     * Add unit to snapshot and keep lookup caches consistent
     */
    void addUnit(const UnitSnapshot& unit) {
        units.push_back(unit);
        invalidateUnitIndexes();
    }
    
    // ========== Cloning ==========
    
    /**
     * Create deep copy of snapshot
     * 
     * Note: Fast because:
     * - std::vector<UnitSnapshot> copy is efficient (POD-like data)
     * - std::map copy is acceptable for small tactical snapshots
     * - Type pointers are shared (no deep copy needed)
     * 
     * @return New snapshot (caller owns)
     */
    std::unique_ptr<GameStateSnapshot> clone() const {
        auto copy = std::make_unique<GameStateSnapshot>();
        
        copy->mapWidth = mapWidth;
        copy->mapHeight = mapHeight;
        copy->currentPlayer = currentPlayer;
        copy->perspective = perspective;
        
        copy->units = units;  // std::vector copy
        copy->terrain = terrain;  // std::map copy
        
        for (int i = 0; i < 8; ++i) {
            copy->playerResources[i] = playerResources[i];
        }

        copy->invalidateUnitIndexes();
        
        return copy;
    }
    
    // ========== Query Methods ==========
    
    /**
     * Find unit by network ID
     * 
     * @param networkID Unit identifier
     * @return Pointer to snapshot or nullptr
     */
    const UnitSnapshot* findUnit(UnitID networkID) const {
        ensureUnitIndexes();
        auto it = unitIndexById.find(networkID);
        if (it != unitIndexById.end()) {
            return &units[it->second];
        }
        return nullptr;
    }
    
    /**
     * Find mutable unit by network ID
     */
    UnitSnapshot* findUnitMutable(UnitID networkID) {
        ensureUnitIndexes();
        auto it = unitIndexById.find(networkID);
        if (it != unitIndexById.end()) {
            invalidateUnitIndexes();
            return &units[it->second];
        }
        return nullptr;
    }
    
    /**
     * Get unit at position
     * 
     * @param pos Map coordinate
     * @return Pointer to snapshot or nullptr
     */
    const UnitSnapshot* getUnitAt(const MapCoordinate& pos) const {
        ensureUnitIndexes();
        auto it = unitIndexByPosition.find(pos);
        if (it != unitIndexByPosition.end()) {
            return &units[it->second];
        }
        return nullptr;
    }
    
    /**
     * Get terrain at position
     * 
     * @param pos Map coordinate
     * @return Field snapshot pointer or nullptr if not in sparse map
     */
    const FieldSnapshot* getTerrainAt(const MapCoordinate& pos) const {
        auto it = terrain.find(pos);
        if (it != terrain.end()) {
            return &it->second;
        }
        return nullptr;
    }
    
    /**
     * Get all units belonging to a player
     * 
     * @param player Player ID
     * @return Vector of pointers to units (do not delete - owned by snapshot)
     */
    std::vector<const UnitSnapshot*> getPlayerUnits(PlayerID player) const {
        std::vector<const UnitSnapshot*> result;
        for (const auto& unit : units) {
            if (unit.owner == player) {
                result.push_back(&unit);
            }
        }
        return result;
    }
    
    /**
     * Check if coordinate is valid
     */
    bool isValidCoordinate(const MapCoordinate& pos) const {
        return pos.x >= 0 && pos.x < mapWidth && 
               pos.y >= 0 && pos.y < mapHeight;
    }
    
    // ========== Memory Profiling ==========
    
    /**
     * Calculate approximate memory usage
     * 
     * For profiling and optimization
     */
    size_t getMemorySize() const {
        size_t size = sizeof(GameStateSnapshot);
        size += units.capacity() * sizeof(UnitSnapshot);
        size += terrain.size() * (sizeof(MapCoordinate) + sizeof(FieldSnapshot));
        size += unitIndexById.size() * (sizeof(UnitID) + sizeof(size_t));
        size += unitIndexByPosition.size() * (sizeof(MapCoordinate) + sizeof(size_t));
        return size;
    }
    
    /**
     * Get statistics for debugging
     */
    struct Stats {
        size_t unitCount;
        size_t terrainFieldCount;
        size_t memoryBytes;
    };
    
    Stats getStats() const {
        return Stats{
            units.size(),
            terrain.size(),
            getMemorySize()
        };
    }

private:
    void invalidateUnitIndexes() const {
        unitIndexesDirty = true;
        positionIndexDirty = true;
    }

    void ensureUnitIndexes() const {
        if (!unitIndexesDirty && !positionIndexDirty && cachedUnitCount == units.size()) {
            return;
        }
        rebuildUnitIndexes();
    }

    void rebuildUnitIndexes() const {
        unitIndexById.clear();
        unitIndexByPosition.clear();
        for (size_t i = 0; i < units.size(); ++i) {
            unitIndexById[units[i].networkID] = i;
            unitIndexByPosition[units[i].getPosition()] = i;
        }
        cachedUnitCount = units.size();
        unitIndexesDirty = false;
        positionIndexDirty = false;
    }

    mutable std::unordered_map<UnitID, size_t> unitIndexById;
    mutable std::unordered_map<MapCoordinate, size_t, MapCoordinateHash> unitIndexByPosition;
    mutable size_t cachedUnitCount;
    mutable bool unitIndexesDirty;
    mutable bool positionIndexDirty;
};

} // namespace mcts
} // namespace asc

#endif // MCTS_GAME_STATE_SNAPSHOT_H
