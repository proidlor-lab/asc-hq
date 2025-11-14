/***************************************************************************
 * combat_ability.cpp - Combat action generation implementation
 ***************************************************************************/

#include "combat_ability.h"
#include "../../../vehicletype.h"
#include <cmath>

namespace asc {
namespace mcts {

bool CombatAbility::isAvailable(
    const IGameState& state,
    const UnitSnapshot& unit) const {
    
    // Can't attack if already attacked this turn
    if (hasAttackedThisTurn(unit)) {
        return false;
    }
    
    // Must have ammo
    if (!hasAmmo(unit)) {
        return false;
    }
    
    // Must have weapons
    if (!appliesToUnitType(unit.type)) {
        return false;
    }
    
    return true;
}

std::vector<Action> CombatAbility::generateActions(
    const IGameState& state,
    const UnitSnapshot& unit) const {
    
    if (!isAvailable(state, unit)) {
        return {};
    }
    
    auto targets = findTargetsInRange(state, unit);
    
    std::vector<Action> actions;
    actions.reserve(targets.size());
    
    for (const auto* target : targets) {
        actions.emplace_back(AttackAction{
            unit.networkID,
            target->getPosition(),
            -1  // Auto-select weapon
        });
    }
    
    return actions;
}

bool CombatAbility::appliesToUnitType(const VehicleType* type) const {
    if (!type) {
        return false;
    }
    
    // Check if unit has any weapons
    // VehicleType has a weapons member (UnitWeapon) with count and weapon array
    return type->weapons.count > 0;
}

std::vector<const UnitSnapshot*> CombatAbility::findTargetsInRange(
    const IGameState& state,
    const UnitSnapshot& unit) const {
    
    std::vector<const UnitSnapshot*> targets;
    
    // Simplified range calculation for MVP
    // TODO (Phase 1.4): Use actual weapon ranges from VehicleType
    constexpr int MAX_WEAPON_RANGE = 10;  // Conservative estimate
    
    // Get all units in snapshot
    for (const auto& potentialTarget : state.getUnits()) {
        // Skip if same unit
        if (potentialTarget.networkID == unit.networkID) {
            continue;
        }
        
        // Skip if same player (friendly fire not allowed)
        if (potentialTarget.owner == unit.owner) {
            continue;
        }
        
        // Skip if destroyed
        if (potentialTarget.getHPPercent() == 0) {
            continue;
        }
        
        // Check range
        int distance = hexDistance(unit.getPosition(), potentialTarget.getPosition());
        if (distance > MAX_WEAPON_RANGE) {
            continue;
        }
        
        // Valid target
        targets.push_back(&potentialTarget);
    }
    
    return targets;
}

int CombatAbility::hexDistance(MapCoordinate a, MapCoordinate b) const {
    // Simplified hex distance (Manhattan distance approximation)
    // TODO (Phase 1.4): Use actual hex distance calculation
    // See: https://www.redblobgames.com/grids/hexagons/#distances
    
    int dx = std::abs(a.x - b.x);
    int dy = std::abs(a.y - b.y);
    
    // Approximate hex distance
    return dx + std::max(0, (dy - dx / 2));
}

} // namespace mcts
} // namespace asc
