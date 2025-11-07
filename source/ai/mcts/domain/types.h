/***************************************************************************
 * types.h - Common types for MCTS domain layer
 * 
 * Purpose: Shared type definitions for game state snapshots
 * 
 * Part of: ASC MCTS AI (Phase 0.1 - Game State Cloning)
 ***************************************************************************/

#ifndef MCTS_DOMAIN_TYPES_H
#define MCTS_DOMAIN_TYPES_H

#include <cstdint>
#include <cstddef>

namespace asc {
namespace mcts {

//! Coordinate on the hex map
struct MapCoordinate {
    int16_t x;
    int16_t y;
    
    MapCoordinate() : x(0), y(0) {}
    MapCoordinate(int16_t x_, int16_t y_) : x(x_), y(y_) {}
    
    bool operator==(const MapCoordinate& other) const {
        return x == other.x && y == other.y;
    }
    
    bool operator<(const MapCoordinate& other) const {
        if (x != other.x) return x < other.x;
        return y < other.y;
    }
};

struct MapCoordinateHash {
    size_t operator()(const MapCoordinate& coord) const noexcept {
        return (static_cast<size_t>(static_cast<uint16_t>(coord.x)) << 16) ^
               static_cast<uint16_t>(coord.y);
    }
};

//! Simple resources structure (matching ASC's Resources)
struct ResourceSnapshot {
    int16_t energy;
    int16_t material;
    int16_t fuel;
    
    ResourceSnapshot() : energy(0), material(0), fuel(0) {}
    ResourceSnapshot(int16_t e, int16_t m, int16_t f) 
        : energy(e), material(m), fuel(f) {}
};

//! Player ID type (0-7 for players, 8 for neutral)
using PlayerID = uint8_t;

//! Network ID for units (unique identifier)
using UnitID = int;

} // namespace mcts
} // namespace asc

#endif // MCTS_DOMAIN_TYPES_H
