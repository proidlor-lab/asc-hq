# MCTS AI - Quick Start Guide

## Current Status

**Phase 0.1: Game State Cloning** - Core implementation complete (70%)

### What's Done ✅
- Snapshot data structures (types, UnitSnapshot, GameStateSnapshot)
- Dependency injection interface (IGameStateReader)
- GameMap adapter (GameStateReader)
- Unit tests (snapshot_test.cpp)
- Build system (Makefile.am)

### What's Pending ⏳
- Compilation and integration testing
- Performance profiling with real GameMap
- Bug fixes and optimization

---

## How to Build

### Prerequisites
- ASC build environment configured
- C++14 compiler
- Automake/Autoconf

### Build Steps

```bash
# 1. Navigate to ASC root
cd /path/to/asc-hq

# 2. Bootstrap (if configure.ac changed)
./bootstrap

# 3. Configure
./configure

# 4. Build MCTS library
make -C source/ai/mcts

# 5. Run tests (after uncommenting in Makefile.am)
# make -C source/ai/mcts check
```

### If Compilation Fails

**Common Issues**:

1. **Missing includes** - Adjust paths in domain/*.cpp
2. **C++14 features** - Ensure compiler supports `-std=c++14`
3. **Legacy code dependencies** - May need to include additional ASC headers

**Debug**:
```bash
# Compile individual file
g++ -std=c++14 -I../../.. -c domain/unit_snapshot.cpp

# Check for undefined symbols
nm -u domain/.libs/unit_snapshot.o
```

---

## How to Use (API)

### Basic Usage

```cpp
#include "ai/mcts/domain/i_game_state_reader.h"
#include "ai/mcts/domain/game_state_snapshot.h"

using namespace asc::mcts;

// In your AI code (has access to GameMap*)
void myAIFunction(GameMap* gameMap) {
    // 1. Create reader (dependency injection)
    auto reader = createGameStateReader(gameMap);
    
    // 2. Create snapshot for tactical area
    std::vector<UnitID> myUnits = {1, 2, 3};  // Unit IDs to include
    MapCoordinate center(50, 50);              // Center of tactical area
    int radius = 10;                           // Radius in hexes
    
    auto snapshot = reader->createTacticalSnapshot(myUnits, center, radius);
    
    // 3. Clone for simulation
    auto simState = snapshot->clone();
    
    // 4. Query units
    const UnitSnapshot* unit = simState->findUnit(1);
    if (unit) {
        std::cout << "Unit at (" << unit->x << ", " << unit->y << ")\n";
        std::cout << "HP: " << unit->getHPPercent() << "%\n";
    }
    
    // 5. Get all player units
    auto playerUnits = simState->getPlayerUnits(0);
    for (const UnitSnapshot* u : playerUnits) {
        // Process each unit
    }
    
    // 6. Check memory usage
    auto stats = simState->getStats();
    std::cout << "Snapshot uses " << stats.memoryBytes << " bytes\n";
}
```

### Advanced: Custom Mock for Testing

```cpp
// Create mock reader for unit tests
class MockGameStateReader : public IGameStateReader {
    // Implement interface methods with test data
    std::vector<const Vehicle*> getPlayerUnits(PlayerID player) const override {
        // Return mock units
    }
    // ... etc
};

// Use in tests
auto mockReader = std::make_unique<MockGameStateReader>();
auto snapshot = mockReader->createTacticalSnapshot(...);
```

---

## How to Test

### Unit Tests

```bash
# 1. Uncomment test binary in Makefile.am
# noinst_PROGRAMS = snapshot_test
# snapshot_test_SOURCES = domain/snapshot_test.cpp
# snapshot_test_LDADD = libmcts.la

# 2. Rebuild
make -C source/ai/mcts

# 3. Run tests
./source/ai/mcts/snapshot_test
```

### Integration Test

```cpp
// In ASC game code
#include "ai/mcts/domain/i_game_state_reader.h"

void testSnapshotIntegration(GameMap* activeMap) {
    auto reader = createGameStateReader(activeMap);
    
    // Test snapshot creation
    auto t_start = std::chrono::high_resolution_clock::now();
    auto snapshot = reader->createFullSnapshot(activeMap->actplayer);
    auto t_end = std::chrono::high_resolution_clock::now();
    
    double elapsed = std::chrono::duration<double, std::milli>(t_end - t_start).count();
    
    std::cout << "Snapshot created in " << elapsed << " ms\n";
    std::cout << "Units: " << snapshot->units.size() << "\n";
    std::cout << "Memory: " << snapshot->getMemorySize() << " bytes\n";
    
    // Test cloning
    t_start = std::chrono::high_resolution_clock::now();
    auto clone = snapshot->clone();
    t_end = std::chrono::high_resolution_clock::now();
    elapsed = std::chrono::duration<double, std::milli>(t_end - t_start).count();
    
    std::cout << "Clone created in " << elapsed << " ms\n";
    
    // Validate
    assert(clone->units.size() == snapshot->units.size());
}
```

---

## Next Steps

### For Completing Phase 0.1

1. **Fix Compilation Issues**
   - Adjust include paths if needed
   - Add missing legacy headers
   - Test on target platform

2. **Integration Testing**
   - Add test hook in ASC AI turn function
   - Test with small map (64×64, 10 units)
   - Test with large map (128×128, 50 units)

3. **Performance Profiling**
   - Measure snapshot creation time
   - Measure clone time
   - Identify bottlenecks (use gprof or perf)

4. **Optimization (if needed)**
   - Profile-guided optimization
   - Consider caching terrain lookups
   - Optimize hex distance calculation

### For Starting Phase 0.2

1. **Read Phase 0.2 Requirements**
   - Review `docs/implementation_roadmap.md` Phase 0.2 section
   - Understand action execution interface

2. **Design Action Interface**
   - `IActionExecutor` interface
   - `SimulationActionExecutor` for snapshots
   - `RealGameActionExecutor` for actual game

3. **Implement Basic Actions**
   - MoveAction
   - AttackAction
   - WaitAction

4. **Simulate on Snapshots**
   - Modify snapshot state (not real game)
   - Handle reaction fire
   - Track fuel/ammo consumption

---

## Troubleshooting

### Compiler Errors

**Error**: `fatal error: gamemap.h: No such file or directory`
- **Fix**: Adjust include path in domain/*.cpp: `#include "../../gamemap.h"`

**Error**: `'make_unique' is not a member of 'std'`
- **Fix**: Ensure C++14: Add `-std=c++14` to AM_CXXFLAGS

**Error**: Undefined reference to `Vehicle::getMovement()`
- **Fix**: Link against ASC main library: Add to LDADD in Makefile.am

### Runtime Errors

**Error**: Segmentation fault in `fromVehicle()`
- **Fix**: Check for null vehicle pointer
- **Debug**: `gdb snapshot_test`, `bt` for backtrace

**Error**: Assertion failed in tests
- **Fix**: Validate test assumptions
- **Debug**: Add debug prints to see actual vs expected values

### Performance Issues

**Issue**: Snapshot creation >5ms
- **Profile**: Use `perf` or `gprof`
- **Check**: Iteration through MapField array may be slow
- **Optimize**: Use Player::vehicleList instead of scanning map

**Issue**: Clone time >1ms
- **Check**: std::map copy may be slow for large terrain
- **Optimize**: Consider flat array for dense tactical areas

---

## Key Files Reference

### Phase 0.1 Core Files
- `domain/types.h` - Common types
- `domain/unit_snapshot.h/cpp` - Compact unit data (~32 bytes)
- `domain/game_state_snapshot.h` - Main snapshot container
- `domain/i_game_state_reader.h` - Abstract interface (DI)
- `domain/game_state_reader.h/cpp` - GameMap adapter
- `domain/snapshot_test.cpp` - Unit tests

### Documentation
- `IMPLEMENTATION_SUMMARY.md` - What was built and why
- `STATUS.md` - Current progress tracking
- `docs/phase_0.1_code_analysis.md` - Legacy code analysis
- `docs/implementation_roadmap.md` - Full project roadmap

### Legacy ASC Files (Read-Only)
- `../../gamemap.h/cpp` - Central game state
- `../../vehicle.h/cpp` - Unit data
- `../../player.h/cpp` - Player resources
- `../../mapfield.h/cpp` - Terrain and visibility

---

## Useful Commands

```bash
# Build just MCTS library
make -C source/ai/mcts

# Clean and rebuild
make -C source/ai/mcts clean
make -C source/ai/mcts

# Check for compilation warnings
make -C source/ai/mcts CXXFLAGS="-Wall -Wextra -Werror"

# Run tests with valgrind (memory leak check)
valgrind --leak-check=full ./source/ai/mcts/snapshot_test

# Profile with gprof
make -C source/ai/mcts CXXFLAGS="-pg"
./source/ai/mcts/snapshot_test
gprof snapshot_test gmon.out

# Size of compiled objects
size source/ai/mcts/domain/.libs/*.o
```

---

## Getting Help

1. **Documentation**: Read `docs/` directory
2. **Code Comments**: All headers have detailed comments
3. **Tests**: See `snapshot_test.cpp` for usage examples
4. **ASC Wiki**: https://github.com/ValHaris/asc-hq/wiki
5. **Project Issues**: GitHub issue tracker

---

## License

GPL (matching ASC project license)
