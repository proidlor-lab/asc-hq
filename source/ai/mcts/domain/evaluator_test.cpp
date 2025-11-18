/***************************************************************************
 * evaluator_test.cpp - Unit tests for tactical evaluators
 *
 * Part of: ASC MCTS AI (Phase 0.3 - Basic Evaluation Function)
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
 ***************************************************************************/

#include "i_tactical_evaluator.h"
#include "simple_combat_evaluator.h"
#include "game_state_snapshot.h"
#include <iostream>
#include <cassert>
#include <cmath>
#include <iomanip>

using namespace asc::mcts;

// ========== Test Utilities ==========

int g_testsPassed = 0;
int g_testsFailed = 0;

#define TEST(name)                                                                   \
   void test_##name();                                                               \
   struct TestRunner_##name {                                                        \
      TestRunner_##name() {                                                          \
         std::cout << "[ TEST ] " << #name << std::endl;                             \
         try {                                                                       \
            test_##name();                                                           \
            std::cout << "[ PASS ] " << #name << std::endl;                          \
            g_testsPassed++;                                                         \
         } catch (const std::exception& e) {                                         \
            std::cout << "[ FAIL ] " << #name << ": " << e.what() << std::endl;      \
            g_testsFailed++;                                                         \
         } catch (...) {                                                             \
            std::cout << "[ FAIL ] " << #name << ": unknown exception" << std::endl; \
            g_testsFailed++;                                                         \
         }                                                                           \
      }                                                                              \
   } g_testRunner_##name;                                                            \
   void test_##name()

#define ASSERT(condition)                                                       \
   if (!(condition)) {                                                          \
      throw std::runtime_error(std::string("Assertion failed: ") + #condition); \
   }

#define ASSERT_NEAR(a, b, epsilon)                                                              \
   if (std::abs((a) - (b)) > (epsilon)) {                                                       \
      throw std::runtime_error(std::string("Values not close: ") + std::to_string(a) + " vs " + \
                               std::to_string(b));                                              \
   }

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

// ========== Factory Tests ==========

TEST(FactoryCreation) {
   auto evaluator = EvaluatorFactory::createSimpleCombatEvaluator();
   ASSERT(evaluator != nullptr);
   ASSERT(std::string(evaluator->getName()) == "SimpleCombatEvaluator");
}

TEST(FactoryDefault) {
   auto evaluator = EvaluatorFactory::createDefault();
   ASSERT(evaluator != nullptr);
}

TEST(EvaluatorClone) {
   auto evaluator = EvaluatorFactory::createSimpleCombatEvaluator();
   auto clone = evaluator->clone();
   ASSERT(clone != nullptr);
   ASSERT(std::string(clone->getName()) == "SimpleCombatEvaluator");
}

// ========== Terminal State Tests ==========

TEST(TerminalState_NoUnits) {
   GameStateSnapshot snapshot = createTestSnapshot(0, 0);
   auto evaluator = EvaluatorFactory::createSimpleCombatEvaluator();

   ASSERT(evaluator->isTerminalState(snapshot, 0));
}

TEST(TerminalState_NoPlayerUnits) {
   GameStateSnapshot snapshot = createTestSnapshot(0, 5);
   auto evaluator = EvaluatorFactory::createSimpleCombatEvaluator();

   ASSERT(evaluator->isTerminalState(snapshot, 0));
}

TEST(TerminalState_NoEnemyUnits) {
   GameStateSnapshot snapshot = createTestSnapshot(5, 0);
   auto evaluator = EvaluatorFactory::createSimpleCombatEvaluator();

   ASSERT(evaluator->isTerminalState(snapshot, 0));
}

TEST(TerminalState_BothHaveUnits) {
   GameStateSnapshot snapshot = createTestSnapshot(5, 5);
   auto evaluator = EvaluatorFactory::createSimpleCombatEvaluator();

   ASSERT(!evaluator->isTerminalState(snapshot, 0));
}

// ========== Material Evaluation Tests ==========

TEST(MaterialEvaluation_Equal) {
   GameStateSnapshot snapshot = createTestSnapshot(5, 5);
   auto evaluator = EvaluatorFactory::createSimpleCombatEvaluator();

   EvaluationContext context;
   context.perspectivePlayer = 0;
   context.materialWeight = 1.0f;
   context.positionWeight = 0.0f;
   context.healthWeight = 0.0f;
   context.threatWeight = 0.0f;

   auto result = evaluator->evaluate(snapshot, context);

   // Equal material should give ~0.0 score
   ASSERT_NEAR(result.materialScore, 0.0f, 0.1f);
}

TEST(MaterialEvaluation_PlayerAdvantage) {
   GameStateSnapshot snapshot = createTestSnapshot(10, 5);
   auto evaluator = EvaluatorFactory::createSimpleCombatEvaluator();

   EvaluationContext context;
   context.perspectivePlayer = 0;

   auto result = evaluator->evaluate(snapshot, context);

   // More player units = positive material score
   ASSERT(result.materialScore > 0.0f);
}

TEST(MaterialEvaluation_EnemyAdvantage) {
   GameStateSnapshot snapshot = createTestSnapshot(3, 10);
   auto evaluator = EvaluatorFactory::createSimpleCombatEvaluator();

   EvaluationContext context;
   context.perspectivePlayer = 0;

   auto result = evaluator->evaluate(snapshot, context);

   // More enemy units = negative material score
   ASSERT(result.materialScore < 0.0f);
}

TEST(MaterialEvaluation_HPWeighting) {
   GameStateSnapshot snapshot = createTestSnapshot(2, 2);

   // Damage player unit
   snapshot.units[0].damage = 50;  // 50% HP

   auto evaluator = EvaluatorFactory::createSimpleCombatEvaluator();
   EvaluationContext context;
   context.perspectivePlayer = 0;

   auto result = evaluator->evaluate(snapshot, context);

   // Damaged unit should reduce material score
   ASSERT(result.materialScore < 0.0f);
}

// ========== Health Evaluation Tests ==========

TEST(HealthEvaluation_FullHP) {
   GameStateSnapshot snapshot = createTestSnapshot(5, 5);
   auto evaluator = EvaluatorFactory::createSimpleCombatEvaluator();

   EvaluationContext context;
   context.perspectivePlayer = 0;

   auto result = evaluator->evaluate(snapshot, context);

   // Both at full HP = ~0.0 health score
   ASSERT_NEAR(result.healthScore, 0.0f, 0.1f);
}

TEST(HealthEvaluation_PlayerDamaged) {
   GameStateSnapshot snapshot = createTestSnapshot(3, 3);

   // Damage all player units
   for (auto& unit : snapshot.units) {
      if (unit.owner == 0) {
         unit.damage = 50;  // 50% HP
      }
   }

   auto evaluator = EvaluatorFactory::createSimpleCombatEvaluator();
   EvaluationContext context;
   context.perspectivePlayer = 0;

   auto result = evaluator->evaluate(snapshot, context);

   // Player damaged, enemy healthy = negative health score
   ASSERT(result.healthScore < -0.3f);
}

TEST(HealthEvaluation_EnemyDamaged) {
   GameStateSnapshot snapshot = createTestSnapshot(3, 3);

   // Damage all enemy units
   for (auto& unit : snapshot.units) {
      if (unit.owner == 1) {
         unit.damage = 60;  // 40% HP
      }
   }

   auto evaluator = EvaluatorFactory::createSimpleCombatEvaluator();
   EvaluationContext context;
   context.perspectivePlayer = 0;

   auto result = evaluator->evaluate(snapshot, context);

   // Enemy damaged, player healthy = positive health score
   ASSERT(result.healthScore > 0.3f);
}

// ========== Position Evaluation Tests ==========

TEST(PositionEvaluation_HeightAdvantage) {
   GameStateSnapshot snapshot = createTestSnapshot(3, 3);

   // Give player units height advantage
   for (auto& unit : snapshot.units) {
      if (unit.owner == 0) {
         unit.height = 5;  // High ground
      } else {
         unit.height = 0;  // Low ground
      }
   }

   auto evaluator = EvaluatorFactory::createSimpleCombatEvaluator();
   EvaluationContext context;
   context.perspectivePlayer = 0;

   auto result = evaluator->evaluate(snapshot, context);

   // Height advantage should give positive position score
   ASSERT(result.positionScore > 0.0f);
}

// ========== Threat Evaluation Tests ==========

TEST(ThreatEvaluation_NoThreats) {
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

   auto evaluator = EvaluatorFactory::createSimpleCombatEvaluator();
   EvaluationContext context;
   context.perspectivePlayer = 0;

   auto result = evaluator->evaluate(snapshot, context);

   // No threats = high threat score (positive)
   ASSERT(result.threatScore > 0.5f);
}

TEST(ThreatEvaluation_AllThreatened) {
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

   auto evaluator = EvaluatorFactory::createSimpleCombatEvaluator();
   EvaluationContext context;
   context.perspectivePlayer = 0;

   auto result = evaluator->evaluate(snapshot, context);

   // All units threatened = low threat score (negative)
   ASSERT(result.threatScore < 0.0f);
}

// ========== Weighted Combination Tests ==========

TEST(WeightedCombination_MaterialOnly) {
   GameStateSnapshot snapshot = createTestSnapshot(10, 5);
   auto evaluator = EvaluatorFactory::createSimpleCombatEvaluator();

   EvaluationContext context;
   context.perspectivePlayer = 0;
   context.materialWeight = 1.0f;
   context.positionWeight = 0.0f;
   context.healthWeight = 0.0f;
   context.threatWeight = 0.0f;

   auto result = evaluator->evaluate(snapshot, context);

   // Only material should matter
   ASSERT_NEAR(result.score, result.materialScore, 0.01f);
}

TEST(WeightedCombination_AllWeights) {
   GameStateSnapshot snapshot = createTestSnapshot(5, 5);
   auto evaluator = EvaluatorFactory::createSimpleCombatEvaluator();

   EvaluationContext context;
   context.perspectivePlayer = 0;
   context.materialWeight = 1.0f;
   context.positionWeight = 0.5f;
   context.healthWeight = 0.8f;
   context.threatWeight = 0.6f;

   auto result = evaluator->evaluate(snapshot, context);

   // Score should be in valid range
   ASSERT(result.score >= -1.0f && result.score <= 1.0f);
}

TEST(WeightedCombination_CustomWeights) {
   GameStateSnapshot snapshot = createTestSnapshot(10, 5);
   auto evaluator = EvaluatorFactory::createSimpleCombatEvaluator();

   EvaluationContext context;
   context.perspectivePlayer = 0;
   context.materialWeight = 2.0f;  // Double weight on material
   context.positionWeight = 0.1f;
   context.healthWeight = 0.1f;
   context.threatWeight = 0.1f;

   auto result = evaluator->evaluate(snapshot, context);

   // Material should dominate
   ASSERT(result.score > 0.0f);  // Player has material advantage
}

// ========== Terminal State Result Tests ==========

TEST(TerminalState_PlayerWins) {
   GameStateSnapshot snapshot = createTestSnapshot(5, 0);
   auto evaluator = EvaluatorFactory::createSimpleCombatEvaluator();

   EvaluationContext context;
   context.perspectivePlayer = 0;

   auto result = evaluator->evaluate(snapshot, context);

   ASSERT(result.isTerminal);
   ASSERT_NEAR(result.score, 1.0f, 0.01f);
}

TEST(TerminalState_PlayerLoses) {
   GameStateSnapshot snapshot = createTestSnapshot(0, 5);
   auto evaluator = EvaluatorFactory::createSimpleCombatEvaluator();

   EvaluationContext context;
   context.perspectivePlayer = 0;

   auto result = evaluator->evaluate(snapshot, context);

   ASSERT(result.isTerminal);
   ASSERT_NEAR(result.score, -1.0f, 0.01f);
}

TEST(TerminalState_Draw) {
   GameStateSnapshot snapshot = createTestSnapshot(0, 0);
   auto evaluator = EvaluatorFactory::createSimpleCombatEvaluator();

   EvaluationContext context;
   context.perspectivePlayer = 0;

   auto result = evaluator->evaluate(snapshot, context);

   ASSERT(result.isTerminal);
   ASSERT_NEAR(result.score, 0.0f, 0.01f);
}

// ========== Edge Cases ==========

TEST(EdgeCase_SingleUnit) {
   GameStateSnapshot snapshot = createTestSnapshot(1, 1);
   auto evaluator = EvaluatorFactory::createSimpleCombatEvaluator();

   EvaluationContext context;
   context.perspectivePlayer = 0;

   auto result = evaluator->evaluate(snapshot, context);

   // Should not crash
   ASSERT(result.score >= -1.0f && result.score <= 1.0f);
}

TEST(EdgeCase_ManyUnits) {
   GameStateSnapshot snapshot = createTestSnapshot(50, 50);
   auto evaluator = EvaluatorFactory::createSimpleCombatEvaluator();

   EvaluationContext context;
   context.perspectivePlayer = 0;

   auto result = evaluator->evaluate(snapshot, context);

   // Should handle large numbers
   ASSERT(result.score >= -1.0f && result.score <= 1.0f);
   ASSERT(result.unitsEvaluated == 100);
}

TEST(EdgeCase_AllUnitsDamaged) {
   GameStateSnapshot snapshot = createTestSnapshot(5, 5);

   // Damage all units heavily
   for (auto& unit : snapshot.units) {
      unit.damage = 90;  // 10% HP
   }

   auto evaluator = EvaluatorFactory::createSimpleCombatEvaluator();
   EvaluationContext context;
   context.perspectivePlayer = 0;

   auto result = evaluator->evaluate(snapshot, context);

   // Should still work with low HP
   ASSERT(result.score >= -1.0f && result.score <= 1.0f);
}

// ========== Main ==========

int main() {
   std::cout << "========================================" << std::endl;
   std::cout << "Tactical Evaluator Tests (Phase 0.3)" << std::endl;
   std::cout << "========================================" << std::endl;
   std::cout << std::endl;

   // Tests run via static constructors

   std::cout << std::endl;
   std::cout << "========================================" << std::endl;
   std::cout << "Test Summary:" << std::endl;
   std::cout << "  Passed: " << g_testsPassed << std::endl;
   std::cout << "  Failed: " << g_testsFailed << std::endl;
   std::cout << "  Total:  " << (g_testsPassed + g_testsFailed) << std::endl;
   std::cout << "========================================" << std::endl;
   std::cout << std::endl;

   if (g_testsFailed == 0) {
      std::cout << "✓ ALL TESTS PASSED" << std::endl;
      return 0;
   } else {
      std::cout << "✗ SOME TESTS FAILED" << std::endl;
      return 1;
   }
}
