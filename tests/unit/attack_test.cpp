/***************************************************************************
 * attack_test.cpp - Google Test suite for attack command system
 *
 * Migrated from: source/unittests/attacktest.cpp
 * Purpose: Test attack mechanics, experience gain, and undo functionality
 *
 * Part of: ASC Test Framework Migration (Phase 2)
 *
 * Original test verified that:
 * - Attack commands execute correctly
 * - Movement and experience are updated properly after attacks
 * - Undo functionality works for attacks
 * - Attacks can destroy units and objects
 * - Visibility changes after object destruction
 ***************************************************************************/

#include <gtest/gtest.h>
#include <memory>
#include "../../source/gamemap.h"
#include "../../source/actions/attackcommand.h"
#include "../../source/loaders.h"
#include "../../source/unittests/unittestutil.h"
#include "../../source/itemrepository.h"
#include "../../source/spfst.h"

// ========== Test Fixture ==========

class AttackTest : public ::testing::Test {
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

// ========== Basic Attack Tests ==========

TEST_F(AttackTest, BasicAttack) {
   game.reset(startMap("unittest-attack.map"));
   ASSERT_NE(game, nullptr) << "Failed to load unittest-attack.map";

   Vehicle* veh = game->getField(3, 6)->vehicle;
   ASSERT_NE(veh, nullptr) << "No vehicle found at (3,6)";
   ASSERT_EQ(veh->getMovement(), 100) << "Vehicle should start with 100 movement";
   ASSERT_EQ(veh->getExperience_offensive(), 0) << "Vehicle should start with 0 offensive experience";
   ASSERT_EQ(veh->getExperience_defensive(), 0) << "Vehicle should start with 0 defensive experience";

   // Execute attack
   attack(veh, MapCoordinate(3, 5));

   EXPECT_EQ(veh->getMovement(), 0) << "Vehicle should have 0 movement after attack";
   EXPECT_EQ(veh->getExperience_offensive(), 2) << "Vehicle should gain 2 offensive experience";
   EXPECT_EQ(veh->getExperience_defensive(), 1) << "Vehicle should gain 1 defensive experience";
   testCargoMovement(veh, 50);

   // Test undo
   ActionResult res = game->actions.undo(createTestingContext(game.get()));
   ASSERT_TRUE(res.successful()) << "Undo should succeed: " << res.getMessage();

   EXPECT_EQ(veh->getMovement(), 100) << "Movement should be restored after undo";
   testCargoMovement(veh, 100);
}

TEST_F(AttackTest, MissileAttack) {
   game.reset(startMap("unittest-attack.map"));
   ASSERT_NE(game, nullptr);

   Vehicle* mam = game->getField(4, 4)->vehicle;
   ASSERT_NE(mam, nullptr);
   ASSERT_EQ(mam->getMovement(), 100);
   ASSERT_EQ(mam->getExperience_offensive(), 0);
   ASSERT_EQ(mam->getExperience_defensive(), 0);

   attack(mam, MapCoordinate(3, 5));

   EXPECT_EQ(mam->getExperience_offensive(), 1) << "Missile should gain 1 offensive experience";
   EXPECT_EQ(mam->getExperience_defensive(), 0) << "Missile should gain 0 defensive experience";
   EXPECT_EQ(mam->getMovement(), 100) << "Missile attack should not consume movement";
   testCargoMovement(mam, 100);
}

TEST_F(AttackTest, DestructiveAttack) {
   game.reset(startMap("unittest-attack.map"));
   ASSERT_NE(game, nullptr);

   Vehicle* v2 = game->getField(9, 14)->vehicle;
   ASSERT_NE(v2, nullptr) << "Target vehicle should exist at (9,14)";

   Vehicle* a2 = game->getField(9, 17)->vehicle;
   ASSERT_NE(a2, nullptr) << "Attacking vehicle should exist at (9,17)";

   // Attack and destroy target
   attack(a2, MapCoordinate(9, 14));

   EXPECT_EQ(game->getField(9, 14)->vehicle, nullptr) << "Target vehicle should be destroyed";

   // Test undo
   ActionResult res = game->actions.undo(createTestingContext(game.get()));
   ASSERT_TRUE(res.successful()) << "Undo should succeed";
   EXPECT_NE(game->getField(9, 14)->vehicle, nullptr) << "Target vehicle should be restored after undo";
}

// ========== Object Attack Tests ==========

TEST_F(AttackTest, ObjectAttack) {
   game.reset(startMap("unittest-objectattack.map"));
   ASSERT_NE(game, nullptr);

   Vehicle* buggy = game->getField(0, 3)->vehicle;
   ASSERT_NE(buggy, nullptr);

   Vehicle* assault = game->getField(1, 2)->vehicle;
   ASSERT_NE(assault, nullptr);

   MapCoordinate h(3, 8);
   MapCoordinate cry(3, 9);

   // Setup: move units into position and attack crystal
   move(buggy, h);
   attack(buggy, cry);
   move(buggy, MapCoordinate(0, 3));
   move(assault, h);

   ObjectType* cryst = objectTypeRepository.getObject_byID(2105);
   ASSERT_NE(cryst, nullptr) << "Crystal object type should exist";

   MapField* fld = game->getField(cry);
   ASSERT_NE(fld->checkForObject(cryst), nullptr) << "Crystal should exist before final attack";

   MapField* view = game->getField(MapCoordinate(4, 11));
   EXPECT_FALSE(fieldvisiblenow(view)) << "Field should not be visible initially";

   // Attack and destroy crystal
   attack(assault, cry);

   EXPECT_EQ(fld->checkForObject(cryst), nullptr) << "Crystal should be destroyed";
   EXPECT_TRUE(fieldvisiblenow(view)) << "Field should be visible after crystal destruction";

   // Test undo
   ActionResult res = game->actions.undo(createTestingContext(game.get()));
   ASSERT_TRUE(res.successful());

   EXPECT_NE(fld->checkForObject(cryst), nullptr) << "Crystal should be restored";
   EXPECT_FALSE(fieldvisiblenow(view)) << "Field visibility should be restored";

   // Undo remaining actions
   ASSERT_TRUE(game->actions.undo(createTestingContext(game.get())).successful());
   ASSERT_TRUE(game->actions.undo(createTestingContext(game.get())).successful());
   ASSERT_TRUE(game->actions.undo(createTestingContext(game.get())).successful());
   ASSERT_TRUE(game->actions.undo(createTestingContext(game.get())).successful());
}

// ========== Visibility Tests ==========

TEST_F(AttackTest, AttackVisibilityChange) {
   game.reset(startMap("unittest-attack-view.map"));
   ASSERT_NE(game, nullptr);

   Vehicle* assault = game->getField(4, 9)->vehicle;
   ASSERT_NE(assault, nullptr);

   Vehicle* ari = game->getField(4, 11)->vehicle;
   ASSERT_NE(ari, nullptr);

   MapCoordinate r(5, 8);
   MapCoordinate j(5, 11);

   MapField* fld = game->getField(MapCoordinate(0, 0));
   EXPECT_TRUE(fieldvisiblenow(fld)) << "Field (0,0) should be visible initially";

   attack(assault, r);

   EXPECT_TRUE(fieldvisiblenow(fld)) << "Field (0,0) should remain visible";

   MapField* fld2 = game->getField(MapCoordinate(6, 8));
   EXPECT_FALSE(fieldvisiblenow(fld2)) << "Field (6,8) should not be visible initially";

   attack(ari, j);

   EXPECT_TRUE(fieldvisiblenow(fld2)) << "Field (6,8) should be visible after attack";

   // Test undo
   ActionResult res = game->actions.undo(createTestingContext(game.get()));
   ASSERT_TRUE(res.successful());

   EXPECT_FALSE(fieldvisiblenow(fld2)) << "Field (6,8) visibility should be restored";
}
