/***************************************************************************
 * mcts_search.cpp - Implementation of MCTS search algorithm
 ***************************************************************************/

#include "mcts_search.h"
#include "../domain/i_action_executor.h"
#include "../domain/simulation_action_executor.h"
#include <algorithm>
#include <random>
#include <chrono>

namespace asc {
namespace mcts {

// ========== Main Search API ==========

MCTSResult MCTSSearch::search(const GameStateSnapshot& initialState, 
                             PlayerID perspective) {
    // Reset previous search
    reset();
    
    // Create root node
    auto rootState = initialState.clone();
    root_ = std::make_unique<MCTSNode>(std::move(rootState), perspective);
    
    // Reset statistics
    iterationsRun_ = 0;
    nodesExpanded_ = 0;
    totalRollouts_ = 0;
    
    // Run search
    auto startTime = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < config_.maxIterations; ++i) {
        if (!runIteration()) {
            break;  // Error or no more actions
        }
        
        // Check time budget
        auto currentTime = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            currentTime - startTime).count();
        
        if (elapsed >= config_.maxTimeMs) {
            break;  // Time limit reached
        }
        
        // Check early termination
        if (config_.enableEarlyTermination && shouldTerminateEarly()) {
            break;  // Clear winner found
        }
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto searchTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        endTime - startTime).count();
    
    // Build result
    MCTSResult result;
    result.iterationsRun = iterationsRun_;
    result.nodesExpanded = nodesExpanded_;
    result.totalRollouts = totalRollouts_;
    result.searchTimeMs = static_cast<double>(searchTime);
    result.treeDepth = getTreeDepth();
    result.treeNodeCount = countNodes();
    
    // Get best action
    result.bestAction = getBestAction();
    
    if (result.bestAction.has_value() && root_) {
        auto* bestChild = root_->selectBestChildByVisits();
        if (bestChild) {
            result.bestMoveVisits = bestChild->getVisits();
            result.bestMoveValue = bestChild->getAverageValue();
            result.bestMoveWinRate = (bestChild->getAverageValue() + 1.0) / 2.0;
        }
    }
    
    return result;
}

MCTSResult MCTSSearch::continueSearch(int additionalIterations) {
    if (!root_) {
        return MCTSResult();  // No tree to continue
    }
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < additionalIterations; ++i) {
        if (!runIteration()) {
            break;
        }
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto searchTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        endTime - startTime).count();
    
    // Build result (same as search())
    MCTSResult result;
    result.iterationsRun = iterationsRun_;
    result.nodesExpanded = nodesExpanded_;
    result.totalRollouts = totalRollouts_;
    result.searchTimeMs = static_cast<double>(searchTime);
    result.treeDepth = getTreeDepth();
    result.treeNodeCount = countNodes();
    result.bestAction = getBestAction();
    
    if (result.bestAction.has_value() && root_) {
        auto* bestChild = root_->selectBestChildByVisits();
        if (bestChild) {
            result.bestMoveVisits = bestChild->getVisits();
            result.bestMoveValue = bestChild->getAverageValue();
            result.bestMoveWinRate = (bestChild->getAverageValue() + 1.0) / 2.0;
        }
    }
    
    return result;
}

std::optional<Action> MCTSSearch::getBestAction() const {
    if (!root_ || !root_->hasChildren()) {
        return std::nullopt;
    }
    
    // Select child with most visits (most robust)
    auto* bestChild = root_->selectBestChildByVisits();
    if (!bestChild || !bestChild->hasAction()) {
        return std::nullopt;
    }
    
    return bestChild->getAction();
}

// ========== MCTS Phases ==========

MCTSNode* MCTSSearch::select(MCTSNode* node) {
    // Navigate tree using UCB1 until reaching leaf or expandable node
    while (node->hasChildren() && node->isFullyExpanded()) {
        node = node->selectBestChildByUCB1(config_.explorationConstant);
        if (!node) {
            return nullptr;  // Should not happen
        }
    }
    
    return node;
}

MCTSNode* MCTSSearch::expand(MCTSNode* node) {
    // Check if node should be expanded
    if (node->getVisits() < config_.minVisitsBeforeExpansion) {
        return node;  // Not enough visits yet, just rollout from here
    }
    
    // Get unexpanded actions
    auto unexpandedActions = getUnexpandedActions(node);
    
    if (unexpandedActions.empty()) {
        node->markFullyExpanded();
        return node;  // No more actions to try
    }
    
    // Limit number of children
    if (node->getChildCount() >= static_cast<size_t>(config_.maxChildrenPerNode)) {
        node->markFullyExpanded();
        return node;
    }
    
    // Select first unexpanded action (could randomize)
    const Action& actionToTry = unexpandedActions[0];
    
    // Create child node
    auto child = createChildNode(node, actionToTry);
    if (!child) {
        return node;  // Failed to create child
    }
    
    // Add child to tree
    MCTSNode* childPtr = node->addChild(std::move(child));
    nodesExpanded_++;
    
    return childPtr;
}

double MCTSSearch::simulate(MCTSNode* node) {
    totalRollouts_++;
    
    // Clone state for simulation
    auto simState = node->getState().clone();
    
    // Create executor for simulation (cast to get access to state)
    auto executor = ActionExecutorFactory::createSimulationExecutor(std::move(simState));
    auto* simExec = static_cast<SimulationActionExecutor*>(executor.get());
    
    // Rollout loop
    int depth = 0;
    while (depth < config_.rolloutDepthLimit) {
        // Get state reference from executor
        const auto& currentState = simExec->getState();
        
        // Check terminal state
        if (evaluator_->isTerminalState(currentState, node->getPerspective())) {
            break;
        }
        
        // Get current player's units
        auto units = currentState.getPlayerUnits(currentState.currentPlayer);
        if (units.empty()) {
            break;  // No units to move
        }
        
        // Simple rollout: select random unit and random action
        static std::random_device rd;
        static std::mt19937 gen(rd());
        
        // Pick random unit
        std::uniform_int_distribution<size_t> unitDist(0, units.size() - 1);
        const auto* unit = units[unitDist(gen)];
        
        // Generate legal actions for this unit
        auto actions = executor->generateLegalActions(unit->networkID);
        if (actions.empty()) {
            break;  // No legal actions
        }
        
        // Pick random action (or best heuristic if configured)
        Action actionToExecute;
        if (config_.useRandomRollout || actions.size() == 1) {
            std::uniform_int_distribution<size_t> actionDist(0, actions.size() - 1);
            actionToExecute = actions[actionDist(gen)];
        } else {
            // Heuristic rollout: evaluate each action and pick best
            // (More expensive but better quality)
            double bestScore = -std::numeric_limits<double>::infinity();
            actionToExecute = actions[0];
            
            EvaluationContext evalCtx;
            evalCtx.perspectivePlayer = node->getPerspective();
            
            for (const auto& action : actions) {
                // Try action
                ExecutionContext execCtx;
                auto result = executor->execute(action, execCtx);
                
                if (result.success) {
                    // Evaluate resulting state
                    auto evalResult = evaluator_->evaluate(simExec->getState(), evalCtx);
                    
                    if (evalResult.score > bestScore) {
                        bestScore = evalResult.score;
                        actionToExecute = action;
                    }
                    
                    // Undo action
                    executor->undo();
                }
            }
        }
        
        // Execute chosen action
        ExecutionContext execCtx;
        auto result = executor->execute(actionToExecute, execCtx);
        
        if (!result.success) {
            break;  // Failed to execute
        }
        
        depth++;
    }
    
    // Evaluate final state
    EvaluationContext evalCtx;
    evalCtx.perspectivePlayer = node->getPerspective();
    auto finalEval = evaluator_->evaluate(simExec->getState(), evalCtx);
    
    return finalEval.score;
}

void MCTSSearch::backpropagate(MCTSNode* node, double value) {
    // Propagate value up the tree
    while (node != nullptr) {
        node->update(value);
        
        // Flip value for opponent (zero-sum game perspective)
        value = -value;
        
        node = node->getParent();
    }
}

// ========== Helper Methods ==========

bool MCTSSearch::runIteration() {
    if (!root_) {
        return false;
    }
    
    // Phase 1: Selection
    MCTSNode* selectedNode = select(root_.get());
    if (!selectedNode) {
        return false;
    }
    
    // Phase 2: Expansion
    MCTSNode* expandedNode = expand(selectedNode);
    if (!expandedNode) {
        return false;
    }
    
    // Phase 3: Simulation
    double rolloutValue = simulate(expandedNode);
    
    // Phase 4: Backpropagation
    backpropagate(expandedNode, rolloutValue);
    
    iterationsRun_++;
    return true;
}

bool MCTSSearch::shouldTerminateEarly() const {
    if (!root_ || !root_->hasChildren()) {
        return false;
    }
    
    auto* bestChild = root_->selectBestChildByVisits();
    if (!bestChild) {
        return false;
    }
    
    // Check if best move has very high win rate
    double winRate = (bestChild->getAverageValue() + 1.0) / 2.0;
    return winRate >= config_.earlyTerminationThreshold;
}

std::vector<Action> MCTSSearch::getUnexpandedActions(MCTSNode* node) const {
    // Clone state and create executor
    auto stateClone = node->getState().clone();
    auto executor = ActionExecutorFactory::createSimulationExecutor(std::move(stateClone));
    auto* simExec = static_cast<SimulationActionExecutor*>(executor.get());
    
    // Get all units for current player
    auto units = simExec->getState().getPlayerUnits(simExec->getState().currentPlayer);
    
    // Collect all legal actions
    std::vector<Action> allActions;
    for (const auto* unit : units) {
        auto unitActions = executor->generateLegalActions(unit->networkID);
        allActions.insert(allActions.end(), unitActions.begin(), unitActions.end());
    }
    
    // Filter out actions already tried (have child nodes)
    std::vector<Action> unexpandedActions;
    for (const auto& action : allActions) {
        bool alreadyTried = false;
        
        for (const auto& child : node->getChildren()) {
            if (child->hasAction() && child->getAction() == action) {
                alreadyTried = true;
                break;
            }
        }
        
        if (!alreadyTried) {
            unexpandedActions.push_back(action);
        }
    }
    
    return unexpandedActions;
}

std::unique_ptr<MCTSNode> MCTSSearch::createChildNode(MCTSNode* parent, 
                                                      const Action& action) {
    // Clone parent state
    auto childState = parent->getState().clone();
    
    // Create executor
    auto executor = ActionExecutorFactory::createSimulationExecutor(std::move(childState));
    
    // Execute action
    ExecutionContext execCtx;
    auto result = executor->execute(action, execCtx);
    
    if (!result.success) {
        return nullptr;  // Action failed
    }
    
    // Get the modified state back from executor
    auto* simExec = static_cast<SimulationActionExecutor*>(executor.get());
    auto finalState = simExec->getState().clone();
    
    // Create child node
    return std::make_unique<MCTSNode>(
        std::move(finalState),
        parent,
        action,
        parent->getPerspective()
    );
}

// ========== Statistics ==========

int MCTSSearch::countNodes() const {
    if (!root_) {
        return 0;
    }
    
    // BFS traversal to count nodes
    int count = 0;
    std::vector<const MCTSNode*> queue;
    queue.push_back(root_.get());
    
    while (!queue.empty()) {
        const MCTSNode* node = queue.back();
        queue.pop_back();
        count++;
        
        for (const auto& child : node->getChildren()) {
            queue.push_back(child.get());
        }
    }
    
    return count;
}

int MCTSSearch::getTreeDepth() const {
    if (!root_) {
        return 0;
    }
    
    // DFS to find maximum depth
    int maxDepth = 0;
    std::vector<const MCTSNode*> stack;
    stack.push_back(root_.get());
    
    while (!stack.empty()) {
        const MCTSNode* node = stack.back();
        stack.pop_back();
        
        int depth = node->getDepth();
        if (depth > maxDepth) {
            maxDepth = depth;
        }
        
        for (const auto& child : node->getChildren()) {
            stack.push_back(child.get());
        }
    }
    
    return maxDepth;
}

} // namespace mcts
} // namespace asc
