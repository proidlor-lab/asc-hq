/***************************************************************************
 * diplomacy_test.cpp - Google Test suite for diplomacy system
 *
 * Migrated from: source/unittests/diplomacytest.cpp
 * Purpose: Test diplomatic relations between players
 *
 * Part of: ASC Test Framework Migration (Phase 2)
 *
 * Original test verified that:
 * - Diplomatic states (PEACE, ALLIANCE, WAR, TRUCE) work correctly
 * - State transitions require approval from both players
 * - Visibility changes based on diplomatic relations
 * - Sneak attacks immediately change relations to WAR
 * - Diplomatic proposals queue correctly between turns
 * - Undo functionality works for diplomatic actions
 * - State symmetry is maintained between players
 ***************************************************************************/

#include <gtest/gtest.h>
#include <memory>
#include "../../source/gamemap.h"
#include "../../source/loaders.h"
#include "../../source/unittests/unittestutil.h"
#include "../../source/spfst.h"
#include "../../source/actions/diplomacycommand.h"

// ========== Helper Functions ==========

void checkSymmetry(const Player& p0, const Player& p1) {
   EXPECT_EQ(p0.diplomacy.getState(p1), p1.diplomacy.getState(p0))
      << "Diplomatic state should be symmetric between players";
}

// ========== Test Fixture ==========

class DiplomacyTest : public ::testing::Test {
  protected:
   std::unique_ptr<GameMap> game;

   void SetUp() override {
      // Game map will be loaded in individual tests as needed
   }

   void TearDown() override {
      game.reset();
   }
};

// ========== Diplomacy Tests ==========

TEST_F(DiplomacyTest, ComplexDiplomaticTransitions) {
   // Test complex diplomatic state transitions through multiple states
   game.reset(startMap("unittest-diplomacy.map"));
   ASSERT_NE(game, nullptr) << "Failed to load unittest-diplomacy.map";

   // Initial state: PEACE between players
   Player& p0 = game->getPlayer(0);
   Player& p1 = game->getPlayer(1);

   EXPECT_EQ(p0.diplomacy.getState(p1), PEACE) << "Should start at PEACE";
   checkSymmetry(p0, p1);

   EXPECT_FALSE(p0.diplomacy.isAllied(p1)) << "Should not be allied";
   EXPECT_FALSE(p0.diplomacy.isHostile(p1)) << "Should not be hostile";

   ASSERT_EQ(game->actplayer, 0) << "Should be player 0's turn";

   EXPECT_FALSE(fieldvisiblenow(game->getField(MapCoordinate(0, 18)), p1.getPosition()))
      << "Field should not be visible to player 1 during PEACE";

   // Player 0 proposes ALLIANCE
   DiplomacyCommand* dc = new DiplomacyCommand(p0);
   dc->newstate(ALLIANCE, p1);
   dc->execute(createTestingContext(game.get()));

   EXPECT_FALSE(p0.diplomacy.isAllied(p1)) << "Alliance not yet active (needs acceptance)";
   checkSymmetry(p0, p1);

   next_turn(game.get(), NextTurnStrategy_Abort(), NULL, -1);
   ASSERT_EQ(game->actplayer, 1) << "Should be player 1's turn";

   EXPECT_FALSE(p0.diplomacy.isAllied(p1)) << "Alliance still not active";
   checkSymmetry(p0, p1);

   // Player 1 accepts ALLIANCE
   dc = new DiplomacyCommand(p1);
   dc->newstate(ALLIANCE, p0);
   dc->execute(createTestingContext(game.get()));

   EXPECT_TRUE(p0.diplomacy.isAllied(p1)) << "Alliance should now be active";
   EXPECT_EQ(p0.diplomacy.getState(p1), ALLIANCE) << "State should be ALLIANCE";
   checkSymmetry(p0, p1);

   // Verify visibility during alliance
   EXPECT_TRUE(fieldvisiblenow(game->getField(MapCoordinate(0, 18)), p1.getPosition()))
      << "Field should be visible to player 1 during ALLIANCE";

   // Player 1 makes a sneak attack
   dc = new DiplomacyCommand(p1);
   dc->sneakAttack(p0);
   dc->execute(createTestingContext(game.get()));
   checkSymmetry(p0, p1);

   EXPECT_EQ(p0.diplomacy.getState(p1), WAR) << "State should immediately be WAR after sneak attack";

   // Verify visibility during war
   EXPECT_FALSE(fieldvisiblenow(game->getField(MapCoordinate(0, 18)), p1.getPosition()))
      << "Field should not be visible to player 1 during WAR";

   next_turn(game.get(), NextTurnStrategy_Abort(), NULL, -1);
   ASSERT_EQ(game->actplayer, 0) << "Should be player 0's turn";

   checkSymmetry(p0, p1);
   EXPECT_EQ(p0.diplomacy.getState(p1), WAR) << "Should still be at WAR";

   // Player 0 proposes peace
   dc = new DiplomacyCommand(p0);
   dc->newstate(PEACE_SV, p1);
   dc->execute(createTestingContext(game.get()));

   checkSymmetry(p0, p1);
   EXPECT_EQ(p0.diplomacy.getState(p1), WAR) << "Still at WAR (peace not accepted)";

   next_turn(game.get(), NextTurnStrategy_Abort(), NULL, -1);
   ASSERT_EQ(game->actplayer, 1) << "Should be player 1's turn";

   // Player 1 only accepts truce (not full peace)
   dc = new DiplomacyCommand(p1);
   dc->newstate(TRUCE, p0);
   dc->execute(createTestingContext(game.get()));

   checkSymmetry(p0, p1);
   EXPECT_EQ(p0.diplomacy.getState(p1), TRUCE) << "State should be TRUCE";

   EXPECT_FALSE(p0.diplomacy.getProposal(p1.getPosition(), NULL)) << "No pending proposals from p0";
   EXPECT_FALSE(p1.diplomacy.getProposal(p0.getPosition(), NULL)) << "No pending proposals from p1";

   next_turn(game.get(), NextTurnStrategy_Abort(), NULL, -1);
   ASSERT_EQ(game->actplayer, 0) << "Should be player 0's turn";

   checkSymmetry(p0, p1);
   EXPECT_EQ(p0.diplomacy.getState(p1), TRUCE) << "Still at TRUCE";

   // Player 0 proposes peace again
   dc = new DiplomacyCommand(p0);
   dc->newstate(PEACE, p1);
   dc->execute(createTestingContext(game.get()));

   next_turn(game.get(), NextTurnStrategy_Abort(), NULL, -1);
   ASSERT_EQ(game->actplayer, 1) << "Should be player 1's turn";

   checkSymmetry(p0, p1);
   EXPECT_EQ(p0.diplomacy.getState(p1), TRUCE) << "Still at TRUCE";

   EXPECT_FALSE(p0.diplomacy.getProposal(p1.getPosition(), NULL)) << "No proposal from p0";
   EXPECT_TRUE(p1.diplomacy.getProposal(p0.getPosition(), NULL)) << "Pending proposal from p1";

   // Player 1 accepts peace and proposes alliance
   dc = new DiplomacyCommand(p1);
   dc->newstate(ALLIANCE, p0);
   dc->execute(createTestingContext(game.get()));

   checkSymmetry(p0, p1);
   EXPECT_EQ(p0.diplomacy.getState(p1), PEACE) << "State should be PEACE";

   EXPECT_TRUE(p0.diplomacy.getProposal(p1.getPosition(), NULL)) << "Pending alliance proposal";
   EXPECT_FALSE(p1.diplomacy.getProposal(p0.getPosition(), NULL)) << "No proposal from p1";

   next_turn(game.get(), NextTurnStrategy_Abort(), NULL, -1);
   ASSERT_EQ(game->actplayer, 0) << "Should be player 0's turn";

   checkSymmetry(p0, p1);
   EXPECT_EQ(p0.diplomacy.getState(p1), PEACE) << "Still at PEACE";

   EXPECT_FALSE(fieldvisiblenow(game->getField(MapCoordinate(0, 18)), p1.getPosition()))
      << "Field should not be visible during PEACE";

   // Player 0 accepts alliance
   dc = new DiplomacyCommand(p0);
   dc->newstate(ALLIANCE, p1);
   dc->execute(createTestingContext(game.get()));

   checkSymmetry(p0, p1);
   EXPECT_EQ(p0.diplomacy.getState(p1), ALLIANCE) << "State should be ALLIANCE";

   EXPECT_TRUE(fieldvisiblenow(game->getField(MapCoordinate(0, 18)), p1.getPosition()))
      << "Field should be visible during ALLIANCE";

   EXPECT_FALSE(p0.diplomacy.getProposal(p1.getPosition(), NULL)) << "No pending proposals";
   EXPECT_FALSE(p1.diplomacy.getProposal(p0.getPosition(), NULL)) << "No pending proposals";

   // Test downgrading from ALLIANCE to TRUCE (requires two-step process)
   next_turn(game.get(), NextTurnStrategy_Abort(), NULL, -1);
   ASSERT_EQ(game->actplayer, 1) << "Should be player 1's turn";

   dc = new DiplomacyCommand(p1);
   dc->newstate(TRUCE, p0);
   dc->execute(createTestingContext(game.get()));

   next_turn(game.get(), NextTurnStrategy_Abort(), NULL, -1);
   ASSERT_EQ(game->actplayer, 0) << "Should be player 0's turn";

   // Player 0 proposes WAR but it requires next turn to take effect
   dc = new DiplomacyCommand(p0);
   dc->newstate(WAR, p1);
   dc->execute(createTestingContext(game.get()));

   checkSymmetry(p0, p1);
   EXPECT_EQ(p0.diplomacy.getState(p1), TRUCE) << "Should still be TRUCE";

   next_turn(game.get(), NextTurnStrategy_Abort(), NULL, -1);
   ASSERT_EQ(game->actplayer, 1) << "Should be player 1's turn";

   checkSymmetry(p0, p1);
   EXPECT_EQ(p0.diplomacy.getState(p1), WAR) << "Should now be WAR";
}

TEST_F(DiplomacyTest, DirectWarDeclaration) {
   // Test that a player can directly declare war
   game.reset(startMap("unittest-diplomacy.map"));
   ASSERT_NE(game, nullptr);

   Player& p0 = game->getPlayer(0);
   Player& p1 = game->getPlayer(1);

   EXPECT_EQ(p0.diplomacy.getState(p1), PEACE) << "Should start at PEACE";
   checkSymmetry(p0, p1);

   EXPECT_FALSE(p0.diplomacy.isAllied(p1));
   EXPECT_FALSE(p0.diplomacy.isHostile(p1));

   // Player 0 declares WAR
   DiplomacyCommand* dc = new DiplomacyCommand(p0);
   dc->newstate(WAR, p1);
   dc->execute(createTestingContext(game.get()));

   next_turn(game.get(), NextTurnStrategy_Abort(), NULL, -1);
   ASSERT_EQ(game->actplayer, 1) << "Should be player 1's turn";

   // WAR should take effect on the next turn
   checkSymmetry(p0, p1);
   EXPECT_EQ(p0.diplomacy.getState(p1), WAR) << "Should be at WAR";
}

TEST_F(DiplomacyTest, UndoWarDeclaration) {
   // Test that undoing a war declaration prevents the war
   game.reset(startMap("unittest-diplomacy.map"));
   ASSERT_NE(game, nullptr);

   Player& p0 = game->getPlayer(0);
   Player& p1 = game->getPlayer(1);

   EXPECT_EQ(p0.diplomacy.getState(p1), PEACE);
   checkSymmetry(p0, p1);

   EXPECT_FALSE(p0.diplomacy.isAllied(p1));
   EXPECT_FALSE(p0.diplomacy.isHostile(p1));

   // Player 0 declares WAR
   DiplomacyCommand* dc = new DiplomacyCommand(p0);
   dc->newstate(WAR, p1);
   dc->execute(createTestingContext(game.get()));

   // Player 0 presses undo
   ActionResult res = game->actions.undo(createTestingContext(game.get()));
   ASSERT_TRUE(res.successful()) << "Undo should succeed";

   next_turn(game.get(), NextTurnStrategy_Abort(), NULL, -1);
   ASSERT_EQ(game->actplayer, 1) << "Should be player 1's turn";

   checkSymmetry(p0, p1);
   EXPECT_EQ(p0.diplomacy.getState(p1), PEACE) << "Should still be at PEACE after undo";

   // Advance another turn to verify nothing happens
   next_turn(game.get(), NextTurnStrategy_Abort(), NULL, -1);
   ASSERT_EQ(game->actplayer, 0) << "Should be player 0's turn";

   checkSymmetry(p0, p1);
   EXPECT_EQ(p0.diplomacy.getState(p1), PEACE) << "Should still be at PEACE";
}
