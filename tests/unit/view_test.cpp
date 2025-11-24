/***************************************************************************
 * view_test.cpp - Google Test suite for visibility and view mechanics
 *
 * Migrated from: source/unittests/viewtest.cpp
 * Purpose: Test map visibility, radar, jamming, and view changes
 *
 * Part of: ASC Test Framework Migration (Phase 2)
 *
 * Original test verified that:
 * - Visibility changes when units move
 * - Attacking units affects visibility
 * - Undo restores visibility correctly
 * - Radar units provide extended visibility
 * - Destroying jammers changes visibility
 * - Visibility works correctly for different players
 ***************************************************************************/

#include <gtest/gtest.h>
#include <memory>
#include "../../source/gamemap.h"
#include "../../source/actions/moveunitcommand.h"
#include "../../source/actions/attackcommand.h"
#include "../../source/loaders.h"
#include "../../source/viewcalculation.h"
#include "../../source/spfst.h"
#include "../../source/unittests/unittestutil.h"

// ========== Test Fixture ==========

class ViewTest : public ::testing::Test {
  protected:
   std::unique_ptr<GameMap> game;

   void SetUp() override {
      // Game map will be loaded in individual tests as needed
   }

   void TearDown() override {
      game.reset();
   }
};

// ========== View Tests ==========

TEST_F(ViewTest, RadarAndVisibility) {
   // Test radar units providing extended visibility
   game.reset(startMap("unittest-view1.map"));
   ASSERT_NE(game, nullptr) << "Failed to load unittest-view1.map";

   Vehicle* radar = game->getField(0, 4)->vehicle;
   ASSERT_NE(radar, nullptr) << "Radar unit should exist at (0,4)";

   // Initially field (3,5) should not be visible
   EXPECT_FALSE(fieldvisiblenow(game->getField(3, 5)))
      << "Field (3,5) should not be visible initially";

   // Move radar unit
   move(radar, MapCoordinate(1, 4));

   // Field (3,5) should now be visible due to radar
   EXPECT_TRUE(fieldvisiblenow(game->getField(3, 5)))
      << "Field (3,5) should be visible after radar moves";

   EXPECT_FALSE(fieldvisiblenow(game->getField(4, 3)))
      << "Field (4,3) should not be visible yet";

   // Attack and destroy enemy unit
   attack(game->getField(1, 6)->vehicle, MapCoordinate(3, 5));

   EXPECT_EQ(game->getField(3, 5)->vehicle, nullptr)
      << "Target vehicle should be destroyed";
   EXPECT_TRUE(fieldvisiblenow(game->getField(4, 3)))
      << "Field (4,3) should now be visible";

   // Test undo attack
   ActionResult res = game->actions.undo(createTestingContext(game.get()));
   ASSERT_TRUE(res.successful()) << "Undo attack should succeed";

   EXPECT_FALSE(fieldvisiblenow(game->getField(4, 3)))
      << "Field (4,3) visibility should be restored";
   EXPECT_TRUE(fieldvisiblenow(game->getField(3, 5)))
      << "Field (3,5) should still be visible";

   // Test undo move
   res = game->actions.undo(createTestingContext(game.get()));
   ASSERT_TRUE(res.successful()) << "Undo move should succeed";

   EXPECT_FALSE(fieldvisiblenow(game->getField(3, 5)))
      << "Field (3,5) should not be visible after undoing radar move";

   // Test BLUE player visibility (player 1)
   EXPECT_TRUE(fieldvisiblenow(game->getField(0, 15), 1))
      << "Field (0,15) should be visible to player 1";

   attack(game->getField(2, 16)->vehicle, MapCoordinate(0, 13));

   EXPECT_FALSE(fieldvisiblenow(game->getField(0, 15), 1))
      << "Field (0,15) should not be visible to player 1 after attack";

   res = game->actions.undo(createTestingContext(game.get()));
   ASSERT_TRUE(res.successful()) << "Undo should succeed";

   EXPECT_TRUE(fieldvisiblenow(game->getField(0, 15), 1))
      << "Field (0,15) visibility should be restored for player 1";
}

TEST_F(ViewTest, SpyplaneMovement) {
   // Test that spyplane can move long distances without taking damage
   game.reset(startMap("unittest-view2.map"));
   ASSERT_NE(game, nullptr) << "Failed to load unittest-view2.map";

   Vehicle* spyplane = game->getField(5, 2)->vehicle;
   ASSERT_NE(spyplane, nullptr) << "Spyplane should exist at (5,2)";
   ASSERT_EQ(spyplane->damage, 0) << "Spyplane should start with no damage";

   EXPECT_FALSE(fieldvisiblenow(game->getField(5, 2), 1))
      << "Field (5,2) should not be visible to player 1";

   // Move spyplane long distance
   move(spyplane, MapCoordinate(5, 18));

   spyplane = game->getField(5, 18)->vehicle;
   ASSERT_NE(spyplane, nullptr) << "Spyplane should be at new position";
   EXPECT_EQ(spyplane->damage, 0) << "Spyplane should not take damage from move";
}

TEST_F(ViewTest, JammerDestruction) {
   // Test that destroying jammer changes visibility
   game.reset(startMap("unittest-jammingdestroyview.map"));
   ASSERT_NE(game, nullptr) << "Failed to load unittest-jammingdestroyview.map";

   Vehicle* ari = game->getField(5, 2)->vehicle;
   ASSERT_NE(ari, nullptr) << "ARI unit should exist at (5,2)";

   MapField* field = game->getField(0, 12);
   MapField* field2 = game->getField(3, 10);

   EXPECT_EQ(fieldVisibility(field), visible_not)
      << "Field (0,12) should not be visible initially";
   EXPECT_EQ(fieldVisibility(field2), visible_not)
      << "Field (3,10) should not be visible initially";

   // Attack jammer at (3,8)
   attack(ari, MapCoordinate(3, 8));

   EXPECT_EQ(fieldVisibility(field), visible_not)
      << "Field (0,12) should still not be visible";
   EXPECT_EQ(fieldVisibility(field2), visible_now)
      << "Field (3,10) should now be visible after jammer destroyed";

   // Test undo
   ActionResult res = game->actions.undo(createTestingContext(game.get()));
   ASSERT_TRUE(res.successful()) << "Undo should succeed";

   EXPECT_EQ(fieldVisibility(field), visible_not)
      << "Field (0,12) visibility should be restored";
   EXPECT_EQ(fieldVisibility(field2), visible_not)
      << "Field (3,10) visibility should be restored";
}
