/***************************************************************************
 * movement_ability.cpp - Movement action generation implementation
 ***************************************************************************/

#include "movement_ability.h"
#include "../../../vehicletype.h"
#include "../../infrastructure/pathfinding_adapter.h"

namespace asc {
namespace mcts {

bool MovementAbility::isAvailable(
    const IGameState& state,
    const UnitSnapshot& unit) const {
    
    // Unit must have movement points
    if (unit.movement <= 0) {
        return false;
    }
    
    // Unit must have fuel (if fuel consumption is tracked)
    if (unit.fuel == 0 && unit.type && unit.type->fuelConsumption > 0) {
        return false;
    }
    
    return true;
}

std::vector<Action> MovementAbility::generateActions(
    const IGameState& state,
    const UnitSnapshot& unit) const {
    
    if (!isAvailable(state, unit)) {
        return {};
    }
    
    // Use full pathfinding if map context available, else fallback to adjacent
    std::vector<MoveAction> moveActions;
    if (contextMap) {
        moveActions = generateReachableMoves(state, unit);
    } else {
        // Fallback to adjacent-only if no map context
        moveActions = generateAdjacentMoves(state, unit);
    }
    
    // Convert MoveAction vector to Action variant vector
    std::vector<Action> actions;
    actions.reserve(moveActions.size());
    
    for (const auto& move : moveActions) {
        actions.emplace_back(move);
    }
    
    return actions;
}

bool MovementAbility::appliesToUnitType(const VehicleType* type) const {
    if (!type) {
        return false;
    }
    
    // Check if unit has any movement capability
    // VehicleType has movement vector for different height levels
    // If any movement value > 0, unit can move
    for (size_t i = 0; i < type->movement.size(); ++i) {
        if (type->movement[i] > 0) {
            return true;
        }
    }
    
    return false;
}

std::vector<MoveAction> MovementAbility::generateReachableMoves(
    const IGameState& state,
    const UnitSnapshot& unit) const {
    
    std::vector<MoveAction> actions;
    
    if (!contextMap) {
        return actions;  // No map context, can't pathfind
    }
    
    // Use PathfindingAdapter to find all reachable positions
    PathfindingAdapter adapter;
    auto reachable = adapter.findReachablePositions(state, unit, contextMap);
    
    // Convert reachable positions to MoveActions
    actions.reserve(reachable.size());
    
    for (const auto& pos : reachable) {
        // Skip current position (no-op move)
        if (pos.position.x == unit.x && pos.position.y == unit.y) {
            continue;
        }
        
        // Generate move action
        actions.emplace_back(unit.networkID, pos.position);
    }
    
    return actions;
}

std::vector<MoveAction> MovementAbility::generateAdjacentMoves(
    const IGameState& state,
    const UnitSnapshot& unit) const {
    
    // FALLBACK: Adjacent-only generation (used if no map context)
    std::vector<MoveAction> actions;
    auto center = unit.getPosition();
    auto adjacent = getAdjacentHexes(center);
    
    for (const auto& dest : adjacent) {
        // Basic bounds checking
        if (dest.x < 0 || dest.y < 0) {
            continue;
        }
        
        // Skip if same as current position
        if (dest.x == center.x && dest.y == center.y) {
            continue;
        }
        
        // Generate action, legality check happens in executor
        actions.emplace_back(unit.networkID, dest);
    }
    
    return actions;
}

std::vector<MapCoordinate> MovementAbility::getAdjacentHexes(
    MapCoordinate center) const {
    
    // Hex grid neighbors (assuming even-q coordinate system)
    // See: https://www.redblobgames.com/grids/hexagons/
    
    std::vector<MapCoordinate> neighbors;
    neighbors.reserve(6);
    
    const bool evenColumn = (center.x % 2 == 0);
    
    if (evenColumn) {
        // Even column offsets
        neighbors.emplace_back(center.x + 1, center.y);     // E
        neighbors.emplace_back(center.x + 1, center.y - 1); // SE
        neighbors.emplace_back(center.x, center.y - 1);     // SW
        neighbors.emplace_back(center.x - 1, center.y);     // W
        neighbors.emplace_back(center.x - 1, center.y - 1); // NW
        neighbors.emplace_back(center.x, center.y + 1);     // NE
    } else {
        // Odd column offsets
        neighbors.emplace_back(center.x + 1, center.y);     // E
        neighbors.emplace_back(center.x, center.y - 1);     // SE
        neighbors.emplace_back(center.x - 1, center.y - 1); // SW
        neighbors.emplace_back(center.x - 1, center.y);     // W
        neighbors.emplace_back(center.x, center.y + 1);     // NW
        neighbors.emplace_back(center.x + 1, center.y + 1); // NE
    }
    
    return neighbors;
}

} // namespace mcts
} // namespace asc
