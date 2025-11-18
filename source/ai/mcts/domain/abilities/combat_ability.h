/***************************************************************************
 * combat_ability.h - Combat action generation capability
 *
 * Purpose: Generate attack actions based on unit weapon capabilities
 *
 * Part of: ASC MCTS AI (Phase 1.2 - Capability-Based Action Generation)
 ***************************************************************************/

#ifndef MCTS_COMBAT_ABILITY_H
#define MCTS_COMBAT_ABILITY_H

#include "i_ability.h"

namespace asc {
namespace mcts {

/**
 * Combat ability - generates AttackActions for armed units
 *
 * Applies to: Any unit with weapons and ammo
 * Generates: AttackActions for all enemies in range
 *
 * MVP Implementation: Simplified range/weapon selection
 * Future: Full weapon system with range, ammo, target type compatibility
 */
class CombatAbility : public IAbility {
  public:
   CombatAbility() = default;
   ~CombatAbility() override = default;

   [[nodiscard]] bool isAvailable(const IGameState& state, const UnitSnapshot& unit) const override;

   [[nodiscard]] std::vector<Action> generateActions(const IGameState& state,
                                                     const UnitSnapshot& unit) const override;

   [[nodiscard]] ActionCategory getCategory() const noexcept override {
      return ActionCategory::Combat;
   }

   [[nodiscard]] int getPriority() const noexcept override {
      return 100;  // Highest priority (combat is most impactful)
   }

   [[nodiscard]] std::string getName() const override { return "CombatAbility"; }

   [[nodiscard]] bool appliesToUnitType(const VehicleType* type) const override;

  private:
   /**
    * Find all enemy units in range
    */
   [[nodiscard]] std::vector<const UnitSnapshot*>
   findTargetsInRange(const IGameState& state, const UnitSnapshot& unit) const;

   /**
    * Check if unit has attacked this turn
    */
   [[nodiscard]] bool hasAttackedThisTurn(const UnitSnapshot& unit) const { return unit.attacked; }

   /**
    * Check if unit has ammo for any weapon
    */
   [[nodiscard]] bool hasAmmo(const UnitSnapshot& unit) const {
      return unit.ammoMask != 0;  // At least one weapon has ammo
   }

   /**
    * Calculate hex distance (simplified)
    */
   [[nodiscard]] int hexDistance(MapCoordinate a, MapCoordinate b) const;
};

}  // namespace mcts
}  // namespace asc

#endif  // MCTS_COMBAT_ABILITY_H
