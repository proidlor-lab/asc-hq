/***************************************************************************
 * mvp_agents.h - Concrete agent implementations for Phase 2.1 MVP
 ***************************************************************************/

#ifndef MCTS_MVP_AGENTS_H
#define MCTS_MVP_AGENTS_H

#include "i_agent.h"
#include "unit_role.h"
#include "reaction_fire_detector.h"

namespace asc {
namespace mcts {

class LegalMoveAgent final : public IAgent {
  public:
   LegalMoveAgent();
   ~LegalMoveAgent() override = default;

   [[nodiscard]] AgentScore evaluate(const Action& action,
                                     const AgentContext& context) const override;
   [[nodiscard]] std::string_view getName() const noexcept override { return name_; }
   [[nodiscard]] float getWeight() const noexcept override { return weight_; }
   void setWeight(float weight) override { weight_ = weight; }
   [[nodiscard]] AgentCategory getCategory() const noexcept override {
      return AgentCategory::SURVIVAL;
   }

  private:
   std::string name_;
   float weight_;
};

class ReactionFireAgent final : public IAgent {
  public:
   ReactionFireAgent();
   ~ReactionFireAgent() override = default;

   [[nodiscard]] AgentScore evaluate(const Action& action,
                                     const AgentContext& context) const override;
   [[nodiscard]] std::string_view getName() const noexcept override { return name_; }
   [[nodiscard]] float getWeight() const noexcept override { return weight_; }
   void setWeight(float weight) override { weight_ = weight; }
   [[nodiscard]] AgentCategory getCategory() const noexcept override {
      return AgentCategory::SURVIVAL;
   }

  private:
   float computeExpectedDamage(const ReactionFireThreat& threat, const AgentContext& context,
                               const MapCoordinate& destination) const;

   std::string name_;
   float weight_;
};

class AggressivenessAgent final : public IAgent {
  public:
   AggressivenessAgent();
   ~AggressivenessAgent() override = default;

   [[nodiscard]] AgentScore evaluate(const Action& action,
                                     const AgentContext& context) const override;
   [[nodiscard]] std::string_view getName() const noexcept override { return name_; }
   [[nodiscard]] float getWeight() const noexcept override { return weight_; }
   void setWeight(float weight) override { weight_ = weight; }
   [[nodiscard]] AgentCategory getCategory() const noexcept override {
      return AgentCategory::TACTICAL;
   }

  private:
   std::string name_;
   float weight_;
};

class TargetPriorityAgent final : public IAgent {
  public:
   TargetPriorityAgent();
   ~TargetPriorityAgent() override = default;

   [[nodiscard]] AgentScore evaluate(const Action& action,
                                     const AgentContext& context) const override;
   [[nodiscard]] std::string_view getName() const noexcept override { return name_; }
   [[nodiscard]] float getWeight() const noexcept override { return weight_; }
   void setWeight(float weight) override { weight_ = weight; }
   [[nodiscard]] AgentCategory getCategory() const noexcept override {
      return AgentCategory::TACTICAL;
   }

  private:
   std::string name_;
   float weight_;
};

class ServiceUtilityAgent final : public IAgent {
  public:
   ServiceUtilityAgent();
   ~ServiceUtilityAgent() override = default;

   [[nodiscard]] AgentScore evaluate(const Action& action,
                                     const AgentContext& context) const override;
   [[nodiscard]] std::string_view getName() const noexcept override { return name_; }
   [[nodiscard]] float getWeight() const noexcept override { return weight_; }
   void setWeight(float weight) override { weight_ = weight; }
   [[nodiscard]] AgentCategory getCategory() const noexcept override {
      return AgentCategory::RESOURCE;
   }

  private:
   std::string name_;
   float weight_;
};

}  // namespace mcts
}  // namespace asc

#endif  // MCTS_MVP_AGENTS_H
