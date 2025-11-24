/***************************************************************************
 * map_test.cpp - Google Test suite for Lua map testing
 *
 * Migrated from: source/unittests/maptest.cpp
 * Purpose: Test map loading and Lua script execution
 *
 * Part of: ASC Test Framework Migration (Phase 2)
 *
 * Original test verified that:
 * - Maps can be loaded successfully
 * - Lua scripts associated with maps execute without errors
 * - Map and script integration works correctly
 ***************************************************************************/

#include <gtest/gtest.h>
#include <memory>
#include "../../source/gamemap.h"
#include "../../source/loaders.h"
#include "../../source/unittests/unittestutil.h"
#include "../../source/lua/luarunner.h"
#include "../../source/lua/luastate.h"
#include "../../source/spfst-legacy.h"

// ========== Test Fixture ==========

class MapTest : public ::testing::Test {
  protected:
   void SetUp() override {
      // Setup if needed
   }

   void TearDown() override {
      // Cleanup
      actmap = nullptr;
   }

   void runScript(const ASCString& script) {
      LuaState state;
      LuaRunner runner(state);
      runner.runFile(script);
      ASSERT_TRUE(runner.getErrors().empty())
         << "Lua script execution failed with errors: " << runner.getErrors();
   }
};

// ========== Map Loading Tests ==========

TEST_F(MapTest, LoadMapAndRunScript) {
   // Load the test map
   std::unique_ptr<GameMap> game(startMap("kam005.map"));
   ASSERT_NE(game, nullptr) << "Failed to load kam005.map";

   // Set the global actmap for Lua context
   actmap = game.get();

   // Run the associated Lua script
   ASSERT_NO_THROW(runScript("kam005.lua")) << "Lua script kam005.lua failed to execute";

   // Clean up global state
   actmap = nullptr;
}
