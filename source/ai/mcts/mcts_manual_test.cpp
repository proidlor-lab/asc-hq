/***************************************************************************
 * mcts_manual_test.cpp - Manual integration test implementation
 ***************************************************************************/

#include "mcts_manual_test.h"
#include "core/mcts_search.h"
#include "domain/i_tactical_evaluator.h"
#include "domain/game_state_reader.h"
#include "../../gamemap.h"
#include "../../vehicle.h"
#include "../../mapfield.h"
#include <iostream>
#include <iomanip>

namespace asc {
namespace mcts {

bool runMCTSManualTest(GameMap* gameMap, int iterations, int playerID) {
   if (!gameMap) {
      std::cout << "ERROR: gameMap is null" << std::endl;
      return false;
   }

   std::cout << "\n";
   std::cout << "╔═══════════════════════════════════════════════════════════╗\n";
   std::cout << "║         MCTS AI - Manual Integration Test                ║\n";
   std::cout << "╚═══════════════════════════════════════════════════════════╝\n";
   std::cout << "\n";

   try {
      // Determine player
      int player = (playerID >= 0) ? playerID : gameMap->actplayer;
      std::cout << "Testing for Player: " << player << "\n";
      std::cout << "Map Size: " << gameMap->xsize << "x" << gameMap->ysize << "\n";

      // Count units
      int playerUnits = 0;
      int totalUnits = 0;
      for (int p = 0; p < 8; ++p) {
         int count = gameMap->player[p].vehicleList.size();
         totalUnits += count;
         if (p == player) {
            playerUnits = count;
         }
      }
      std::cout << "Player " << player << " units: " << playerUnits << "\n";
      std::cout << "Total units on map: " << totalUnits << "\n";
      std::cout << "\n";

      if (playerUnits == 0) {
         std::cout << "⚠️  WARNING: Player has no units to move!\n";
         return false;
      }

      // Create game state reader
      std::cout << "1. Creating game state snapshot...\n";
      GameStateReader reader(gameMap);
      auto snapshot = reader.createFullSnapshot(static_cast<PlayerID>(player));
      std::cout << "   ✓ Snapshot created: " << snapshot->units.size() << " units\n";
      std::cout << "\n";

      // Create evaluator
      std::cout << "2. Creating tactical evaluator...\n";
      auto evaluator = EvaluatorFactory::createSimpleCombatEvaluator();
      std::cout << "   ✓ SimpleCombatEvaluator created\n";
      std::cout << "\n";

      // Configure MCTS
      MCTSConfig config;
      config.maxIterations = iterations;
      config.maxTimeMs = 5000;  // 5 second timeout
      config.rolloutDepthLimit = 10;
      config.explorationConstant = 1.414;  // sqrt(2)

      std::cout << "3. Configuring MCTS search...\n";
      std::cout << "   - Max iterations: " << config.maxIterations << "\n";
      std::cout << "   - Time limit: " << config.maxTimeMs << " ms\n";
      std::cout << "   - Rollout depth: " << config.rolloutDepthLimit << "\n";
      std::cout << "   - Exploration: " << config.explorationConstant << "\n";
      std::cout << "\n";

      // Create MCTS search
      MCTSSearch search(std::move(evaluator), config);

      // Run search
      std::cout << "4. Running MCTS search...\n";
      auto startTime = std::chrono::high_resolution_clock::now();
      auto result = search.search(*snapshot, static_cast<PlayerID>(player));
      auto endTime = std::chrono::high_resolution_clock::now();
      auto elapsed =
         std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

      std::cout << "   ✓ Search completed in " << elapsed << " ms\n";
      std::cout << "\n";

      // Display results
      std::cout << "═══════════════════════════════════════════════════════════\n";
      std::cout << "                    SEARCH RESULTS                         \n";
      std::cout << "═══════════════════════════════════════════════════════════\n";
      std::cout << "\n";

      std::cout << "Search Statistics:\n";
      std::cout << "  Iterations run:    " << result.iterationsRun << " / " << config.maxIterations
                << "\n";
      std::cout << "  Nodes expanded:    " << result.nodesExpanded << "\n";
      std::cout << "  Total rollouts:    " << result.totalRollouts << "\n";
      std::cout << "  Search time:       " << result.searchTimeMs << " ms\n";
      std::cout << "  Tree depth:        " << result.treeDepth << "\n";
      std::cout << "  Tree nodes:        " << result.treeNodeCount << "\n";
      std::cout << "\n";

      if (result.bestAction.has_value()) {
         std::cout << "✓ BEST ACTION FOUND:\n";
         std::cout << "  Visits:            " << result.bestMoveVisits << "\n";
         std::cout << "  Average value:     " << std::fixed << std::setprecision(3)
                   << result.bestMoveValue << "\n";
         std::cout << "  Win rate:          " << std::fixed << std::setprecision(1)
                   << (result.bestMoveWinRate * 100.0) << "%\n";
         std::cout << "\n";

         // Display action details
         const auto& action = result.bestAction.value();
         std::visit(
            [&](const auto& act) {
               using T = std::decay_t<decltype(act)>;
               if constexpr (std::is_same_v<T, MoveAction>) {
                  const Vehicle* unit = gameMap->getUnit(act.unitID);
                  std::cout << "  Action Type:       MOVE\n";
                  std::cout << "  Unit ID:           " << act.unitID;
                  if (unit) {
                     std::cout << " (" << unit->typ->name << ")";
                  }
                  std::cout << "\n";
                  std::cout << "  Target position:   (" << act.destination.x << ", "
                            << act.destination.y << ")\n";
               } else if constexpr (std::is_same_v<T, AttackAction>) {
                  const Vehicle* attacker = gameMap->getUnit(act.attackerID);
                  const MapField* targetField = gameMap->getField(act.target.x, act.target.y);
                  const Vehicle* targetUnit = targetField ? targetField->vehicle : nullptr;
                  std::cout << "  Action Type:       ATTACK\n";
                  std::cout << "  Attacker ID:       " << act.attackerID;
                  if (attacker) {
                     std::cout << " (" << attacker->typ->name << ")";
                  }
                  std::cout << "\n";
                  std::cout << "  Target Position:   (" << act.target.x << ", " << act.target.y
                            << ")";
                  if (targetUnit) {
                     std::cout << " [Unit: " << targetUnit->networkid << " - "
                               << targetUnit->typ->name << "]";
                  }
                  std::cout << "\n";
               } else if constexpr (std::is_same_v<T, WaitAction>) {
                  const Vehicle* unit = gameMap->getUnit(act.unitID);
                  std::cout << "  Action Type:       WAIT\n";
                  std::cout << "  Unit ID:           " << act.unitID;
                  if (unit) {
                     std::cout << " (" << unit->typ->name << ")";
                  }
                  std::cout << "\n";
               }
            },
            action);
      } else {
         std::cout << "⚠️  NO ACTION FOUND\n";
         std::cout << "  (No legal moves available or search failed)\n";
      }

      std::cout << "\n";
      std::cout << "═══════════════════════════════════════════════════════════\n";
      std::cout << "✓ TEST COMPLETED SUCCESSFULLY\n";
      std::cout << "═══════════════════════════════════════════════════════════\n";
      std::cout << "\n";

      return true;

   } catch (const std::exception& e) {
      std::cout << "\n";
      std::cout << "✗ ERROR: " << e.what() << "\n";
      std::cout << "\n";
      return false;
   }
}

bool mctsQuickSanityCheck() {
   std::cout << "\n";
   std::cout << "╔═══════════════════════════════════════════════════════════╗\n";
   std::cout << "║         MCTS AI - Quick Sanity Check                     ║\n";
   std::cout << "╚═══════════════════════════════════════════════════════════╝\n";
   std::cout << "\n";

   try {
      // Test 1: Create evaluator
      std::cout << "1. Testing evaluator factory...\n";
      auto evaluator = EvaluatorFactory::createSimpleCombatEvaluator();
      if (!evaluator) {
         std::cout << "   ✗ Failed to create evaluator\n";
         return false;
      }
      std::cout << "   ✓ SimpleCombatEvaluator created\n";

      // Test 2: Create test snapshot
      std::cout << "2. Testing snapshot creation...\n";
      GameStateSnapshot snapshot;
      snapshot.mapWidth = 50;
      snapshot.mapHeight = 50;
      snapshot.currentPlayer = 0;
      snapshot.perspective = 0;

      UnitSnapshot unit;
      unit.networkID = 1;
      unit.owner = 0;
      unit.x = 10;
      unit.y = 10;
      unit.damage = 0;
      unit.movement = 100;
      snapshot.addUnit(unit);
      std::cout << "   ✓ Test snapshot created with " << snapshot.units.size() << " units\n";

      // Test 3: Test evaluation
      std::cout << "3. Testing state evaluation...\n";
      EvaluationContext evalCtx;
      evalCtx.perspectivePlayer = 0;
      auto evalResult = evaluator->evaluate(snapshot, evalCtx);
      std::cout << "   ✓ Evaluation score: " << evalResult.score << "\n";

      // Test 4: Test MCTS config
      std::cout << "4. Testing MCTS configuration...\n";
      MCTSConfig config;
      config.maxIterations = 10;
      config.maxTimeMs = 1000;
      std::cout << "   ✓ MCTS config created\n";

      // Test 5: Create MCTS search
      std::cout << "5. Testing MCTS search engine...\n";
      MCTSSearch search(std::move(evaluator), config);
      std::cout << "   ✓ MCTS search engine created\n";

      std::cout << "\n";
      std::cout << "═══════════════════════════════════════════════════════════\n";
      std::cout << "✓ ALL COMPONENTS INITIALIZED SUCCESSFULLY\n";
      std::cout << "═══════════════════════════════════════════════════════════\n";
      std::cout << "\n";

      return true;

   } catch (const std::exception& e) {
      std::cout << "\n";
      std::cout << "✗ SANITY CHECK FAILED: " << e.what() << "\n";
      std::cout << "\n";
      return false;
   }
}

}  // namespace mcts
}  // namespace asc
