/***************************************************************************
 * ability_registry.cpp - Ability registry implementation
 ***************************************************************************/

#include "ability_registry.h"
#include "movement_ability.h"
#include "combat_ability.h"
#include "meta_ability.h"
#include <algorithm>

namespace asc {
namespace mcts {

AbilityRegistry& AbilityRegistry::instance() {
   static AbilityRegistry registry;
   return registry;
}

AbilityRegistry::AbilityRegistry() {
   initializeDefaultAbilities();
}

void AbilityRegistry::initializeDefaultAbilities() {
   // Register default abilities for MVP
   registerAbility("movement", std::make_unique<MovementAbility>());
   registerAbility("combat", std::make_unique<CombatAbility>());
   registerAbility("meta", std::make_unique<MetaAbility>());
}

void AbilityRegistry::registerAbility(std::string name, std::unique_ptr<IAbility> ability) {
   if (!ability) {
      return;
   }

   abilities_[std::move(name)] = std::move(ability);
   sortedAbilitiesDirty_ = true;  // Invalidate cache
}

std::vector<IAbility*> AbilityRegistry::getAbilitiesForUnitType(const VehicleType* type) const {
   if (sortedAbilitiesDirty_) {
      rebuildSortedCache();
   }

   std::vector<IAbility*> applicable;

   for (auto* ability : sortedAbilities_) {
      if (ability && ability->appliesToUnitType(type)) {
         applicable.push_back(ability);
      }
   }

   return applicable;
}

std::vector<IAbility*> AbilityRegistry::getAbilitiesByCategory(ActionCategory category) const {
   if (sortedAbilitiesDirty_) {
      rebuildSortedCache();
   }

   std::vector<IAbility*> filtered;

   for (auto* ability : sortedAbilities_) {
      if (ability && ability->getCategory() == category) {
         filtered.push_back(ability);
      }
   }

   return filtered;
}

IAbility* AbilityRegistry::getAbility(const std::string& name) const {
   auto it = abilities_.find(name);
   return (it != abilities_.end()) ? it->second.get() : nullptr;
}

std::vector<IAbility*> AbilityRegistry::getAllAbilities() const {
   if (sortedAbilitiesDirty_) {
      rebuildSortedCache();
   }

   return sortedAbilities_;
}

void AbilityRegistry::clear() {
   abilities_.clear();
   sortedAbilities_.clear();
   sortedAbilitiesDirty_ = true;
}

void AbilityRegistry::rebuildSortedCache() const {
   sortedAbilities_.clear();
   sortedAbilities_.reserve(abilities_.size());

   for (const auto& [name, ability] : abilities_) {
      sortedAbilities_.push_back(ability.get());
   }

   // Sort by priority (descending)
   std::sort(sortedAbilities_.begin(), sortedAbilities_.end(),
             [](const IAbility* a, const IAbility* b) {
                if (!a)
                   return false;
                if (!b)
                   return true;
                return a->getPriority() > b->getPriority();
             });

   sortedAbilitiesDirty_ = false;
}

// ========== AbilityActionGenerator Implementation ==========

std::vector<Action> AbilityActionGenerator::generateAllActions(const IGameState& state,
                                                               UnitID unitID,
                                                               const AbilityRegistry& registry) {
   // Find unit in snapshot
   const auto* unit = state.findUnit(unitID);
   if (!unit) {
      return {};  // Unit not found
   }

   // Get applicable abilities for this unit type
   auto abilities = registry.getAbilitiesForUnit(*unit);

   // Generate actions from all applicable abilities
   std::vector<Action> allActions;

   for (auto* ability : abilities) {
      if (!ability)
         continue;

      // Check if ability is available for this unit in current state
      if (!ability->isAvailable(state, *unit)) {
         continue;
      }

      // Generate actions from this ability
      auto abilityActions = ability->generateActions(state, *unit);

      // Append to result
      allActions.insert(allActions.end(), std::make_move_iterator(abilityActions.begin()),
                        std::make_move_iterator(abilityActions.end()));
   }

   return allActions;
}

std::vector<Action> AbilityActionGenerator::generateAllActionsWithContext(
   const IGameState& state, UnitID unitID, GameMap* legacyMap, const AbilityRegistry& registry) {
   // Find unit in snapshot
   const auto* unit = state.findUnit(unitID);
   if (!unit) {
      return {};  // Unit not found
   }

   // Get applicable abilities for this unit type
   auto abilities = registry.getAbilitiesForUnit(*unit);

   // Generate actions from all applicable abilities
   std::vector<Action> allActions;

   for (auto* ability : abilities) {
      if (!ability)
         continue;

      // Set context on abilities that need it (MovementAbility, etc.)
      // Use dynamic_cast to check if ability supports context
      if (auto* movementAbility = dynamic_cast<MovementAbility*>(ability)) {
         movementAbility->setContext(legacyMap);
      }
      // Future: Add context to other abilities that need legacy map access

      // Check if ability is available for this unit in current state
      if (!ability->isAvailable(state, *unit)) {
         continue;
      }

      // Generate actions from this ability
      auto abilityActions = ability->generateActions(state, *unit);

      // Append to result
      allActions.insert(allActions.end(), std::make_move_iterator(abilityActions.begin()),
                        std::make_move_iterator(abilityActions.end()));
   }

   return allActions;
}

std::vector<Action>
AbilityActionGenerator::generateActionsByCategory(const IGameState& state, UnitID unitID,
                                                  ActionCategory category,
                                                  const AbilityRegistry& registry) {
   // Find unit in snapshot
   const auto* unit = state.findUnit(unitID);
   if (!unit) {
      return {};
   }

   // Get abilities for this category
   auto abilities = registry.getAbilitiesByCategory(category);

   // Filter to only those applicable to this unit type
   std::vector<Action> categoryActions;

   for (auto* ability : abilities) {
      if (!ability)
         continue;

      // Check if ability applies to this unit type
      if (!ability->appliesToUnitType(unit->type)) {
         continue;
      }

      // Check if ability is available
      if (!ability->isAvailable(state, *unit)) {
         continue;
      }

      // Generate actions
      auto actions = ability->generateActions(state, *unit);

      categoryActions.insert(categoryActions.end(), std::make_move_iterator(actions.begin()),
                             std::make_move_iterator(actions.end()));
   }

   return categoryActions;
}

std::vector<Action> AbilityActionGenerator::generatePlayerActions(const IGameState& state,
                                                                  PlayerID playerID,
                                                                  const AbilityRegistry& registry) {
   // Get all units for this player
   auto units = state.getPlayerUnits(playerID);

   std::vector<Action> allActions;

   for (const auto* unit : units) {
      if (!unit)
         continue;

      // Generate actions for this unit
      auto unitActions = generateAllActions(state, unit->networkID, registry);

      allActions.insert(allActions.end(), std::make_move_iterator(unitActions.begin()),
                        std::make_move_iterator(unitActions.end()));
   }

   return allActions;
}

}  // namespace mcts
}  // namespace asc
