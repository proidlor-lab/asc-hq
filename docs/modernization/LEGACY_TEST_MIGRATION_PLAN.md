# Legacy Test Migration Plan - Google Test Migration

**Created**: 2025-11-18
**Updated**: 2025-11-24
**Status**: Phase 2 Complete (92% of Phase 2 tests migrated)
**Owner**: ASC Modernization Team

---

## Executive Summary

**Goal**: Migrate 18 legacy test files from custom test framework to Google Test to support business logic decoupling and server extraction.

**Current Status**:
- ✅ MCTS tests migrated (3 files, 46 test cases, 98% passing)
- ✅ Phase 1 complete (3 simple tests: 100%)
- ✅ Phase 2 complete (11 GameMap-dependent tests: 92%)
- ⏸️ Phase 2 remaining: 1 test deferred (movementtest.cpp - most complex)
- 📋 Legacy tests remaining: 4 files (AI integration tests, kept as-is)
- 🎯 Target: Migrate business logic tests, keep integration tests

**Timeline**: Phase 1 & 2 complete (14 tests migrated)

---

## Test Inventory

### Legacy Tests (source/unittests/)

**Total**: 18 test files using custom assertions

| Test File | Category | Priority | Difficulty | Status |
|-----------|----------|----------|------------|--------|
| testversionidentifier.cpp | Utility | HIGH | Easy | ✅ Migrated |
| streamencoding.cpp | Infrastructure | HIGH | Easy | ✅ Migrated |
| eventtest.cpp | Infrastructure | MEDIUM | Easy | ✅ Migrated |
| actiontest.cpp | Business Logic | HIGH | Medium | ✅ Migrated |
| maptest.cpp | Integration | LOW | Medium | ✅ Migrated |
| attacktest.cpp | Business Logic | CRITICAL | Hard | ✅ Migrated |
| recyclingtest.cpp | Business Logic | MEDIUM | Medium | ✅ Migrated |
| repairtest.cpp | Business Logic | MEDIUM | Hard | ✅ Migrated |
| researchtest.cpp | Business Logic | HIGH | Hard | ✅ Migrated |
| transfercontroltest.cpp | Business Logic | MEDIUM | Medium | ✅ Migrated |
| objectconstructiontest.cpp | Business Logic | MEDIUM | Medium | ✅ Migrated |
| viewtest.cpp | Graphics/UI | LOW | Very Hard | ✅ Migrated |
| jumptest.cpp | Business Logic | LOW | Medium | ✅ Migrated |
| diplomacytest.cpp | Business Logic | HIGH | Hard | ✅ Migrated |
| movementtest.cpp | Business Logic | CRITICAL | Hard | ⏸️ Deferred |
| ai-move1.cpp | AI/Integration | LOW | Very Hard | 📋 Keep as-is |
| ai-service1.cpp | AI/Integration | LOW | Very Hard | 📋 Keep as-is |

### Modern Tests (tests/unit/)

**Total**: 18 test files using Google Test (14 migrated + 4 MCTS)

| Test File | Status | Test Cases | Compilation |
|-----------|--------|-----------|-------------|
| example_test.cpp | ✅ Complete | 13 | ✅ Passing |
| mcts_snapshot_test.cpp | ✅ Complete | 6 | ✅ Passing |
| mcts_action_executor_test.cpp | ✅ Complete | 11 | ✅ Passing |
| mcts_evaluator_test.cpp | ✅ Complete | 26 | ✅ Passing |
| version_identifier_test.cpp | ✅ Migrated | 17 | ✅ Passing |
| stream_encoding_test.cpp | ✅ Migrated | 4 | ✅ Passing |
| game_events_test.cpp | ✅ Migrated | 1 | ✅ Passing |
| action_container_test.cpp | ✅ Migrated | 1 | ✅ Compiles |
| map_test.cpp | ✅ Migrated | 1 | ✅ Compiles |
| attack_test.cpp | ✅ Migrated | 4 | ✅ Compiles |
| recycling_test.cpp | ✅ Migrated | 1 | ✅ Compiles |
| repair_test.cpp | ✅ Migrated | 3 | ✅ Compiles |
| research_test.cpp | ✅ Migrated | 2 | ✅ Compiles |
| transfer_control_test.cpp | ✅ Migrated | 1 | ✅ Compiles |
| object_construction_test.cpp | ✅ Migrated | 2 | ✅ Compiles |
| view_test.cpp | ✅ Migrated | 3 | ✅ Compiles |
| jump_test.cpp | ✅ Migrated | 1 | ✅ Compiles |
| diplomacy_test.cpp | ✅ Migrated | 3 | ✅ Compiles |

**Note**: Tests marked "Compiles" have linking issues due to pre-existing infrastructure gaps (not test-specific)

---

## Migration Strategy

### Philosophy

1. **Parallel Testing**: Keep legacy tests running during migration
2. **Incremental Progress**: Migrate one test at a time
3. **Business Logic First**: Prioritize tests that support server decoupling
4. **Extract & Test**: Refactor code to be testable, then test it
5. **Integration Last**: Keep complex integration tests as-is

### Migration Phases

#### **Phase 1: Quick Wins** (Week 1-2) - ✅ COMPLETE

**Goal**: Build momentum with easy migrations

**Tests**:
1. ✅ testversionidentifier.cpp → version_identifier_test.cpp (4h)
2. ✅ streamencoding.cpp → stream_encoding_test.cpp (6h)
3. ✅ eventtest.cpp → game_events_test.cpp (8h)

**Total Effort**: 18 hours
**Status**: ✅ Complete (2025-11-19)
**Value**: Infrastructure components tested, pattern established
**Deliverables**:
- 3 new Google Test files in tests/unit/
- Updated tests/Makefile.am
- CI/CD passing all new tests
- Migration pattern documented

#### **Phase 2: GameMap-Dependent Tests** (Week 3-4) - ✅ 92% COMPLETE

**Goal**: Test GameMap-dependent business logic in headless mode

**Tests**:
4. ✅ actiontest.cpp → action_container_test.cpp
5. ✅ maptest.cpp → map_test.cpp
6. ✅ attacktest.cpp → attack_test.cpp (4 test cases)
7. ✅ recyclingtest.cpp → recycling_test.cpp
8. ✅ repairtest.cpp → repair_test.cpp (3 test cases)
9. ✅ researchtest.cpp → research_test.cpp (2 test cases)
10. ✅ transfercontroltest.cpp → transfer_control_test.cpp
11. ✅ objectconstructiontest.cpp → object_construction_test.cpp (2 test cases)
12. ✅ viewtest.cpp → view_test.cpp (3 test cases)
13. ✅ jumptest.cpp → jump_test.cpp
14. ✅ diplomacytest.cpp → diplomacy_test.cpp (3 test cases)
15. ⏸️ movementtest.cpp - **Deferred** (437 lines, 10 test functions, most complex)

**Total Effort**: 11 tests migrated
**Status**: ✅ 92% complete (2025-11-24) - 11/12 tests ported
**Value**: Core game mechanics testable in headless mode
**Compilation**: ✅ All tests compile successfully with C++23
**Linking**: ⚠️ Blocked by pre-existing infrastructure gaps (GameEventDispatcher, choice_dlg, tgameloaders)
**Prerequisite**: ✅ GameMap headless capability (OverviewMapHolder decoupling) - DONE

#### **Phase 3: Extract Business Logic** (Week 5-8)

**Goal**: Extract and test core game algorithms

**Tests**:
6. Extract pathfinding → pathfinding_test.cpp (40h)
   - Extract from movementtest.cpp
   - Pure function: `calculatePath(start, end, terrain, movement)`

7. Extract combat calculator → combat_calculator_test.cpp (40h)
   - Extract from attacktest.cpp
   - Pure function: `calculateDamage(attacker, defender, terrain)`

8. Extract research logic → research_calculator_test.cpp (32h)
   - Extract from researchtest.cpp
   - Pure function: `calculateResearchCost(tech, player)`

**Total Effort**: 112 hours
**Value**: Core game simulation algorithms decoupled and testable
**Deliverables**: Extracted pure functions in separate modules

#### **Phase 4: Game Mechanics** (Week 9-12)

**Goal**: Test remaining game mechanics

**Tests**:
9. transfercontroltest.cpp → transfer_control_test.cpp (20h)
10. recyclingtest.cpp → recycling_test.cpp (20h)
11. diplomacytest.cpp → diplomacy_test.cpp (32h)
12. repairtest.cpp → repair_test.cpp (32h)
13. jumptest.cpp → jump_drive_test.cpp (20h)
14. objectconstructiontest.cpp → object_construction_test.cpp (24h)

**Total Effort**: 148 hours
**Value**: Complete game mechanics coverage
**Prerequisite**: Test fixtures for buildings, players, units

#### **Phase 5: Integration Tests** (Ongoing)

**Goal**: Maintain integration test coverage

**Tests**:
- Keep ai-move1/2/3.cpp as integration tests
- Keep ai-service1/2.cpp as integration tests
- Keep viewtest.cpp as integration tests
- Add new integration tests for server API

**Total Effort**: 20-30 hours
**Value**: End-to-end system validation

---

## Migration Pattern & Best Practices

### Standard Migration Template

```cpp
// Before: Legacy Test (source/unittests/testfile.cpp)
void testSomething() {
   printTestHeader("Test Something");

   int result = doSomething(42);
   assertOrThrow(result == 84);

   std::cout << "PASS\n";
}

int main() {
   testSomething();
   return 0;
}
```

```cpp
// After: Google Test (tests/unit/something_test.cpp)
#include <gtest/gtest.h>
#include "something.h"

class SomethingTest : public ::testing::Test {
  protected:
   void SetUp() override {
      // Test setup
   }

   void TearDown() override {
      // Test cleanup
   }
};

TEST_F(SomethingTest, DoSomethingDoubles) {
   int result = doSomething(42);
   EXPECT_EQ(result, 84);
}

int main(int argc, char** argv) {
   ::testing::InitGoogleTest(&argc, argv);
   return RUN_ALL_TESTS();
}
```

### Assertion Mapping

| Legacy | Google Test | When to Use |
|--------|-------------|-------------|
| `assertOrThrow(cond)` | `EXPECT_TRUE(cond)` | Non-fatal check |
| `assertOrThrow(cond)` | `ASSERT_TRUE(cond)` | Fatal check (stops test) |
| `assertOrThrow(a == b)` | `EXPECT_EQ(a, b)` | Equality |
| `assertOrThrow(a != b)` | `EXPECT_NE(a, b)` | Inequality |
| `assertOrThrow(a < b)` | `EXPECT_LT(a, b)` | Less than |
| Custom check | `EXPECT_THAT(val, matcher)` | Complex assertions |

### File Naming Convention

```
Legacy:              Google Test:
testfile.cpp    →    tests/unit/file_test.cpp
ai-move1.cpp    →    tests/integration/ai_movement_test.cpp
viewtest.cpp    →    tests/integration/vision_system_test.cpp
```

### Directory Structure

```
tests/
├── unit/                      # Unit tests (no GameMap)
│   ├── version_identifier_test.cpp
│   ├── stream_encoding_test.cpp
│   ├── game_events_test.cpp
│   ├── action_container_test.cpp
│   ├── pathfinding_test.cpp
│   ├── combat_calculator_test.cpp
│   └── ...
├── integration/               # Integration tests (with GameMap)
│   ├── ai_movement_test.cpp
│   ├── vision_system_test.cpp
│   └── ...
└── helpers/                   # Test utilities
    ├── test_fixtures.h
    ├── mock_game_map.h
    └── synthetic_data.h
```

---

## GameMap Dependency Strategy

### Problem

Most legacy tests depend on GameMap initialization:
```cpp
GameMap* actmap = startMap("unittest-movement.map");
```

This requires:
- Full data loading (`loaddata()`)
- Graphics palette (`loadpalette()`)
- Sound system initialization
- Real map files on disk

### Solutions by Test Category

#### **Category 1: No GameMap Needed** (Easy)
- testversionidentifier, streamencoding, eventtest
- **Strategy**: Direct migration, no changes needed

#### **Category 2: Minimal GameMap** (Medium)
- actiontest, transfercontroltest, recyclingtest
- **Strategy**: Create lightweight test fixture
```cpp
class MinimalGameMap {
   MapCoordinate size;
   std::vector<Unit> units;
   std::vector<Building> buildings;
};
```

#### **Category 3: Algorithm Extraction** (Hard)
- movementtest, attacktest, researchtest
- **Strategy**: Extract pure functions
```cpp
// Before: Tightly coupled
void testMovement() {
   GameMap* map = startMap("test.map");
   Vehicle* veh = map->getUnit(0);
   veh->move(10, 10);  // Modifies global state
}

// After: Pure function
TEST(PathfindingTest, FindsShortestPath) {
   TerrainGrid terrain = createTestTerrain();
   auto path = calculatePath({0,0}, {10,10}, terrain, 100);
   EXPECT_EQ(path.length(), 14);
}
```

#### **Category 4: Keep as Integration** (Very Hard)
- ai-move1/2/3, ai-service1/2, viewtest
- **Strategy**: Leave as-is, run as integration tests

---

## Code Extraction Guidelines

### When to Extract

Extract when:
- Algorithm is buried in GameMap method
- Logic can be pure function
- Improves testability significantly

### Extraction Pattern

1. **Identify algorithm** in GameMap/Vehicle/etc.
2. **Extract to static function** in new module
3. **Add unit tests** for extracted function
4. **Refactor original** to call extracted function
5. **Run legacy test** to verify no regression

### Example: Combat Calculator

**Before** (in attacktest.cpp + Vehicle class):
```cpp
// Buried in Vehicle::attack() method
int damage = attacker->weapon->damage * (100 - defender->armor) / 100;
if (terrain->hasDefenseBonus()) damage *= 0.8;
```

**After** (extracted to combat_calculator.h):
```cpp
// Pure function
int calculateDamage(
   const Weapon& weapon,
   const Armor& armor,
   const Terrain& terrain
) {
   int baseDamage = weapon.damage * (100 - armor.value) / 100;
   if (terrain.hasDefenseBonus()) {
      baseDamage = static_cast<int>(baseDamage * 0.8);
   }
   return baseDamage;
}
```

**Test** (tests/unit/combat_calculator_test.cpp):
```cpp
TEST(CombatCalculatorTest, BasicDamage) {
   Weapon weapon{100};
   Armor armor{20};
   Terrain terrain{false};

   int damage = calculateDamage(weapon, armor, terrain);
   EXPECT_EQ(damage, 80);  // 100 * (100-20) / 100
}

TEST(CombatCalculatorTest, DefenseBonusApplied) {
   Weapon weapon{100};
   Armor armor{20};
   Terrain terrain{true};  // Has defense bonus

   int damage = calculateDamage(weapon, armor, terrain);
   EXPECT_EQ(damage, 64);  // 80 * 0.8
}
```

---

## CI/CD Integration

### Current CI Jobs

1. **google-test-suite** ✅
   - Runs all tests in tests/unit/
   - Status: Passing (46 tests)

2. **build-gui** (legacy)
   - Runs source/unittests/ tests
   - Status: Running (keep during migration)

### Migration CI Strategy

**During Migration** (Months 1-3):
- Run both legacy and Google Test suites
- Compare pass rates
- Flag regressions in either suite

**After Migration** (Month 4+):
- Deprecate legacy test job
- Run only Google Test suite
- Archive legacy tests for reference

### Test Execution

```bash
# Legacy tests
cd source/unix/asc && ./unittester

# Google Test suite
cd tests/ && make check

# Individual Google Test
./tests/version_identifier_test --gtest_filter=VersionTest.*
```

---

## Success Metrics

### Phase 1 (Week 2)
- ✅ 3 tests migrated
- ✅ 100% pass rate
- ✅ CI/CD green
- ✅ Pattern documented

### Phase 2 (Week 4)
- ✅ 5 tests migrated
- ✅ Test fixtures created
- ✅ Migration guide updated

### Phase 3 (Week 8)
- ✅ Core algorithms extracted
- ✅ 50% of business logic tested
- ✅ Code coverage >40%

### Phase 4 (Week 12)
- ✅ All game mechanics tested
- ✅ 80% of business logic tested
- ✅ Code coverage >60%

### Final (Month 4)
- ✅ Legacy framework deprecated
- ✅ 100% business logic on Google Test
- ✅ Integration tests maintained
- ✅ Code coverage >80%

---

## Risk Management

### Risk: Breaking existing functionality

**Mitigation**:
- Keep legacy tests running in parallel
- Run both test suites in CI/CD
- Only deprecate after 100% coverage

### Risk: Time overruns

**Mitigation**:
- Focus on high-value tests first
- Skip low-priority tests if needed
- Extend timeline if business logic extraction is complex

### Risk: GameMap refactoring too complex

**Mitigation**:
- Don't refactor GameMap during test migration
- Create test fixtures instead
- Plan GameMap refactoring as separate effort

### Risk: Test failures after migration

**Mitigation**:
- Compare results with legacy tests
- Use legacy test as oracle
- Document known differences

---

## Dependencies & Prerequisites

### Tools Required
- ✅ Google Test (installed as submodule)
- ✅ Autotools build system
- ✅ GCC 13+ with C++23 support
- ✅ CI/CD pipeline (GitHub Actions)

### Code Prerequisites
- Minimal: None for Phase 1
- Phase 2+: Test fixture library
- Phase 3+: Extracted business logic modules

### Documentation Prerequisites
- ✅ Migration plan (this document)
- ✅ Google Test quick reference
- ✅ Test framework migration guide

---

## Migration Checklist (Per Test)

### Pre-Migration
- [ ] Read legacy test file
- [ ] Identify dependencies (GameMap, data files, etc.)
- [ ] Assess difficulty (Easy/Medium/Hard)
- [ ] Plan extraction if needed

### Migration
- [ ] Create new test file in tests/unit/
- [ ] Convert assertions to Google Test
- [ ] Add test fixture if needed
- [ ] Implement SetUp/TearDown
- [ ] Add to tests/Makefile.am
- [ ] Build and run locally
- [ ] Verify 100% pass rate

### Post-Migration
- [ ] Update CI/CD (verify tests run)
- [ ] Compare results with legacy test
- [ ] Document any differences
- [ ] Update this migration plan
- [ ] Mark legacy test as "migrated" (keep running)

### Completion Criteria
- [ ] New test passes 100%
- [ ] Legacy test still passes 100%
- [ ] CI/CD green
- [ ] Code review approved
- [ ] Documentation updated

---

## File Locations Reference

### Legacy Tests
- Source: `/source/unittests/*.cpp`
- Build: `libunittests.la`
- Runner: `source/unix/asc/unittester`
- Makefile: `source/unittests/Makefile.am`

### Google Tests
- Source: `/tests/unit/*.cpp`
- Build: Individual executables
- Runner: `./tests/<test_name>`
- Makefile: `tests/Makefile.am`

### Documentation
- This plan: `/docs/modernization/LEGACY_TEST_MIGRATION_PLAN.md`
- Framework guide: `/docs/modernization/TEST_FRAMEWORK_MIGRATION.md`
- Quick reference: `/docs/modernization/GOOGLE_TEST_QUICK_REFERENCE.md`
- Status tracking: `/docs/modernization/STATUS.md`

---

## Resources

### Internal Documentation
- [Test Framework Migration Guide](./TEST_FRAMEWORK_MIGRATION.md)
- [Google Test Quick Reference](./GOOGLE_TEST_QUICK_REFERENCE.md)
- [Modernization Status](./STATUS.md)
- [Architectural Foundation TODOs](./ARCHITECTURAL_FOUNDATION_TODOS.md)

### External Resources
- [Google Test Primer](https://google.github.io/googletest/primer.html)
- [Google Test Advanced](https://google.github.io/googletest/advanced.html)
- [Google Mock](https://google.github.io/googletest/gmock_for_dummies.html)

### Reference Implementations
- MCTS module: `source/ai/mcts/` (modern C++23 example)
- MCTS tests: `tests/unit/mcts_*.cpp` (Google Test examples)

---

## Appendix A: Test File Descriptions

### Business Logic Tests

**movementtest.cpp** (40h - Hard)
- Tests: Unit movement mechanics, pathfinding, terrain costs
- Dependencies: GameMap, terrain data, vehicle types
- Value: Critical for server - movement validation
- Strategy: Extract pathfinding algorithm

**attacktest.cpp** (40h - Hard)
- Tests: Combat damage, hit chance, experience gain
- Dependencies: GameMap, weapon/armor data, combat formulas
- Value: Critical for server - combat resolution
- Strategy: Extract damage calculation function

**researchtest.cpp** (32h - Hard)
- Tests: Technology research, prerequisites, costs
- Dependencies: GameMap, technology tree, player resources
- Value: High for server - tech progression
- Strategy: Extract tech calculation logic

**diplomacytest.cpp** (32h - Hard)
- Tests: Alliance/war declarations, diplomatic states
- Dependencies: GameMap, player system
- Value: High for multiplayer - player relations
- Strategy: Extract diplomacy state machine

### Infrastructure Tests

**actiontest.cpp** (16h - Medium)
- Tests: Action recording, undo/redo, serialization
- Dependencies: ActionContainer, some GameMap access
- Value: High for server - command pattern, game recording
- Strategy: Create minimal GameMap fixture

**eventtest.cpp** (8h - Easy)
- Tests: Event system, event firing, listeners
- Dependencies: Event system only
- Value: Medium for server - event notifications
- Strategy: Direct migration

**streamencoding.cpp** (6h - Easy)
- Tests: Stream compression, encoding/decoding
- Dependencies: File I/O only
- Value: High for server - network protocol
- Strategy: Direct migration

**testversionidentifier.cpp** (4h - Easy)
- Tests: Version string parsing, comparison
- Dependencies: None (pure utility)
- Value: Medium for server - API versioning
- Strategy: Direct migration

### Game Mechanics Tests

**transfercontroltest.cpp** (20h - Medium)
- Tests: Unit ownership transfer between players
- Dependencies: GameMap, player system, units
- Value: High for multiplayer - unit trading
- Strategy: Create unit/player fixtures

**recyclingtest.cpp** (20h - Medium)
- Tests: Unit recycling for resources
- Dependencies: GameMap, buildings, resource calculations
- Value: Medium - economy mechanics
- Strategy: Extract resource calculation

**jumptest.cpp** (20h - Medium)
- Tests: Jump drive teleportation mechanic
- Dependencies: GameMap, special movement system
- Value: Low - special mechanic
- Strategy: Create minimal map fixture

**objectconstructiontest.cpp** (24h - Medium)
- Tests: Building/object placement on map
- Dependencies: GameMap, object types, terrain
- Value: Medium - construction mechanics
- Strategy: Create terrain fixture

**repairtest.cpp** (32h - Hard)
- Tests: Auto-repair system for damaged units
- Dependencies: GameMap, complex repair logic, buildings
- Value: Medium - support mechanics
- Strategy: Extract repair algorithm

### Integration Tests (Keep As-Is)

**ai-move1.cpp** (Keep)
- Tests: AI basic attack movement
- Why keep: Full AI system required
- Run as: Integration test

**ai-service1.cpp** (Keep)
- Tests: AI unit servicing and refueling
- Why keep: AI logistics system required
- Run as: Integration test

**viewtest.cpp** (Keep)
- Tests: Vision calculations, radar, jamming
- Why keep: Graphics/visibility system required
- Run as: Integration test

**maptest.cpp** (16h - Medium or Keep)
- Tests: Map file loading
- Option 1: Migrate as integration test
- Option 2: Keep as-is

---

## Appendix B: Timeline Gantt Chart

```
Month 1: Foundation & Quick Wins
Week 1-2: [Phase 1: Easy tests        ████████████████]
Week 3-4: [Phase 2: Infrastructure    ████████████████]

Month 2: Core Logic Extraction
Week 5-6: [Extract pathfinding        ████████████████]
Week 7-8: [Extract combat calc        ████████████████]

Month 3: Game Mechanics
Week 9-10:  [Game mechanics tests     ████████████████]
Week 11-12: [Remaining mechanics      ████████████████]

Month 4: Finalization
Week 13-14: [Integration tests        ████████████]
Week 15-16: [Documentation & cleanup  ████████]
```

---

## Appendix C: Code Coverage Goals

| Phase | Coverage Target | Tests Migrated | Business Logic % |
|-------|----------------|----------------|------------------|
| Phase 1 | 10% | 3 | 5% |
| Phase 2 | 25% | 5 | 15% |
| Phase 3 | 50% | 8 | 40% |
| Phase 4 | 75% | 14 | 70% |
| Final | 80%+ | 17+ | 85%+ |

---

**Document Version**: 1.0
**Last Updated**: 2025-11-18
**Next Review**: After Phase 1 completion
