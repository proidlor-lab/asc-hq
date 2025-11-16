/***************************************************************************
 * mcts_config.h - Shared MCTS configuration struct
 * 
 * Purpose: Provide a lightweight, dependency-free definition of MCTSConfig
 *          so components can configure the search engine without pulling
 *          in the full MCTS search headers.
 ***************************************************************************/

#ifndef MCTS_CONFIG_H
#define MCTS_CONFIG_H

#include <string>

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
    int maxActionsExpansion{20};           // Agent-pruned actions per expansion
    int maxActionsRollout{5};              // Quick rollout action cap

    // Optimization
    bool enableEarlyTermination{true};     // Stop if clear winner found
    double earlyTerminationThreshold{0.95}; // Win probability threshold

    // Agent thresholds
    float vetoThreshold{-0.9f};            // Hard veto cutoff
    float pruneThreshold{-0.5f};           // Discard low-utility actions
    std::string agentProfile{"Balanced"};  // Weight profile name

    constexpr MCTSConfig() noexcept = default;
};

} // namespace mcts
} // namespace asc

#endif // MCTS_CONFIG_H
