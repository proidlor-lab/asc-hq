# Google Test Quick Reference

**Created**: 2025-11-17
**Status**: Active

Quick reference guide for using Google Test in ASC modernization project.

---

## 📍 Where Are The Tests?

```
asc-hq-codex/
├── tests/                          # Google Test suite (NEW)
│   ├── unit/
│   │   └── example_test.cpp       # 13 tests, all passing
│   ├── integration/                # Future integration tests
│   ├── helpers/                    # Test fixtures & mocks
│   └── README.md                   # Detailed getting started guide
├── source/unittests/               # Legacy custom tests (parallel)
└── source/ai/mcts/                 # MCTS tests (to be migrated)
    ├── snapshot_test.cpp
    ├── action_executor_test.cpp
    └── evaluator_test.cpp
```

---

## 🚀 Quick Start

### Build and Run All Tests

```bash
# From project root
./configure
cd tests/
make check
```

### Run Specific Test

```bash
# Run one test suite
./example_test --gtest_filter=StringTest.*

# Run one test case
./example_test --gtest_filter=ExampleTest.BasicArithmetic

# List all tests
./example_test --gtest_list_tests
```

### Run with Colored Output

```bash
./example_test --gtest_color=yes
```

---

## 🔄 CI/CD Integration

### When Tests Run Automatically

✅ **Every `git push`** to any branch
✅ **Every pull request**
✅ **Manual trigger** via GitHub Actions UI

### View Test Results

1. Go to **GitHub** → Your repository
2. Click **"Actions"** tab
3. Select latest workflow run
4. Click **"Google Test Suite"** job
5. View test output:
   ```
   [==========] Running 13 tests from 8 test suites.
   [  PASSED  ] 13 tests.
   ```

### Download Test Logs

- Navigate to workflow run → **Artifacts** section
- Download: `google-test-results`
- Contains: `example_test.log`, `test-suite.log`
- **Retention**: 30 days

### Build Status

- ✅ **Green check**: All tests passed
- ❌ **Red X**: One or more tests failed → **Build fails**
- View summary in `build-status` job

---

## 📝 Writing New Tests

### Basic Test Structure

```cpp
#include <gtest/gtest.h>

// Simple test
TEST(TestSuiteName, TestName) {
   EXPECT_EQ(actual, expected);
   ASSERT_NE(pointer, nullptr);  // Fatal assertion
}
```

### Test with Fixture (Setup/Teardown)

```cpp
class MyFixture : public ::testing::Test {
protected:
   void SetUp() override {
      // Runs before each test
      data = std::make_unique<MyData>();
   }

   void TearDown() override {
      // Runs after each test
      data.reset();
   }

   std::unique_ptr<MyData> data;
};

TEST_F(MyFixture, TestName) {
   ASSERT_NE(data, nullptr);
   EXPECT_TRUE(data->isValid());
}
```

### Parameterized Test

```cpp
class ParamTest : public ::testing::TestWithParam<int> {};

TEST_P(ParamTest, CheckValue) {
   int value = GetParam();
   EXPECT_GT(value, 0);
}

INSTANTIATE_TEST_SUITE_P(
   Values,
   ParamTest,
   ::testing::Values(1, 5, 10, 42)
);
```

---

## 🧪 Common Assertions

### Equality

```cpp
EXPECT_EQ(a, b);        // a == b (non-fatal)
ASSERT_EQ(a, b);        // a == b (fatal, stops test)
EXPECT_NE(a, b);        // a != b
```

### Comparisons

```cpp
EXPECT_LT(a, b);        // a < b
EXPECT_LE(a, b);        // a <= b
EXPECT_GT(a, b);        // a > b
EXPECT_GE(a, b);        // a >= b
```

### Boolean

```cpp
EXPECT_TRUE(condition);
EXPECT_FALSE(condition);
```

### Strings

```cpp
EXPECT_STREQ(str1, str2);           // C strings
EXPECT_EQ(str1, str2);              // std::string
EXPECT_THAT(str, HasSubstr("sub")); // Contains substring
```

### Floating Point

```cpp
EXPECT_NEAR(val1, val2, abs_error);
EXPECT_DOUBLE_EQ(val1, val2);
EXPECT_FLOAT_EQ(val1, val2);
```

### Exceptions

```cpp
EXPECT_THROW(statement, exception_type);
EXPECT_NO_THROW(statement);
EXPECT_ANY_THROW(statement);
```

---

## 📦 Adding Tests to Build

### 1. Write Test File

Create `tests/unit/my_new_test.cpp`

### 2. Update `tests/Makefile.am`

```makefile
# Add to TESTS variable
TESTS = example_test my_new_test

check_PROGRAMS = $(TESTS)

# Define the test
my_new_test_SOURCES = unit/my_new_test.cpp
my_new_test_LDADD = \
   libgtest.la \
   libgtest_main.la
my_new_test_LDFLAGS = -pthread

# If testing MCTS code:
# my_new_test_LDADD += $(top_builddir)/source/ai/mcts/libmcts.la
```

### 3. Rebuild

```bash
cd tests/
make
make check
```

---

## 🎯 Current Test Status

### Example Test Suite

**File**: `tests/unit/example_test.cpp`
**Test Suites**: 8
**Test Cases**: 13
**Status**: ✅ All passing

**Test Coverage**:
- ✅ Basic arithmetic
- ✅ Test fixtures
- ✅ String operations
- ✅ Container operations
- ✅ Boolean logic
- ✅ Modern C++23 features
- ✅ Floating point comparisons
- ✅ Exception handling
- ✅ Parameterized tests

---

## 🔍 Debugging Failed Tests

### Run in Verbose Mode

```bash
./example_test --gtest_color=yes
```

### Run Specific Test

```bash
# Isolate failing test
./example_test --gtest_filter=MyTest.FailingCase
```

### Check Test Log

```bash
# View test output
cat test-suite.log

# View specific test log
cat example_test.log
```

### Local vs. CI Differences

**Local**: May have different environment variables, installed libraries
**CI**: Clean Ubuntu 24.04, specific library versions

**Tip**: CI failures often reveal environment assumptions

---

## 📊 CI/CD Pipeline Details

### Job Name

`google-test-suite`

### Build Steps

```yaml
1. Checkout code (with submodules: recursive)
2. Install dependencies (apt-get)
3. Bootstrap (./bootstrap)
4. Configure (../configure)
5. Build tests (make -j2)
6. Run tests (make check)
7. Upload results (artifacts)
```

### Environment

- **OS**: Ubuntu 24.04
- **Compiler**: GCC with C++23 support
- **Build**: Out-of-tree (`build-gtest/` directory)
- **Parallel Jobs**: 2 (`make -j2`)

### Timeouts

- **Build**: ~2-3 minutes
- **Test Execution**: <1 second (13 tests)
- **Total Job**: ~3-5 minutes

---

## 📚 Documentation Links

### Project Documentation

- **Migration Plan**: [TEST_FRAMEWORK_MIGRATION.md](./TEST_FRAMEWORK_MIGRATION.md)
- **Getting Started**: [tests/README.md](../../tests/README.md)
- **Status**: [STATUS.md](./STATUS.md)

### Google Test Resources

- [Google Test Primer](https://google.github.io/googletest/primer.html)
- [Advanced Guide](https://google.github.io/googletest/advanced.html)
- [Google Mock](https://google.github.io/googletest/gmock_for_dummies.html)

### MCTS Reference Implementation

- **Location**: `source/ai/mcts/`
- **Tests**: `snapshot_test.cpp`, `action_executor_test.cpp`, `evaluator_test.cpp`
- **Style**: Modern C++23, clean architecture

---

## 🚦 Test Best Practices

### 1. Test Naming

```cpp
// Good: Descriptive, action-oriented
TEST(VehicleTest, MovementReducesFuelByDistance)

// Bad: Vague, numbered
TEST(VehicleTest, Test1)
```

### 2. One Assertion Per Test (When Possible)

```cpp
// Good: Focused
TEST(MathTest, AdditionWorks) {
   EXPECT_EQ(2 + 2, 4);
}

// Less ideal: Multiple concerns
TEST(MathTest, AllMathWorks) {
   EXPECT_EQ(2 + 2, 4);
   EXPECT_EQ(5 - 3, 2);
   EXPECT_EQ(3 * 3, 9);
}
```

### 3. Use EXPECT Over ASSERT

```cpp
// Preferred: Test continues after failure
EXPECT_EQ(value, 42);

// Use only when test can't continue
ASSERT_NE(pointer, nullptr);
EXPECT_EQ(pointer->getValue(), 42);
```

### 4. Test Edge Cases

```cpp
TEST(StringTest, EmptyString) {
   std::string empty;
   EXPECT_TRUE(empty.empty());
   EXPECT_EQ(empty.length(), 0);
}

TEST(PointerTest, NullPointer) {
   Widget* ptr = nullptr;
   EXPECT_EQ(ptr, nullptr);
}
```

---

## 🔄 Migration Status

### Phase 1: Infrastructure ✅ Complete

- [x] Google Test installed
- [x] Test directory created
- [x] Example tests written
- [x] CI/CD integration
- [x] Documentation complete

### Phase 2: MCTS Migration 📋 Pending

- [ ] Migrate `snapshot_test.cpp`
- [ ] Migrate `action_executor_test.cpp`
- [ ] Migrate `evaluator_test.cpp`

### Phase 3: Coverage 📋 Future

- [ ] Set up lcov/gcov
- [ ] Generate coverage reports
- [ ] Integrate with CI/CD
- [ ] Set coverage targets (80%+)

---

## ❓ FAQ

**Q: Do tests run on every commit?**
A: Yes, automatically via GitHub Actions.

**Q: Can I run tests locally before pushing?**
A: Yes! `cd tests && make check`

**Q: What happens if a test fails in CI?**
A: The build fails, and you'll see a red X on your commit/PR.

**Q: How do I add a new test?**
A: Create test file, update `tests/Makefile.am`, run `make check`.

**Q: Can I run just one test?**
A: Yes: `./example_test --gtest_filter=MyTest.SpecificCase`

**Q: Where are test logs?**
A: Local: `tests/*.log`, CI: Download from Artifacts section

**Q: Why 13 tests?**
A: Example test suite demonstrates various Google Test features.

**Q: What's next?**
A: Phase 2: Migrate MCTS tests to Google Test framework.

---

## 🎉 Summary

- ✅ **Google Test active** and running in CI/CD
- ✅ **13 tests passing** in example suite
- ✅ **Automated testing** on every commit
- ✅ **Clean build environment** (out-of-tree)
- ✅ **Test results visible** in GitHub Actions
- 📋 **Next**: Migrate MCTS tests (Phase 2)

**Quick command**: `cd tests && make check` 🚀

---

**Created**: 2025-11-17
**Last Updated**: 2025-11-17
**Maintainer**: ASC Modernization Team
