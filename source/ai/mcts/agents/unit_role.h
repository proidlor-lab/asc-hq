/***************************************************************************
 * unit_role.h - Helpers for classifying unit roles/capabilities
 ***************************************************************************/

#ifndef MCTS_UNIT_ROLE_H
#define MCTS_UNIT_ROLE_H

#include "../domain/unit_snapshot.h"

namespace asc {
namespace mcts {

enum class UnitRole { COMBAT_PRIMARY, SERVICE_PRIMARY, HYBRID };

class UnitRoleClassifier {
  public:
   static UnitRole detectRole(const UnitSnapshot& unit) noexcept;
   static UnitRole detectRole(const VehicleType* type) noexcept;

   static bool hasServiceAbility(const VehicleType* type) noexcept;
   static bool hasRepairAbility(const VehicleType* type) noexcept;
   static bool hasRefuelAbility(const VehicleType* type) noexcept;
   static bool hasSupplyAbility(const VehicleType* type) noexcept;
   static bool hasOffensiveCapability(const VehicleType* type) noexcept;
   static int getMaxWeaponRange(const VehicleType* type) noexcept;
};

}  // namespace mcts
}  // namespace asc

#endif  // MCTS_UNIT_ROLE_H
