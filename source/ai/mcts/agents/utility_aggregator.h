/***************************************************************************
 * utility_aggregator.h - Combine agent scores into a single utility value
 ***************************************************************************/

#ifndef MCTS_UTILITY_AGGREGATOR_H
#define MCTS_UTILITY_AGGREGATOR_H

#include "agent_context.h"
#include "i_agent.h"

#include <vector>

namespace asc {
namespace mcts {

/**
 * Aggregates results provided by multiple agents using a weighted sum.
 */
class UtilityAggregator {
  public:
   UtilityAggregator() = default;

   explicit UtilityAggregator(AgentAggregationConfig config) : config_(config) {}

   /**
    * Evaluate all agents and return a combined score.
    */
   [[nodiscard]] AggregatedAgentScore aggregate(const std::vector<const IAgent*>& agents,
                                                const Action& action,
                                                const AgentContext& context) const;

  private:
   AgentAggregationConfig config_{};
};

}  // namespace mcts
}  // namespace asc

#endif  // MCTS_UTILITY_AGGREGATOR_H
