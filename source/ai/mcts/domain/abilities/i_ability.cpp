/***************************************************************************
 * i_ability.cpp - Implementation of ability helper functions
 ***************************************************************************/

#include "i_ability.h"
#include <algorithm>

namespace asc {
namespace mcts {

const char* actionCategoryToString(ActionCategory category) noexcept {
    switch (category) {
        case ActionCategory::Movement: return "Movement";
        case ActionCategory::Combat: return "Combat";
        case ActionCategory::Service: return "Service";
        case ActionCategory::Build: return "Build";
        case ActionCategory::Cargo: return "Cargo";
        case ActionCategory::Special: return "Special";
        case ActionCategory::Meta: return "Meta";
        default: return "Unknown";
    }
}

ActionCategory getActionCategory(const Action& action) noexcept {
    return std::visit([](const auto& a) -> ActionCategory {
        using T = std::decay_t<decltype(a)>;
        if constexpr (std::is_same_v<T, MoveAction>) {
            return ActionCategory::Movement;
        } else if constexpr (std::is_same_v<T, AttackAction>) {
            return ActionCategory::Combat;
        } else if constexpr (std::is_same_v<T, WaitAction>) {
            return ActionCategory::Meta;
        } else {
            return ActionCategory::Special;  // Fallback
        }
    }, action);
}

std::vector<ActionCategory> getUnitActionCategories(
    const UnitSnapshot& unit,
    const std::vector<IAbility*>& abilities) {
    
    std::vector<ActionCategory> categories;
    categories.reserve(abilities.size());
    
    for (const auto* ability : abilities) {
        if (ability && ability->appliesToUnitType(unit.type)) {
            categories.push_back(ability->getCategory());
        }
    }
    
    // Remove duplicates
    std::sort(categories.begin(), categories.end());
    categories.erase(std::unique(categories.begin(), categories.end()), categories.end());
    
    return categories;
}

} // namespace mcts
} // namespace asc
