/***************************************************************************
 * ability_registry.h - Central registry for unit abilities
 *
 * Purpose: Map unit types to their available abilities
 *          Enable capability-based action generation
 *
 * Part of: ASC MCTS AI (Phase 1.2 - Capability-Based Action Generation)
 ***************************************************************************/

#ifndef MCTS_ABILITY_REGISTRY_H
#define MCTS_ABILITY_REGISTRY_H

#include "i_ability.h"
#include <map>
#include <vector>
#include <memory>
#include <string>

// Forward declarations
class GameMap;

namespace asc {
namespace mcts {

/**
 * Central registry for unit abilities
 *
 * Design:
 * - Singleton pattern for global access
 * - Lazy initialization with default abilities
 * - Extensible: Can register custom abilities at runtime
 *
 * Usage:
 * ```cpp
 * auto& registry = AbilityRegistry::instance();
 * auto abilities = registry.getAbilitiesForUnit(unit);
 * for (auto* ability : abilities) {
 *     if (ability->isAvailable(state, unit)) {
 *         auto actions = ability->generateActions(state, unit);
 *     }
 * }
 * ```
 */
class AbilityRegistry {
  public:
   /**
    * Get singleton instance
    */
   static AbilityRegistry& instance();

   /**
    * Register a new ability
    *
    * @param name Unique identifier for this ability
    * @param ability Ability instance (registry takes ownership)
    */
   void registerAbility(std::string name, std::unique_ptr<IAbility> ability);

   /**
    * Get all abilities applicable to a unit type
    *
    * @param type Unit type (from VehicleType)
    * @return Vector of ability pointers (non-owning)
    *
    * Note: Returned pointers are valid until registry is destroyed
    */
   [[nodiscard]] std::vector<IAbility*> getAbilitiesForUnitType(const VehicleType* type) const;

   /**
    * Get all abilities applicable to a specific unit instance
    *
    * @param unit Unit snapshot
    * @return Vector of ability pointers (non-owning)
    */
   [[nodiscard]] std::vector<IAbility*> getAbilitiesForUnit(const UnitSnapshot& unit) const {
      return getAbilitiesForUnitType(unit.type);
   }

   /**
    * Get abilities by category
    *
    * @param category Action category filter
    * @return All abilities in this category
    */
   [[nodiscard]] std::vector<IAbility*> getAbilitiesByCategory(ActionCategory category) const;

   /**
    * Get ability by name
    *
    * @param name Ability name
    * @return Ability pointer or nullptr if not found
    */
   [[nodiscard]] IAbility* getAbility(const std::string& name) const;

   /**
    * Get all registered abilities
    *
    * @return Vector of all abilities (sorted by priority)
    */
   [[nodiscard]] std::vector<IAbility*> getAllAbilities() const;

   /**
    * Clear all registered abilities
    *
    * Used for testing or runtime reconfiguration
    */
   void clear();

   /**
    * Get number of registered abilities
    */
   [[nodiscard]] size_t size() const { return abilities_.size(); }

  private:
   // Private constructor for singleton
   AbilityRegistry();

   // Initialize default abilities (Movement, Combat, Meta)
   void initializeDefaultAbilities();

   // Storage: ability name -> ability instance
   std::map<std::string, std::unique_ptr<IAbility>> abilities_;

   // Cached sorted abilities by priority (for faster iteration)
   mutable std::vector<IAbility*> sortedAbilities_;
   mutable bool sortedAbilitiesDirty_{true};

   void rebuildSortedCache() const;
};

/**
 * Action generator using ability registry
 *
 * This replaces the hard-coded action generation in SimulationActionExecutor.
 *
 * Usage:
 * ```cpp
 * AbilityActionGenerator generator;
 * auto actions = generator.generateAllActions(state, unitID);
 * ```
 */
class AbilityActionGenerator {
  public:
   /**
    * Generate all actions for a unit using registered abilities
    *
    * @param state Current game state
    * @param unitID Unit to generate actions for
    * @param registry Ability registry (defaults to singleton)
    * @return Vector of all possible actions
    */
   [[nodiscard]] static std::vector<Action>
   generateAllActions(const IGameState& state, UnitID unitID,
                      const AbilityRegistry& registry = AbilityRegistry::instance());

   /**
    * Generate all actions with legacy map context (for pathfinding)
    *
    * @param state Current game state
    * @param unitID Unit to generate actions for
    * @param legacyMap Legacy GameMap for pathfinding (optional)
    * @param registry Ability registry (defaults to singleton)
    * @return Vector of all possible actions
    */
   [[nodiscard]] static std::vector<Action>
   generateAllActionsWithContext(const IGameState& state, UnitID unitID, GameMap* legacyMap,
                                 const AbilityRegistry& registry = AbilityRegistry::instance());

   /**
    * Generate actions filtered by category
    *
    * @param state Current game state
    * @param unitID Unit to generate actions for
    * @param category Action category filter
    * @param registry Ability registry (defaults to singleton)
    * @return Vector of actions in specified category
    */
   [[nodiscard]] static std::vector<Action>
   generateActionsByCategory(const IGameState& state, UnitID unitID, ActionCategory category,
                             const AbilityRegistry& registry = AbilityRegistry::instance());

   /**
    * Generate actions for all units of a player
    *
    * @param state Current game state
    * @param playerID Player to generate actions for
    * @param registry Ability registry (defaults to singleton)
    * @return Vector of all possible actions for player's units
    */
   [[nodiscard]] static std::vector<Action>
   generatePlayerActions(const IGameState& state, PlayerID playerID,
                         const AbilityRegistry& registry = AbilityRegistry::instance());
};

}  // namespace mcts
}  // namespace asc

#endif  // MCTS_ABILITY_REGISTRY_H
