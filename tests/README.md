# Google Test Suite - Getting Started

**Created**: 2025-11-17
**Status**: Active - Phase 1 Complete

---

## Overview

This directory contains the modern Google Test-based test suite for ASC. It runs alongside the legacy test framework during the migration period.

## Directory Structure

```
tests/
├── README.md           # This file
├── Makefile.am         # Autotools build configuration
├── unit/               # Unit tests (single components)
│   └── example_test.cpp
├── integration/        # Integration tests (multiple components)
└── helpers/            # Test fixtures and mock objects
```

---

## Building and Running Tests

### Prerequisites

Google Test is included as a git submodule. Ensure submodules are initialized:

```bash
git submodule update --init --recursive
```

### Building Tests

```bash
# From project root
./bootstrap
./configure

# Build tests
cd tests/
make

# Or build and run tests
make check
```

### Running Tests

```bash
# Run all tests
make check

# Run specific test
./example_test

# Run tests with verbose output
./example_test --gtest_color=yes

# Run specific test cases
./example_test --gtest_filter=StringTest.*

# List all available tests
./example_test --gtest_list_tests
```

---

## Writing New Tests

### 1. Create a Test File

Create a new file in the appropriate directory:
- `unit/` - for testing individual functions/classes
- `integration/` - for testing interactions between components

### 2. Basic Test Structure

```cpp
#include <gtest/gtest.h>

// Your includes here
#include "my_component.h"

namespace asc {
namespace test {

// Simple test
TEST(MyComponentTest, BasicFunctionality) {
   MyComponent component;
   EXPECT_EQ(component.getValue(), 42);
}

// Test with fixture (for setup/teardown)
class MyComponentFixture : public ::testing::Test {
protected:
   void SetUp() override {
      component = std::make_unique<MyComponent>();
   }

   void TearDown() override {
      component.reset();
   }

   std::unique_ptr<MyComponent> component;
};

TEST_F(MyComponentFixture, AdvancedTest) {
   ASSERT_NE(component, nullptr);
   EXPECT_TRUE(component->isValid());
}

} // namespace test
} // namespace asc
```

### 3. Add Test to Makefile.am

Edit `tests/Makefile.am` and add your test:

```makefile
# Add to TESTS list
TESTS = example_test my_new_test

check_PROGRAMS = $(TESTS)

# Define the test
my_new_test_SOURCES = unit/my_new_test.cpp
my_new_test_LDADD = \
   libgtest.la \
   libgtest_main.la \
   -pthread

# If testing MCTS code, link against MCTS library
# my_new_test_LDADD += $(top_builddir)/source/ai/mcts/libmcts.la
```

### 4. Rebuild and Test

```bash
make
make check
```

---

## Google Test Assertions

### Basic Assertions

```cpp
// Equality
EXPECT_EQ(actual, expected);    // Non-fatal
ASSERT_EQ(actual, expected);    // Fatal (stops test)

// Inequality
EXPECT_NE(a, b);

// Comparisons
EXPECT_LT(a, b);  // Less than
EXPECT_LE(a, b);  // Less than or equal
EXPECT_GT(a, b);  // Greater than
EXPECT_GE(a, b);  // Greater than or equal

// Boolean
EXPECT_TRUE(condition);
EXPECT_FALSE(condition);

// Floating point (with epsilon)
EXPECT_NEAR(val1, val2, abs_error);
EXPECT_DOUBLE_EQ(val1, val2);
EXPECT_FLOAT_EQ(val1, val2);
```

### String Assertions

```cpp
EXPECT_STREQ(str1, str2);      // C-style strings
EXPECT_EQ(str1, str2);         // std::string
EXPECT_THAT(str, HasSubstr("pattern"));  // Contains substring
```

### Exception Assertions

```cpp
EXPECT_THROW(statement, exception_type);
EXPECT_NO_THROW(statement);
EXPECT_ANY_THROW(statement);
```

---

## Advanced Features

### Parameterized Tests

Test the same logic with different inputs:

```cpp
class ParameterizedTest : public ::testing::TestWithParam<int> {
};

TEST_P(ParameterizedTest, TestWithValue) {
   int value = GetParam();
   EXPECT_GT(value, 0);
}

INSTANTIATE_TEST_SUITE_P(
   PositiveIntegers,
   ParameterizedTest,
   ::testing::Values(1, 5, 10, 42, 100)
);
```

### Test Fixtures

For tests that need setup/teardown:

```cpp
class GameStateTest : public ::testing::Test {
protected:
   void SetUp() override {
      // Runs before each test
      game_state = createTestGameState();
   }

   void TearDown() override {
      // Runs after each test
      game_state.reset();
   }

   std::unique_ptr<GameState> game_state;
};

TEST_F(GameStateTest, InitialState) {
   EXPECT_EQ(game_state->getTurnNumber(), 0);
}
```

### Death Tests

Test that code terminates correctly:

```cpp
TEST(DeathTest, CrashesOnInvalidInput) {
   EXPECT_DEATH(dangerousFunction(nullptr), "assertion failed");
}
```

---

## Best Practices

1. **One assertion per test** (when possible)
   - Makes failures easier to diagnose
   - Tests are more focused

2. **Use descriptive test names**
   ```cpp
   // Good
   TEST(VehicleTest, MovementReducesFuelByDistance)

   // Bad
   TEST(VehicleTest, Test1)
   ```

3. **Use EXPECT over ASSERT** unless test can't continue
   - EXPECT: continues test after failure
   - ASSERT: stops test after failure

4. **Organize tests by component**
   - Keep related tests together
   - Use test fixtures for shared setup

5. **Test edge cases**
   - Empty inputs
   - Null pointers
   - Boundary values
   - Error conditions

6. **Keep tests fast**
   - Unit tests should run in milliseconds
   - Use integration tests for slower tests

---

## CI/CD Integration

Tests run automatically on every commit via GitHub Actions. See:
- `.github/workflows/ci.yml` - CI configuration
- Job: `google-test-suite`

View test results:
1. Go to GitHub repository
2. Click "Actions" tab
3. Select latest workflow run
4. Check "Google Test Suite" job

---

## Migration Status

**Phase 1**: ✅ Complete (Infrastructure setup)
**Phase 2**: Pending (Migrate MCTS tests)
**Phase 3**: Pending (Incremental legacy migration)
**Phase 4**: Pending (Retirement of legacy framework)

See `docs/modernization/TEST_FRAMEWORK_MIGRATION.md` for full migration plan.

---

## Troubleshooting

### "Google Test not found"

Ensure submodules are initialized:
```bash
git submodule update --init --recursive
```

### Build failures

```bash
# Clean and rebuild
make clean
make
```

### Tests not running in CI

Check that:
1. Test is added to `TESTS` variable in `Makefile.am`
2. Test executable is in `check_PROGRAMS`
3. `configure.ac` detects Google Test correctly

---

## Resources

- [Google Test Primer](https://google.github.io/googletest/primer.html)
- [Google Test Advanced Guide](https://google.github.io/googletest/advanced.html)
- [Google Mock](https://google.github.io/googletest/gmock_for_dummies.html)
- Project Docs: `docs/modernization/TEST_FRAMEWORK_MIGRATION.md`

---

## Questions?

See:
- `docs/modernization/TEST_FRAMEWORK_MIGRATION.md` - Full migration documentation
- `docs/modernization/STATUS.md` - Current project status
- MCTS module: `source/ai/mcts/` - Reference for modern C++23 patterns
