/***************************************************************************
 * unit_snapshot.h - Compact unit data for MCTS simulations
 * 
 * Purpose: Lightweight representation of a single unit's state
 *          Target: ~20-30 bytes per unit (vs. full Vehicle object)
 * 
 * Part of: ASC MCTS AI (Phase 0.1 - Game State Cloning)
 ***************************************************************************/

#ifndef MCTS_UNIT_SNAPSHOT_H
#define MCTS_UNIT_SNAPSHOT_H

#include "types.h"
#include <cstdint>

// Forward declare ASC types (we only store pointers, not copies)
class VehicleType;
class Vehicle;

namespace asc {
namespace mcts {

/**
 * Compact snapshot of a single unit's tactical state
 * 
 * Design:
 * - Stores only tactically-relevant data for MCTS simulations
 * - Uses compact types (int8_t, int16_t) to minimize memory
 * - Stores VehicleType pointer (immutable, can share)
 * - Omits: graphics, GUI state, cargo (MVP simplification)
 * 
 * Size estimate: ~24-32 bytes per unit
 */
struct UnitSnapshot {
    // Identity
    UnitID networkID;                    // 4 bytes - Unique unit identifier
    const VehicleType* type;             // 8 bytes - Pointer to immutable type data (shared)
    
    // Position
    int16_t x, y;                        // 4 bytes - Map coordinates
    int8_t height;                       // 1 byte  - Height level (0-7)
    
    // State
    uint8_t damage;                      // 1 byte  - Damage level (0-100, inverted: HP = 100-damage)
    uint8_t owner;                       // 1 byte  - Player ID (0-7, 8=neutral)
    
    // Movement & Combat
    int16_t movement;                    // 2 bytes - Remaining movement points
    uint16_t fuel;                       // 2 bytes - Fuel in tank
    bool attacked;                       // 1 byte  - Already attacked this turn?
    
    // Combat - Compressed ammunition (simplified for MVP)
    // Note: Full ammo[16] array omitted for MVP - assume full ammo for tactical sim
    // Can be extended post-MVP if ammo management becomes critical
    uint16_t ammoMask;                   // 2 bytes - Bitmask: which weapons have ammo
    
    // Experience (optional - can omit for MVP if size critical)
    int16_t experienceOffensive;         // 2 bytes - Offensive experience (scaled)
    int16_t experienceDefensive;         // 2 bytes - Defensive experience (scaled)
    
    // Direction (for some tactical considerations)
    uint8_t direction;                   // 1 byte  - Facing direction
    
    // Padding for alignment (explicit)
    uint8_t _padding;                    // 1 byte
    
    // Total: ~32 bytes per unit (with experience)
    //        ~28 bytes per unit (without experience)
    
    UnitSnapshot() 
        : networkID(0), type(nullptr), x(0), y(0), height(0), 
          damage(0), owner(0), movement(0), fuel(0), attacked(false),
          ammoMask(0xFFFF), experienceOffensive(0), experienceDefensive(0),
          direction(0), _padding(0)
    {}
    
    /**
     * Factory method: Create snapshot from legacy Vehicle object
     * 
     * @param vehicle Pointer to ASC Vehicle object
     * @return UnitSnapshot populated with vehicle's data
     */
    static UnitSnapshot fromVehicle(const Vehicle* vehicle);
    
    /**
     * Get map coordinate
     */
    MapCoordinate getPosition() const {
        return MapCoordinate(x, y);
    }
    
    /**
     * Calculate approximate HP percentage (0-100)
     */
    uint8_t getHPPercent() const {
        return static_cast<uint8_t>(100 - damage);
    }
    
    /**
     * Check if unit is destroyed
     */
    bool isDestroyed() const {
        return damage >= 100;
    }
    
    /**
     * Check if unit can move
     */
    bool canMove() const {
        return movement > 0 && fuel > 0 && !isDestroyed();
    }
    
    /**
     * Check if unit can attack
     */
    bool canAttack() const {
        return !attacked && ammoMask != 0 && !isDestroyed();
    }
};

} // namespace mcts
} // namespace asc

#endif // MCTS_UNIT_SNAPSHOT_H
