/***************************************************************************
 * utility_aggregator.cpp
 ***************************************************************************/

#include "utility_aggregator.h"

#include <algorithm>
#include <numeric>

namespace asc {
namespace mcts {

AggregatedAgentScore UtilityAggregator::aggregate(const std::vector<const IAgent*>& agents,
                                                  const Action& action,
                                                  const AgentContext& context) const {
   AgentAggregationConfig localConfig = config_;
   localConfig.vetoThreshold = context.aggregationConfig.vetoThreshold;
   localConfig.pruneThreshold = context.aggregationConfig.pruneThreshold;
   AggregatedAgentScore result;
   result.contributions.reserve(agents.size());

   float weightedSum = 0.0f;
   float totalWeight = 0.0f;

   for (const IAgent* agent : agents) {
      if (!agent) {
         continue;
      }

      AgentScore score = agent->evaluate(action, context);
      result.contributions.push_back(score);

      if (score.isVeto || score.utility <= localConfig.vetoThreshold) {
         result.vetoed = true;
         result.normalizedScore = -1.0f;
         result.passesThreshold = false;
         return result;
      }

      const float clampedConfidence = std::clamp(score.confidence, 0.0f, 1.0f);
      const float weight = std::max(agent->getWeight(), 0.0f) * clampedConfidence;
      weightedSum += score.utility * weight;
      totalWeight += weight;
   }

   if (totalWeight > 0.0f) {
      result.normalizedScore = weightedSum / totalWeight;
   } else {
      result.normalizedScore = 0.0f;
   }

   result.passesThreshold = (result.normalizedScore >= localConfig.pruneThreshold);
   return result;
}

}  // namespace mcts
}  // namespace asc
