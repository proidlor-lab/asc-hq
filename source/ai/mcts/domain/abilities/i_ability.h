/***************************************************************************
 * i_ability.h - Unit capability/ability interface
 *
 * Purpose: Define actions a unit can perform based on its capabilities
 *          Enables data-driven action generation from unit properties
 *
 * Part of: ASC MCTS AI (Phase 1.2 - Capability-Based Action Generation)
 * Architecture: Hybrid approach - abilities map unit types to actions
 ***************************************************************************/

#ifndef MCTS_I_ABILITY_H
#define MCTS_I_ABILITY_H

#include "../action_types.h"
#include "../i_game_state.h"
#include "../unit_snapshot.h"
#include <vector>
#include <string>
#include <memory>

namespace asc {
namespace mcts {

/**
 * Action categories for organizing abilities
 *
 * Used for filtering, prioritization, and agent evaluation
 */
enum class ActionCategory : uint8_t {
   Movement,  // Move, retreat, reposition
   Combat,    // Attack, counter-attack
   Service,   // Repair, refuel, resupply
   Build,     // Construct buildings, deploy units
   Cargo,     // Load, unload, transport
   Special,   // Map-specific, unit-specific abilities
   Meta       // Wait, end turn, pass
};

/**
 * String representation for debugging
 */
[[nodiscard]] const char* actionCategoryToString(ActionCategory category) noexcept;

/**
 * Get action category from concrete action (helper for agents)
 */
[[nodiscard]] ActionCategory getActionCategory(const Action& action) noexcept;

/**
 * Unit ability interface
 *
 * An ability represents something a unit CAN DO (move, attack, repair, etc.)
 * Abilities are tied to unit capabilities (from VehicleType).
 *
 * Design:
 * - Stateless: All context passed in parameters
 * - Reusable: Same ability instance can evaluate many units
 * - Extensible: New abilities can be added without modifying core code
 *
 * Example abilities:
 * - MovementAbility: Generates MoveActions based on unit movement type
 * - CombatAbility: Generates AttackActions based on unit weapons
 * - RepairAbility: Generates RepairActions for engineer units
 * - MinelayAbility: Generates MinelayActions for minelayer units
 */
class IAbility {
  public:
   virtual ~IAbility() = default;

   /**
    * Check if this ability is available for the given unit
    *
    * @param state Current game state
    * @param unit Unit to check
    * @return true if unit can use this ability
    *
    * Examples:
    * - MovementAbility: returns true if unit.movement > 0
    * - CombatAbility: returns true if unit has weapons and ammo
    * - RepairAbility: returns true if unit.type has repair capability
    */
   [[nodiscard]] virtual bool isAvailable(const IGameState& state,
                                          const UnitSnapshot& unit) const = 0;

   /**
    * Generate all actions for this ability
    *
    * @param state Current game state
    * @param unit Unit performing the action
    * @return Vector of actions this unit can perform with this ability
    *
    * Examples:
    * - MovementAbility: returns MoveActions for all reachable hexes
    * - CombatAbility: returns AttackActions for all enemies in range
    * - RepairAbility: returns RepairActions for damaged friendlies in range
    *
    * Note: This only generates CAPABILITY-based actions.
    *       Legality checking (blocked terrain, etc.) happens later.
    */
   [[nodiscard]] virtual std::vector<Action> generateActions(const IGameState& state,
                                                             const UnitSnapshot& unit) const = 0;

   /**
    * Get action category for this ability
    *
    * Used for filtering (e.g., "generate only combat actions")
    * and agent evaluation (e.g., "combat-focused agent")
    */
   [[nodiscard]] virtual ActionCategory getCategory() const noexcept = 0;

   /**
    * Get priority for action generation ordering
    *
    * Higher priority abilities generate actions first.
    * Useful for optimization (generate high-value actions early).
    *
    * Suggested priorities:
    * - Combat: 100 (highest - most impactful)
    * - Movement: 90
    * - Service: 80
    * - Build: 70
    * - Meta (wait): 0 (lowest - last resort)
    */
   [[nodiscard]] virtual int getPriority() const noexcept = 0;

   /**
    * Get human-readable name for debugging/logging
    */
   [[nodiscard]] virtual std::string getName() const = 0;

   /**
    * Check if this ability applies to the given unit type
    *
    * This is used by the AbilityRegistry to map unit types to abilities.
    *
    * @param type Unit type to check (can be nullptr)
    * @return true if this ability applies to units of this type
    *
    * Examples:
    * - MovementAbility: returns true for all types with movement > 0
    * - CombatAbility: returns true for all types with weapons
    * - RepairAbility: returns true only for engineer types
    */
   [[nodiscard]] virtual bool appliesToUnitType(const VehicleType* type) const = 0;
};

/**
 * Helper function: Get all action categories for a unit
 *
 * Useful for agents to understand unit capabilities
 */
[[nodiscard]] std::vector<ActionCategory>
getUnitActionCategories(const UnitSnapshot& unit, const std::vector<IAbility*>& abilities);

}  // namespace mcts
}  // namespace asc

#endif  // MCTS_I_ABILITY_H
