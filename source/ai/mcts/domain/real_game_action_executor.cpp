/***************************************************************************
 * real_game_action_executor.cpp - Implementation stub for real game executor
 * 
 * Part of: ASC MCTS AI (Phase 0.2 - Action Execution Interface)
 * Status: STUB - Full implementation in Phase 1.4
 ***************************************************************************/

#include "real_game_action_executor.h"
#include "../../../gamemap.h"
#include <stdexcept>

namespace asc {
namespace mcts {

// ========== Constructor ==========

RealGameActionExecutor::RealGameActionExecutor(GameMap* gameMap)
    : gameMap_(gameMap)
{
    if (!gameMap_) {
        throw std::invalid_argument("RealGameActionExecutor: gameMap cannot be null");
    }
    
    // Create state reader for reading game state
    stateReader_ = std::make_unique<GameStateReader>(gameMap_);
}

// ========== IActionExecutor Interface ==========

ActionResult RealGameActionExecutor::execute(
    const Action& action,
    const ExecutionContext& context
) {
    // Phase 0.2 STUB: Not yet implemented
    // Will be implemented in Phase 1.4 when integrating with ASC
    
    invalidateSnapshot();
    
    return std::visit(overloaded {
        [this, &context](const MoveAction& m) { return executeMoveReal(m, context); },
        [this, &context](const AttackAction& a) { return executeAttackReal(a, context); },
        [this, &context](const WaitAction& w) { return executeWaitReal(w, context); }
    }, action);
}

ActionResult RealGameActionExecutor::isLegal(const Action& action) const {
    // Phase 0.2 STUB: Basic implementation
    // Post-MVP: Full legality checking using ASC Command system
    
    return std::visit(overloaded {
        [](const MoveAction&) { return ActionResult::success(); },
        [](const AttackAction&) { return ActionResult::success(); },
        [](const WaitAction&) { return ActionResult::success(); }
    }, action);
}

std::vector<Action> RealGameActionExecutor::generateLegalActions(
    UnitID unitID,
    bool includeWait
) const {
    // Phase 0.2 STUB: Not yet implemented
    // Will be implemented in Phase 1.4
    
    std::vector<Action> actions;
    
    if (includeWait) {
        actions.emplace_back(WaitAction{unitID});
    }
    
    return actions;
}

const GameStateSnapshot& RealGameActionExecutor::getState() const {
    if (snapshotDirty_ || !cachedSnapshot_) {
        refreshSnapshot();
    }
    return *cachedSnapshot_;
}

// ========== Action Execution Stubs ==========

ActionResult RealGameActionExecutor::executeMoveReal(
    const MoveAction& action,
    const ExecutionContext& context
) {
    // Phase 0.2 STUB
    // TODO Phase 1.4: Create MoveUnitCommand and execute it
    
    context.log("RealGameActionExecutor::executeMoveReal - STUB (not yet implemented)");
    
    return ActionResult::failure("Not yet implemented (Phase 0.2 stub)");
}

ActionResult RealGameActionExecutor::executeAttackReal(
    const AttackAction& action,
    const ExecutionContext& context
) {
    // Phase 0.2 STUB
    // TODO Phase 1.4: Create AttackCommand and execute it
    
    context.log("RealGameActionExecutor::executeAttackReal - STUB (not yet implemented)");
    
    return ActionResult::failure("Not yet implemented (Phase 0.2 stub)");
}

ActionResult RealGameActionExecutor::executeWaitReal(
    const WaitAction& action,
    const ExecutionContext& context
) {
    // Phase 0.2 STUB
    // TODO Phase 1.4: Mark unit as done (consume movement)
    
    context.log("RealGameActionExecutor::executeWaitReal - STUB (not yet implemented)");
    
    return ActionResult::failure("Not yet implemented (Phase 0.2 stub)");
}

// ========== Helper Methods ==========

void RealGameActionExecutor::refreshSnapshot() const {
    // Create fresh snapshot from GameMap
    // Use createFullSnapshot for simplicity (Phase 0.2 stub)
    // Post-MVP: Can optimize with createTacticalSnapshot for specific regions
    cachedSnapshot_ = stateReader_->createFullSnapshot(0); // Perspective: player 0
    
    snapshotDirty_ = false;
}

} // namespace mcts
} // namespace asc
