/***************************************************************************
 * combat_calculator_adapter.h - Adapter implementing ICombatCalculator
 *
 * Purpose: Bridge between the generic combat calculator interface used by
 *          tactical agents and the concrete `CombatCalculator` helper that
 *          wraps ASC's combat formulas.
 *
 * Part of: ASC MCTS AI (Phase 2.1 - Utility-Agent Framework)
 ***************************************************************************/

#ifndef MCTS_COMBAT_CALCULATOR_ADAPTER_H
#define MCTS_COMBAT_CALCULATOR_ADAPTER_H

#include "i_combat_calculator.h"
#include "combat_calculator.h"

namespace asc {
namespace mcts {

/**
 * Concrete adapter that delegates to `CombatCalculator` to evaluate combat
 * damage using ASC's real formulas.
 */
class CombatCalculatorAdapter final : public ICombatCalculator {
  public:
   CombatCalculatorAdapter() = default;
   ~CombatCalculatorAdapter() override = default;

   CombatCalculatorAdapter(const CombatCalculatorAdapter&) = delete;
   CombatCalculatorAdapter& operator=(const CombatCalculatorAdapter&) = delete;
   CombatCalculatorAdapter(CombatCalculatorAdapter&&) noexcept = default;
   CombatCalculatorAdapter& operator=(CombatCalculatorAdapter&&) noexcept = default;

   [[nodiscard]] float calculateExpectedDamage(const UnitSnapshot& attacker,
                                               const UnitSnapshot& target,
                                               const MapCoordinate& attackFrom) const override;

  private:
   [[nodiscard]] static int hexDistance(const MapCoordinate& a, const MapCoordinate& b) noexcept;
   [[nodiscard]] static float computeDamageForWeapon(const UnitSnapshot& attacker,
                                                     const UnitSnapshot& target, int weaponIndex,
                                                     int distance);
};

}  // namespace mcts
}  // namespace asc

#endif  // MCTS_COMBAT_CALCULATOR_ADAPTER_H
