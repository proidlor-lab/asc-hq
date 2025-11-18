/***************************************************************************
 * mcts_evaluator_test.cpp - Google Test suite for MCTS tactical evaluators
 *
 * Migrated from: source/ai/mcts/domain/evaluator_test.cpp
 * Purpose: Unit tests for tactical evaluation functions
 *
 * Test Coverage:
 * - Factory creation
 * - Terminal state detection
 * - Material evaluation
 * - Position evaluation
 * - Health evaluation
 * - Threat evaluation
 * - Weighted combination
 * - Edge cases
 *
 * Part of: ASC MCTS AI - Test Framework Migration (Phase 2)
 ***************************************************************************/

#include <gtest/gtest.h>
#include "i_tactical_evaluator.h"
#include "simple_combat_evaluator.h"
#include "game_state_snapshot.h"
#include <cmath>

using namespace asc::mcts;

// ========== Test Helpers ==========

/**
 * Create simple test snapshot with units
 */
GameStateSnapshot createTestSnapshot(int playerUnits, int enemyUnits) {
   GameStateSnapshot snapshot;
   snapshot.mapWidth = 100;
   snapshot.mapHeight = 100;
   snapshot.currentPlayer = 0;
   snapshot.perspective = 0;

   // Add player units (player 0)
   for (int i = 0; i < playerUnits; ++i) {
      UnitSnapshot unit;
      unit.networkID = i;
      unit.owner = 0;
      unit.x = 10 + i * 2;
      unit.y = 10;
      unit.damage = 0;  // Full HP
      unit.movement = 100;
      unit.fuel = 1000;
      unit.attacked = false;
      unit.ammoMask = 0xFFFF;
      snapshot.addUnit(unit);
   }

   // Add enemy units (player 1)
   for (int i = 0; i < enemyUnits; ++i) {
      UnitSnapshot unit;
      unit.networkID = playerUnits + i;
      unit.owner = 1;
      unit.x = 50 + i * 2;
      unit.y = 50;
      unit.damage = 0;  // Full HP
      unit.movement = 100;
      unit.fuel = 1000;
      unit.attacked = false;
      unit.ammoMask = 0xFFFF;
      snapshot.addUnit(unit);
   }

   return snapshot;
}

// ========== Test Fixture ==========

class EvaluatorTest : public ::testing::Test {
  protected:
   void SetUp() override {
      evaluator = EvaluatorFactory::createSimpleCombatEvaluator();
   }

   std::unique_ptr<ITacticalEvaluator> evaluator;
};

// ========== Factory Tests ==========

TEST_F(EvaluatorTest, FactoryCreation) {
   auto eval = EvaluatorFactory::createSimpleCombatEvaluator();
   ASSERT_NE(eval, nullptr);
   EXPECT_STREQ(eval->getName(), "SimpleCombatEvaluator");
}

TEST_F(EvaluatorTest, FactoryDefault) {
   auto eval = EvaluatorFactory::createDefault();
   ASSERT_NE(eval, nullptr);
}

TEST_F(EvaluatorTest, EvaluatorClone) {
   auto clone = evaluator->clone();
   ASSERT_NE(clone, nullptr);
   EXPECT_STREQ(clone->getName(), "SimpleCombatEvaluator");
}

// ========== Terminal State Tests ==========

TEST_F(EvaluatorTest, TerminalStateNoUnits) {
   GameStateSnapshot snapshot = createTestSnapshot(0, 0);
   EXPECT_TRUE(evaluator->isTerminalState(snapshot, 0));
}

TEST_F(EvaluatorTest, TerminalStateNoPlayerUnits) {
   GameStateSnapshot snapshot = createTestSnapshot(0, 5);
   EXPECT_TRUE(evaluator->isTerminalState(snapshot, 0));
}

TEST_F(EvaluatorTest, TerminalStateNoEnemyUnits) {
   GameStateSnapshot snapshot = createTestSnapshot(5, 0);
   EXPECT_TRUE(evaluator->isTerminalState(snapshot, 0));
}

TEST_F(EvaluatorTest, TerminalStateBothHaveUnits) {
   GameStateSnapshot snapshot = createTestSnapshot(5, 5);
   EXPECT_FALSE(evaluator->isTerminalState(snapshot, 0));
}

// ========== Material Evaluation Tests ==========

TEST_F(EvaluatorTest, MaterialEvaluationEqual) {
   GameStateSnapshot snapshot = createTestSnapshot(5, 5);

   EvaluationContext context;
   context.perspectivePlayer = 0;
   context.materialWeight = 1.0f;
   context.positionWeight = 0.0f;
   context.healthWeight = 0.0f;
   context.threatWeight = 0.0f;

   auto result = evaluator->evaluate(snapshot, context);

   // Equal material should give ~0.0 score
   EXPECT_NEAR(result.materialScore, 0.0f, 0.1f);
}

TEST_F(EvaluatorTest, MaterialEvaluationPlayerAdvantage) {
   GameStateSnapshot snapshot = createTestSnapshot(10, 5);

   EvaluationContext context;
   context.perspectivePlayer = 0;

   auto result = evaluator->evaluate(snapshot, context);

   // More player units = positive material score
   EXPECT_GT(result.materialScore, 0.0f);
}

TEST_F(EvaluatorTest, MaterialEvaluationEnemyAdvantage) {
   GameStateSnapshot snapshot = createTestSnapshot(3, 10);

   EvaluationContext context;
   context.perspectivePlayer = 0;

   auto result = evaluator->evaluate(snapshot, context);

   // More enemy units = negative material score
   EXPECT_LT(result.materialScore, 0.0f);
}

TEST_F(EvaluatorTest, MaterialEvaluationHPWeighting) {
   GameStateSnapshot snapshot = createTestSnapshot(2, 2);

   // Damage player unit
   snapshot.units[0].damage = 50;  // 50% HP

   EvaluationContext context;
   context.perspectivePlayer = 0;

   auto result = evaluator->evaluate(snapshot, context);

   // Damaged unit should reduce material score
   EXPECT_LT(result.materialScore, 0.0f);
}

// ========== Health Evaluation Tests ==========

TEST_F(EvaluatorTest, HealthEvaluationFullHP) {
   GameStateSnapshot snapshot = createTestSnapshot(5, 5);

   EvaluationContext context;
   context.perspectivePlayer = 0;

   auto result = evaluator->evaluate(snapshot, context);

   // Both at full HP = ~0.0 health score
   EXPECT_NEAR(result.healthScore, 0.0f, 0.1f);
}

TEST_F(EvaluatorTest, HealthEvaluationPlayerDamaged) {
   GameStateSnapshot snapshot = createTestSnapshot(3, 3);

   // Damage all player units
   for (auto& unit : snapshot.units) {
      if (unit.owner == 0) {
         unit.damage = 50;  // 50% HP
      }
   }

   EvaluationContext context;
   context.perspectivePlayer = 0;

   auto result = evaluator->evaluate(snapshot, context);

   // Player damaged, enemy healthy = negative health score
   EXPECT_LT(result.healthScore, -0.3f);
}

TEST_F(EvaluatorTest, HealthEvaluationEnemyDamaged) {
   GameStateSnapshot snapshot = createTestSnapshot(3, 3);

   // Damage all enemy units
   for (auto& unit : snapshot.units) {
      if (unit.owner == 1) {
         unit.damage = 60;  // 40% HP
      }
   }

   EvaluationContext context;
   context.perspectivePlayer = 0;

   auto result = evaluator->evaluate(snapshot, context);

   // Enemy damaged, player healthy = positive health score
   EXPECT_GT(result.healthScore, 0.3f);
}

// ========== Position Evaluation Tests ==========

TEST_F(EvaluatorTest, PositionEvaluationHeightAdvantage) {
   GameStateSnapshot snapshot = createTestSnapshot(3, 3);

   // Give player units height advantage
   for (auto& unit : snapshot.units) {
      if (unit.owner == 0) {
         unit.height = 5;  // High ground
      } else {
         unit.height = 0;  // Low ground
      }
   }

   EvaluationContext context;
   context.perspectivePlayer = 0;

   auto result = evaluator->evaluate(snapshot, context);

   // Height advantage should give positive position score
   EXPECT_GT(result.positionScore, 0.0f);
}

// ========== Threat Evaluation Tests ==========

TEST_F(EvaluatorTest, ThreatEvaluationNoThreats) {
   GameStateSnapshot snapshot;
   snapshot.mapWidth = 100;
   snapshot.mapHeight = 100;

   // Player units at one side
   for (int i = 0; i < 3; ++i) {
      UnitSnapshot unit;
      unit.networkID = i;
      unit.owner = 0;
      unit.x = 10 + i;
      unit.y = 10;
      snapshot.addUnit(unit);
   }

   // Enemy units far away (>10 hexes)
   for (int i = 0; i < 3; ++i) {
      UnitSnapshot unit;
      unit.networkID = 10 + i;
      unit.owner = 1;
      unit.x = 80 + i;
      unit.y = 80;
      snapshot.addUnit(unit);
   }

   EvaluationContext context;
   context.perspectivePlayer = 0;

   auto result = evaluator->evaluate(snapshot, context);

   // No threats = high threat score (positive)
   EXPECT_GT(result.threatScore, 0.5f);
}

TEST_F(EvaluatorTest, ThreatEvaluationAllThreatened) {
   GameStateSnapshot snapshot;
   snapshot.mapWidth = 100;
   snapshot.mapHeight = 100;

   // Player units
   for (int i = 0; i < 3; ++i) {
      UnitSnapshot unit;
      unit.networkID = i;
      unit.owner = 0;
      unit.x = 50 + i;
      unit.y = 50;
      snapshot.addUnit(unit);
   }

   // Enemy units nearby (<10 hexes)
   for (int i = 0; i < 3; ++i) {
      UnitSnapshot unit;
      unit.networkID = 10 + i;
      unit.owner = 1;
      unit.x = 55 + i;
      unit.y = 55;
      snapshot.addUnit(unit);
   }

   EvaluationContext context;
   context.perspectivePlayer = 0;

   auto result = evaluator->evaluate(snapshot, context);

   // All units threatened = low threat score (negative)
   EXPECT_LT(result.threatScore, 0.0f);
}

// ========== Weighted Combination Tests ==========

TEST_F(EvaluatorTest, WeightedCombinationMaterialOnly) {
   GameStateSnapshot snapshot = createTestSnapshot(10, 5);

   EvaluationContext context;
   context.perspectivePlayer = 0;
   context.materialWeight = 1.0f;
   context.positionWeight = 0.0f;
   context.healthWeight = 0.0f;
   context.threatWeight = 0.0f;

   auto result = evaluator->evaluate(snapshot, context);

   // Only material should matter
   EXPECT_NEAR(result.score, result.materialScore, 0.01f);
}

TEST_F(EvaluatorTest, WeightedCombinationAllWeights) {
   GameStateSnapshot snapshot = createTestSnapshot(5, 5);

   EvaluationContext context;
   context.perspectivePlayer = 0;
   context.materialWeight = 1.0f;
   context.positionWeight = 0.5f;
   context.healthWeight = 0.8f;
   context.threatWeight = 0.6f;

   auto result = evaluator->evaluate(snapshot, context);

   // Score should be in valid range
   EXPECT_GE(result.score, -1.0f);
   EXPECT_LE(result.score, 1.0f);
}

TEST_F(EvaluatorTest, WeightedCombinationCustomWeights) {
   GameStateSnapshot snapshot = createTestSnapshot(10, 5);

   EvaluationContext context;
   context.perspectivePlayer = 0;
   context.materialWeight = 2.0f;  // Double weight on material
   context.positionWeight = 0.1f;
   context.healthWeight = 0.1f;
   context.threatWeight = 0.1f;

   auto result = evaluator->evaluate(snapshot, context);

   // Material should dominate
   EXPECT_GT(result.score, 0.0f);  // Player has material advantage
}

// ========== Terminal State Result Tests ==========

TEST_F(EvaluatorTest, TerminalStatePlayerWins) {
   GameStateSnapshot snapshot = createTestSnapshot(5, 0);

   EvaluationContext context;
   context.perspectivePlayer = 0;

   auto result = evaluator->evaluate(snapshot, context);

   EXPECT_TRUE(result.isTerminal);
   EXPECT_NEAR(result.score, 1.0f, 0.01f);
}

TEST_F(EvaluatorTest, TerminalStatePlayerLoses) {
   GameStateSnapshot snapshot = createTestSnapshot(0, 5);

   EvaluationContext context;
   context.perspectivePlayer = 0;

   auto result = evaluator->evaluate(snapshot, context);

   EXPECT_TRUE(result.isTerminal);
   EXPECT_NEAR(result.score, -1.0f, 0.01f);
}

TEST_F(EvaluatorTest, TerminalStateDraw) {
   GameStateSnapshot snapshot = createTestSnapshot(0, 0);

   EvaluationContext context;
   context.perspectivePlayer = 0;

   auto result = evaluator->evaluate(snapshot, context);

   EXPECT_TRUE(result.isTerminal);
   EXPECT_NEAR(result.score, 0.0f, 0.01f);
}

// ========== Edge Cases ==========

TEST_F(EvaluatorTest, EdgeCaseSingleUnit) {
   GameStateSnapshot snapshot = createTestSnapshot(1, 1);

   EvaluationContext context;
   context.perspectivePlayer = 0;

   auto result = evaluator->evaluate(snapshot, context);

   // Should not crash
   EXPECT_GE(result.score, -1.0f);
   EXPECT_LE(result.score, 1.0f);
}

TEST_F(EvaluatorTest, EdgeCaseManyUnits) {
   GameStateSnapshot snapshot = createTestSnapshot(50, 50);

   EvaluationContext context;
   context.perspectivePlayer = 0;

   auto result = evaluator->evaluate(snapshot, context);

   // Should handle large numbers
   EXPECT_GE(result.score, -1.0f);
   EXPECT_LE(result.score, 1.0f);
   EXPECT_EQ(result.unitsEvaluated, 100);
}

TEST_F(EvaluatorTest, EdgeCaseAllUnitsDamaged) {
   GameStateSnapshot snapshot = createTestSnapshot(5, 5);

   // Damage all units heavily
   for (auto& unit : snapshot.units) {
      unit.damage = 90;  // 10% HP
   }

   EvaluationContext context;
   context.perspectivePlayer = 0;

   auto result = evaluator->evaluate(snapshot, context);

   // Should still work with low HP
   EXPECT_GE(result.score, -1.0f);
   EXPECT_LE(result.score, 1.0f);
}

// ========== Main ==========

int main(int argc, char** argv) {
   ::testing::InitGoogleTest(&argc, argv);
   return RUN_ALL_TESTS();
}
