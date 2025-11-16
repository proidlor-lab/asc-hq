/***************************************************************************
 * agent_context.h - Shared context objects for tactical agents
 *
 * Part of: ASC MCTS AI (Phase 2.1 - Utility-Agent Framework)
 ***************************************************************************/

#ifndef MCTS_AGENT_CONTEXT_H
#define MCTS_AGENT_CONTEXT_H

#include "../domain/types.h"
#include "../domain/unit_snapshot.h"
#include "../domain/game_state_snapshot.h"
#include "../domain/action_types.h"

#include <string>
#include <vector>

// Forward declarations from legacy ASC code
class GameMap;

namespace asc {
namespace mcts {

class ICombatCalculator;
class ReactionFireDetector;

/**
 * Categories allow us to express coarse agent priorities.
 * Survival-oriented agents may veto or outweigh tactical ones.
 */
enum class AgentCategory {
    SURVIVAL,
    TACTICAL,
    STRATEGIC,
    RESOURCE,
    COORDINATION
};

/**
 * Result returned by individual agents.
 */
struct AgentScore {
    float utility{0.0f};      ///< Normalized utility [-1.0, +1.0]
    float confidence{1.0f};   ///< Confidence weight [0.0, 1.0]
    bool isVeto{false};       ///< Hard veto flag
    std::string reasoning;    ///< Optional trace/debug info
};

/**
 * Aggregation parameters shared across agents and rollout policies.
 */
struct AgentAggregationConfig {
    float vetoThreshold{-0.9f};
    float pruneThreshold{-0.5f};
};

/**
 * Shared read-only data passed to every agent evaluation call.
 */
struct AgentContext {
    const GameStateSnapshot& state;          ///< Current tactical snapshot
    const UnitSnapshot* actingUnit{nullptr}; ///< Unit currently being planned
    PlayerID playerId{0};                    ///< Owner of acting unit
    const GameMap* gameMap{nullptr};         ///< Optional legacy map pointer
    const ReactionFireDetector* rfDetector{nullptr}; ///< Cached RF data (optional)
    const ICombatCalculator* combatCalculator{nullptr}; ///< Combat calculator adapter
    bool isFastRollout{false};               ///< True for rollout policy evaluations
    int searchDepth{0};                      ///< Current search depth
    AgentAggregationConfig aggregationConfig;///< Threshold configuration
};

/**
 * Aggregated score returned by UtilityAggregator.
 */
struct AggregatedAgentScore {
    float normalizedScore{0.0f};             ///< Weighted result [-1.0,+1.0]
    bool vetoed{false};                      ///< True if any agent vetoed
    bool passesThreshold{true};              ///< False if pruned via threshold
    std::vector<AgentScore> contributions;   ///< Individual agent outputs
};

} // namespace mcts
} // namespace asc

#endif // MCTS_AGENT_CONTEXT_H
