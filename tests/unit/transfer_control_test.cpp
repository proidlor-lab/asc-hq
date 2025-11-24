/***************************************************************************
 * transfer_control_test.cpp - Google Test suite for unit transfer mechanics
 *
 * Migrated from: source/unittests/transfercontroltest.cpp
 * Purpose: Test transferring control of units and buildings between players
 *
 * Part of: ASC Test Framework Migration (Phase 2)
 *
 * Original test verified that:
 * - Units can be transferred between players
 * - Visibility changes when units change ownership
 * - Undo functionality works for transfer commands
 * - Buildings can be transferred between players
 * - Cargo units transfer with their parent building
 ***************************************************************************/

#include <gtest/gtest.h>
#include <memory>
#include "../../source/gamemap.h"
#include "../../source/actions/transfercontrolcommand.h"
#include "../../source/loaders.h"
#include "../../source/unittests/unittestutil.h"
#include "../../source/spfst.h"

// ========== Test Fixture ==========

class TransferControlTest : public ::testing::Test {
  protected:
   std::unique_ptr<GameMap> game;

   void SetUp() override {
      // Game map will be loaded in individual tests as needed
   }

   void TearDown() override {
      game.reset();
   }
};

// ========== Transfer Control Tests ==========

TEST_F(TransferControlTest, TransferVehicleAndBuilding) {
   // Test transferring vehicles and buildings between players
   game.reset(startMap("unittest-transfercontrol.map"));
   ASSERT_NE(game, nullptr) << "Failed to load unittest-transfercontrol.map";

   // Test vehicle transfer
   Vehicle* veh = game->getField(4, 8)->vehicle;
   ASSERT_NE(veh, nullptr) << "Vehicle should exist at (4,8)";
   ASSERT_EQ(veh->getOwner(), 0) << "Vehicle should start owned by player 0";

   MapField* fld = game->getField(4, 0);
   EXPECT_TRUE(fieldvisiblenow(fld, 0)) << "Field should be visible to player 0";
   EXPECT_FALSE(fieldvisiblenow(fld, 3)) << "Field should not be visible to player 3";

   // Transfer vehicle to player 3
   std::unique_ptr<TransferControlCommand> tcc(new TransferControlCommand(veh));
   tcc->setReceiver(&game->player[3]);
   ActionResult res = tcc->execute(createTestingContext(game.get()));
   ASSERT_TRUE(res.successful()) << "Transfer command should succeed: " << res.getMessage();
   tcc.release();

   // Verify ownership and visibility changed
   EXPECT_EQ(veh->getOwner(), 3) << "Vehicle should now be owned by player 3";
   EXPECT_FALSE(fieldvisiblenow(fld, 0)) << "Field should no longer be visible to player 0";
   EXPECT_TRUE(fieldvisiblenow(fld, 3)) << "Field should now be visible to player 3";

   // Test undo
   res = game->actions.undo(createTestingContext(game.get()));
   ASSERT_TRUE(res.successful()) << "Undo should succeed";

   EXPECT_EQ(veh->getOwner(), 0) << "Vehicle ownership should be restored to player 0";
   EXPECT_EQ(fieldVisibility(fld, 0), visible_now) << "Field should be visible to player 0 again";
   EXPECT_EQ(fieldVisibility(fld, 3), visible_not) << "Field should not be visible to player 3 again";

   // Test building transfer
   Building* bld = game->getField(14, 7)->building;
   ASSERT_NE(bld, nullptr) << "Building should exist at (14,7)";
   ASSERT_EQ(bld->getOwner(), 0) << "Building should start owned by player 0";

   std::unique_ptr<TransferControlCommand> tcc2(new TransferControlCommand(bld));
   tcc2->setReceiver(&game->player[1]);
   ActionResult res2 = tcc2->execute(createTestingContext(game.get()));
   ASSERT_TRUE(res2.successful()) << "Building transfer should succeed: " << res2.getMessage();
   tcc2.release();

   // Verify building and cargo ownership changed
   EXPECT_EQ(bld->getOwner(), 1) << "Building should now be owned by player 1";
   EXPECT_NE(getFirstCargo(bld), nullptr) << "Building should have cargo";
   EXPECT_EQ(getFirstCargo(bld)->getOwner(), 1) << "Cargo should also be owned by player 1";
}
