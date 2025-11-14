/***************************************************************************
 * real_game_action_executor.h - Action executor for real ASC GameMap
 * 
 * Purpose: Execute MCTS-decided actions on actual game state
 *          Integrates with ASC Command system
 * 
 * Part of: ASC MCTS AI (Phase 0.2 - Action Execution Interface)
 * Status: STUB - Will be fully implemented when integrating with ASC game
 ***************************************************************************/

#ifndef MCTS_REAL_GAME_ACTION_EXECUTOR_H
#define MCTS_REAL_GAME_ACTION_EXECUTOR_H

#include "i_action_executor.h"
#include "game_state_reader.h"
#include <memory>

// Forward declarations
class GameMap;

namespace asc {
namespace mcts {

/**
 * Action executor for real game (operates on GameMap)
 * 
 * Phase 0.2 Status: STUB
 * 
 * This executor will:
 * 1. Convert MCTS actions to ASC Commands
 * 2. Execute Commands on real GameMap
 * 3. Handle ASC action queue integration
 * 
 * Full implementation planned for Phase 1.4 (Integration & End-to-End Test)
 */
class RealGameActionExecutor : public IActionExecutor {
public:
    /**
     * Constructor
     * 
     * @param gameMap Pointer to ASC GameMap (executor does NOT own)
     */
    explicit RealGameActionExecutor(GameMap* gameMap);
    
    ~RealGameActionExecutor() override = default;
    
    // Disable copy/move (manages external GameMap reference)
    RealGameActionExecutor(const RealGameActionExecutor&) = delete;
    RealGameActionExecutor& operator=(const RealGameActionExecutor&) = delete;
    RealGameActionExecutor(RealGameActionExecutor&&) = delete;
    RealGameActionExecutor& operator=(RealGameActionExecutor&&) = delete;
    
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
    
    // Undo not supported for real game executor
    bool undo() override { return false; }
    
    [[nodiscard]] const IGameState& getState() const override;
    
private:
    GameMap* gameMap_;  // Non-owning pointer to real game map
    
    // Cached snapshot (created on-demand, read-only view)
    mutable std::unique_ptr<GameStateSnapshot> cachedSnapshot_;
    mutable bool snapshotDirty_ = true;
    
    // Game state reader (dependency injection)
    std::unique_ptr<GameStateReader> stateReader_;
    
    // ========== Action Execution (stubs for Phase 0.2) ==========
    
    [[nodiscard]] ActionResult executeMoveReal(
        const MoveAction& action,
        const ExecutionContext& context
    );
    
    [[nodiscard]] ActionResult executeAttackReal(
        const AttackAction& action,
        const ExecutionContext& context
    );
    
    [[nodiscard]] ActionResult executeWaitReal(
        const WaitAction& action,
        const ExecutionContext& context
    );
    
    // ========== Helper Methods ==========
    
    void invalidateSnapshot() const {
        snapshotDirty_ = true;
    }
    
    void refreshSnapshot() const;
};

} // namespace mcts
} // namespace asc

#endif // MCTS_REAL_GAME_ACTION_EXECUTOR_H
