/***************************************************************************
 * unit_role.cpp
 ***************************************************************************/

#include "unit_role.h"

#include "../../../vehicletype.h"
#include <algorithm>

namespace asc {
namespace mcts {

namespace {

constexpr int kMaxWeapons = 16;

}  // namespace

UnitRole UnitRoleClassifier::detectRole(const UnitSnapshot& unit) noexcept {
   return detectRole(unit.type);
}

UnitRole UnitRoleClassifier::detectRole(const VehicleType* type) noexcept {
   const bool service = hasServiceAbility(type);
   const bool combat = hasOffensiveCapability(type);

   if (service && !combat) {
      return UnitRole::SERVICE_PRIMARY;
   }
   if (service && combat) {
      return UnitRole::HYBRID;
   }
   return UnitRole::COMBAT_PRIMARY;
}

bool UnitRoleClassifier::hasServiceAbility(const VehicleType* type) noexcept {
   return hasRepairAbility(type) || hasRefuelAbility(type) || hasSupplyAbility(type);
}

bool UnitRoleClassifier::hasRepairAbility(const VehicleType* type) noexcept {
   if (type == nullptr || type->weapons.count == 0) {
      return false;
   }
   const int weaponCount = std::min<int>(type->weapons.count, kMaxWeapons);
   for (int i = 0; i < weaponCount; ++i) {
      if (type->weapons.weapon[i].service()) {
         return true;
      }
   }
   return false;
}

bool UnitRoleClassifier::hasRefuelAbility(const VehicleType* type) noexcept {
   if (type == nullptr || type->weapons.count == 0) {
      return false;
   }
   const int weaponCount = std::min<int>(type->weapons.count, kMaxWeapons);
   for (int i = 0; i < weaponCount; ++i) {
      if (type->weapons.weapon[i].canRefuel()) {
         return true;
      }
   }
   return false;
}

bool UnitRoleClassifier::hasSupplyAbility(const VehicleType* type) noexcept {
   if (type == nullptr || type->weapons.count == 0) {
      return false;
   }
   const int weaponCount = std::min<int>(type->weapons.count, kMaxWeapons);
   for (int i = 0; i < weaponCount; ++i) {
      if (type->weapons.weapon[i].service() || type->weapons.weapon[i].placeObjects()) {
         return true;
      }
   }
   return false;
}

bool UnitRoleClassifier::hasOffensiveCapability(const VehicleType* type) noexcept {
   if (type == nullptr || type->weapons.count == 0) {
      return false;
   }
   const int weaponCount = std::min<int>(type->weapons.count, kMaxWeapons);
   for (int i = 0; i < weaponCount; ++i) {
      if (type->weapons.weapon[i].offensive()) {
         return true;
      }
   }
   return false;
}

int UnitRoleClassifier::getMaxWeaponRange(const VehicleType* type) noexcept {
   if (type == nullptr || type->weapons.count == 0) {
      return 0;
   }
   int maxRange = 0;
   const int weaponCount = std::min<int>(type->weapons.count, kMaxWeapons);
   for (int i = 0; i < weaponCount; ++i) {
      const auto& weapon = type->weapons.weapon[i];
      if (!weapon.offensive()) {
         continue;
      }
      maxRange = std::max(maxRange, weapon.maxdistance / 10);
   }
   return maxRange;
}

}  // namespace mcts
}  // namespace asc
