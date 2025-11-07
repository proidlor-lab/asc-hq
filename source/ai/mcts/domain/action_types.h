/***************************************************************************
 * action_types.h - Action representation for MCTS simulations
 * 
 * Purpose: Lightweight, value-based action types for tactical AI
 * 
 * Part of: ASC MCTS AI (Phase 0.2 - Action Execution Interface)
 * Updated: C++23 with std::variant, concepts, and constexpr
 ***************************************************************************/

#ifndef MCTS_ACTION_TYPES_H
#define MCTS_ACTION_TYPES_H

#include "types.h"
#include <variant>
#include <optional>
#include <string>
#include <compare>

namespace asc {
namespace mcts {

/**
 * Action categories for MCTS (Phase 0.2 MVP)
 * 
 * Extensible design: More action types can be added post-MVP
 * (e.g., RepairAction, ServiceAction, BuildAction, etc.)
 */

/**
 * Move action: Move unit to destination
 * 
 * C++23: constexpr, three-way comparison, aggregate initialization
 */
struct MoveAction {
    UnitID unitID;                    // Unit to move
    MapCoordinate destination;        // Target position
    int8_t targetHeight{0};           // Target height level (0 if same)
    
    constexpr MoveAction() noexcept = default;
    
    constexpr MoveAction(UnitID id, MapCoordinate dest, int8_t height = 0) noexcept
        : unitID(id), destination(dest), targetHeight(height) {}
    
    // C++20: Three-way comparison
    constexpr auto operator<=>(const MoveAction&) const noexcept = default;
    
    [[nodiscard]] std::string toString() const;
};

/**
 * Attack action: Attack target position
 * 
 * C++23: constexpr, three-way comparison
 */
struct AttackAction {
    UnitID attackerID;                // Attacking unit
    MapCoordinate target;             // Target position
    int weaponIndex{-1};              // Weapon to use (-1 = auto-select)
    
    constexpr AttackAction() noexcept = default;
    
    constexpr AttackAction(UnitID attacker, MapCoordinate tgt, int weapon = -1) noexcept
        : attackerID(attacker), target(tgt), weaponIndex(weapon) {}
    
    // C++20: Three-way comparison
    constexpr auto operator<=>(const AttackAction&) const noexcept = default;
    
    [[nodiscard]] std::string toString() const;
};

/**
 * Wait action: Unit does nothing this turn
 * 
 * C++23: constexpr, minimal footprint
 */
struct WaitAction {
    UnitID unitID;                    // Unit to wait
    
    constexpr WaitAction() noexcept = default;
    explicit constexpr WaitAction(UnitID id) noexcept : unitID(id) {}
    
    // C++20: Three-way comparison
    constexpr auto operator<=>(const WaitAction&) const noexcept = default;
    
    [[nodiscard]] std::string toString() const;
};

/**
 * Type-safe action variant (C++17: std::variant)
 * 
 * Extensible: Add new action types here as needed
 * 
 * Example post-MVP extensions:
 * - RepairAction
 * - ServiceAction (fuel/ammo transfer)
 * - BuildAction
 * - ResearchAction
 */
using Action = std::variant<MoveAction, AttackAction, WaitAction>;

/**
 * Action result type
 * 
 * C++23: constexpr, modern enum class
 */
enum class ActionResultCode : uint8_t {
    Success = 0,           // Action executed successfully
    Failure,              // Generic failure
    IllegalAction,        // Action is not legal (e.g., invalid destination)
    InsufficientMovement, // Unit doesn't have enough movement points
    InsufficientAmmo,     // Unit doesn't have ammo for attack
    InsufficientFuel,     // Unit doesn't have fuel
    OutOfRange,           // Target is out of range
    UnitNotFound,         // Unit ID not found in snapshot
    TargetNotFound,       // Target not found
    Blocked,              // Path blocked or destination occupied
    ReactionFire          // Unit was destroyed by reaction fire
};

/**
 * Detailed action result with error information
 * 
 * C++23: constexpr, std::optional for optional error messages
 */
struct ActionResult {
    ActionResultCode code;
    std::optional<std::string> message;  // Optional error message
    
    // Damage dealt (for combat actions, -1 if not applicable)
    int damageDealt{-1};
    
    // Fuel consumed (for movement actions, -1 if not applicable)
    int fuelConsumed{-1};
    
    // Movement points consumed
    int movementConsumed{-1};
    
    constexpr ActionResult() noexcept 
        : code(ActionResultCode::Success) {}
    
    explicit constexpr ActionResult(ActionResultCode c) noexcept 
        : code(c) {}
    
    constexpr ActionResult(ActionResultCode c, std::string msg) noexcept
        : code(c), message(std::move(msg)) {}
    
    [[nodiscard]] constexpr bool isSuccess() const noexcept {
        return code == ActionResultCode::Success;
    }
    
    [[nodiscard]] constexpr bool isFailure() const noexcept {
        return !isSuccess();
    }
    
    [[nodiscard]] std::string toString() const;
    
    // Factory methods for common results
    [[nodiscard]] static constexpr ActionResult success() noexcept {
        return ActionResult(ActionResultCode::Success);
    }
    
    [[nodiscard]] static ActionResult failure(std::string msg) noexcept {
        return ActionResult(ActionResultCode::Failure, std::move(msg));
    }
    
    [[nodiscard]] static ActionResult illegalAction(std::string msg) noexcept {
        return ActionResult(ActionResultCode::IllegalAction, std::move(msg));
    }
};

/**
 * Action visitor helper (C++23: concept-based)
 * 
 * Usage:
 *   Action action = MoveAction{...};
 *   std::visit(overloaded {
 *       [](const MoveAction& m) { ... },
 *       [](const AttackAction& a) { ... },
 *       [](const WaitAction& w) { ... }
 *   }, action);
 */
template<class... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};

// C++17: Deduction guide for overloaded
template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

/**
 * Get action type as string (for debugging/logging)
 */
[[nodiscard]] inline std::string getActionTypeName(const Action& action) {
    return std::visit(overloaded {
        [](const MoveAction&) { return std::string("Move"); },
        [](const AttackAction&) { return std::string("Attack"); },
        [](const WaitAction&) { return std::string("Wait"); }
    }, action);
}

/**
 * Get unit ID from any action type (C++23: constexpr if)
 */
[[nodiscard]] inline UnitID getActionUnitID(const Action& action) {
    return std::visit(overloaded {
        [](const MoveAction& m) { return m.unitID; },
        [](const AttackAction& a) { return a.attackerID; },
        [](const WaitAction& w) { return w.unitID; }
    }, action);
}

/**
 * Convert action to string representation
 */
[[nodiscard]] inline std::string actionToString(const Action& action) {
    return std::visit(overloaded {
        [](const MoveAction& m) { return m.toString(); },
        [](const AttackAction& a) { return a.toString(); },
        [](const WaitAction& w) { return w.toString(); }
    }, action);
}

} // namespace mcts
} // namespace asc

#endif // MCTS_ACTION_TYPES_H
