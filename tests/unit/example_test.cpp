/**
 * Example Google Test - Validates Google Test Framework Setup
 *
 * This is a simple smoke test to verify that:
 * 1. Google Test is properly integrated with the build system
 * 2. Tests can be compiled and executed
 * 3. The testing infrastructure is working
 *
 * Created: 2025-11-17
 * Part of ASC modernization effort (Tier 1, Task 4)
 */

#include <gtest/gtest.h>
#include <string>
#include <vector>

// Namespace to avoid polluting global namespace
namespace asc {
namespace test {

/**
 * Test fixture for basic functionality tests
 */
class ExampleTest : public ::testing::Test {
protected:
   void SetUp() override {
      // Setup code that runs before each test
      test_value = 42;
   }

   void TearDown() override {
      // Cleanup code that runs after each test
   }

   int test_value;
};

/**
 * Basic arithmetic test
 */
TEST_F(ExampleTest, BasicArithmetic) {
   EXPECT_EQ(2 + 2, 4);
   EXPECT_NE(2 + 2, 5);
   EXPECT_LT(3, 5);
   EXPECT_GT(10, 5);
}

/**
 * Test fixture value
 */
TEST_F(ExampleTest, FixtureValue) {
   EXPECT_EQ(test_value, 42);
   test_value = 100;
   EXPECT_EQ(test_value, 100);
}

/**
 * String operations test
 */
TEST(StringTest, BasicStringOps) {
   std::string hello = "Hello";
   std::string world = "World";
   std::string combined = hello + " " + world;

   EXPECT_EQ(combined, "Hello World");
   EXPECT_EQ(hello.length(), 5);
   EXPECT_TRUE(combined.find("World") != std::string::npos);
}

/**
 * Container test
 */
TEST(ContainerTest, VectorOperations) {
   std::vector<int> numbers = {1, 2, 3, 4, 5};

   EXPECT_EQ(numbers.size(), 5);
   EXPECT_EQ(numbers[0], 1);
   EXPECT_EQ(numbers[4], 5);

   numbers.push_back(6);
   EXPECT_EQ(numbers.size(), 6);
   EXPECT_EQ(numbers.back(), 6);
}

/**
 * Boolean logic test
 */
TEST(BooleanTest, LogicOperations) {
   EXPECT_TRUE(true);
   EXPECT_FALSE(false);
   EXPECT_TRUE(1 == 1);
   EXPECT_FALSE(1 == 2);
}

/**
 * Modern C++23 test - demonstrates that C++23 features work
 */
TEST(ModernCppTest, Cpp23Features) {
   // Auto type deduction
   auto value = 42;
   EXPECT_EQ(value, 42);

   // Range-based for loop
   std::vector<int> numbers = {1, 2, 3};
   int sum = 0;
   for (const auto& num : numbers) {
      sum += num;
   }
   EXPECT_EQ(sum, 6);

   // Structured bindings (C++17)
   std::pair<int, std::string> pair = {42, "answer"};
   auto [number, text] = pair;
   EXPECT_EQ(number, 42);
   EXPECT_EQ(text, "answer");
}

/**
 * Floating point comparison test
 */
TEST(FloatTest, FloatingPointComparisons) {
   double a = 0.1 + 0.2;
   double b = 0.3;

   // Use EXPECT_NEAR for floating point comparisons
   EXPECT_NEAR(a, b, 0.0001);

   // Or use EXPECT_DOUBLE_EQ for reasonable epsilon
   EXPECT_DOUBLE_EQ(1.0 / 3.0 * 3.0, 1.0);
}

/**
 * Exception handling test
 */
TEST(ExceptionTest, ThrowAndCatch) {
   // Test that an exception is thrown
   EXPECT_THROW({
      throw std::runtime_error("Expected error");
   }, std::runtime_error);

   // Test that no exception is thrown
   EXPECT_NO_THROW({
      int x = 42;
      (void)x;  // Suppress unused variable warning
   });
}

/**
 * Parameterized test example (demonstrates Google Test features)
 */
class ParameterizedTest : public ::testing::TestWithParam<int> {
};

TEST_P(ParameterizedTest, IsPositive) {
   int value = GetParam();
   EXPECT_GT(value, 0);
}

INSTANTIATE_TEST_SUITE_P(
   PositiveNumbers,
   ParameterizedTest,
   ::testing::Values(1, 5, 10, 42, 100)
);

} // namespace test
} // namespace asc

/**
 * Main function - provided by libgtest_main.la
 * No need to write main() when using gtest_main
 */
