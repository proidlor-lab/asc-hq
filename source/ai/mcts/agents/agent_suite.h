/***************************************************************************
 * agent_suite.h - Manage and apply tactical agents for scoring actions
 ***************************************************************************/

#ifndef MCTS_AGENT_SUITE_H
#define MCTS_AGENT_SUITE_H

#include "agent_context.h"
#include "agent_weight_config.h"
#include "mvp_agents.h"
#include "reaction_fire_detector.h"
#include "utility_aggregator.h"

#include "../domain/combat_calculator_adapter.h"
#include "../domain/game_state_snapshot.h"

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace asc {
namespace mcts {

struct ScoredAction {
    Action action;
    AggregatedAgentScore score;
};

class AgentSuite {
public:
    AgentSuite();
    ~AgentSuite() = default;

    bool loadWeightFile(const std::string& path);
    void selectProfile(const std::string& profile);

    [[nodiscard]] std::vector<ScoredAction> scoreActions(
        const GameStateSnapshot& state,
        PlayerID player,
        const std::vector<Action>& actions,
        const ICombatCalculator* calculator,
        const AgentAggregationConfig& config,
        int searchDepth,
        bool isRollout = false
    ) const;

    [[nodiscard]] const std::vector<std::unique_ptr<IAgent>>& agents() const { return agents_; }

private:
    void initializeDefaultAgents();
    void refreshAgentPointers();
    void applyActiveProfile();
    static std::vector<std::filesystem::path> defaultWeightPaths();

    AgentWeightConfig weightConfig_;
    std::string activeProfile_;
    std::vector<std::unique_ptr<IAgent>> agents_;
    std::vector<const IAgent*> agentPtrs_;
    UtilityAggregator aggregator_;
};

} // namespace mcts
} // namespace asc

#endif // MCTS_AGENT_SUITE_H
