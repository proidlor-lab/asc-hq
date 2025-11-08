/***************************************************************************
 * mcts_node.h - MCTS tree node with UCB1 statistics
 * 
 * Purpose: Represent nodes in the MCTS search tree
 *          Track visit counts, win statistics, and child relationships
 * 
 * Part of: ASC MCTS AI (Phase 1 - Core MCTS Engine)
 * 
 * Design:
 * - Lightweight node structure for efficient tree operations
 * - UCB1 formula for exploration/exploitation balance
 * - Parent/child relationships for tree traversal
 * - Action association (which action led to this state)
 ***************************************************************************/

#ifndef MCTS_NODE_H
#define MCTS_NODE_H

#include "../domain/game_state_snapshot.h"
#include "../domain/action_types.h"
#include <memory>
#include <vector>
#include <cmath>
#include <limits>

namespace asc {
namespace mcts {

/**
 * MCTS Node - represents a game state in the search tree
 * 
 * Statistics tracked:
 * - visits: How many times this node was visited
 * - totalValue: Sum of all rollout scores (for averaging)
 * - children: Child nodes (expanded actions)
 * - action: Action that led to this state (from parent)
 * 
 * UCB1 Formula: exploitation + exploration
 *   exploitation = averageValue = totalValue / visits
 *   exploration = C * sqrt(ln(parent_visits) / visits)
 * 
 * Design: Value object with smart pointer ownership
 */
class MCTSNode {
public:
    // ========== Construction ==========
    
    /**
     * Create root node (no parent, no action)
     * 
     * @param state Game state at this node
     * @param perspective Player ID for evaluation perspective
     */
    MCTSNode(std::unique_ptr<GameStateSnapshot> state, PlayerID perspective)
        : state_(std::move(state))
        , parent_(nullptr)
        , action_(std::nullopt)
        , perspective_(perspective)
        , visits_(0)
        , totalValue_(0.0)
        , isFullyExpanded_(false)
    {}
    
    /**
     * Create child node (with parent and action)
     * 
     * @param state Game state at this node
     * @param parent Pointer to parent node
     * @param action Action that led to this state
     * @param perspective Player ID for evaluation
     */
    MCTSNode(std::unique_ptr<GameStateSnapshot> state, 
             MCTSNode* parent,
             const Action& action,
             PlayerID perspective)
        : state_(std::move(state))
        , parent_(parent)
        , action_(action)
        , perspective_(perspective)
        , visits_(0)
        , totalValue_(0.0)
        , isFullyExpanded_(false)
    {}
    
    // Prevent copying (use smart pointers for ownership)
    MCTSNode(const MCTSNode&) = delete;
    MCTSNode& operator=(const MCTSNode&) = delete;
    
    // Allow moving
    MCTSNode(MCTSNode&&) = default;
    MCTSNode& operator=(MCTSNode&&) = default;
    
    // ========== State Access ==========
    
    const GameStateSnapshot& getState() const noexcept { 
        return *state_; 
    }
    
    GameStateSnapshot& getStateMutable() noexcept { 
        return *state_; 
    }
    
    PlayerID getPerspective() const noexcept { 
        return perspective_; 
    }
    
    // ========== Tree Navigation ==========
    
    MCTSNode* getParent() const noexcept { 
        return parent_; 
    }
    
    const std::vector<std::unique_ptr<MCTSNode>>& getChildren() const noexcept {
        return children_;
    }
    
    bool hasChildren() const noexcept {
        return !children_.empty();
    }
    
    size_t getChildCount() const noexcept {
        return children_.size();
    }
    
    bool isRoot() const noexcept {
        return parent_ == nullptr;
    }
    
    bool isLeaf() const noexcept {
        return children_.empty();
    }
    
    // ========== Action Information ==========
    
    bool hasAction() const noexcept {
        return action_.has_value();
    }
    
    const Action& getAction() const {
        return action_.value();
    }
    
    // ========== Statistics ==========
    
    int getVisits() const noexcept {
        return visits_;
    }
    
    double getTotalValue() const noexcept {
        return totalValue_;
    }
    
    /**
     * Get average value (exploitation component)
     * 
     * @return Average rollout value or 0.0 if never visited
     */
    double getAverageValue() const noexcept {
        if (visits_ == 0) return 0.0;
        return totalValue_ / static_cast<double>(visits_);
    }
    
    /**
     * Calculate UCB1 value for this node
     * 
     * UCB1 = exploitation + exploration
     *      = (totalValue / visits) + C * sqrt(ln(parent_visits) / visits)
     * 
     * @param explorationConstant C parameter (typically sqrt(2))
     * @return UCB1 value (higher = more promising)
     */
    double getUCB1Value(double explorationConstant = 1.414213562373095) const noexcept {
        // Unvisited nodes have infinite UCB1 (prioritize exploration)
        if (visits_ == 0) {
            return std::numeric_limits<double>::infinity();
        }
        
        // No parent = root node, just return average value
        if (parent_ == nullptr) {
            return getAverageValue();
        }
        
        // UCB1 formula
        double exploitation = getAverageValue();
        double exploration = explorationConstant * 
            std::sqrt(std::log(static_cast<double>(parent_->visits_)) / 
                     static_cast<double>(visits_));
        
        return exploitation + exploration;
    }
    
    // ========== Tree Modification ==========
    
    /**
     * Add child node to this node
     * 
     * @param child Unique pointer to child node
     * @return Raw pointer to added child (for convenience)
     */
    MCTSNode* addChild(std::unique_ptr<MCTSNode> child) {
        MCTSNode* rawPtr = child.get();
        children_.push_back(std::move(child));
        return rawPtr;
    }
    
    /**
     * Update statistics after rollout
     * 
     * @param value Rollout result value (-1.0 to +1.0)
     */
    void update(double value) noexcept {
        visits_++;
        totalValue_ += value;
    }
    
    /**
     * Mark node as fully expanded (all legal actions tried)
     */
    void markFullyExpanded() noexcept {
        isFullyExpanded_ = true;
    }
    
    /**
     * Check if node is fully expanded
     */
    bool isFullyExpanded() const noexcept {
        return isFullyExpanded_;
    }
    
    // ========== Best Child Selection ==========
    
    /**
     * Select best child by visit count (most robust)
     * 
     * Used for final move selection after search
     * 
     * @return Pointer to best child or nullptr if no children
     */
    MCTSNode* selectBestChildByVisits() const {
        if (children_.empty()) {
            return nullptr;
        }
        
        MCTSNode* best = children_[0].get();
        for (const auto& child : children_) {
            if (child->getVisits() > best->getVisits()) {
                best = child.get();
            }
        }
        return best;
    }
    
    /**
     * Select best child by average value (most optimistic)
     * 
     * @return Pointer to best child or nullptr if no children
     */
    MCTSNode* selectBestChildByValue() const {
        if (children_.empty()) {
            return nullptr;
        }
        
        MCTSNode* best = children_[0].get();
        for (const auto& child : children_) {
            if (child->getAverageValue() > best->getAverageValue()) {
                best = child.get();
            }
        }
        return best;
    }
    
    /**
     * Select best child by UCB1 (for tree policy)
     * 
     * @param explorationConstant C parameter
     * @return Pointer to best child or nullptr if no children
     */
    MCTSNode* selectBestChildByUCB1(double explorationConstant = 1.414213562373095) const {
        if (children_.empty()) {
            return nullptr;
        }
        
        MCTSNode* best = children_[0].get();
        double bestUCB1 = best->getUCB1Value(explorationConstant);
        
        for (const auto& child : children_) {
            double ucb1 = child->getUCB1Value(explorationConstant);
            if (ucb1 > bestUCB1) {
                best = child.get();
                bestUCB1 = ucb1;
            }
        }
        return best;
    }
    
    // ========== Debugging ==========
    
    /**
     * Get depth in tree (root = 0)
     */
    int getDepth() const noexcept {
        int depth = 0;
        const MCTSNode* node = this;
        while (node->parent_ != nullptr) {
            depth++;
            node = node->parent_;
        }
        return depth;
    }
    
    /**
     * Get statistics for debugging
     */
    struct Stats {
        int visits;
        double averageValue;
        int childCount;
        int depth;
        bool isFullyExpanded;
    };
    
    Stats getStats() const noexcept {
        return Stats{
            visits_,
            getAverageValue(),
            static_cast<int>(children_.size()),
            getDepth(),
            isFullyExpanded_
        };
    }

private:
    // State
    std::unique_ptr<GameStateSnapshot> state_;
    
    // Tree structure
    MCTSNode* parent_;  // Raw pointer (parent owns us)
    std::vector<std::unique_ptr<MCTSNode>> children_;  // We own children
    
    // Action that led to this state (from parent)
    std::optional<Action> action_;
    
    // Player perspective for evaluation
    PlayerID perspective_;
    
    // Statistics
    int visits_;
    double totalValue_;
    bool isFullyExpanded_;
};

} // namespace mcts
} // namespace asc

#endif // MCTS_NODE_H
