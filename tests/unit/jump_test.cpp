/***************************************************************************
 * jump_test.cpp - Google Test suite for jump drive mechanics
 *
 * Migrated from: source/unittests/jumptest.cpp
 * Purpose: Test jump drive command and teleportation mechanics
 *
 * Part of: ASC Test Framework Migration (Phase 2)
 *
 * Original test verified that:
 * - Jump drive command can be executed
 * - Units take damage when using jump drive
 * - Units are marked as attacked after jumping
 ***************************************************************************/

#include <gtest/gtest.h>
#include <memory>
#include "../../source/gamemap.h"
#include "../../source/actions/jumpdrivecommand.h"
#include "../../source/loaders.h"
#include "../../source/itemrepository.h"
#include "../../source/unittests/unittestutil.h"

// ========== Test Fixture ==========

class JumpTest : public ::testing::Test {
  protected:
   std::unique_ptr<GameMap> game;

   void SetUp() override {
      // Game map will be loaded in individual tests as needed
   }

   void TearDown() override {
      game.reset();
   }
};

// ========== Jump Drive Tests ==========

TEST_F(JumpTest, JumpDrive) {
   // Test jump drive mechanics
   game.reset(startMap("testjump.map"));
   ASSERT_NE(game, nullptr) << "Failed to load testjump.map";

   Vehicle* veh = game->getField(5, 14)->vehicle;
   ASSERT_NE(veh, nullptr) << "Vehicle should exist at (5,14)";
   ASSERT_EQ(veh->damage, 0) << "Vehicle should start with no damage";

   // Execute jump drive command
   JumpDriveCommand* jdc = new JumpDriveCommand(veh);
   jdc->setDestination(MapCoordinate(5, 4));
   ActionResult res = jdc->execute(createTestingContext(game.get()));

   // Verify jump effects
   EXPECT_TRUE(veh->attacked) << "Vehicle should be marked as attacked after jump";
   EXPECT_GT(veh->damage, 0) << "Vehicle should take damage from jumping";
}
