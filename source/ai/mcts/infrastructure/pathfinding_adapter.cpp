/***************************************************************************
 * pathfinding_adapter.cpp - Pathfinding adapter implementation
 ***************************************************************************/

#include "pathfinding_adapter.h"

#if MCTS_ENABLE_LEGACY_PATHFINDING
#include "../../../astar2.h"
#include "../../../vehicle.h"
#include "../../../vehicletype.h"
#include "../../../gamemap.h"
#include "../../../mapfield.h"
#include "../../../player.h"
#endif

namespace asc {
namespace mcts {

std::vector<ReachablePosition>
PathfindingAdapter::findReachablePositions(const IGameState& state, const UnitSnapshot& unit,
                                           GameMap* legacyMap) const {
#if MCTS_ENABLE_LEGACY_PATHFINDING
   if (!legacyMap || !unit.type) {
      return {};
   }

   Vehicle* tempVehicle = createTempVehicle(unit, legacyMap);
   if (!tempVehicle) {
      return {};
   }

   try {
      AStar3D pathfinder(legacyMap, tempVehicle, false);
      pathfinder.findAllAccessibleFields();

      auto positions = extractReachablePositions(pathfinder);

      cleanupTempVehicle(tempVehicle, legacyMap);

      return positions;

   } catch (...) {
      cleanupTempVehicle(tempVehicle, legacyMap);
      return {};
   }
#else
   (void) state;
   (void) unit;
   (void) legacyMap;
   return {};
#endif
}

// ===== Private Helpers =====
// Full implementation ENABLED for main ASC game binary

Vehicle* PathfindingAdapter::createTempVehicle(const UnitSnapshot& unit, GameMap* map) const {
#if MCTS_ENABLE_LEGACY_PATHFINDING
   if (!unit.type || !map) {
      return nullptr;
   }

   try {
      Vehicle* vehicle = new Vehicle(unit.type, map, unit.owner);

      vehicle->networkid = unit.networkID;
      vehicle->xpos = unit.x;
      vehicle->ypos = unit.y;
      vehicle->height = (1 << unit.height);
      vehicle->attacked = unit.attacked;
      vehicle->direction = unit.direction;
      vehicle->damage = unit.damage;

      return vehicle;

   } catch (...) {
      return nullptr;
   }
#else
   (void) unit;
   (void) map;
   return nullptr;
#endif
}

void PathfindingAdapter::cleanupTempVehicle(Vehicle* vehicle, GameMap* map) const {
#if MCTS_ENABLE_LEGACY_PATHFINDING
   if (!vehicle || !map) {
      return;
   }

   try {
      MapField* field = map->getField(vehicle->xpos, vehicle->ypos);
      if (field && field->vehicle == vehicle) {
         field->vehicle = nullptr;
      }

      delete vehicle;

   } catch (...) {
      delete vehicle;
   }
#else
   (void) vehicle;
   (void) map;
#endif
}

void PathfindingAdapter::syncNearbyUnits(const IGameState& state, MapCoordinate center, int radius,
                                         GameMap* map) const {
#if MCTS_ENABLE_LEGACY_PATHFINDING
   // Not needed for current implementation
   // AStar3D works with existing map state
#else
   (void) state;
   (void) center;
   (void) radius;
   (void) map;
#endif
}

std::vector<ReachablePosition>
PathfindingAdapter::extractReachablePositions(AStar3D& pathfinder) const {
   std::vector<ReachablePosition> positions;

#if MCTS_ENABLE_LEGACY_PATHFINDING
   for (auto it = pathfinder.visited.begin(); it != pathfinder.visited.end(); ++it) {
      const AStar3D::Node& node = *it;

      if (!node.canStop) {
         continue;
      }

      MapCoordinate pos;
      pos.x = node.h.x;
      pos.y = node.h.y;

      int cost = static_cast<int>(node.gval);

      int8_t height = static_cast<int8_t>(node.h.getNumericalHeight());
      if (height < 0) {
         height = static_cast<int8_t>(node.enterHeight);
      }

      bool attacked = node.hasAttacked;

      positions.emplace_back(pos, cost, height, attacked, true);
   }
#else
   (void) pathfinder;
#endif

   return positions;
}

}  // namespace mcts
}  // namespace asc
