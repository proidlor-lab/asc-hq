/***************************************************************************
 * mcts_test.cpp - Unit tests for MCTS engine
 * 
 * Part of: ASC MCTS AI (Phase 1 - Core MCTS Engine)
 * 
 * Test Coverage:
 * - MCTS Node creation and statistics
 * - UCB1 calculation
 * - Tree navigation
 * - Search execution
 * - Best move selection
 * - Integration with Phase 0 components
 ***************************************************************************/

#include "mcts_node.h"
#include "mcts_search.h"
#include "../domain/i_tactical_evaluator.h"
#include "../domain/game_state_snapshot.h"
#include <iostream>
#include <cassert>
#include <cmath>

using namespace asc::mcts;

// ========== Test Utilities ==========

int g_testsPassed = 0;
int g_testsFailed = 0;

#define TEST(name) \
    void test_##name(); \
    struct TestRunner_##name { \
        TestRunner_##name() { \
            std::cout << "[ TEST ] " << #name << std::endl; \
            try { \
                test_##name(); \
                std::cout << "[ PASS ] " << #name << std::endl; \
                g_testsPassed++; \
            } catch (const std::exception& e) { \
                std::cout << "[ FAIL ] " << #name << ": " << e.what() << std::endl; \
                g_testsFailed++; \
            } catch (...) { \
                std::cout << "[ FAIL ] " << #name << ": unknown exception" << std::endl; \
                g_testsFailed++; \
            } \
        } \
    } g_testRunner_##name; \
    void test_##name()

#define ASSERT(condition) \
    if (!(condition)) { \
        throw std::runtime_error(std::string("Assertion failed: ") + #condition); \
    }

#define ASSERT_NEAR(a, b, epsilon) \
    if (std::abs((a) - (b)) > (epsilon)) { \
        throw std::runtime_error(std::string("Values not close: ") + \
            std::to_string(a) + " vs " + std::to_string(b)); \
    }

// ========== Test Helpers ==========

GameStateSnapshot createTestState(int playerUnits, int enemyUnits) {
    GameStateSnapshot snapshot;
    snapshot.mapWidth = 50;
    snapshot.mapHeight = 50;
    snapshot.currentPlayer = 0;
    snapshot.perspective = 0;
    
    // Add player units
    for (int i = 0; i < playerUnits; ++i) {
        UnitSnapshot unit;
        unit.networkID = i;
        unit.owner = 0;
        unit.x = 10 + i * 2;
        unit.y = 10;
        unit.damage = 0;
        unit.movement = 100;
        unit.fuel = 1000;
        unit.attacked = false;
        unit.ammoMask = 0xFFFF;
        snapshot.addUnit(unit);
    }
    
    // Add enemy units
    for (int i = 0; i < enemyUnits; ++i) {
        UnitSnapshot unit;
        unit.networkID = playerUnits + i;
        unit.owner = 1;
        unit.x = 30 + i * 2;
        unit.y = 30;
        unit.damage = 0;
        unit.movement = 100;
        unit.fuel = 1000;
        unit.attacked = false;
        unit.ammoMask = 0xFFFF;
        snapshot.addUnit(unit);
    }
    
    return snapshot;
}

// ========== MCTS Node Tests ==========

TEST(NodeCreation_Root) {
    auto state = std::make_unique<GameStateSnapshot>(createTestState(3, 3));
    MCTSNode node(std::move(state), 0);
    
    ASSERT(node.isRoot());
    ASSERT(node.isLeaf());
    ASSERT(!node.hasChildren());
    ASSERT(!node.hasAction());
    ASSERT(node.getVisits() == 0);
    ASSERT(node.getDepth() == 0);
}

TEST(NodeCreation_Child) {
    auto rootState = std::make_unique<GameStateSnapshot>(createTestState(3, 3));
    MCTSNode root(std::move(rootState), 0);
    
    auto childState = std::make_unique<GameStateSnapshot>(createTestState(3, 3));
    MoveAction move{0, MapCoordinate(12, 10)};
    Action action = move;
    
    MCTSNode child(std::move(childState), &root, action, 0);
    
    ASSERT(!child.isRoot());
    ASSERT(child.isLeaf());
    ASSERT(child.hasAction());
    ASSERT(child.getParent() == &root);
    ASSERT(child.getDepth() == 1);
}

TEST(NodeStatistics_Update) {
    auto state = std::make_unique<GameStateSnapshot>(createTestState(3, 3));
    MCTSNode node(std::move(state), 0);
    
    // Initial state
    ASSERT(node.getVisits() == 0);
    ASSERT(node.getAverageValue() == 0.0);
    
    // Update with positive value
    node.update(0.5);
    ASSERT(node.getVisits() == 1);
    ASSERT_NEAR(node.getAverageValue(), 0.5, 0.001);
    
    // Update with negative value
    node.update(-0.3);
    ASSERT(node.getVisits() == 2);
    ASSERT_NEAR(node.getAverageValue(), 0.1, 0.001);  // (0.5 - 0.3) / 2
}

TEST(NodeUCB1_UnvisitedNode) {
    auto state = std::make_unique<GameStateSnapshot>(createTestState(3, 3));
    MCTSNode node(std::move(state), 0);
    
    // Unvisited nodes should have infinite UCB1
    double ucb1 = node.getUCB1Value();
    ASSERT(std::isinf(ucb1));
}

TEST(NodeUCB1_Calculation) {
    // Create parent
    auto parentState = std::make_unique<GameStateSnapshot>(createTestState(3, 3));
    MCTSNode parent(std::move(parentState), 0);
    parent.update(0.5);
    parent.update(0.3);  // 2 visits
    
    // Create child
    auto childState = std::make_unique<GameStateSnapshot>(createTestState(3, 3));
    MoveAction move{0, MapCoordinate(12, 10)};
    MCTSNode child(std::move(childState), &parent, Action(move), 0);
    child.update(0.6);  // 1 visit
    
    // UCB1 = exploitation + exploration
    // exploitation = 0.6
    // exploration = sqrt(2) * sqrt(ln(2) / 1) ≈ 1.177
    // UCB1 ≈ 1.777
    double ucb1 = child.getUCB1Value();
    ASSERT(ucb1 > 1.5);
    ASSERT(ucb1 < 2.0);
}

TEST(NodeTree_AddChild) {
    auto rootState = std::make_unique<GameStateSnapshot>(createTestState(3, 3));
    MCTSNode root(std::move(rootState), 0);
    
    ASSERT(root.getChildCount() == 0);
    
    // Add child
    auto childState = std::make_unique<GameStateSnapshot>(createTestState(3, 3));
    MoveAction move{0, MapCoordinate(12, 10)};
    auto child = std::make_unique<MCTSNode>(std::move(childState), &root, Action(move), 0);
    
    MCTSNode* childPtr = root.addChild(std::move(child));
    
    ASSERT(root.getChildCount() == 1);
    ASSERT(!root.isLeaf());
    ASSERT(root.hasChildren());
    ASSERT(childPtr != nullptr);
}

TEST(NodeTree_BestChildSelection) {
    auto rootState = std::make_unique<GameStateSnapshot>(createTestState(3, 3));
    MCTSNode root(std::move(rootState), 0);
    root.update(0.5);
    root.update(0.3);
    
    // Add three children with different statistics
    auto child1State = std::make_unique<GameStateSnapshot>(createTestState(3, 3));
    MoveAction move1{0, MapCoordinate(12, 10)};
    auto child1 = std::make_unique<MCTSNode>(std::move(child1State), &root, Action(move1), 0);
    child1->update(0.3);
    child1->update(0.4);  // 2 visits, avg = 0.35
    MCTSNode* child1Ptr = root.addChild(std::move(child1));
    
    auto child2State = std::make_unique<GameStateSnapshot>(createTestState(3, 3));
    MoveAction move2{1, MapCoordinate(14, 10)};
    auto child2 = std::make_unique<MCTSNode>(std::move(child2State), &root, Action(move2), 0);
    child2->update(0.8);
    child2->update(0.9);
    child2->update(0.7);  // 3 visits, avg = 0.8
    MCTSNode* child2Ptr = root.addChild(std::move(child2));
    
    auto child3State = std::make_unique<GameStateSnapshot>(createTestState(3, 3));
    MoveAction move3{2, MapCoordinate(16, 10)};
    auto child3 = std::make_unique<MCTSNode>(std::move(child3State), &root, Action(move3), 0);
    child3->update(0.6);  // 1 visit, avg = 0.6
    root.addChild(std::move(child3));
    
    // Best by visits: child2 (3 visits)
    MCTSNode* bestByVisits = root.selectBestChildByVisits();
    ASSERT(bestByVisits == child2Ptr);
    
    // Best by value: child2 (0.8)
    MCTSNode* bestByValue = root.selectBestChildByValue();
    ASSERT(bestByValue == child2Ptr);
}

// ========== MCTS Search Tests ==========

TEST(SearchCreation) {
    auto evaluator = asc::mcts::EvaluatorFactory::createSimpleCombatEvaluator();
    MCTSConfig config;
    config.maxIterations = 10;
    
    MCTSSearch search(std::move(evaluator), config);
    
    ASSERT(search.getRootNode() == nullptr);
}

TEST(SearchBasic_SimpleScenario) {
    auto state = createTestState(2, 2);
    
    auto evaluator = asc::mcts::EvaluatorFactory::createSimpleCombatEvaluator();
    MCTSConfig config;
    config.maxIterations = 50;
    config.maxTimeMs = 1000;
    config.rolloutDepthLimit = 5;
    
    MCTSSearch search(std::move(evaluator), config);
    
    auto result = search.search(state, 0);
    
    ASSERT(result.iterationsRun > 0);
    ASSERT(result.iterationsRun <= 50);
    ASSERT(result.nodesExpanded > 0);
    ASSERT(result.searchTimeMs < 1500);  // Should be well under time limit
}

TEST(SearchResult_BestAction) {
    auto state = createTestState(3, 2);
    
    auto evaluator = asc::mcts::EvaluatorFactory::createSimpleCombatEvaluator();
    MCTSConfig config;
    config.maxIterations = 100;
    config.rolloutDepthLimit = 3;
    
    MCTSSearch search(std::move(evaluator), config);
    
    auto result = search.search(state, 0);
    
    // Should find a best action
    ASSERT(result.bestAction.has_value());
    ASSERT(result.bestMoveVisits > 0);
}

TEST(SearchStatistics_TreeGrowth) {
    auto state = createTestState(2, 2);
    
    auto evaluator = asc::mcts::EvaluatorFactory::createSimpleCombatEvaluator();
    MCTSConfig config;
    config.maxIterations = 100;
    config.rolloutDepthLimit = 3;
    
    MCTSSearch search(std::move(evaluator), config);
    
    auto result = search.search(state, 0);
    
    ASSERT(result.treeNodeCount > 1);  // Should have root + children
    ASSERT(result.treeDepth > 0);
    ASSERT(result.totalRollouts == result.iterationsRun);
}

TEST(SearchConfig_IterationLimit) {
    auto state = createTestState(2, 2);
    
    auto evaluator = asc::mcts::EvaluatorFactory::createSimpleCombatEvaluator();
    MCTSConfig config;
    config.maxIterations = 20;
    config.maxTimeMs = 10000;  // High time limit
    
    MCTSSearch search(std::move(evaluator), config);
    
    auto result = search.search(state, 0);
    
    // Should stop at iteration limit
    ASSERT(result.iterationsRun <= 20);
}

TEST(SearchConfig_ExplorationConstant) {
    auto state = createTestState(3, 2);
    
    // High exploration
    auto evaluator1 = asc::mcts::EvaluatorFactory::createSimpleCombatEvaluator();
    MCTSConfig config1;
    config1.maxIterations = 50;
    config1.explorationConstant = 2.0;  // More exploration
    
    MCTSSearch search1(std::move(evaluator1), config1);
    auto result1 = search1.search(state, 0);
    
    // Low exploration
    auto evaluator2 = asc::mcts::EvaluatorFactory::createSimpleCombatEvaluator();
    MCTSConfig config2;
    config2.maxIterations = 50;
    config2.explorationConstant = 0.5;  // More exploitation
    
    MCTSSearch search2(std::move(evaluator2), config2);
    auto result2 = search2.search(state, 0);
    
    // Both should find actions
    ASSERT(result1.bestAction.has_value());
    ASSERT(result2.bestAction.has_value());
}

TEST(SearchIntegration_WithEvaluation) {
    // Create scenario where player has advantage
    auto state = createTestState(5, 2);  // Player has more units
    
    auto evaluator = asc::mcts::EvaluatorFactory::createSimpleCombatEvaluator();
    MCTSConfig config;
    config.maxIterations = 100;
    config.rolloutDepthLimit = 5;
    
    MCTSSearch search(std::move(evaluator), config);
    
    auto result = search.search(state, 0);
    
    // Should recognize advantage
    ASSERT(result.bestAction.has_value());
    ASSERT(result.bestMoveWinRate > 0.3);  // Should have decent win rate
}

TEST(SearchContinue) {
    auto state = createTestState(2, 2);
    
    auto evaluator = asc::mcts::EvaluatorFactory::createSimpleCombatEvaluator();
    MCTSConfig config;
    config.maxIterations = 20;
    
    MCTSSearch search(std::move(evaluator), config);
    
    // Initial search
    auto result1 = search.search(state, 0);
    int firstIterations = result1.iterationsRun;
    
    // Continue search
    auto result2 = search.continueSearch(30);
    
    // Should have more iterations total
    ASSERT(result2.iterationsRun > firstIterations);
    ASSERT(result2.treeNodeCount >= result1.treeNodeCount);
}

TEST(SearchReset) {
    auto state = createTestState(2, 2);
    
    auto evaluator = asc::mcts::EvaluatorFactory::createSimpleCombatEvaluator();
    MCTSConfig config;
    config.maxIterations = 20;
    
    MCTSSearch search(std::move(evaluator), config);
    
    search.search(state, 0);
    ASSERT(search.getRootNode() != nullptr);
    
    search.reset();
    ASSERT(search.getRootNode() == nullptr);
}

// ========== Edge Cases ==========

TEST(EdgeCase_NoUnits) {
    auto state = createTestState(0, 0);
    
    auto evaluator = asc::mcts::EvaluatorFactory::createSimpleCombatEvaluator();
    MCTSConfig config;
    config.maxIterations = 10;
    
    MCTSSearch search(std::move(evaluator), config);
    
    auto result = search.search(state, 0);
    
    // Should handle gracefully (terminal state)
    ASSERT(result.iterationsRun >= 0);
}

TEST(EdgeCase_SingleUnit) {
    auto state = createTestState(1, 0);
    
    auto evaluator = asc::mcts::EvaluatorFactory::createSimpleCombatEvaluator();
    MCTSConfig config;
    config.maxIterations = 20;
    
    MCTSSearch search(std::move(evaluator), config);
    
    auto result = search.search(state, 0);
    
    // Should work with minimal units
    ASSERT(result.iterationsRun > 0);
}

// ========== Main ==========

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "MCTS Engine Tests (Phase 1)" << std::endl;
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
