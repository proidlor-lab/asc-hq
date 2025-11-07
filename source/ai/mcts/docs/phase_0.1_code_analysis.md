# Phase 0.1 Legacy Code Analysis: Game State Cloning

**Date**: 2025-11-06  
**Phase**: 0.1 - Game State Cloning & Snapshots  
**Goal**: Identify all legacy ASC code that must be adapted/extended for snapshot functionality

---

## Executive Summary

**Critical Challenge**: `GameMap` has **NO copy constructor** and is extremely large (10-50 MB). Phase 0.1 must create lightweight snapshots (~10-20 KB for 20 units) that can be copied in <5ms.

**Strategy**: Build wrapper/adapter layer around existing ASC structures without modifying legacy code where possible.

---

## 1. CRITICAL Legacy Files (Must Interface With)

### 1.1 Core Game State

| File | Class | Critical For | Access Pattern | Issues |
|------|-------|--------------|----------------|--------|
| `gamemap.h/cpp` | `GameMap` | **THE** central data structure | Direct pointer access | ❌ No copy constructor<br>⚠️ Huge (MB-sized)<br>⚠️ Complex pointer hierarchy |
| `player.h/cpp` | `Player` | Unit/building lists, resources | `getPlayer(id)` | ✅ Direct lists available<br>⚠️ Contains large data |
| `vehicle.h/cpp` | `Vehicle` | Unit state (pos, HP, movement) | Via `Player::vehicleList` | ❌ No copy constructor<br>✅ Well-structured data |
| `buildings.h/cpp` | `Building` | Building state | Via `Player::buildingList` | Similar to Vehicle |
| `mapfield.h/cpp` | `MapField` | Terrain, visibility, objects | `getField(x,y)` | ✅ Simple struct-like<br>⚠️ Large array |

**Key Methods Needed**:
```cpp
// GameMap - READ ONLY access for snapshots
GameMap::getField(int x, int y) -> MapField*
GameMap::getPlayer(PlayerID) -> Player&
GameMap::getUnit(int networkid) -> Vehicle*
GameMap::getgameparameter() -> int

// Player
Player::vehicleList -> list<Vehicle*>
Player::buildingList -> list<Building*>
Player::research -> Research&

// Vehicle
Vehicle::xpos, ypos -> position
Vehicle::damage -> HP (inverted: HP = 100 - damage)
Vehicle::_movement -> remaining movement
Vehicle::attacked -> bool
Vehicle::ammo[16] -> ammunition array
Vehicle::typ -> VehicleType* (immutable)
Vehicle::networkid -> unique ID
```

---

### 1.2 Type System (Immutable References - Can Share)

| File | Class | Purpose | Snapshot Need |
|------|-------|---------|---------------|
| `vehicletype.h/cpp` | `VehicleType` | Unit stats (immutable) | **Store pointer only** (don't copy) |
| `buildingtype.h/cpp` | `BuildingType` | Building stats | Pointer only |
| `terraintype.h/cpp` | `TerrainType` | Terrain stats | Pointer only |

✅ **Good News**: These are immutable rulesets - snapshots only need to store **pointers**, not copies!

---

### 1.3 Visibility & Game State

| File | Function/Class | Purpose | Issues |
|------|----------------|---------|--------|
| `viewcalculation.h/cpp` | `fieldvisiblenow()` | Check field visibility | ⚠️ Global function, not OOP<br>⚠️ Complex calculation |
| `mapfield.h` | `MapField::visible` | Bitmapped visibility | ✅ Simple bitfield |
| `reactionfire.h/cpp` | `tsearchreactionfireingunits` | Find RF-capable units | ⚠️ Complex search<br>✅ Can reuse |

---

## 2. Files Requiring Wrapper/Adapter Interfaces

### 2.1 NEW: GameState Snapshot Classes (To Be Created)

```
source/ai/mcts/domain/
├── game_state_snapshot.h/cpp       ✨ NEW - Core snapshot structure
├── unit_snapshot.h/cpp             ✨ NEW - Lightweight unit data
├── terrain_snapshot.h/cpp          ✨ NEW - Sector-level terrain
├── snapshot_serializer.h/cpp       ✨ NEW - Fast serialize/deserialize
└── snapshot_restorer.h/cpp         ✨ NEW - Restore to temp GameMap
```

**Dependencies**: Must read from `GameMap`, `Vehicle`, `Building`, `MapField`

---

### 2.2 NEW: State Access Abstraction

```
source/ai/mcts/infrastructure/
├── game_state_reader.h/cpp         ✨ NEW - Safe read-only GameMap access
├── unit_query.h/cpp                ✨ NEW - Query units (range, visibility)
├── field_query.h/cpp               ✨ NEW - Query terrain, objects
└── resource_query.h/cpp            ✨ NEW - Query player resources
```

**Purpose**: Hide legacy API complexity, provide clean interface to MCTS code

---

## 3. Files That MAY Need Extension (Low Priority)

### 3.1 Serialization System

| File | Usage | Modification Needed? |
|------|-------|---------------------|
| `basestrm.h/cpp` | Stream interface | ⚠️ **Maybe** - For fast binary snapshot serialization |
| `sgstream.h/cpp` | Savegame serialization | ℹ️ Reference only - too slow for MCTS |

**Decision**: Likely implement **custom lightweight binary format** instead of reusing `tnstream` (which is designed for disk I/O, not speed).

---

### 3.2 Action System (Phase 0.2, not 0.1!)

| File | Purpose | Phase 0.1 Relevance |
|------|---------|-------------------|
| `actions/action.h` | Base action class | ⏭️ **Phase 0.2** only |
| `actions/moveunitcommand.h` | Move execution | ⏭️ Phase 0.2 |
| `actions/attackcommand.h` | Attack execution | ⏭️ Phase 0.2 |

**Note**: Phase 0.1 is **read-only** snapshots. Action execution comes in Phase 0.2.

---

## 4. Detailed Legacy Code Interfaces

### 4.1 GameMap Structure (from `gamemap.h`)

**Critical Members for Snapshots**:
```cpp
class GameMap {
public:
    // Dimensions
    int xsize, ysize;                    // ✅ Copy to snapshot
    
    // Field array
    MapField* field;                     // ⚠️ LARGE! Selective copy only
    
    // Players
    Player player[9];                    // ⚠️ Copy relevant player data only
    int actplayer;                       // ✅ Current player ID
    
    // Time
    GameTime time;                       // ✅ Copy (small struct)
    
    // Resources (BI mode)
    Resources bi_resource[8];            // ✅ Copy if _resourcemode == 1
    int _resourcemode;                   // ✅ Copy flag
    
    // Parameters
    int* game_parameter;                 // ℹ️ Reference only (immutable)
    
    // ID Manager
    IDManager idManager;                 // ℹ️ For unit lookup
    
    // Methods we'll use
    MapField* getField(int x, int y);
    Player& getPlayer(PlayerID p);
    Vehicle* getUnit(int networkid);
    int getgameparameter(GameParameter num);
};
```

**Snapshot Strategy**:
- ✅ Copy: xsize, ysize, actplayer, time, resourcemode
- ⚠️ Selective: Only relevant MapField data (terrain, visibility)
- ⚠️ Selective: Only active units/buildings
- ❌ Skip: Graphics, events, messages, replay data

---

### 4.2 Vehicle Structure (from `vehicle.h`)

**Essential Data (~50-80 bytes per unit)**:
```cpp
class Vehicle : public ContainerBase {
public:
    // Identity
    int networkid;                       // ✅ 4 bytes - Unique ID
    const VehicleType* typ;              // ✅ 8 bytes - Pointer (share!)
    
    // Position & State
    int xpos, ypos;                      // ✅ 8 bytes
    int height;                          // ✅ 4 bytes
    int damage;                          // ✅ 4 bytes (HP = 100 - damage)
    
    // Movement
    int _movement;                       // ✅ 4 bytes
    Resources tank;                      // ✅ 12 bytes (fuel)
    
    // Combat
    bool attacked;                       // ✅ 1 byte
    int ammo[16];                        // ✅ 64 bytes (can compress)
    int weapstrength[16];                // ⚠️ 64 bytes (maybe skip?)
    
    // Experience (optional for tactical MCTS)
    int experience_offensive;            // ⚠️ 4 bytes (skip for MVP?)
    int experience_defensive;            // ⚠️ 4 bytes (skip for MVP?)
    
    // Cargo (complex - defer to Phase 2)
    Cargo cargo;                         // ⏭️ Skip for MVP
    
    // Owner
    // Inherited from ContainerBase
    int owner;                           // ✅ 4 bytes
};
```

**Snapshot Size**: ~50-80 bytes per unit (without cargo/experience)
- 20 units = 1-1.6 KB ✅ **Target achieved!**

---

### 4.3 MapField Structure (from `mapfield.h`)

**For Tactical MCTS** (local area only, ~10x10 = 100 fields):
```cpp
class MapField {
public:
    // Terrain
    TerrainType::Weather* typ;           // ✅ 8 bytes - Pointer
    
    // Minerals (if relevant)
    Uint8 fuel, material;                // ⚠️ 2 bytes - Maybe skip
    
    // Visibility
    Uint16 visible;                      // ✅ 2 bytes - Bitmapped
    
    // Occupancy
    Vehicle* vehicle;                    // ✅ 8 bytes - Pointer or NULL
    Vehicle* secondvehicle;              // ✅ 8 bytes - Temp during move
    Building* building;                  // ✅ 8 bytes - Pointer or NULL
    
    // Mines
    list<Mine> mines;                    // ⚠️ Variable - Important for tactics!
    
    // Objects (terrain decorations)
    vector<Object> objects;              // ⏭️ Skip for MVP (not tactical)
};
```

**Snapshot Strategy**:
- ✅ Full copy for **local tactical area** (~10x10 around unit group)
- ℹ️ Sparse representation for **strategic layer** (only key sectors)

**Size**: 100 fields × 40 bytes = 4 KB ✅ Acceptable

---

## 5. Integration Points & Wrapper Design

### 5.1 Snapshot Creation Flow

```
MCTS Request Snapshot
        ↓
GameStateReader (NEW)
    ↓ reads from ↓
  GameMap (LEGACY)
        ↓
 UnitSnapshot (NEW)      TerrainSnapshot (NEW)
        ↓                       ↓
    GameStateSnapshot (NEW)
        ↓
 Binary Serialization (NEW - optional for caching)
```

### 5.2 Snapshot Restoration Flow

```
MCTS needs simulation
        ↓
GameStateSnapshot
        ↓
SnapshotRestorer (NEW)
        ↓
Temporary GameMap clone (LIMITED)
        ↓
MCTS simulation runs
        ↓
Discard temporary GameMap
```

**Key Decision**: Do we need **full GameMap restoration** or can we work with **snapshot data directly**?

**Recommendation**: For Phase 1 (Tactical MCTS), work **directly with snapshot data** - avoid GameMap reconstruction complexity!

---

## 6. Files NOT Needed for Phase 0.1

### Skip These (For Now):

| File/Directory | Reason |
|----------------|--------|
| `actions/*` | Phase 0.2 - Action execution |
| `ai/*` | Legacy AI - reference only, don't integrate yet |
| `dialog/*`, `widgets/*` | GUI - irrelevant |
| `network/*` | Multiplayer - irrelevant |
| `replay.h/cpp` | Replay system - different use case |
| `events.h`, `gameevents.*` | Map events - MVP ignores these |
| `research.h/cpp` | Research - Phase 4+ |
| `resourcenet.h/cpp` | Resource networks - Strategic layer (Phase 3) |
| `weatherarea.h/cpp` | Weather system - defer until needed |

---

## 7. NEW Code Structure for Phase 0.1

### Directory: `/source/ai/mcts/domain/`

#### 7.1 Core Snapshot Classes

**`game_state_snapshot.h/cpp`**:
```cpp
class GameStateSnapshot {
    // Metadata
    int mapWidth, mapHeight;
    int currentPlayer;
    GameTime time;
    
    // Unit data (compact)
    std::vector<UnitSnapshot> units;
    
    // Terrain data (sparse - only relevant sectors)
    std::map<SectorID, TerrainSnapshot> terrainSectors;
    
    // Visibility cache (per player)
    VisibilitySnapshot visibility;
    
    // Resources
    Resources playerResources[8];
    
    // Fast copy/clone
    GameStateSnapshot* clone() const;
    
    // Size tracking
    size_t getMemorySize() const;
};
```

**`unit_snapshot.h/cpp`**:
```cpp
struct UnitSnapshot {
    int networkID;
    const VehicleType* type;  // Pointer to immutable data
    
    // Position
    int16_t x, y;             // 2+2 = 4 bytes
    int8_t height;            // 1 byte
    
    // State
    uint8_t damage;           // 0-100, 1 byte
    uint8_t movement;         // Scaled, 1 byte
    uint8_t owner;            // 0-8, 1 byte
    bool attacked;            // 1 byte
    
    // Combat (compressed)
    uint16_t ammoMask;        // Bitmask for which weapons have ammo
    uint8_t ammoValues[8];    // Only non-zero ammo (max 8 weapons typically)
    
    // Fuel (if needed)
    uint16_t fuel;            // 2 bytes
    
    // Total: ~20-30 bytes per unit
};
```

**`terrain_snapshot.h/cpp`**:
```cpp
class TerrainSnapshot {
    // For tactical layer: Full data for small area
    struct FieldData {
        const TerrainType* terrain;  // Pointer
        uint16_t visibility;          // Bitmask
        int networkIDonField;         // Unit ID or -1
        bool hasMine;                 // Simplified
    };
    
    // Sparse storage (only non-empty fields)
    std::map<MapCoordinate, FieldData> fields;
    
    // Sector-level aggregates for strategic layer
    struct SectorInfo {
        float avgTerrainType;   // Dominant terrain
        uint8_t controlPlayer;  // Who controls
        uint16_t unitCount;     // Units in sector
    };
};
```

---

### 7.2 Reader/Query Classes

**`game_state_reader.h/cpp`** - **Adapts GameMap to clean interface**:
```cpp
class GameStateReader {
    const GameMap* map;
    
public:
    GameStateReader(const GameMap* m) : map(m) {}
    
    // Unit queries
    std::vector<Vehicle*> getAllUnits(PlayerID player) const;
    std::vector<Vehicle*> getUnitsInRange(MapCoordinate center, int range) const;
    Vehicle* getUnitAt(MapCoordinate pos) const;
    
    // Field queries
    const TerrainType* getTerrainAt(MapCoordinate pos) const;
    bool isFieldVisible(MapCoordinate pos, PlayerID observer) const;
    std::vector<Mine*> getMinesAt(MapCoordinate pos) const;
    
    // Resource queries
    Resources getPlayerResources(PlayerID player) const;
    
    // Snapshot creation
    GameStateSnapshot* createSnapshot(PlayerID perspective) const;
    GameStateSnapshot* createTacticalSnapshot(
        const std::vector<int>& unitIDs,
        MapCoordinate center,
        int radius
    ) const;
};
```

---

## 8. Implementation Risks & Mitigations

### 8.1 Critical Risks

| Risk | Probability | Impact | Mitigation |
|------|-------------|--------|------------|
| **Snapshot too large** | MEDIUM | CRITICAL | Start with minimal data, profile, optimize |
| **Copy too slow** | HIGH | CRITICAL | Use binary memcpy where possible, avoid STL copies |
| **Pointer invalidation** | MEDIUM | HIGH | Store IDs, not pointers; validate on access |
| **Memory leaks** | LOW | MEDIUM | RAII, smart pointers in new code |
| **Legacy code modification required** | LOW | HIGH | Avoid! Use wrappers only |

### 8.2 Performance Targets

| Metric | Target | Measurement Strategy |
|--------|--------|---------------------|
| Snapshot creation | <5ms for 20 units | Profile with `std::chrono` |
| Snapshot size | <20 KB for 20 units + terrain | `sizeof()` + runtime tracking |
| Snapshot copy | <1ms | std::chrono |
| Memory overhead | <500 MB for 1000 snapshots | Track allocations |

---

## 9. Phase 0.1 Implementation Checklist

### Week 1: Foundation

- [ ] Create directory structure `/source/ai/mcts/domain/`
- [ ] Implement `UnitSnapshot` struct (minimal)
- [ ] Implement `GameStateSnapshot` class (minimal)
- [ ] Write unit tests for snapshot creation
- [ ] **Measure**: Snapshot size for 1, 10, 20 units

### Week 2: Reader & Integration

- [ ] Implement `GameStateReader` class
- [ ] Method: `getAllUnits()` 
- [ ] Method: `createSnapshot()`
- [ ] Integration test: Read from real GameMap, create snapshot
- [ ] **Measure**: Creation time

### Week 3: Optimization & Validation

- [ ] Profile snapshot creation
- [ ] Optimize slow paths (likely: field iteration, ammo copying)
- [ ] Implement fast binary serialization (optional)
- [ ] Validation: Snapshot data == original GameMap data
- [ ] **Measure**: All performance targets

---

## 10. Summary: Legacy Code Interface Points

### READ Access Required (No Modification)

1. ✅ **GameMap**: via `getField()`, `getPlayer()`, `getUnit()`
2. ✅ **Player**: via `vehicleList`, `buildingList`, `research`
3. ✅ **Vehicle**: all public members (read-only)
4. ✅ **MapField**: terrain, visibility, occupancy
5. ✅ **VehicleType/BuildingType**: immutable references

### NEW Wrapper Classes

1. ✨ `GameStateReader` - clean interface to GameMap
2. ✨ `GameStateSnapshot` - lightweight state container
3. ✨ `UnitSnapshot` - compact unit data
4. ✨ `TerrainSnapshot` - sparse terrain data

### Files That Stay UNTOUCHED

- ✅ All of `actions/*` (Phase 0.2)
- ✅ All of `ai/*` (legacy AI, reference only)
- ✅ All GUI/dialog code
- ✅ All network code
- ✅ Action system (used later in Phase 0.2)

---

## Conclusion

**Phase 0.1 is feasible WITHOUT modifying legacy ASC code.** The strategy is:

1. **Read-only wrappers** around `GameMap`, `Player`, `Vehicle`
2. **New snapshot classes** in `/source/ai/mcts/domain/`
3. **Selective copying** - only tactical-relevant data
4. **Binary efficiency** - compact structs, minimal overhead

**Estimated LOC**: ~1500-2000 lines (snapshot classes + readers + tests)

**Key Success Factor**: Keep snapshots **minimal and fast**. Start with 20-30 bytes per unit, expand only if needed.
