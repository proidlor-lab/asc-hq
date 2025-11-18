/***************************************************************************
 * version_identifier_test.cpp - Google Test suite for VersionIdentifier
 *
 * Migrated from: source/unittests/testversionidentifier.cpp
 * Purpose: Test version string parsing and comparison
 *
 * Part of: ASC Test Framework Migration (Phase 1)
 ***************************************************************************/

#include <gtest/gtest.h>
#include "versionidentifier.h"

// ========== Test Fixture ==========

class VersionIdentifierTest : public ::testing::Test {
  protected:
   void SetUp() override {
      // No setup needed for this pure utility class
   }
};

// ========== Equality Tests ==========

TEST_F(VersionIdentifierTest, EqualityWithPatch) {
   EXPECT_EQ(VersionIdentifier("1.2.0"), VersionIdentifier("1.2.0"));
}

TEST_F(VersionIdentifierTest, EqualityWithoutPatch) {
   EXPECT_EQ(VersionIdentifier("1.2.0"), VersionIdentifier("1.2"));
}

TEST_F(VersionIdentifierTest, InequalityWithDifferentPatch) {
   EXPECT_NE(VersionIdentifier("1.2.0"), VersionIdentifier("1.2.1"));
}

TEST_F(VersionIdentifierTest, InequalityWithExtraComponent) {
   EXPECT_NE(VersionIdentifier("1.2.0"), VersionIdentifier("1.2.0.1"));
}

TEST_F(VersionIdentifierTest, InequalityWithDifferentMajor) {
   EXPECT_NE(VersionIdentifier("1.2.0"), VersionIdentifier("11.2.0"));
}

// ========== Comparison Tests (<=, >=) ==========

TEST_F(VersionIdentifierTest, LessOrEqualSameVersion) {
   EXPECT_LE(VersionIdentifier("1.2.0"), VersionIdentifier("1.2.0"));
}

TEST_F(VersionIdentifierTest, LessOrEqualWithoutPatch) {
   EXPECT_LE(VersionIdentifier("1.2.0"), VersionIdentifier("1.2"));
}

TEST_F(VersionIdentifierTest, GreaterOrEqualWithoutPatch) {
   EXPECT_GE(VersionIdentifier("1.2.0"), VersionIdentifier("1.2"));
}

TEST_F(VersionIdentifierTest, GreaterOrEqualSameVersion) {
   EXPECT_GE(VersionIdentifier("1.2.0"), VersionIdentifier("1.2.0"));
}

TEST_F(VersionIdentifierTest, GreaterOrEqualWithHigherMinor) {
   EXPECT_GE(VersionIdentifier("1.3"), VersionIdentifier("1.2.0.1"));
}

TEST_F(VersionIdentifierTest, GreaterOrEqualWithManyComponents) {
   EXPECT_GE(VersionIdentifier("1.3.0.0.2.0.1.0"), VersionIdentifier("1.2.0.1"));
}

// ========== Less Than Tests ==========

TEST_F(VersionIdentifierTest, LessThanWithHigherMinor) {
   EXPECT_LT(VersionIdentifier("1.2.0"), VersionIdentifier("1.3"));
}

TEST_F(VersionIdentifierTest, LessThanWithHigherPatch) {
   EXPECT_LT(VersionIdentifier("1.2.0"), VersionIdentifier("1.2.1"));
}

TEST_F(VersionIdentifierTest, LessThanWithExtraComponent) {
   EXPECT_LT(VersionIdentifier("1.2.0"), VersionIdentifier("1.2.0.1"));
}

// ========== Not Less Than Tests ==========

TEST_F(VersionIdentifierTest, NotLessThanWithHigherMinor) {
   EXPECT_FALSE(VersionIdentifier("1.3") < VersionIdentifier("1.2.0"));
}

TEST_F(VersionIdentifierTest, NotLessThanWithHigherPatch) {
   EXPECT_FALSE(VersionIdentifier("1.2.1") < VersionIdentifier("1.2.0"));
}

TEST_F(VersionIdentifierTest, NotLessThanWithMoreComponents) {
   EXPECT_FALSE(VersionIdentifier("1.2.0.1") < VersionIdentifier("1.2.0"));
}

// ========== Main ==========

int main(int argc, char** argv) {
   ::testing::InitGoogleTest(&argc, argv);
   return RUN_ALL_TESTS();
}
