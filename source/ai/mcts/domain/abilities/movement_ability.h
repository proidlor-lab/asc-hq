/***************************************************************************
 * movement_ability.h - Movement action generation capability
 *
 * Purpose: Generate move actions based on unit movement capabilities
 *
 * Part of: ASC MCTS AI (Phase 1.2 - Capability-Based Action Generation)
 ***************************************************************************/

#ifndef MCTS_MOVEMENT_ABILITY_H
#define MCTS_MOVEMENT_ABILITY_H

#include "i_ability.h"

// Forward declarations
class GameMap;

namespace asc {
namespace mcts {

/**
 * Movement ability - generates MoveActions for units that can move
 *
 * Applies to: Any unit with movement > 0
 * Generates: MoveActions to all reachable hexes within movement budget
 *
 * Implementation: Uses ASC's AStar3D pathfinding via PathfindingAdapter
 * Handles: Terrain costs, height changes, containers, reaction fire zones
 */
class MovementAbility : public IAbility {
  public:
   MovementAbility() = default;
   ~MovementAbility() override = default;

   [[nodiscard]] bool isAvailable(const IGameState& state, const UnitSnapshot& unit) const override;

   [[nodiscard]] std::vector<Action> generateActions(const IGameState& state,
                                                     const UnitSnapshot& unit) const override;

   [[nodiscard]] ActionCategory getCategory() const noexcept override {
      return ActionCategory::Movement;
   }

   [[nodiscard]] int getPriority() const noexcept override {
      return 90;  // High priority (second to combat)
   }

   [[nodiscard]] std::string getName() const override { return "MovementAbility"; }

   [[nodiscard]] bool appliesToUnitType(const VehicleType* type) const override;

   /**
    * Set context for action generation (GameMap for pathfinding)
    * Called by AbilityActionGenerator before generateActions()
    */
   void setContext(GameMap* map) { contextMap = map; }

  private:
   // Context provided by action generator
   mutable GameMap* contextMap = nullptr;
   /**
    * Generate movement actions using full pathfinding
    *
    * Uses AStar3D via PathfindingAdapter to find all reachable positions
    */
   [[nodiscard]] std::vector<MoveAction> generateReachableMoves(const IGameState& state,
                                                                const UnitSnapshot& unit) const;

   /**
    * LEGACY: Generate moves to adjacent hexes only (fallback if no map context)
    */
   [[nodiscard]] std::vector<MoveAction> generateAdjacentMoves(const IGameState& state,
                                                               const UnitSnapshot& unit) const;

   /**
    * Get 6 adjacent hex coordinates (used by fallback)
    */
   [[nodiscard]] std::vector<MapCoordinate> getAdjacentHexes(MapCoordinate center) const;
};

}  // namespace mcts
}  // namespace asc

#endif  // MCTS_MOVEMENT_ABILITY_H
