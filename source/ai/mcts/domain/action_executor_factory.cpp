/***************************************************************************
 * action_executor_factory.cpp - Factory for creating action executors
 * 
 * Part of: ASC MCTS AI (Phase 0.2 - Action Execution Interface)
 ***************************************************************************/

#include "i_action_executor.h"
#include "simulation_action_executor.h"
#include "real_game_action_executor.h"

namespace asc {
namespace mcts {

std::unique_ptr<IActionExecutor> ActionExecutorFactory::createSimulationExecutor(
    std::unique_ptr<GameStateSnapshot> snapshot
) {
    return std::make_unique<SimulationActionExecutor>(std::move(snapshot));
}

std::unique_ptr<IActionExecutor> ActionExecutorFactory::createRealGameExecutor(
    GameMap* gameMap
) {
    return std::make_unique<RealGameActionExecutor>(gameMap);
}

} // namespace mcts
} // namespace asc
