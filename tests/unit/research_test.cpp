/***************************************************************************
 * research_test.cpp - Google Test suite for research and technology system
 *
 * Migrated from: source/unittests/researchtest.cpp
 * Purpose: Test technology research mechanics and production line dependencies
 *
 * Part of: ASC Test Framework Migration (Phase 2)
 *
 * Original test verified that:
 * - Research progress accumulates over turns
 * - Technologies can be researched using DirectResearchCommand
 * - Research completion enables new production lines
 * - Undo functionality works for research actions
 * - Technology prerequisites are properly enforced
 * - Research order affects available technologies
 ***************************************************************************/

#include <gtest/gtest.h>
#include <memory>
#include <vector>
#include <algorithm>
#include "../../source/gamemap.h"
#include "../../source/loaders.h"
#include "../../source/unittests/unittestutil.h"
#include "../../source/spfst.h"
#include "../../source/actions/directresearchcommand.h"
#include "../../source/itemrepository.h"
#include "../../source/actions/buildproductionlinecommand.h"

// ========== Helper Functions ==========

bool vectorContains(std::vector<const Technology*>& v, const Technology* t) {
   return std::find(v.begin(), v.end(), t) != v.end();
}

// ========== Test Fixture ==========

class ResearchTest : public ::testing::Test {
  protected:
   std::unique_ptr<GameMap> game;

   void SetUp() override {
      // Game map will be loaded in individual tests as needed
   }

   void TearDown() override {
      game.reset();
   }
};

// ========== Research Tests ==========

TEST_F(ResearchTest, BasicResearchAndTechTree) {
   // Test basic research mechanics and technology tree progression
   game.reset(startMap("unittest-research.map"));

   Player& p0 = game->getPlayer(0);
   Research& r = p0.research;

   ASSERT_EQ(r.progress, 0) << "Research should start at 0 progress";
   ASSERT_FALSE(DirectResearchCommand::available(p0)) << "Research command should not be available yet";

   // Advance one turn - should accumulate research points
   next_turn(game.get(), NextTurnStrategy_Abort(), NULL, -1);
   ASSERT_EQ(game->actplayer, 0) << "Should be player 0's turn";

   EXPECT_EQ(r.progress, 40) << "Research progress should be 40 after one turn";
   EXPECT_TRUE(DirectResearchCommand::available(p0)) << "Research command should now be available";

   // Get technology references
   Technology* t1 = technologyRepository.getObject_byID(1000000002);
   ASSERT_NE(t1, nullptr) << "Technology T1 should exist";

   Technology* t2 = technologyRepository.getObject_byID(1000000003);
   ASSERT_NE(t2, nullptr) << "Technology T2 should exist";

   Technology* tjam = technologyRepository.getObject_byID(1000000051);
   ASSERT_NE(tjam, nullptr) << "Jammer technology should exist";

   // Check available technologies
   DirectResearchCommand* drc = new DirectResearchCommand(p0);
   std::vector<const Technology*> avTechs = drc->getAvailableTechnologies(true);

   EXPECT_TRUE(vectorContains(avTechs, t1)) << "T1 should be available";
   EXPECT_TRUE(vectorContains(avTechs, t2)) << "T2 should be available";
   EXPECT_TRUE(vectorContains(avTechs, tjam)) << "Jammer tech should be available";

   // Research T1
   drc->setTechnology(t1);
   ActionResult res = drc->execute(createTestingContext(game.get()));
   ASSERT_TRUE(res.successful()) << "Research command should succeed";

   // Advance turns until research completes
   next_turn(game.get(), NextTurnStrategy_Abort(), NULL, -1);
   next_turn(game.get(), NextTurnStrategy_Abort(), NULL, -1);
   next_turn(game.get(), NextTurnStrategy_Abort(), NULL, -1);

   EXPECT_EQ(r.activetechnology, nullptr) << "Research should be complete";
   EXPECT_EQ(r.progress, 20) << "Research progress should be 20";

   // Check available technologies after completing T1
   drc = new DirectResearchCommand(p0);
   avTechs = drc->getAvailableTechnologies(true);

   EXPECT_FALSE(vectorContains(avTechs, t1)) << "T1 should no longer be available";
   EXPECT_TRUE(vectorContains(avTechs, t2)) << "T2 should still be available";
   EXPECT_TRUE(vectorContains(avTechs, tjam)) << "Jammer tech should still be available";

   // Research T2
   drc->setTechnology(t2);
   res = drc->execute(createTestingContext(game.get()));
   ASSERT_TRUE(res.successful()) << "Research T2 should succeed";

   // Test production line availability before jammer research
   Building* bld = game->getField(MapCoordinate(7, 8))->building;
   ASSERT_NE(bld, nullptr) << "Building should exist at (7,8)";

   VehicleType* jam = vehicleTypeRepository.getObject_byID(1000000051);
   ASSERT_NE(jam, nullptr) << "Jammer vehicle type should exist";

   BuildProductionLineCommand* bplc = new BuildProductionLineCommand(bld);
   std::vector<const VehicleType*> prods = bplc->productionLinesBuyable();
   EXPECT_EQ(std::find(prods.begin(), prods.end(), jam), prods.end())
      << "Jammer should not be buildable before research";

   // Research jammer technology
   drc = new DirectResearchCommand(p0);
   drc->setTechnology(tjam);
   res = drc->execute(createTestingContext(game.get()));

   EXPECT_EQ(r.progress, 5) << "Research progress should be 5 after instant research";

   // Verify jammer is now buildable
   prods = bplc->productionLinesBuyable();
   EXPECT_NE(std::find(prods.begin(), prods.end(), jam), prods.end())
      << "Jammer should be buildable after research";

   bplc->setProduction(jam);
   res = bplc->execute(createTestingContext(game.get()));
   ASSERT_TRUE(res.successful()) << "Building production line should succeed";

   // Test undo chain
   res = game->actions.undo(createTestingContext(game.get()));  // undo build production line
   ASSERT_TRUE(res.successful()) << "Undo build production line should succeed";

   res = game->actions.undo(createTestingContext(game.get()));  // undo research jammer
   ASSERT_TRUE(res.successful()) << "Undo research jammer should succeed";

   res = game->actions.undo(createTestingContext(game.get()));  // undo research T2
   ASSERT_TRUE(res.successful()) << "Undo research T2 should succeed";

   EXPECT_EQ(r.activetechnology, nullptr) << "No active research after undo";
   EXPECT_EQ(r.progress, 20) << "Research progress should be restored to 20";

   // Verify jammer is no longer buildable after undo
   BuildProductionLineCommand bplc2(bld);
   prods = bplc2.productionLinesBuyable();
   EXPECT_EQ(std::find(prods.begin(), prods.end(), jam), prods.end())
      << "Jammer should not be buildable after undo";
}

TEST_F(ResearchTest, AlternativeResearchPath) {
   // Test researching jammer first affects technology availability
   game.reset(startMap("unittest-research.map"));

   Player& p0 = game->getPlayer(0);
   Research& r = p0.research;

   ASSERT_EQ(r.progress, 0) << "Research should start at 0 progress";
   ASSERT_FALSE(DirectResearchCommand::available(p0)) << "Research command should not be available yet";

   next_turn(game.get(), NextTurnStrategy_Abort(), NULL, -1);
   ASSERT_EQ(game->actplayer, 0) << "Should be player 0's turn";

   EXPECT_EQ(r.progress, 40) << "Research progress should be 40";
   EXPECT_TRUE(DirectResearchCommand::available(p0)) << "Research command should be available";

   // Get technology references
   Technology* t1 = technologyRepository.getObject_byID(1000000002);
   ASSERT_NE(t1, nullptr);

   Technology* t2 = technologyRepository.getObject_byID(1000000003);
   ASSERT_NE(t2, nullptr);

   Technology* tjam = technologyRepository.getObject_byID(1000000051);
   ASSERT_NE(tjam, nullptr);

   DirectResearchCommand* drc = new DirectResearchCommand(p0);
   std::vector<const Technology*> avTechs = drc->getAvailableTechnologies(true);

   EXPECT_TRUE(vectorContains(avTechs, t1));
   EXPECT_TRUE(vectorContains(avTechs, t2));
   EXPECT_TRUE(vectorContains(avTechs, tjam));

   // Research jammer first (different path than test 1)
   drc->setTechnology(tjam);
   ActionResult res = drc->execute(createTestingContext(game.get()));
   ASSERT_TRUE(res.successful()) << "Research jammer should succeed";

   next_turn(game.get(), NextTurnStrategy_Abort(), NULL, -1);
   next_turn(game.get(), NextTurnStrategy_Abort(), NULL, -1);
   next_turn(game.get(), NextTurnStrategy_Abort(), NULL, -1);

   EXPECT_EQ(r.activetechnology, nullptr) << "Research should be complete";
   EXPECT_EQ(r.progress, 15) << "Research progress should be 15";

   // Check available technologies after completing jammer
   drc = new DirectResearchCommand(p0);
   avTechs = drc->getAvailableTechnologies(true);

   EXPECT_FALSE(vectorContains(avTechs, t1)) << "T1 should not be available after jammer";
   EXPECT_TRUE(vectorContains(avTechs, t2)) << "T2 should be available";
   EXPECT_FALSE(vectorContains(avTechs, tjam)) << "Jammer should not be available (already researched)";

   // Research T2
   drc->setTechnology(t2);
   res = drc->execute(createTestingContext(game.get()));
   ASSERT_TRUE(res.successful()) << "Research T2 should succeed";

   // Verify jammer is buildable since we researched it
   Building* bld = game->getField(MapCoordinate(7, 8))->building;
   ASSERT_NE(bld, nullptr);

   VehicleType* jam = vehicleTypeRepository.getObject_byID(1000000051);
   ASSERT_NE(jam, nullptr);

   BuildProductionLineCommand bplc(bld);
   std::vector<const VehicleType*> prods = bplc.productionLinesBuyable();
   EXPECT_NE(std::find(prods.begin(), prods.end(), jam), prods.end())
      << "Jammer should be buildable since we researched it";
}
