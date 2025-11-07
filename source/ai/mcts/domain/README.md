# Domain Layer - Game State Snapshots

Contains adapters that translate ASC game states, actions, and scoring heuristics into the generic interfaces expected by the core engine.

## Phase 0.1: Game State Cloning ✅ IMPLEMENTED

### Overview

Provides lightweight, fast-to-copy game state snapshots for MCTS simulations.

**Performance Targets**:
- Snapshot size: <20 KB for 20 units + local terrain
- Clone time: <1 ms
- Creation time: <5 ms

### Architecture - Dependency Injection

Uses **dependency injection** pattern to decouple MCTS from legacy GameMap:

```cpp
// Interface (abstract)
IGameStateReader* reader = createGameStateReader(gameMap);

// Usage
auto snapshot = reader->createTacticalSnapshot(unitIDs, center, radius);
auto clone = snapshot->clone();
```

### Files

#### Core Interfaces (DI)
- **`i_game_state_reader.h`** - Abstract interface for reading game state
- **`game_state_reader.h/cpp`** - Concrete adapter for GameMap

#### Data Structures
- **`types.h`** - Common types (MapCoordinate, ResourceSnapshot, etc.)
- **`unit_snapshot.h/cpp`** - Compact unit state (~32 bytes)
- **`game_state_snapshot.h`** - Main snapshot container

#### Testing
- **`snapshot_test.cpp`** - Unit tests and performance benchmarks

### Usage Example

```cpp
#include "i_game_state_reader.h"

using namespace asc::mcts;

// Create reader (DI)
auto reader = createGameStateReader(gameMap);

// Create tactical snapshot
std::vector<UnitID> myUnits = {1, 2, 3, 4, 5};
MapCoordinate center(50, 50);
auto snapshot = reader->createTacticalSnapshot(myUnits, center, 10);

// Clone for simulation
auto simState = snapshot->clone();

// Query units
const UnitSnapshot* unit = simState->findUnit(1);
```

### Design Principles

1. **Selective Copying**: Only tactical-relevant data
2. **Sparse Storage**: Only non-default terrain stored
3. **Shared Pointers**: Immutable type data (VehicleType, TerrainType)
4. **Dependency Injection**: Clean interfaces, testable code
5. **Fast Cloning**: std::vector copy for POD-like structs
