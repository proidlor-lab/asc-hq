/***************************************************************************
 * reaction_fire_detector.cpp
 ***************************************************************************/

#include "reaction_fire_detector.h"

#include "../../../vehicletype.h"

#include <algorithm>
#include <cmath>

namespace asc {
namespace mcts {

namespace {

constexpr int kMaxWeaponSlots = 16;

bool weaponHasReactionFire(const SingleWeapon& weapon) {
   return weapon.reactionFireShots > 0;
}

}  // namespace

void ReactionFireDetector::initialize(const GameStateSnapshot* state, PlayerID friendlyPlayer,
                                      const ICombatCalculator* calculator) {
   state_ = state;
   friendlyPlayer_ = friendlyPlayer;
   calculator_ = calculator;
   rebuild();
}

void ReactionFireDetector::rebuild() {
   threatMap_.clear();
   if (state_ == nullptr) {
      return;
   }

   for (const UnitSnapshot& unit : state_->units) {
      if (unit.owner == friendlyPlayer_ || unit.isDestroyed()) {
         continue;
      }
      addThreatArea(unit);
   }
}

const std::vector<ReactionFireThreat>&
ReactionFireDetector::getThreatsAt(const MapCoordinate& coordinate) const {
   const auto it = threatMap_.find(coordinate);
   if (it == threatMap_.end()) {
      return emptyThreats_;
   }
   return it->second;
}

void ReactionFireDetector::removeThreat(UnitID attackerId) {
   for (auto& [coord, threats] : threatMap_) {
      threats.erase(std::remove_if(threats.begin(), threats.end(),
                                   [attackerId](const ReactionFireThreat& threat) {
                                      return threat.attackerId == attackerId;
                                   }),
                    threats.end());
   }
}

bool ReactionFireDetector::hasThreats(const MapCoordinate& coordinate) const {
   return threatMap_.find(coordinate) != threatMap_.end();
}

int ReactionFireDetector::hexDistance(const MapCoordinate& a, const MapCoordinate& b) noexcept {
   const int dx = static_cast<int>(b.x) - static_cast<int>(a.x);
   const int dy = static_cast<int>(b.y) - static_cast<int>(a.y);
   return (std::abs(dx) + std::abs(dy) + std::abs(dx - dy)) / 2;
}

bool ReactionFireDetector::isStaticUnit(const UnitSnapshot& unit) const {
   if (unit.type == nullptr) {
      return unit.movement < 2;
   }

   if (static_cast<int>(unit.movement) < 2) {
      return true;
   }

   if (unit.type->movement.empty()) {
      return true;
   }

   const auto maxMove = std::max_element(unit.type->movement.begin(), unit.type->movement.end());
   return (maxMove == unit.type->movement.end()) || (*maxMove < 2);
}

bool ReactionFireDetector::supportsReactionFire(const VehicleType* type) const {
   if (type == nullptr || type->weapons.count == 0) {
      return false;
   }

   const int weaponCount = std::min<int>(type->weapons.count, kMaxWeaponSlots);
   for (int i = 0; i < weaponCount; ++i) {
      if (weaponHasReactionFire(type->weapons.weapon[i])) {
         return true;
      }
   }
   return false;
}

int ReactionFireDetector::getMaxReactionRange(const VehicleType* type) const {
   if (type == nullptr || type->weapons.count == 0) {
      return 0;
   }

   int maxRange = 0;
   const int weaponCount = std::min<int>(type->weapons.count, kMaxWeaponSlots);
   for (int i = 0; i < weaponCount; ++i) {
      const SingleWeapon& weapon = type->weapons.weapon[i];
      if (!weaponHasReactionFire(weapon)) {
         continue;
      }
      maxRange = std::max(maxRange, weapon.maxdistance / 10);
   }
   return maxRange;
}

void ReactionFireDetector::addThreatArea(const UnitSnapshot& unit) {
   if (!supportsReactionFire(unit.type)) {
      return;
   }

   const MapCoordinate origin = unit.getPosition();
   const int maxRange = getMaxReactionRange(unit.type);
   if (maxRange <= 0) {
      return;
   }

   ReactionFireThreat threat;
   threat.attackerId = unit.networkID;
   threat.attackPosition = origin;
   threat.maxRange = maxRange;
   threat.isStatic = isStaticUnit(unit);
   threat.attacker = &unit;

   for (int dx = -maxRange; dx <= maxRange; ++dx) {
      for (int dy = -maxRange; dy <= maxRange; ++dy) {
         MapCoordinate candidate(static_cast<int16_t>(origin.x + dx),
                                 static_cast<int16_t>(origin.y + dy));

         if (candidate.x < 0 || candidate.y < 0 || candidate.x >= state_->mapWidth ||
             candidate.y >= state_->mapHeight) {
            continue;
         }

         if (hexDistance(origin, candidate) > maxRange) {
            continue;
         }

         threatMap_[candidate].push_back(threat);
      }
   }
}

}  // namespace mcts
}  // namespace asc
