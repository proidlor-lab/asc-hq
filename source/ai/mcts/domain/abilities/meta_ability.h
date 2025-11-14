/***************************************************************************
 * meta_ability.h - Meta-game actions (wait, pass, end turn)
 * 
 * Purpose: Generate universal actions available to all units
 * 
 * Part of: ASC MCTS AI (Phase 1.2 - Capability-Based Action Generation)
 ***************************************************************************/

#ifndef MCTS_META_ABILITY_H
#define MCTS_META_ABILITY_H

#include "i_ability.h"

namespace asc {
namespace mcts {

/**
 * Meta ability - generates WaitAction (do nothing)
 * 
 * Applies to: ALL units (universal action)
 * Generates: Single WaitAction
 * 
 * This is always available as a "last resort" action.
 */
class MetaAbility : public IAbility {
public:
    MetaAbility() = default;
    ~MetaAbility() override = default;
    
    [[nodiscard]] bool isAvailable(
        const IGameState& state,
        const UnitSnapshot& unit) const override {
        // Wait is always available
        return true;
    }
    
    [[nodiscard]] std::vector<Action> generateActions(
        const IGameState& state,
        const UnitSnapshot& unit) const override {
        // Generate single wait action
        std::vector<Action> actions;
        actions.emplace_back(WaitAction{unit.networkID});
        return actions;
    }
    
    [[nodiscard]] ActionCategory getCategory() const noexcept override {
        return ActionCategory::Meta;
    }
    
    [[nodiscard]] int getPriority() const noexcept override {
        return 0;  // Lowest priority (last resort)
    }
    
    [[nodiscard]] std::string getName() const override {
        return "MetaAbility";
    }
    
    [[nodiscard]] bool appliesToUnitType(const VehicleType* type) const override {
        // Wait applies to all unit types
        return true;
    }
};

} // namespace mcts
} // namespace asc

#endif // MCTS_META_ABILITY_H
