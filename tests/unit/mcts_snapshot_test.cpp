/***************************************************************************
 * mcts_snapshot_test.cpp - Google Test suite for MCTS snapshot system
 *
 * Migrated from: source/ai/mcts/domain/snapshot_test.cpp
 * Purpose: Validate snapshot creation, copying, and performance
 *
 * Part of: ASC MCTS AI - Test Framework Migration (Phase 2)
 ***************************************************************************/

#include <gtest/gtest.h>
#include "game_state_reader.h"
#include "game_state_snapshot.h"
#include "unit_snapshot.h"
#include <chrono>

using namespace asc::mcts;
using namespace std::chrono;

// ========== Test Utilities ==========

class Timer {
   time_point<high_resolution_clock> start;

  public:
   Timer() : start(high_resolution_clock::now()) {}

   double elapsedMs() const {
      auto end = high_resolution_clock::now();
      return duration_cast<microseconds>(end - start).count() / 1000.0;
   }
};

// ========== Test Fixture ==========

class SnapshotTest : public ::testing::Test {
  protected:
   void SetUp() override {
      // Setup code if needed
   }

   void TearDown() override {
      // Cleanup code if needed
   }
};

// ========== Tests ==========

/**
 * Test 1: UnitSnapshot size
 *
 * Verify that UnitSnapshot is compact (<= 48 bytes)
 */
TEST_F(SnapshotTest, UnitSnapshotSize) {
   size_t size = sizeof(UnitSnapshot);

   // Target: <= 48 bytes (accounting for compiler alignment/padding)
   EXPECT_LE(size, 48) << "UnitSnapshot size: " << size << " bytes";

   if (size <= 32) {
      std::cout << "  ✓ Excellent: Within ideal 32-byte target!\n";
   } else if (size <= 40) {
      std::cout << "  ✓ Good: " << size << " bytes (alignment padding added)\n";
   } else {
      std::cout << "  ⚠ Acceptable: " << size << " bytes (check for unnecessary fields)\n";
   }
}

/**
 * Test 2: Snapshot creation (stub)
 *
 * Note: This test needs a real GameMap instance.
 * For MVP, this will be tested in integration tests with ASC.
 */
TEST_F(SnapshotTest, SnapshotCreationStub) {
   // Stub test - requires GameMap instance for full test
   // Will be validated in integration tests
   SUCCEED() << "Requires GameMap instance for full test - will be validated in integration tests";
}

/**
 * Test 3: Snapshot cloning performance
 *
 * Verify that cloning is fast (<1ms)
 */
TEST_F(SnapshotTest, SnapshotCloningPerformance) {
   // Create synthetic snapshot with 20 units
   GameStateSnapshot snapshot;
   snapshot.mapWidth = 128;
   snapshot.mapHeight = 128;
   snapshot.currentPlayer = 0;

   for (int i = 0; i < 20; ++i) {
      UnitSnapshot unit;
      unit.networkID = i;
      unit.x = i * 5;
      unit.y = i * 5;
      unit.damage = 0;
      unit.owner = 0;
      snapshot.units.push_back(unit);
   }

   // Add some terrain
   for (int i = 0; i < 100; ++i) {
      MapCoordinate pos(i % 10, i / 10);
      FieldSnapshot field;
      field.visibilityMask = 0xFF;
      snapshot.terrain[pos] = field;
   }

   // Measure clone time
   Timer timer;
   auto clone = snapshot.clone();
   auto* cloneSnapshot = dynamic_cast<GameStateSnapshot*>(clone.get());
   double elapsed = timer.elapsedMs();

   ASSERT_NE(cloneSnapshot, nullptr) << "Clone must produce GameStateSnapshot";
   EXPECT_LT(elapsed, 1.0) << "Clone time: " << elapsed << " ms";
   EXPECT_EQ(cloneSnapshot->units.size(), snapshot.units.size())
      << "Original units: " << snapshot.units.size()
      << ", Cloned units: " << cloneSnapshot->units.size();

   std::cout << "Clone time: " << elapsed << " ms\n";
   std::cout << "Memory size: " << cloneSnapshot->getMemorySize() << " bytes\n";
}

/**
 * Test 4: Snapshot memory usage
 *
 * Verify that snapshot is compact (<20 KB for 20 units)
 */
TEST_F(SnapshotTest, SnapshotMemoryUsage) {
   GameStateSnapshot snapshot;
   snapshot.mapWidth = 128;
   snapshot.mapHeight = 128;

   // Add 20 units
   for (int i = 0; i < 20; ++i) {
      UnitSnapshot unit;
      unit.networkID = i;
      unit.x = i * 5;
      unit.y = i * 5;
      snapshot.units.push_back(unit);
   }

   // Add tactical terrain (10×10 area)
   for (int y = 0; y < 10; ++y) {
      for (int x = 0; x < 10; ++x) {
         MapCoordinate pos(x, y);
         FieldSnapshot field;
         field.visibilityMask = 0xFF;
         snapshot.terrain[pos] = field;
      }
   }

   size_t size = snapshot.getMemorySize();
   auto stats = snapshot.getStats();

   EXPECT_LT(size, 20 * 1024)
      << "Snapshot memory: " << size << " bytes (" << (size / 1024.0) << " KB)\n"
      << "  Units: " << stats.unitCount << "\n"
      << "  Terrain fields: " << stats.terrainFieldCount;
}

/**
 * Test 5: Query methods
 */
TEST_F(SnapshotTest, QueryMethods) {
   GameStateSnapshot snapshot;
   snapshot.mapWidth = 100;
   snapshot.mapHeight = 100;

   // Add test units
   UnitSnapshot unit1;
   unit1.networkID = 1;
   unit1.x = 10;
   unit1.y = 10;
   unit1.owner = 0;
   snapshot.units.push_back(unit1);

   UnitSnapshot unit2;
   unit2.networkID = 2;
   unit2.x = 20;
   unit2.y = 20;
   unit2.owner = 1;
   snapshot.units.push_back(unit2);

   // Test findUnit
   const UnitSnapshot* found = snapshot.findUnit(1);
   ASSERT_NE(found, nullptr) << "Should find unit 1";
   EXPECT_EQ(found->networkID, 1) << "Unit ID mismatch";

   // Test getUnitAt
   const UnitSnapshot* atPos = snapshot.getUnitAt(MapCoordinate(10, 10));
   ASSERT_NE(atPos, nullptr) << "Should find unit at (10,10)";
   EXPECT_EQ(atPos->networkID, 1) << "Unit at position mismatch";

   // Test getPlayerUnits
   auto player0Units = snapshot.getPlayerUnits(0);
   EXPECT_EQ(player0Units.size(), 1) << "Player 0 should have 1 unit";

   auto player1Units = snapshot.getPlayerUnits(1);
   EXPECT_EQ(player1Units.size(), 1) << "Player 1 should have 1 unit";
}

/**
 * Test 6: Stress test - many clones
 */
TEST_F(SnapshotTest, StressTestManyClones) {
   GameStateSnapshot snapshot;
   snapshot.mapWidth = 128;
   snapshot.mapHeight = 128;

   for (int i = 0; i < 20; ++i) {
      UnitSnapshot unit;
      unit.networkID = i;
      unit.x = i * 5;
      unit.y = i * 5;
      snapshot.units.push_back(unit);
   }

   Timer timer;
   for (int i = 0; i < 100; ++i) {
      auto clone = snapshot.clone();
      // Simulate some work
      clone->findUnit(i % 20);
   }
   double elapsed = timer.elapsedMs();

   EXPECT_LT(elapsed, 100.0)
      << "100 clones in " << elapsed << " ms\n"
      << "Average: " << (elapsed / 100.0) << " ms per clone";
}

// ========== Main ==========

int main(int argc, char** argv) {
   ::testing::InitGoogleTest(&argc, argv);
   return RUN_ALL_TESTS();
}
