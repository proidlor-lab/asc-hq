/***************************************************************************
 * unit_snapshot.cpp - Implementation of UnitSnapshot factory
 *
 * Part of: ASC MCTS AI (Phase 0.1 - Game State Cloning)
 ***************************************************************************/

#include "unit_snapshot.h"
#include "vehicle.h"
#include "vehicletype.h"
#include "typen.h"

#include <algorithm>
#include <limits>

namespace asc {
namespace mcts {

UnitSnapshot UnitSnapshot::fromVehicle(const Vehicle* vehicle) {
   if (!vehicle) {
      return UnitSnapshot();  // Return empty snapshot
   }

   UnitSnapshot snapshot;

   // Identity
   snapshot.networkID = vehicle->networkid;
   snapshot.type = vehicle->typ;

   // Position
   snapshot.x = static_cast<int16_t>(vehicle->xpos);
   snapshot.y = static_cast<int16_t>(vehicle->ypos);
   snapshot.height = static_cast<int8_t>(vehicle->height);

   // State
   snapshot.damage = static_cast<uint8_t>(vehicle->damage);
   snapshot.owner = static_cast<uint8_t>(vehicle->getOwner());

   // Movement & Combat
   snapshot.movement = static_cast<int16_t>(vehicle->getMovement());
   snapshot.fuel = static_cast<uint16_t>(vehicle->getTank().fuel);
   snapshot.attacked = vehicle->attacked;

   // Ammunition - Simplified bitmask for MVP
   // Check which weapons have ammo (bit set = has ammo)
   snapshot.ammoMask = 0;
   for (int i = 0; i < 16; ++i) {
      if (vehicle->ammo[i] > 0) {
         snapshot.ammoMask |= (1 << i);
      }
   }

   // Experience (scaled down for compact storage)
   // ASC uses high resolution (up to 1,000,000), we scale to fit int16_t
   // Store as percentage of max experience (0-10000 = 0-100.00%) using global max
   constexpr int maxExp = maxunitexperience / Vehicle::experienceResolution;
   const auto scaleExperience = [maxExp](int experience) -> int16_t {
      if (maxExp <= 0) {
         return 0;
      }
      experience = std::clamp(experience, 0, maxExp);
      const int scaled = (experience * 10000) / maxExp;
      return static_cast<int16_t>(
         std::min(scaled, static_cast<int>(std::numeric_limits<int16_t>::max())));
   };

   snapshot.experienceOffensive = scaleExperience(vehicle->getExperience_offensive());
   snapshot.experienceDefensive = scaleExperience(vehicle->getExperience_defensive());

   // Direction
   snapshot.direction = vehicle->direction;

   return snapshot;
}

}  // namespace mcts
}  // namespace asc
