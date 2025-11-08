/***************************************************************************
 * i_action_executor.h - Interface for executing actions on game state
 * 
 * Purpose: Dependency injection interface for action execution
 *          Enables both simulation (on snapshots) and real game execution
 * 
 * Part of: ASC MCTS AI (Phase 0.2 - Action Execution Interface)
 * Updated: C++23 with concepts and modern patterns
 ***************************************************************************/

#ifndef MCTS_I_ACTION_EXECUTOR_H
#define MCTS_I_ACTION_EXECUTOR_H

#include "action_types.h"
#include "game_state_snapshot.h"
#include <memory>
#include <vector>
#include <functional>

// Forward declaration
class GameMap;

namespace asc {
namespace mcts {

/**
 * Execution context for actions
 * 
 * Provides configuration and callbacks for action execution
 */
struct ExecutionContext {
    // Callbacks (optional)
    std::function<void(const std::string&)> onLog;  // Logging callback
    std::function<void(const Action&)> onActionExecuted;  // Action executed callback
    
    // Flags
    bool enableReactionFire{true};      // Simulate reaction fire?
    bool enableFuelConsumption{true};   // Consume fuel?
    bool enableAmmoConsumption{true};   // Consume ammo?
    bool enableLogging{false};          // Enable debug logging?
    
    // Performance limits (for simulation)
    int maxPathfindingNodes{1000};      // Limit A* search
    
    ExecutionContext() = default;
    
    // Log helper
    void log(const std::string& msg) const {
        if (enableLogging && onLog) {
            onLog(msg);
        }
    }
};

/**
 * Interface for executing actions on game state
 * 
 * Two implementations:
 * 1. SimulationActionExecutor - Executes on GameStateSnapshot (fast, for MCTS)
 * 2. RealGameActionExecutor - Executes on real GameMap (for final AI decision)
 * 
 * Design Pattern: Strategy + Dependency Injection
 * Benefits:
 * - Testable (can mock executor)
 * - Flexible (simulation vs. real execution)
 * - Clean separation of concerns
 */
class IActionExecutor {
public:
    virtual ~IActionExecutor() = default;
    
    /**
     * Execute action on game state
     * 
     * @param action Action to execute
     * @param context Execution context (configuration/callbacks)
     * @return Result of execution
     */
    [[nodiscard]] virtual ActionResult execute(
        const Action& action,
        const ExecutionContext& context = ExecutionContext()
    ) = 0;
    
    /**
     * Check if action is legal (without executing)
     * 
     * Fast pre-check before expensive simulation
     * 
     * @param action Action to validate
     * @return Result (Success if legal, error code otherwise)
     */
    [[nodiscard]] virtual ActionResult isLegal(const Action& action) const = 0;
    
    /**
     * Generate all legal actions for a unit
     * 
     * Used by action generators in MCTS
     * 
     * @param unitID Unit to generate actions for
     * @param includeWait Include wait action?
     * @return Vector of legal actions
     */
    [[nodiscard]] virtual std::vector<Action> generateLegalActions(
        UnitID unitID,
        bool includeWait = true
    ) const = 0;
    
    /**
     * Undo last action (optional, for simulation rollback)
     * 
     * Not all executors support undo (e.g., RealGameActionExecutor may not)
     * 
     * @return true if undo succeeded, false if not supported or failed
     */
    virtual bool undo() { return false; }
    
    /**
     * Get current game state (read-only)
     * 
     * For simulation executors, this returns the snapshot
     * For real executors, this may create a snapshot from GameMap
     */
    [[nodiscard]] virtual const GameStateSnapshot& getState() const = 0;
};

/**
 * Factory for creating action executors
 * 
 * C++23: Static factory pattern
 */
class ActionExecutorFactory {
public:
    /**
     * Create simulation executor (operates on snapshot)
     * 
     * @param snapshot Initial state (executor takes ownership)
     * @return Unique pointer to executor
     */
    [[nodiscard]] static std::unique_ptr<IActionExecutor> createSimulationExecutor(
        std::unique_ptr<GameStateSnapshot> snapshot
    );
    
    /**
     * Create real game executor (operates on GameMap)
     * 
     * @param gameMap Pointer to game map (executor does NOT take ownership)
     * @return Unique pointer to executor
     */
    [[nodiscard]] static std::unique_ptr<IActionExecutor> createRealGameExecutor(
        GameMap* gameMap
    );
};

} // namespace mcts
} // namespace asc

#endif // MCTS_I_ACTION_EXECUTOR_H
