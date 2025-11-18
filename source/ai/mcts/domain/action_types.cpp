/***************************************************************************
 * action_types.cpp - Action type implementations
 *
 * Part of: ASC MCTS AI (Phase 0.2 - Action Execution Interface)
 ***************************************************************************/

#include "action_types.h"
#include <sstream>

namespace asc {
namespace mcts {

std::string MoveAction::toString() const {
   std::ostringstream oss;
   oss << "Move(unit=" << unitID << ", dest=(" << destination.x << "," << destination.y << ")";
   if (targetHeight != 0) {
      oss << ", height=" << static_cast<int>(targetHeight);
   }
   oss << ")";
   return oss.str();
}

std::string AttackAction::toString() const {
   std::ostringstream oss;
   oss << "Attack(attacker=" << attackerID << ", target=(" << target.x << "," << target.y << ")";
   if (weaponIndex >= 0) {
      oss << ", weapon=" << weaponIndex;
   }
   oss << ")";
   return oss.str();
}

std::string WaitAction::toString() const {
   std::ostringstream oss;
   oss << "Wait(unit=" << unitID << ")";
   return oss.str();
}

std::string ActionResult::toString() const {
   std::ostringstream oss;

   switch (code) {
      case ActionResultCode::Success:
         oss << "Success";
         break;
      case ActionResultCode::Failure:
         oss << "Failure";
         break;
      case ActionResultCode::IllegalAction:
         oss << "IllegalAction";
         break;
      case ActionResultCode::InsufficientMovement:
         oss << "InsufficientMovement";
         break;
      case ActionResultCode::InsufficientAmmo:
         oss << "InsufficientAmmo";
         break;
      case ActionResultCode::InsufficientFuel:
         oss << "InsufficientFuel";
         break;
      case ActionResultCode::OutOfRange:
         oss << "OutOfRange";
         break;
      case ActionResultCode::UnitNotFound:
         oss << "UnitNotFound";
         break;
      case ActionResultCode::TargetNotFound:
         oss << "TargetNotFound";
         break;
      case ActionResultCode::Blocked:
         oss << "Blocked";
         break;
      case ActionResultCode::ReactionFire:
         oss << "ReactionFire";
         break;
   }

   if (message.has_value()) {
      oss << ": " << message.value();
   }

   if (damageDealt >= 0) {
      oss << " (damage=" << damageDealt << ")";
   }
   if (fuelConsumed >= 0) {
      oss << " (fuel=" << fuelConsumed << ")";
   }
   if (movementConsumed >= 0) {
      oss << " (movement=" << movementConsumed << ")";
   }

   return oss.str();
}

}  // namespace mcts
}  // namespace asc
