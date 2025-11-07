/***************************************************************************
 * game_state_snapshot.h - Lightweight game state for MCTS simulations
 * 
 * Purpose: Compact, fast-to-copy game state representation
 *          Target: <20 KB for 20 units + local terrain
 * 
 * Part of: ASC MCTS AI (Phase 0.1 - Game State Cloning)
 * Updated: C++23 with std::flat_map for 2× faster cloning
 ***************************************************************************/

#ifndef MCTS_GAME_STATE_SNAPSHOT_H
#define MCTS_GAME_STATE_SNAPSHOT_H

#include "types.h"
#include "unit_snapshot.h"
#include "cpp23_compat.h"  // C++23 compatibility (fast_map)
#include <vector>
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
    
    constexpr FieldSnapshot() noexcept  // C++23: constexpr
        : terrain(nullptr), visibilityMask(0), unitID(-1), 
          hasMine(false), _padding{0, 0, 0}
    {}
};

/**
 * Lightweight game state snapshot for MCTS
 * 
 * Design Principles:
 * - Selective copying: Only tactically-relevant data
 * - Sparse storage: Only non-default terrain (using fast_map for cache-locality)
 * - Shared pointers: Immutable type data (TerrainType, VehicleType)
 * - Fast cloning: std::vector copy is efficient for small datasets
 * 
 * Size Target: <20 KB for tactical snapshot (20 units, 10×10 terrain)
 * - Units: 20 × 32 bytes = 640 bytes
 * - Terrain: 100 × 32 bytes = 3200 bytes (fast_map has less overhead than map)
 * - Metadata: ~100 bytes
 * - Total: ~4 KB (well within target!)
 * 
 * C++23 Optimizations:
 * - fast_map (std::flat_map when available, falls back to std::map on GCC 13)
 * - constexpr where possible (compile-time optimization)
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
    
    // C++23: Using fast_map (std::flat_map when available, std::map fallback)
    // Benefits: 2× faster cloning with flat_map, better memory locality, less overhead
    // Key: (x, y) coordinate, Value: Field data
    fast_map<MapCoordinate, FieldSnapshot> terrain;
    
    // ========== Resources (if needed for tactical decisions) ==========
    
    ResourceSnapshot playerResources[8];  // Per-player resources
    
    // ========== Constructors ==========
    
    constexpr GameStateSnapshot() noexcept  // C++23: constexpr constructor
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
     * Note: Ultra-fast because:
     * - std::vector<UnitSnapshot> copy is efficient (POD-like data)
     * - fast_map copy (std::flat_map when available, std::map fallback)
     * - Type pointers are shared (no deep copy needed)
     * 
     * C++23 Optimization: With std::flat_map (GCC 14+) cloning is ~2× faster
     * 
     * @return New snapshot (caller owns)
     */
    std::unique_ptr<GameStateSnapshot> clone() const {
        auto copy = std::make_unique<GameStateSnapshot>();
        
        copy->mapWidth = mapWidth;
        copy->mapHeight = mapHeight;
        copy->currentPlayer = currentPlayer;
        copy->perspective = perspective;
        
        copy->units = units;      // std::vector copy (fast)
        copy->terrain = terrain;  // fast_map copy (std::flat_map if available)
        
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
     * C++23: fast_map lookup (optimized with std::flat_map if available)
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
     * Check if coordinate is valid (C++23: constexpr)
     */
    constexpr bool isValidCoordinate(const MapCoordinate& pos) const noexcept {
        return pos.x >= 0 && pos.x < mapWidth && 
               pos.y >= 0 && pos.y < mapHeight;
    }
    
    // ========== Memory Profiling ==========
    
    /**
     * Calculate approximate memory usage
     * 
     * For profiling and optimization
     * C++23: fast_map (std::flat_map has lower overhead when available)
     */
    size_t getMemorySize() const {
        size_t size = sizeof(GameStateSnapshot);
        size += units.capacity() * sizeof(UnitSnapshot);
        // Terrain storage (more efficient with std::flat_map on GCC 14+)
        size += terrain.size() * sizeof(std::pair<MapCoordinate, FieldSnapshot>);
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
    
    constexpr Stats getStats() const {  // C++23: constexpr
        return Stats{
            units.size(),
            terrain.size(),
            getMemorySize()
        };
    }

private:
    constexpr void invalidateUnitIndexes() const noexcept {  // C++23: constexpr
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
