/***************************************************************************
 * agent_suite.cpp
 ***************************************************************************/

#include "agent_suite.h"

#include <algorithm>
#include <filesystem>

namespace asc {
namespace mcts {

namespace {

const UnitSnapshot* findUnitForAction(const GameStateSnapshot& state, const Action& action) {
    UnitID id = std::visit(overloaded {
        [](const MoveAction& move) { return move.unitID; },
        [](const AttackAction& attack) { return attack.attackerID; },
        [](const WaitAction& wait) { return wait.unitID; }
    }, action);
    return state.findUnit(id);
}

} // namespace

AgentSuite::AgentSuite()
    : activeProfile_("AgentWeights.Balanced"),
      aggregator_(AgentAggregationConfig{}) {
    initializeDefaultAgents();
    refreshAgentPointers();

    bool loaded = false;
    for (const auto& path : defaultWeightPaths()) {
        if (std::filesystem::exists(path) && loadWeightFile(path.string())) {
            loaded = true;
            break;
        }
    }

    if (!loaded) {
        applyActiveProfile();
    }
}

bool AgentSuite::loadWeightFile(const std::string& path) {
    if (!weightConfig_.loadFromFile(path)) {
        return false;
    }
    applyActiveProfile();
    return true;
}

void AgentSuite::selectProfile(const std::string& profile) {
    activeProfile_ = "AgentWeights." + profile;
    if (!weightConfig_.hasProfile(activeProfile_)) {
        return;
    }
    weightConfig_.selectProfile(profile);
    applyActiveProfile();
}

std::vector<ScoredAction> AgentSuite::scoreActions(
    const GameStateSnapshot& state,
    PlayerID player,
    const std::vector<Action>& actions,
    const ICombatCalculator* calculator,
    const AgentAggregationConfig& config,
    int searchDepth,
    bool isRollout) const {

    std::vector<ScoredAction> scored;
    scored.reserve(actions.size());

    ReactionFireDetector rfDetector;
    rfDetector.initialize(&state, player, calculator);

    for (const auto& action : actions) {
        const UnitSnapshot* unit = findUnitForAction(state, action);
        if (!unit) {
            continue;
        }

        AgentContext agentContext{
            state,
            unit,
            player,
            nullptr,
            &rfDetector,
            calculator,
            isRollout,
            searchDepth,
            config
        };

        auto aggregated = aggregator_.aggregate(agentPtrs_, action, agentContext);
        if (aggregated.vetoed || !aggregated.passesThreshold) {
            continue;
        }

        scored.push_back(ScoredAction{action, aggregated});
    }

    std::sort(scored.begin(), scored.end(),
              [](const ScoredAction& lhs, const ScoredAction& rhs) {
                  return lhs.score.normalizedScore > rhs.score.normalizedScore;
              });

    return scored;
}

void AgentSuite::initializeDefaultAgents() {
    agents_.emplace_back(std::make_unique<LegalMoveAgent>());
    agents_.emplace_back(std::make_unique<ReactionFireAgent>());
    agents_.emplace_back(std::make_unique<AggressivenessAgent>());
    agents_.emplace_back(std::make_unique<TargetPriorityAgent>());
    agents_.emplace_back(std::make_unique<ServiceUtilityAgent>());
}

void AgentSuite::refreshAgentPointers() {
    agentPtrs_.clear();
    agentPtrs_.reserve(agents_.size());
    for (const auto& agent : agents_) {
        agentPtrs_.push_back(agent.get());
    }
}

void AgentSuite::applyActiveProfile() {
    for (const auto& agent : agents_) {
        agent->setWeight(weightConfig_.getWeight(std::string(agent->getName())));
    }
}

std::vector<std::filesystem::path> AgentSuite::defaultWeightPaths() {
    return {
        std::filesystem::path("mcts_agents.ini"),
        std::filesystem::path("source/ai/mcts/mcts_agents.ini")
    };
}

} // namespace mcts
} // namespace asc
