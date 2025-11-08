/***************************************************************************
 * mcts_search.h - Core MCTS search algorithm
 * 
 * Purpose: Implement Monte Carlo Tree Search for tactical decision-making
 *          Integrates: state cloning, action execution, state evaluation
 * 
 * Part of: ASC MCTS AI (Phase 1 - Core MCTS Engine)
 * 
 * MCTS Phases:
 * 1. Selection   - Navigate tree using UCB1
 * 2. Expansion   - Add new child node
 * 3. Simulation  - Rollout to terminal state or depth limit
 * 4. Backpropagation - Update statistics up the tree
 * 
 * Design:
 * - Dependency injection (evaluator, executor)
 * - Configurable parameters (iterations, exploration constant)
 * - Stateful search (maintains tree between calls)
 * - Anytime algorithm (can be stopped early)
 ***************************************************************************/

#ifndef MCTS_SEARCH_H
#define MCTS_SEARCH_H

#include "mcts_node.h"
#include "../domain/i_tactical_evaluator.h"
#include "../domain/i_action_executor.h"
#include <memory>
#include <optional>
#include <chrono>

namespace asc {
namespace mcts {

/**
 * Configuration for MCTS search
 * 
 * Tunable parameters for controlling search behavior
 */
struct MCTSConfig {
    // Search budget
    int maxIterations{1000};               // Max MCTS iterations
    int maxTimeMs{5000};                   // Max search time in milliseconds
    
    // UCB1 parameters
    double explorationConstant{1.414};     // C in UCB1 formula (sqrt(2))
    
    // Rollout parameters
    int rolloutDepthLimit{10};             // Max moves in rollout
    bool useRandomRollout{false};          // Random vs. heuristic rollout
    
    // Expansion control
    int minVisitsBeforeExpansion{1};       // Visit parent N times before expanding
    int maxChildrenPerNode{25};            // Limit branching factor
    
    // Optimization
    bool enableEarlyTermination{true};     // Stop if clear winner found
    double earlyTerminationThreshold{0.95}; // Win probability threshold
    
    constexpr MCTSConfig() noexcept = default;
};

/**
 * Result of MCTS search
 * 
 * Contains best action and search statistics
 */
struct MCTSResult {
    // Best action found
    std::optional<Action> bestAction;
    
    // Search statistics
    int iterationsRun{0};
    int nodesExpanded{0};
    int totalRollouts{0};
    double searchTimeMs{0.0};
    
    // Best move statistics
    int bestMoveVisits{0};
    double bestMoveValue{0.0};
    double bestMoveWinRate{0.0};
    
    // Tree statistics
    int treeDepth{0};
    int treeNodeCount{0};
    
    constexpr MCTSResult() noexcept = default;
};

/**
 * MCTS Search Engine
 * 
 * Implements full MCTS algorithm:
 * - Builds search tree iteratively
 * - Uses UCB1 for selection
 * - Evaluates states using ITacticalEvaluator
 * - Executes actions using IActionExecutor
 * 
 * Design Pattern: Strategy Pattern (configurable components)
 */
class MCTSSearch {
public:
    /**
     * Create MCTS search engine
     * 
     * @param evaluator State evaluator (ownership transferred)
     * @param config Search configuration
     */
    MCTSSearch(std::unique_ptr<ITacticalEvaluator> evaluator,
               const MCTSConfig& config = MCTSConfig())
        : evaluator_(std::move(evaluator))
        , config_(config)
        , root_(nullptr)
    {}
    
    // Prevent copying (owns unique resources)
    MCTSSearch(const MCTSSearch&) = delete;
    MCTSSearch& operator=(const MCTSSearch&) = delete;
    
    // Allow moving
    MCTSSearch(MCTSSearch&&) = default;
    MCTSSearch& operator=(MCTSSearch&&) = default;
    
    // ========== Main Search API ==========
    
    /**
     * Search for best action from given state
     * 
     * Creates new search tree and runs MCTS iterations
     * 
     * @param initialState Starting game state (cloned internally)
     * @param perspective Player ID to search for
     * @return Search result with best action and statistics
     */
    MCTSResult search(const GameStateSnapshot& initialState, 
                     PlayerID perspective);
    
    /**
     * Continue previous search (if tree exists)
     * 
     * Allows incremental search / anytime behavior
     * 
     * @param additionalIterations How many more iterations to run
     * @return Updated search result
     */
    MCTSResult continueSearch(int additionalIterations);
    
    /**
     * Get best action from current tree
     * 
     * Can be called during or after search
     * 
     * @return Best action or nullopt if no tree
     */
    std::optional<Action> getBestAction() const;
    
    /**
     * Clear search tree (free memory)
     */
    void reset() {
        root_.reset();
    }
    
    // ========== Configuration ==========
    
    void setConfig(const MCTSConfig& config) {
        config_ = config;
    }
    
    const MCTSConfig& getConfig() const noexcept {
        return config_;
    }
    
    // ========== Statistics ==========
    
    /**
     * Get root node (for inspection/debugging)
     */
    const MCTSNode* getRootNode() const {
        return root_.get();
    }
    
    /**
     * Count total nodes in tree
     */
    int countNodes() const;
    
    /**
     * Get maximum depth of tree
     */
    int getTreeDepth() const;

private:
    // ========== MCTS Phases ==========
    
    /**
     * Phase 1: Selection
     * 
     * Navigate tree using UCB1 until reaching expandable node
     * 
     * @param node Starting node (usually root)
     * @return Node to expand
     */
    MCTSNode* select(MCTSNode* node);
    
    /**
     * Phase 2: Expansion
     * 
     * Add one child node for an untried action
     * 
     * @param node Node to expand
     * @return New child node or nullptr if fully expanded
     */
    MCTSNode* expand(MCTSNode* node);
    
    /**
     * Phase 3: Simulation (Rollout)
     * 
     * Play out game from node until terminal state or depth limit
     * 
     * @param node Starting node
     * @return Rollout value (-1.0 to +1.0 from perspective player's view)
     */
    double simulate(MCTSNode* node);
    
    /**
     * Phase 4: Backpropagation
     * 
     * Update statistics from leaf to root
     * 
     * @param node Leaf node to start from
     * @param value Rollout result value
     */
    void backpropagate(MCTSNode* node, double value);
    
    // ========== Helper Methods ==========
    
    /**
     * Run one MCTS iteration (all 4 phases)
     * 
     * @return true if iteration completed successfully
     */
    bool runIteration();
    
    /**
     * Check if we should stop search early
     * 
     * @return true if clear winner found
     */
    bool shouldTerminateEarly() const;
    
    /**
     * Get unexpanded actions for a node
     * 
     * @param node Node to check
     * @return List of actions not yet tried
     */
    std::vector<Action> getUnexpandedActions(MCTSNode* node) const;
    
    /**
     * Create child node by applying action
     * 
     * @param parent Parent node
     * @param action Action to apply
     * @return New child node or nullptr on error
     */
    std::unique_ptr<MCTSNode> createChildNode(MCTSNode* parent, 
                                              const Action& action);
    
    // ========== Member Variables ==========
    
    std::unique_ptr<ITacticalEvaluator> evaluator_;
    MCTSConfig config_;
    std::unique_ptr<MCTSNode> root_;
    
    // Statistics
    int iterationsRun_{0};
    int nodesExpanded_{0};
    int totalRollouts_{0};
};

} // namespace mcts
} // namespace asc

#endif // MCTS_SEARCH_H
