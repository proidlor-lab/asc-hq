/***************************************************************************
 * mcts_action_executor_test.cpp - Google Test suite for MCTS action execution
 *
 * Migrated from: source/ai/mcts/domain/action_executor_test.cpp
 * Purpose: Unit tests for action execution interface
 *
 * Part of: ASC MCTS AI - Test Framework Migration (Phase 2)
 ***************************************************************************/

#include <gtest/gtest.h>
#include "simulation_action_executor.h"
#include "i_action_executor.h"
#include "action_types.h"
#include <iomanip>

using namespace asc::mcts;

// ========== Test Fixture ==========

class ActionExecutorTest : public ::testing::Test {
  protected:
   std::unique_ptr<GameStateSnapshot> createTestSnapshot() {
      auto snapshot = std::make_unique<GameStateSnapshot>();

      snapshot->mapWidth = 20;
      snapshot->mapHeight = 20;
      snapshot->currentPlayer = 0;
      snapshot->perspective = 0;

      // Add test units
      UnitSnapshot unit1;
      unit1.networkID = 1;
      unit1.x = 5;
      unit1.y = 5;
      unit1.height = 0;
      unit1.owner = 0;
      unit1.damage = 0;
      unit1.movement = 500;
      unit1.fuel = 1000;
      unit1.attacked = false;
      unit1.ammoMask = 0xFFFF;
      unit1.type = nullptr;  // Will need actual VehicleType for real tests

      UnitSnapshot unit2;
      unit2.networkID = 2;
      unit2.x = 8;
      unit2.y = 5;
      unit2.height = 0;
      unit2.owner = 1;  // Enemy
      unit2.damage = 0;
      unit2.movement = 300;
      unit2.fuel = 800;
      unit2.attacked = false;
      unit2.ammoMask = 0xFFFF;
      unit2.type = nullptr;

      snapshot->addUnit(unit1);
      snapshot->addUnit(unit2);

      return snapshot;
   }
};

// ========== Test Cases ==========

TEST_F(ActionExecutorTest, ActionTypeCreation) {
   MoveAction move{1, MapCoordinate(6, 5), 0};
   EXPECT_EQ(move.unitID, 1);
   EXPECT_EQ(move.destination.x, 6);
   EXPECT_EQ(move.destination.y, 5);

   AttackAction attack{1, MapCoordinate(8, 5), 0};
   EXPECT_EQ(attack.attackerID, 1);
   EXPECT_EQ(attack.target.x, 8);

   WaitAction wait{1};
   EXPECT_EQ(wait.unitID, 1);
}

TEST_F(ActionExecutorTest, ActionVariant) {
   Action action = MoveAction{1, MapCoordinate(6, 5)};
   std::string typeName = getActionTypeName(action);
   EXPECT_EQ(typeName, "Move");

   action = AttackAction{1, MapCoordinate(8, 5)};
   typeName = getActionTypeName(action);
   EXPECT_EQ(typeName, "Attack");

   UnitID unitID = getActionUnitID(action);
   EXPECT_EQ(unitID, 1);
}

TEST_F(ActionExecutorTest, ExecutorCreation) {
   auto snapshot = createTestSnapshot();
   auto executor = std::make_unique<SimulationActionExecutor>(std::move(snapshot));

   const auto& state = executor->getState();
   EXPECT_EQ(state.getUnits().size(), 2);
   EXPECT_EQ(state.getMapWidth(), 20);
   EXPECT_EQ(state.getMapHeight(), 20);
}

TEST_F(ActionExecutorTest, MoveActionLegality) {
   auto snapshot = createTestSnapshot();
   auto executor = std::make_unique<SimulationActionExecutor>(std::move(snapshot));

   // Legal move to neighbor
   MoveAction legalMove{1, MapCoordinate(6, 5)};
   ActionResult result = executor->isLegal(legalMove);
   EXPECT_TRUE(result.isSuccess()) << "Legal move rejected: " << result.toString();

   // Illegal move (occupied)
   MoveAction illegalMove{1, MapCoordinate(8, 5)};
   result = executor->isLegal(illegalMove);
   EXPECT_TRUE(result.isFailure()) << "Illegal move to occupied field should be rejected";

   // Illegal move (nonexistent unit)
   MoveAction nonexistentUnit{999, MapCoordinate(6, 5)};
   result = executor->isLegal(nonexistentUnit);
   EXPECT_EQ(result.code, ActionResultCode::UnitNotFound) << "Nonexistent unit not properly rejected";
}

TEST_F(ActionExecutorTest, MoveActionExecution) {
   auto snapshot = createTestSnapshot();
   auto executor = std::make_unique<SimulationActionExecutor>(std::move(snapshot));

   // Execute legal move
   ExecutionContext context;
   context.enableReactionFire = false;  // Disable RF for simple test

   MoveAction move{1, MapCoordinate(6, 5)};
   ActionResult result = executor->execute(move, context);

   ASSERT_TRUE(result.isSuccess()) << "Move execution failed: " << result.toString();

   // Verify unit moved
   const auto& state = executor->getState();
   const UnitSnapshot* unit = state.findUnit(1);
   ASSERT_NE(unit, nullptr);

   EXPECT_EQ(unit->x, 6);
   EXPECT_EQ(unit->y, 5);
   EXPECT_GT(result.movementConsumed, 0) << "Movement should be consumed";
}

TEST_F(ActionExecutorTest, AttackActionLegality) {
   auto snapshot = createTestSnapshot();
   auto executor = std::make_unique<SimulationActionExecutor>(std::move(snapshot));

   // Legal attack
   AttackAction legalAttack{1, MapCoordinate(8, 5)};
   ActionResult result = executor->isLegal(legalAttack);
   EXPECT_TRUE(result.isSuccess()) << "Legal attack rejected: " << result.toString();

   // Illegal attack (no target)
   AttackAction noTarget{1, MapCoordinate(10, 10)};
   result = executor->isLegal(noTarget);
   EXPECT_EQ(result.code, ActionResultCode::TargetNotFound)
      << "Attack with no target should be rejected";
}

TEST_F(ActionExecutorTest, AttackActionExecution) {
   auto snapshot = createTestSnapshot();
   auto executor = std::make_unique<SimulationActionExecutor>(std::move(snapshot));

   // Get defender's initial HP
   const UnitSnapshot* defenderBefore = executor->getState().findUnit(2);
   int initialDamage = defenderBefore->damage;

   // Execute attack
   ExecutionContext context;
   AttackAction attack{1, MapCoordinate(8, 5), 0};
   ActionResult result = executor->execute(attack, context);

   ASSERT_TRUE(result.isSuccess()) << "Attack execution failed: " << result.toString();
   EXPECT_GT(result.damageDealt, 0) << "No damage dealt";

   // Verify defender took damage
   const UnitSnapshot* defenderAfter = executor->getState().findUnit(2);
   EXPECT_GT(defenderAfter->damage, initialDamage)
      << "Defender damage: " << initialDamage << " -> " << defenderAfter->damage;

   // Verify attacker marked as attacked
   const UnitSnapshot* attacker = executor->getState().findUnit(1);
   EXPECT_TRUE(attacker->attacked) << "Attacker not marked as attacked";
}

TEST_F(ActionExecutorTest, WaitActionExecution) {
   auto snapshot = createTestSnapshot();
   auto executor = std::make_unique<SimulationActionExecutor>(std::move(snapshot));

   // Get initial movement
   const UnitSnapshot* unitBefore = executor->getState().findUnit(1);
   int initialMovement = unitBefore->movement;

   // Execute wait
   ExecutionContext context;
   WaitAction wait{1};
   ActionResult result = executor->execute(wait, context);

   ASSERT_TRUE(result.isSuccess()) << "Wait execution failed: " << result.toString();

   // Verify movement consumed
   const UnitSnapshot* unitAfter = executor->getState().findUnit(1);
   EXPECT_EQ(unitAfter->movement, 0) << "Movement not consumed by wait";
}

TEST_F(ActionExecutorTest, ActionGeneration) {
   auto snapshot = createTestSnapshot();
   auto executor = std::make_unique<SimulationActionExecutor>(std::move(snapshot));

   // Generate actions for unit 1
   auto actions = executor->generateLegalActions(1, true);

   ASSERT_FALSE(actions.empty()) << "No actions generated";

   // Check for action types
   bool hasMoves = false;
   bool hasAttacks = false;
   bool hasWait = false;

   for (const auto& action : actions) {
      std::string typeName = getActionTypeName(action);
      if (typeName == "Move")
         hasMoves = true;
      if (typeName == "Attack")
         hasAttacks = true;
      if (typeName == "Wait")
         hasWait = true;
   }

   EXPECT_TRUE(hasMoves) << "No move actions generated";
   EXPECT_TRUE(hasAttacks) << "No attack actions generated";
   EXPECT_TRUE(hasWait) << "No wait action generated";
}

TEST_F(ActionExecutorTest, ExecutorCloning) {
   auto snapshot = createTestSnapshot();
   auto executor1 = std::make_unique<SimulationActionExecutor>(std::move(snapshot));

   // Clone executor
   auto executor2 = executor1->clone();

   ASSERT_NE(executor2, nullptr) << "Failed to clone executor";
   EXPECT_EQ(executor2->getState().getUnits().size(), 2) << "Cloned state has wrong unit count";

   // Modify original
   ExecutionContext context;
   context.enableReactionFire = false;
   MoveAction move{1, MapCoordinate(6, 5)};
   executor1->execute(move, context);

   // Verify clone is independent
   const UnitSnapshot* unit1 = executor1->getState().findUnit(1);
   const UnitSnapshot* unit2 = executor2->getState().findUnit(1);

   EXPECT_TRUE(unit1->x != unit2->x || unit1->y != unit2->y)
      << "Cloned executor shares state with original";
}

TEST_F(ActionExecutorTest, UndoFunctionality) {
   auto snapshot = createTestSnapshot();
   auto executor = std::make_unique<SimulationActionExecutor>(std::move(snapshot));

   // Save initial position
   const UnitSnapshot* unitBefore = executor->getState().findUnit(1);
   int16_t initialX = unitBefore->x;
   int16_t initialY = unitBefore->y;

   // Execute move
   ExecutionContext context;
   context.enableReactionFire = false;
   MoveAction move{1, MapCoordinate(6, 5)};
   ActionResult result = executor->execute(move, context);

   ASSERT_TRUE(result.isSuccess()) << "Move failed: " << result.toString();

   // Undo
   bool undone = executor->undo();
   ASSERT_TRUE(undone) << "Undo failed";

   // Verify position restored
   const UnitSnapshot* unitAfter = executor->getState().findUnit(1);
   EXPECT_EQ(unitAfter->x, initialX);
   EXPECT_EQ(unitAfter->y, initialY);
}

// ========== Main ==========

int main(int argc, char** argv) {
   ::testing::InitGoogleTest(&argc, argv);
   return RUN_ALL_TESTS();
}
