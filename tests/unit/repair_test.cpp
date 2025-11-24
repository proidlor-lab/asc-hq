/***************************************************************************
 * repair_test.cpp - Google Test suite for unit repair mechanics
 *
 * Migrated from: source/unittests/repairtest.cpp
 * Purpose: Test automatic and manual unit repair, self-damage mechanics
 *
 * Part of: ASC Test Framework Migration (Phase 2)
 *
 * Original test verified that:
 * - Units automatically repair over time
 * - Manual repair commands work correctly on cargo units
 * - Experience decreases when units are repaired
 * - Self-damage mechanics work correctly (units take damage over time)
 * - Units can be destroyed by accumulating self-damage
 ***************************************************************************/

#include <gtest/gtest.h>
#include <memory>
#include "../../source/gamemap.h"
#include "../../source/actions/moveunitcommand.h"
#include "../../source/actions/repairunitcommand.h"
#include "../../source/loaders.h"
#include "../../source/itemrepository.h"
#include "../../source/unittests/unittestutil.h"
#include "../../source/spfst.h"

// ========== MapHolder Helper Class ==========
// NextTurnStrategy_Abort can delete the map if the game cannot be continued.
// To avoid double deallocation, we must intercept the event and release the unique_ptr

class MapHolder : public sigc::trackable {
   std::unique_ptr<GameMap> game;

  public:
   MapHolder(GameMap* gamemap) : game(gamemap) {
      GameMap::sigMapDeletion.connect(sigc::mem_fun(*this, &MapHolder::reset));
   }

   GameMap* get() { return game.get(); }

   GameMap* operator->() const { return game.operator->(); }

  private:
   void reset(GameMap& gamemap) {
      if (&gamemap == game.get())
         game.release();
   }
};

// ========== Test Fixture ==========

class RepairTest : public ::testing::Test {
  protected:
   void SetUp() override {
      // Setup if needed
   }

   void TearDown() override {
      // Cleanup handled by MapHolder
   }
};

// ========== Repair Tests ==========

TEST_F(RepairTest, AutoRepair) {
   // Test that units automatically repair damage over time
   MapHolder game(startMap("unittest-repair.map"));

   Vehicle* veh = game->getField(3, 6)->vehicle;
   ASSERT_NE(veh, nullptr) << "Vehicle should exist at (3,6)";

   ASSERT_EQ(veh->damage, 40) << "Vehicle should start with 40 damage";

   // Advance one turn - should auto-repair 5 damage
   next_turn(game.get(), NextTurnStrategy_Abort(), NULL, -1);

   EXPECT_EQ(veh->damage, 35) << "Vehicle should repair to 35 damage after one turn";

   // Advance 10 more turns - should continue repairing
   for (int i = 0; i < 10; ++i) {
      next_turn(game.get(), NextTurnStrategy_Abort(), NULL, -1);
   }

   EXPECT_EQ(veh->damage, 10) << "Vehicle should have 10 damage after 11 turns total";
}

TEST_F(RepairTest, ManualRepair) {
   // Test manual repair command on cargo units
   MapHolder game(startMap("unittest-repair.map"));

   Vehicle* carrier = game->getField(1, 9)->vehicle;
   ASSERT_NE(carrier, nullptr) << "Carrier should exist at (1,9)";

   Vehicle* aircraft = carrier->getCargo(0);
   ASSERT_NE(aircraft, nullptr) << "Aircraft should be loaded in carrier";
   ASSERT_EQ(aircraft->getExperience_offensive(), 10) << "Aircraft should start with 10 offensive experience";
   ASSERT_EQ(aircraft->getExperience_defensive(), 10) << "Aircraft should start with 10 defensive experience";
   ASSERT_EQ(aircraft->damage, 50) << "Aircraft should start with 50 damage";

   // Verify repair command is available
   ASSERT_TRUE(RepairUnitCommand::avail(carrier)) << "Repair command should be available for carrier";

   std::unique_ptr<RepairUnitCommand> ruc(new RepairUnitCommand(carrier));

   ASSERT_EQ(ruc->getInternalTargets().size(), 1) << "Should have one repairable cargo unit";
   ASSERT_EQ(ruc->getInternalTargets()[0], aircraft) << "Repairable target should be the aircraft";

   ruc->setTarget(aircraft);

   ActionResult res = ruc->execute(createTestingContext(game.get()));
   if (res.successful())
      ruc.release();
   else
      FAIL() << "Repair command failed: " << res.getMessage();

   // Verify repair worked and experience was reduced
   EXPECT_EQ(aircraft->damage, 0) << "Aircraft should be fully repaired";
   EXPECT_EQ(aircraft->getExperience_offensive(), 9) << "Offensive experience should decrease by 1";
   EXPECT_EQ(aircraft->getExperience_defensive(), 9) << "Defensive experience should decrease by 1";
}

TEST_F(RepairTest, SelfDamage) {
   // Test that units take self-damage over time and can be destroyed
   MapHolder game(startMap("unittest-selfdamage.map"));

   Vehicle* u1 = game->getField(4, 7)->vehicle;
   Vehicle* u2 = game->getField(5, 6)->vehicle;

   ASSERT_EQ(u1->damage, 0) << "Unit 1 should start with 0 damage";
   ASSERT_EQ(u2->damage, 79) << "Unit 2 should start with 79 damage";

   // Advance one turn - both units should take self-damage
   next_turn(game.get(), NextTurnStrategy_Abort(), NULL, -1);

   EXPECT_EQ(u1->damage, 10) << "Unit 1 should take 10 self-damage";
   EXPECT_EQ(u2->damage, 89) << "Unit 2 should have 89 damage";

   // Advance two more turns - unit 2 should be destroyed
   next_turn(game.get(), NextTurnStrategy_Abort(), NULL, -1);
   next_turn(game.get(), NextTurnStrategy_Abort(), NULL, -1);

   u2 = game->getField(5, 6)->vehicle;
   EXPECT_EQ(u2, nullptr) << "Unit 2 should be destroyed by self-damage";
}
