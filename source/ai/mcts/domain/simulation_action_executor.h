/***************************************************************************
 * simulation_action_executor.h - Action executor for MCTS simulations
 * 
 * Purpose: Execute actions on GameStateSnapshot (not real GameMap)
 *          Optimized for performance: no GUI, no network, no persistence
 * 
 * Part of: ASC MCTS AI (Phase 0.2 - Action Execution Interface)
 * Updated: C++23 with modern memory management
 ***************************************************************************/

#ifndef MCTS_SIMULATION_ACTION_EXECUTOR_H
#define MCTS_SIMULATION_ACTION_EXECUTOR_H

#include "i_action_executor.h"
#include "game_state_snapshot.h"
#include <memory>
#include <vector>
#include <deque>

namespace asc {
namespace mcts {

/**
 * Action executor for MCTS simulations
 * 
 * Executes actions on GameStateSnapshot (lightweight copy of game state)
 * 
 * Design:
 * - Operates on mutable snapshot (owned by executor)
 * - Fast: No ASC Command overhead, no GUI updates
 * - Simplified: MVP focuses on move + attack
 * - Extendable: Can add more action types post-MVP
 * 
 * Performance targets:
 * - Execute move: <0.1ms
 * - Execute attack: <0.2ms
 * - Generate legal actions: <1ms for 20 units
 */
class SimulationActionExecutor : public IActionExecutor {
public:
    /**
     * Constructor
     * 
     * @param initialState Initial game state (executor takes ownership)
     */
    explicit SimulationActionExecutor(std::unique_ptr<GameStateSnapshot> initialState);
    
    ~SimulationActionExecutor() override = default;
    
    // Disable copy (use clone() instead)
    SimulationActionExecutor(const SimulationActionExecutor&) = delete;
    SimulationActionExecutor& operator=(const SimulationActionExecutor&) = delete;
    
    // Enable move
    SimulationActionExecutor(SimulationActionExecutor&&) noexcept = default;
    SimulationActionExecutor& operator=(SimulationActionExecutor&&) noexcept = default;
    
    // ========== IActionExecutor Interface ==========
    
    [[nodiscard]] ActionResult execute(
        const Action& action,
        const ExecutionContext& context = ExecutionContext()
    ) override;
    
    [[nodiscard]] ActionResult isLegal(const Action& action) const override;
    
    [[nodiscard]] std::vector<Action> generateLegalActions(
        UnitID unitID,
        bool includeWait = true
    ) const override;
    
    bool undo() override;
    
    [[nodiscard]] const GameStateSnapshot& getState() const override {
        return *state_;
    }
    
    // ========== Simulation-Specific Methods ==========
    
    /**
     * Get mutable state (for testing/debugging)
     */
    [[nodiscard]] GameStateSnapshot& getStateMutable() {
        return *state_;
    }
    
    /**
     * Clone executor with current state
     * 
     * Useful for tree search (create child nodes)
     */
    [[nodiscard]] std::unique_ptr<SimulationActionExecutor> clone() const;
    
    /**
     * Reset to initial state
     */
    void reset(std::unique_ptr<GameStateSnapshot> newState);
    
    /**
     * Get action history (for debugging)
     */
    [[nodiscard]] const std::vector<Action>& getActionHistory() const {
        return actionHistory_;
    }
    
private:
    // ========== State ==========
    
    std::unique_ptr<GameStateSnapshot> state_;
    std::vector<Action> actionHistory_;
    
    // Undo stack (stores pre-action snapshots)
    // Limited depth to avoid memory bloat
    static constexpr size_t MAX_UNDO_DEPTH = 10;
    std::deque<std::unique_ptr<GameStateSnapshot>> undoStack_;
    
    // ========== Action Execution (type-specific) ==========
    
    [[nodiscard]] ActionResult executeMove(
        const MoveAction& action,
        const ExecutionContext& context
    );
    
    [[nodiscard]] ActionResult executeAttack(
        const AttackAction& action,
        const ExecutionContext& context
    );
    
    [[nodiscard]] ActionResult executeWait(
        const WaitAction& action,
        const ExecutionContext& context
    );
    
    // ========== Legality Checks ==========
    
    [[nodiscard]] ActionResult checkMoveLegal(const MoveAction& action) const;
    [[nodiscard]] ActionResult checkAttackLegal(const AttackAction& action) const;
    [[nodiscard]] ActionResult checkWaitLegal(const WaitAction& action) const;
    
    // ========== Action Generation ==========
    
    [[nodiscard]] std::vector<MoveAction> generateMoveActions(UnitID unitID) const;
    [[nodiscard]] std::vector<AttackAction> generateAttackActions(UnitID unitID) const;
    
    // ========== Helper Methods ==========
    
    /**
     * Calculate movement cost (simplified A* for MVP)
     * 
     * @param unit Unit snapshot
     * @param from Starting position
     * @param to Destination position
     * @return Movement cost (-1 if unreachable)
     */
    [[nodiscard]] int calculateMovementCost(
        const UnitSnapshot& unit,
        const MapCoordinate& from,
        const MapCoordinate& to
    ) const;
    
    /**
     * Check if position is reachable by unit
     */
    [[nodiscard]] bool isReachable(
        const UnitSnapshot& unit,
        const MapCoordinate& dest
    ) const;
    
    /**
     * Calculate attack damage (simplified combat for MVP)
     * 
     * @param attacker Attacking unit
     * @param defender Defending unit
     * @param weaponIndex Weapon to use (-1 = auto-select)
     * @return Damage dealt (0-100)
     */
    [[nodiscard]] int calculateDamage(
        const UnitSnapshot& attacker,
        const UnitSnapshot& defender,
        int weaponIndex
    ) const;
    
    /**
     * Check if attacker can hit target
     * 
     * @param attacker Attacking unit
     * @param target Target position
     * @param weaponIndex Weapon index (-1 = auto-select)
     * @return Weapon index if in range, -1 if out of range
     */
    [[nodiscard]] int canHitTarget(
        const UnitSnapshot& attacker,
        const MapCoordinate& target,
        int weaponIndex
    ) const;
    
    /**
     * Simulate reaction fire (simplified for MVP)
     * 
     * @param movingUnit Unit that is moving
     * @param path Path the unit is taking
     * @param context Execution context
     * @return true if unit survived, false if destroyed
     */
    [[nodiscard]] bool simulateReactionFire(
        UnitSnapshot& movingUnit,
        const std::vector<MapCoordinate>& path,
        const ExecutionContext& context
    );
    
    /**
     * Get hex distance between two positions
     */
    [[nodiscard]] static constexpr int hexDistance(
        const MapCoordinate& a,
        const MapCoordinate& b
    ) noexcept {
        // Hex distance formula
        int dx = b.x - a.x;
        int dy = b.y - a.y;
        return (abs(dx) + abs(dy) + abs(dx - dy)) / 2;
    }
    
    /**
     * Get neighboring positions (6 hex neighbors)
     */
    [[nodiscard]] std::vector<MapCoordinate> getNeighbors(
        const MapCoordinate& pos
    ) const;
    
    /**
     * Save state for undo
     */
    void saveUndoState();
};

} // namespace mcts
} // namespace asc

#endif // MCTS_SIMULATION_ACTION_EXECUTOR_H
