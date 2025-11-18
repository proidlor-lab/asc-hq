/***************************************************************************
 * agent_weight_config.h - INI-based weight loading for agents
 ***************************************************************************/

#ifndef MCTS_AGENT_WEIGHT_CONFIG_H
#define MCTS_AGENT_WEIGHT_CONFIG_H

#include <map>
#include <string>

namespace asc {
namespace mcts {

class AgentWeightConfig {
  public:
   bool loadFromFile(const std::string& path);
   void selectProfile(const std::string& profile);

   [[nodiscard]] float getWeight(const std::string& agentName) const;
   [[nodiscard]] bool hasProfile(const std::string& profile) const;
   [[nodiscard]] const std::string& activeProfile() const noexcept { return activeProfile_; }

  private:
   using WeightTable = std::map<std::string, float>;
   std::map<std::string, WeightTable> profiles_;
   std::string activeProfile_{"AgentWeights.Balanced"};
};

}  // namespace mcts
}  // namespace asc

#endif  // MCTS_AGENT_WEIGHT_CONFIG_H
