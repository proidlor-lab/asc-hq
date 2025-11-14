/***************************************************************************
 * i_combat_calculator.h - Interface for combat damage calculation adapters
 *
 * Purpose: Provide an abstraction layer so tactical components (e.g.
 *          ReactionFireDetector, rollout policies) can reuse ASC's combat
 *          formulas without directly depending on concrete helpers.
 *
 * Part of: ASC MCTS AI (Phase 2.1 - Utility-Agent Framework)
 ***************************************************************************/

#ifndef MCTS_I_COMBAT_CALCULATOR_H
#define MCTS_I_COMBAT_CALCULATOR_H

#include "types.h"
#include "unit_snapshot.h"

namespace asc {
namespace mcts {

/**
 * Interface for computing expected combat damage using ASC's real formulas.
 */
class ICombatCalculator {
public:
    virtual ~ICombatCalculator() = default;

    /**
     * Calculate the expected damage the attacker would inflict on the target
     * when attacking from the specified coordinate.
     *
     * @param attacker Snapshot of the attacking unit
     * @param target Snapshot of the defending unit
     * @param attackFrom Coordinate the attack is evaluated from
     * @return Expected damage in hit points (0-100)
     */
    [[nodiscard]] virtual float calculateExpectedDamage(
        const UnitSnapshot& attacker,
        const UnitSnapshot& target,
        const MapCoordinate& attackFrom
    ) const = 0;
};

} // namespace mcts
} // namespace asc

#endif // MCTS_I_COMBAT_CALCULATOR_H
