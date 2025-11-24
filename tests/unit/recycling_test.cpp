/***************************************************************************
 * recycling_test.cpp - Google Test suite for unit recycling
 *
 * Migrated from: source/unittests/recyclingtest.cpp
 * Purpose: Test vehicle/unit recycling mechanics
 *
 * Part of: ASC Test Framework Migration (Phase 2)
 *
 * Original test verified that:
 * - Units can be recycled from buildings
 * - Recycling command executes successfully
 ***************************************************************************/

#include <gtest/gtest.h>
#include <memory>
#include "../../source/gamemap.h"
#include "../../source/actions/recycleunitcommand.h"
#include "../../source/loaders.h"
#include "../../source/unittests/unittestutil.h"
#include "../../source/spfst.h"

// ========== Test Fixture ==========

class RecyclingTest : public ::testing::Test {
  protected:
   std::unique_ptr<GameMap> game;

   void SetUp() override {
      // Game map will be loaded in individual tests as needed
   }

   void TearDown() override {
      // GameMap destructor handles cleanup
      game.reset();
   }
};

// ========== Recycling Tests ==========

TEST_F(RecyclingTest, RecycleUnit) {
   game.reset(startMap("unittest-recycle.map"));
   ASSERT_NE(game, nullptr) << "Failed to load unittest-recycle.map";

   Building* bld = game->getField(5, 8)->building;
   ASSERT_NE(bld, nullptr) << "No building found at (5,8)";

   Vehicle* veh = getFirstCargo(bld);
   ASSERT_NE(veh, nullptr) << "Building should contain a vehicle for recycling";

   // Create and execute recycle command
   RecycleUnitCommand* ruc = new RecycleUnitCommand(bld);
   ruc->setUnit(veh);
   ActionResult res = ruc->execute(createTestingContext(game.get()));

   EXPECT_TRUE(res.successful()) << "Recycle command should succeed: " << res.getMessage();
}
