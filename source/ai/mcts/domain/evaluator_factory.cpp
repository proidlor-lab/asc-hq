/***************************************************************************
 * evaluator_factory.cpp - Factory for creating tactical evaluators
 ***************************************************************************/

#include "i_tactical_evaluator.h"
#include "simple_combat_evaluator.h"

namespace asc {
namespace mcts {

std::unique_ptr<ITacticalEvaluator> EvaluatorFactory::createSimpleCombatEvaluator() {
    return std::make_unique<SimpleCombatEvaluator>();
}

} // namespace mcts
} // namespace asc
