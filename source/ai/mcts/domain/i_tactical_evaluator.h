/***************************************************************************
 * i_tactical_evaluator.h - Interface for tactical state evaluation
 *
 * Purpose: Evaluate game states for MCTS simulation/rollout
 *          Returns score indicating how favorable a state is for a player
 *
 * Part of: ASC MCTS AI (Phase 0.3 - Basic Evaluation Function)
 *
 * Design:
 * - Dependency injection pattern (same as IActionExecutor)
 * - Stateless evaluation (no side effects)
 * - Fast execution (<1ms target)
 * - Normalized scores (-1.0 to +1.0)
 ***************************************************************************/

#ifndef MCTS_I_TACTICAL_EVALUATOR_H
#define MCTS_I_TACTICAL_EVALUATOR_H

#include "i_game_state.h"
#include "types.h"
#include <memory>

namespace asc {
namespace mcts {

/**
 * Configuration for evaluation
 *
 * Allows tuning heuristic weights and enabling/disabling specific features
 */
struct EvaluationContext {
   // Which player to evaluate from perspective of
   PlayerID perspectivePlayer{0};

   // Feature weights (0.0 = disabled, 1.0 = normal weight)
   float materialWeight{1.0f};
   float positionWeight{0.5f};
   float healthWeight{0.8f};
   float threatWeight{0.6f};

   // Enable/disable specific heuristics
   bool enableReactionFirePenalty{true};
   bool enableFireConcentrationBonus{true};
   bool enableDefensivePositionBonus{true};
   bool enableTerrainBonus{true};

   constexpr EvaluationContext() noexcept = default;
};

/**
 * Result of state evaluation
 *
 * Contains both the final score and breakdown for debugging
 */
struct EvaluationResult {
   // Final score (-1.0 = worst for perspective player, +1.0 = best)
   float score{0.0f};

   // Score breakdown (for debugging/tuning)
   float materialScore{0.0f};
   float positionScore{0.0f};
   float healthScore{0.0f};
   float threatScore{0.0f};

   // Metadata
   int unitsEvaluated{0};
   bool isTerminal{false};  // Game over state?

   constexpr EvaluationResult() noexcept = default;

   /**
    * Create terminal state result (C++23: constexpr)
    */
   static constexpr EvaluationResult terminal(float score) noexcept {
      EvaluationResult result;
      result.score = score;
      result.isTerminal = true;
      return result;
   }
};

/**
 * Interface for tactical state evaluators
 *
 * Implementations provide different evaluation strategies:
 * - SimpleCombatEvaluator: Material-based (Phase 0.3 MVP)
 * - UtilityBasedEvaluator: Agent-based scoring (Phase 1.2+)
 * - NeuralEvaluator: ML-based (future)
 *
 * Design Pattern: Strategy Pattern + Dependency Injection
 */
class ITacticalEvaluator {
  public:
   virtual ~ITacticalEvaluator() = default;

   /**
    * Evaluate game state from perspective of a player
    *
    * @param snapshot Game state to evaluate
    * @param context Configuration (weights, enabled features)
    * @return Evaluation result with score and breakdown
    *
    * Performance target: <1ms per call
    * Score range: -1.0 (losing) to +1.0 (winning)
    */
   virtual EvaluationResult evaluate(const IGameState& snapshot,
                                     const EvaluationContext& context) const = 0;

   /**
    * Quick check if state is terminal (game over)
    *
    * @param snapshot Game state
    * @param player Player to check
    * @return true if player has won/lost (no units left, etc.)
    */
   virtual bool isTerminalState(const IGameState& snapshot, PlayerID player) const = 0;

   /**
    * Clone evaluator (for MCTS tree nodes if needed)
    *
    * Note: Most evaluators are stateless, so can return shared pointer
    *
    * @return New evaluator instance
    */
   virtual std::unique_ptr<ITacticalEvaluator> clone() const = 0;

   /**
    * Get human-readable name for debugging
    */
   virtual const char* getName() const noexcept = 0;
};

/**
 * Factory for creating evaluators
 *
 * Dependency injection pattern (same as ActionExecutorFactory)
 */
class EvaluatorFactory {
  public:
   /**
    * Create simple combat evaluator (material-based)
    *
    * @return SimpleCombatEvaluator instance
    */
   static std::unique_ptr<ITacticalEvaluator> createSimpleCombatEvaluator();

   /**
    * Create default evaluator (for MVP = SimpleCombatEvaluator)
    */
   static std::unique_ptr<ITacticalEvaluator> createDefault() {
      return createSimpleCombatEvaluator();
   }
};

}  // namespace mcts
}  // namespace asc

#endif  // MCTS_I_TACTICAL_EVALUATOR_H
