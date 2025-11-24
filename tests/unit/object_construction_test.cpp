/***************************************************************************
 * object_construction_test.cpp - Google Test suite for object construction/removal
 *
 * Migrated from: source/unittests/objectconstructiontest.cpp
 * Purpose: Test spawning and removing objects on the map
 *
 * Part of: ASC Test Framework Migration (Phase 2)
 *
 * Original test verified that:
 * - Objects can be spawned on map fields (e.g., roads replacing crystals)
 * - Object spawning can be undone
 * - Objects can be removed from map fields
 * - Object removal affects visibility
 * - Object removal can be undone
 ***************************************************************************/

#include <gtest/gtest.h>
#include <memory>
#include "../../source/gamemap.h"
#include "../../source/loaders.h"
#include "../../source/unittests/unittestutil.h"
#include "../../source/spfst.h"
#include "../../source/actions/spawnobject.h"
#include "../../source/actions/putobjectcommand.h"
#include "../../source/itemrepository.h"

// ========== Test Fixture ==========

class ObjectConstructionTest : public ::testing::Test {
  protected:
   std::unique_ptr<GameMap> game;

   void SetUp() override {
      // Game map will be loaded in individual tests as needed
   }

   void TearDown() override {
      game.reset();
   }
};

// ========== Object Construction Tests ==========

TEST_F(ObjectConstructionTest, SpawnObject) {
   // Test spawning objects on map fields (e.g., building a road)
   game.reset(startMap("unittest-objectconstruction.map"));
   ASSERT_NE(game, nullptr) << "Failed to load unittest-objectconstruction.map";

   ObjectType* crystals = objectTypeRepository.getObject_byID(2105);
   ASSERT_NE(crystals, nullptr) << "Crystal object type should exist";

   ObjectType* road = objectTypeRepository.getObject_byID(1);
   ASSERT_NE(road, nullptr) << "Road object type should exist";

   MapCoordinate pos(4, 8);
   MapField* fld = game->getField(pos);
   ASSERT_NE(fld, nullptr) << "Map field should exist at (4,8)";

   EXPECT_NE(fld->checkForObject(crystals), nullptr) << "Crystals should exist initially";

   // Spawn a road object (replaces crystals)
   SpawnObject so(game.get(), pos, 1);
   ActionResult res = so.execute(createTestingContext(game.get()));
   ASSERT_TRUE(res.successful()) << "Spawn object should succeed: " << res.getMessage();

   EXPECT_EQ(fld->checkForObject(crystals), nullptr) << "Crystals should be removed";
   EXPECT_NE(fld->checkForObject(road), nullptr) << "Road should now exist";

   // Test undo
   res = so.undo(createTestingContext(game.get()));
   ASSERT_TRUE(res.successful()) << "Undo should succeed";

   EXPECT_NE(fld->checkForObject(crystals), nullptr) << "Crystals should be restored";
   EXPECT_EQ(fld->checkForObject(road), nullptr) << "Road should be removed";
}

TEST_F(ObjectConstructionTest, RemoveObject) {
   // Test removing objects from map fields and visibility changes
   game.reset(startMap("unittest-objectremoval.map"));
   ASSERT_NE(game, nullptr) << "Failed to load unittest-objectremoval.map";

   ObjectType* wood = objectTypeRepository.getObject_byID(181);
   ASSERT_NE(wood, nullptr) << "Wood object type should exist";

   MapCoordinate pos(7, 9);
   MapField* fld = game->getField(pos);
   ASSERT_NE(fld, nullptr) << "Map field should exist at (7,9)";

   EXPECT_NE(fld->checkForObject(wood), nullptr) << "Wood should exist initially";

   MapCoordinate unit(7, 10);
   Vehicle* v = game->getField(unit)->vehicle;
   ASSERT_NE(v, nullptr) << "Vehicle should exist at (7,10)";

   MapField* view = game->getField(MapCoordinate(8, 7));
   EXPECT_EQ(fieldVisibility(view), visible_not) << "View field should not be visible initially";

   // Execute put object command (removes wood)
   PutObjectCommand* po = new PutObjectCommand(v);
   ActionResult res = po->searchFields();
   ASSERT_TRUE(res.successful()) << "Search fields should succeed";

   po->setTarget(pos, wood->id);
   res = po->execute(createTestingContext(game.get()));
   ASSERT_TRUE(res.successful()) << "Put object command should succeed: " << res.getMessage();

   // Verify object removed and visibility changed
   EXPECT_TRUE(fieldVisibility(view) >= visible_now) << "View field should now be visible";
   EXPECT_EQ(fld->checkForObject(wood), nullptr) << "Wood should be removed";

   // Test undo
   res = po->undo(createTestingContext(game.get()));
   ASSERT_TRUE(res.successful()) << "Undo should succeed";

   EXPECT_EQ(fieldVisibility(view), visible_not) << "View field visibility should be restored";
   EXPECT_NE(fld->checkForObject(wood), nullptr) << "Wood should be restored";
}
