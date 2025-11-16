# Test Framework Modernization

**Purpose**: Migrate from custom test framework to Google Test while maintaining legacy tests.

**Created**: 2025-11-16
**Status**: Planning & Initial Setup

---

## Current State

### Legacy Test Framework

**Location**: `source/unittests/`

**Characteristics**:
- Custom assertion macros
- ~15 test files covering core game mechanics
- Integrated with main build via `libunittests.la`
- Tests: movement, attack, diplomacy, research, etc.

**Example**:
```cpp
// Custom assertions in legacy tests
void assertEqual(int actual, int expected, const char* msg);
void assertTrue(bool condition, const char* msg);
```

### MCTS Tests

**Location**: `source/ai/mcts/`

**Characteristics**:
- Standalone test binaries
- Manual assertions with `assert()`
- Modern C++23 style
- 3 test files: snapshot_test, action_executor_test, evaluator_test
- 97% pass rate (64/66 tests)

---

## Migration Strategy

### Phase 1: Parallel Infrastructure (Week 1) - **CURRENT**

**Goal**: Set up Google Test alongside existing tests

**Actions**:
1. ✅ Install Google Test as git submodule or package
2. ✅ Create `tests/` directory for new Google Test-based tests
3. ✅ Set up CMake or Autotools integration
4. ✅ Write example Google Test to validate setup
5. ✅ Update CI/CD to run both test suites

**Outcome**: Both test frameworks coexist. New tests use Google Test.

### Phase 2: Migrate MCTS Tests (Week 2)

**Goal**: Convert MCTS tests to Google Test as template

**Actions**:
1. Migrate `snapshot_test.cpp` to Google Test
2. Migrate `action_executor_test.cpp`
3. Migrate `evaluator_test.cpp`
4. Document migration pattern

**Outcome**: MCTS module fully on Google Test. Pattern established for others.

### Phase 3: Incremental Legacy Migration (Ongoing)

**Goal**: Gradually migrate legacy tests when touching related code

**Actions**:
- When refactoring a module, migrate its tests
- Priority: High-value tests (attack, movement, game events)
- Low priority: Deprecated features

**Outcome**: Gradual migration over 3-6 months.

### Phase 4: Retirement (Month 6+)

**Goal**: Remove custom test framework

**Actions**:
1. Ensure all critical tests migrated
2. Archive or delete remaining legacy tests
3. Remove custom framework code

**Outcome**: Single modern test framework.

---

## Google Test vs Catch2

### Google Test (Recommended)

**Pros**:
- Industry standard
- Excellent IDE integration
- Mature, well-documented
- Good mocking support (Google Mock)
- Used by LLVM, Chromium, etc.

**Cons**:
- Slightly more verbose than Catch2
- Requires separate library

**Example**:
```cpp
#include <gtest/gtest.h>

TEST(UnitSnapshotTest, Size) {
   EXPECT_LE(sizeof(UnitSnapshot), 48);
}

TEST(GameStateTest, Clone) {
   GameStateSnapshot original;
   auto clone = original.clone();
   ASSERT_NE(clone, nullptr);
}
```

### Catch2 (Alternative)

**Pros**:
- Header-only (easier integration)
- Less verbose
- Beautiful output
- BDD-style tests possible

**Cons**:
- Slower compile times (header-only)
- Less IDE support than Google Test

**Decision**: Use **Google Test** for consistency with industry and better tooling.

---

## Directory Structure

```
asc-hq-codex/
├── source/
│   ├── unittests/           # Legacy tests (keep for now)
│   │   ├── attacktest.cpp
│   │   ├── movementtest.cpp
│   │   └── ...
│   └── ai/mcts/
│       ├── snapshot_test.cpp     # To migrate
│       ├── action_executor_test.cpp
│       └── evaluator_test.cpp
├── tests/                   # NEW: Google Test tests
│   ├── CMakeLists.txt       # Test build config
│   ├── unit/
│   │   ├── game_state_test.cpp
│   │   ├── unit_snapshot_test.cpp
│   │   └── ...
│   ├── integration/
│   │   ├── mcts_integration_test.cpp
│   │   └── ...
│   └── helpers/
│       ├── test_fixtures.h
│       └── mock_objects.h
└── third_party/
    └── googletest/          # Git submodule
```

---

## Installation

### Option 1: Git Submodule (Recommended)

```bash
# Add Google Test as submodule
git submodule add https://github.com/google/googletest.git third_party/googletest
git submodule update --init --recursive

# Configure autotools to build it
# (Add to configure.ac and tests/Makefile.am)
```

### Option 2: System Package

```bash
# Install from package manager
sudo apt-get install libgtest-dev libgmock-dev

# Note: On Ubuntu, you may need to build it:
cd /usr/src/googletest
sudo cmake .
sudo make
sudo cp lib/*.a /usr/lib
```

---

## Integration with Autotools

### Update `configure.ac`

```autoconf
# Add Google Test check
PKG_CHECK_MODULES([GTEST], [gtest >= 1.10.0], [have_gtest=yes], [have_gtest=no])
AM_CONDITIONAL([HAVE_GTEST], [test "x$have_gtest" = "xyes"])

if test "x$have_gtest" = "xyes"; then
    AC_MSG_NOTICE([Google Test found - building modern tests])
else
    AC_MSG_WARN([Google Test not found - skipping modern tests])
fi
```

### Create `tests/Makefile.am`

```makefile
if HAVE_GTEST

# Google Test-based tests
TESTS = unit_snapshot_test action_executor_test

check_PROGRAMS = $(TESTS)

# Common flags
AM_CPPFLAGS = -I$(top_srcdir)/source \
              -I$(top_srcdir)/source/ai/mcts \
              $(GTEST_CFLAGS)

AM_CXXFLAGS = -std=c++23 -Wall -Wextra

# Unit snapshot test
unit_snapshot_test_SOURCES = unit/unit_snapshot_test.cpp
unit_snapshot_test_LDADD = $(GTEST_LIBS) \
                           $(top_builddir)/source/ai/mcts/libmcts.la

# Action executor test
action_executor_test_SOURCES = unit/action_executor_test.cpp
action_executor_test_LDADD = $(GTEST_LIBS) \
                             $(top_builddir)/source/ai/mcts/libmcts.la

endif # HAVE_GTEST
```

---

## Example Migration: snapshot_test.cpp

### Before (Custom Assertions)

```cpp
void testUnitSnapshotSize() {
   printTestHeader("Test 1: UnitSnapshot Size");

   size_t size = sizeof(UnitSnapshot);
   std::cout << "UnitSnapshot size: " << size << " bytes\n";

   assertTrue(size <= 48, "UnitSnapshot should be <= 48 bytes");
   std::cout << "PASS\n";
}

int main() {
   testUnitSnapshotSize();
   testGameStateSize();
   // ... more tests
   return 0;
}
```

### After (Google Test)

```cpp
#include <gtest/gtest.h>
#include "unit_snapshot.h"
#include "game_state_snapshot.h"

using namespace asc::mcts;

class UnitSnapshotTest : public ::testing::Test {
protected:
   void SetUp() override {
      // Setup code
   }
};

TEST_F(UnitSnapshotTest, SizeIsCompact) {
   size_t size = sizeof(UnitSnapshot);
   EXPECT_LE(size, 48) << "UnitSnapshot size: " << size << " bytes";

   if (size <= 32) {
      std::cout << "✓ Excellent: Within ideal 32-byte target!\n";
   }
}

TEST_F(UnitSnapshotTest, DefaultConstruction) {
   UnitSnapshot unit;
   EXPECT_EQ(unit.unitId, 0);
   EXPECT_EQ(unit.health, 0);
}

// Main
int main(int argc, char** argv) {
   ::testing::InitGoogleTest(&argc, argv);
   return RUN_ALL_TESTS();
}
```

---

## Benefits

### Immediate

- ✅ Modern test framework for new code
- ✅ Better assertions and error messages
- ✅ Test fixtures and parameterized tests
- ✅ Mocking support (Google Mock)
- ✅ XML output for CI/CD integration

### Long-term

- ✅ Industry-standard testing
- ✅ Better IDE integration
- ✅ Easier onboarding for new contributors
- ✅ Test coverage reporting
- ✅ Benchmarking support

---

## Test Coverage Reporting

### Using lcov/gcov

```bash
# Build with coverage flags
./configure CXXFLAGS="-g -O0 --coverage"
make

# Run tests
make check

# Generate coverage report
lcov --capture --directory . --output-file coverage.info
lcov --remove coverage.info '/usr/*' '*/third_party/*' --output-file coverage.info
genhtml coverage.info --output-directory coverage_html

# View report
firefox coverage_html/index.html
```

### CI/CD Integration

```yaml
# In .github/workflows/ci.yml
- name: Run tests with coverage
  run: |
    ./configure CXXFLAGS="--coverage"
    make check

- name: Generate coverage report
  run: |
    lcov --capture --directory . --output-file coverage.info
    lcov --list coverage.info

- name: Upload to Codecov
  uses: codecov/codecov-action@v3
  with:
    files: ./coverage.info
```

---

## Migration Checklist

### Phase 1 Setup
- [ ] Install Google Test (submodule or package)
- [ ] Create `tests/` directory structure
- [ ] Update `configure.ac` with Google Test check
- [ ] Create `tests/Makefile.am`
- [ ] Write first Google Test (smoke test)
- [ ] Update CI/CD to run Google Tests
- [ ] Document setup in this file

### Phase 2 MCTS Migration
- [ ] Migrate `snapshot_test.cpp`
- [ ] Migrate `action_executor_test.cpp`
- [ ] Migrate `evaluator_test.cpp`
- [ ] Verify all MCTS tests pass
- [ ] Update MCTS documentation

### Phase 3 Coverage
- [ ] Set up lcov/gcov
- [ ] Generate baseline coverage report
- [ ] Integrate with CI/CD
- [ ] Set coverage targets (e.g., 80%)

---

## Risks & Mitigation

### Risk: Breaking existing tests during migration

**Mitigation**: Keep legacy tests running in parallel. Only remove after full migration and validation.

### Risk: Google Test dependency issues

**Mitigation**: Use git submodule for version control. Document installation clearly.

### Risk: Time cost of migration

**Mitigation**: Incremental migration. Focus on high-value tests first (MCTS, core game mechanics).

---

## Success Criteria

**Phase 1 Complete When**:
- ✅ Google Test installed and building
- ✅ At least 1 test running in CI/CD
- ✅ Documentation complete

**Phase 2 Complete When**:
- ✅ All MCTS tests migrated to Google Test
- ✅ Original MCTS tests passing at same rate (97%+)
- ✅ Migration pattern documented

**Fully Complete When**:
- ✅ All critical tests on Google Test
- ✅ Test coverage >80%
- ✅ Legacy framework removed
- ✅ CI/CD running only Google Test

---

## Resources

- [Google Test Primer](https://google.github.io/googletest/primer.html)
- [Google Test Advanced Guide](https://google.github.io/googletest/advanced.html)
- [Google Mock](https://google.github.io/googletest/gmock_for_dummies.html)
- [Autotools with Google Test](https://mesonbuild.com/Unit-tests.html)

---

**Next Steps**: Begin Phase 1 setup - install Google Test and create first modern test.
