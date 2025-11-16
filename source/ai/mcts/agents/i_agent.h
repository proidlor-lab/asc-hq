/***************************************************************************
 * i_agent.h - Agent interfaces for the tactical utility framework
 ***************************************************************************/

#ifndef MCTS_I_AGENT_H
#define MCTS_I_AGENT_H

#include "agent_context.h"

#include <string_view>

namespace asc {
namespace mcts {

/**
 * Base interface for all agents participating in action scoring.
 */
class IAgent {
public:
    virtual ~IAgent() = default;

    /**
     * Evaluate a candidate action.
     */
    [[nodiscard]] virtual AgentScore evaluate(
        const Action& action,
        const AgentContext& context
    ) const = 0;

    /**
     * Textual identifier for configuration/logging.
     */
    [[nodiscard]] virtual std::string_view getName() const noexcept = 0;

    /**
     * Weight applied during aggregation.
     */
    [[nodiscard]] virtual float getWeight() const noexcept = 0;
    virtual void setWeight(float weight) = 0;

    /**
     * Category used for diagnostic prioritization.
     */
    [[nodiscard]] virtual AgentCategory getCategory() const noexcept = 0;
};

/**
 * Interface for agents that evaluate resulting states instead of actions.
 */
class IStateEvaluationAgent {
public:
    virtual ~IStateEvaluationAgent() = default;

    [[nodiscard]] virtual AgentScore evaluateState(
        const GameStateSnapshot& state,
        const AgentContext& context
    ) const = 0;
};

} // namespace mcts
} // namespace asc

#endif // MCTS_I_AGENT_H
