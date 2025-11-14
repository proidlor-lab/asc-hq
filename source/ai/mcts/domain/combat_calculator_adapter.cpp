/***************************************************************************
 * combat_calculator_adapter.cpp - Adapter implementation for combat damage
 ***************************************************************************/

#include "combat_calculator_adapter.h"
#include "combat_calculator.h"
#include "unit_snapshot.h"
#include "types.h"

#include <algorithm>

// Forward declaration for ASC vehicle data
#include "../../../vehicletype.h"

namespace asc {
namespace mcts {

float CombatCalculatorAdapter::calculateExpectedDamage(
    const UnitSnapshot& attacker,
    const UnitSnapshot& target,
    const MapCoordinate& attackFrom
) const {
    const MapCoordinate targetPos = target.getPosition();
    int distance = hexDistance(attackFrom, targetPos);
    if (distance <= 0) {
        distance = 1; // minimum actionable distance (adjacent attack)
    }

    // Fallback: if we have no weapon data, rely on CombatCalculator's internal fallback
    const VehicleType* attackerType = attacker.type;
    if (attackerType == nullptr || attackerType->weapons.count == 0) {
        return static_cast<float>(computeDamageForWeapon(attacker, target, 0, distance));
    }

    const VehicleType* targetType = target.type;
    const int weaponCount = std::min<int>(attackerType->weapons.count, 16);

    int bestDamage = 0;

    for (int weaponIndex = 0; weaponIndex < weaponCount; ++weaponIndex) {
        // Check ammo availability
        if (!(attacker.ammoMask & (1 << weaponIndex))) {
            continue;
        }

        const auto& weapon = attackerType->weapons.weapon[weaponIndex];

        // Convert range (ASC stores as multiples of 10)
        const int minRange = (weapon.mindistance + 9) / 10;
        const int maxRange = weapon.maxdistance / 10;
        if (distance < minRange || distance > maxRange) {
            continue;
        }

        // Ensure weapon can target this unit type/height
        if (targetType != nullptr) {
            auto targetCheck = CombatCalculator::canWeaponTarget(
                weapon, attacker.height, target.height, targetType->movemalustyp);
            if (!targetCheck.canTarget) {
                continue;
            }
        }

        const int damage = computeDamageForWeapon(attacker, target, weaponIndex, distance);
        bestDamage = std::max(bestDamage, damage);
    }

    return static_cast<float>(bestDamage);
}

int CombatCalculatorAdapter::hexDistance(
    const MapCoordinate& a,
    const MapCoordinate& b
) noexcept {
    const int dx = static_cast<int>(b.x) - static_cast<int>(a.x);
    const int dy = static_cast<int>(b.y) - static_cast<int>(a.y);
    return (std::abs(dx) + std::abs(dy) + std::abs(dx - dy)) / 2;
}

float CombatCalculatorAdapter::computeDamageForWeapon(
    const UnitSnapshot& attacker,
    const UnitSnapshot& target,
    int weaponIndex,
    int distance
) {
    // TODO: Integrate terrain defense bonus and hemming factor once available
    constexpr int terrainDefenseBonus = 0;
    constexpr float hemmingFactor = 1.0f;

    return static_cast<float>(CombatCalculator::calculateDamage(
        attacker,
        target,
        weaponIndex,
        distance,
        terrainDefenseBonus,
        hemmingFactor));
}

} // namespace mcts
} // namespace asc
