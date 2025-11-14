/***************************************************************************
 * action_executor_factory.cpp - Factory for creating action executors
 * 
 * Part of: ASC MCTS AI (Phase 0.2 - Action Execution Interface)
 ***************************************************************************/

#include "i_action_executor.h"
#include "simulation_action_executor.h"
#include "real_game_action_executor.h"
#include <stdexcept>

namespace asc {
namespace mcts {

std::unique_ptr<IActionExecutor> ActionExecutorFactory::createSimulationExecutor(
    std::unique_ptr<IGameState> snapshot,
    GameMap* legacyMap
) {
    auto* concrete = dynamic_cast<GameStateSnapshot*>(snapshot.release());
    if (!concrete) {
        throw std::invalid_argument(
            "SimulationActionExecutor currently requires GameStateSnapshot state");
    }
    std::unique_ptr<GameStateSnapshot> concretePtr(concrete);
    return std::make_unique<SimulationActionExecutor>(std::move(concretePtr), legacyMap);
}

std::unique_ptr<IActionExecutor> ActionExecutorFactory::createRealGameExecutor(
    GameMap* gameMap
) {
    return std::make_unique<RealGameActionExecutor>(gameMap);
}

} // namespace mcts
} // namespace asc
