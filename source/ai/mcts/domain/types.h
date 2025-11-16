/***************************************************************************
 * types.h - Common types for MCTS domain layer
 *
 * Purpose: Shared type definitions for game state snapshots
 *
 * Part of: ASC MCTS AI (Phase 0.1 - Game State Cloning)
 * Updated: C++23 with constexpr optimizations
 ***************************************************************************/

#ifndef MCTS_DOMAIN_TYPES_H
#define MCTS_DOMAIN_TYPES_H

#include <cstdint>
#include <cstddef>
#include <compare>  // C++20 three-way comparison

namespace asc {
namespace mcts {

//! Coordinate on the hex map (C++23: constexpr + three-way comparison)
struct MapCoordinate {
   int16_t x;
   int16_t y;

   constexpr MapCoordinate() noexcept : x(0), y(0) {}
   constexpr MapCoordinate(int16_t x_, int16_t y_) noexcept : x(x_), y(y_) {}

   // C++20: Three-way comparison operator (required for std::flat_map)
   constexpr auto operator<=>(const MapCoordinate&) const noexcept = default;
   constexpr bool operator==(const MapCoordinate&) const noexcept = default;
};

struct MapCoordinateHash {
   constexpr size_t operator()(const MapCoordinate& coord) const noexcept {
      return (static_cast<size_t>(static_cast<uint16_t>(coord.x)) << 16) ^
             static_cast<uint16_t>(coord.y);
   }
};

//! Simple resources structure (matching ASC's Resources, C++23: constexpr)
struct ResourceSnapshot {
   int16_t energy;
   int16_t material;
   int16_t fuel;

   constexpr ResourceSnapshot() noexcept : energy(0), material(0), fuel(0) {}
   constexpr ResourceSnapshot(int16_t e, int16_t m, int16_t f) noexcept
      : energy(e), material(m), fuel(f) {}

   // C++20: Three-way comparison
   constexpr auto operator<=>(const ResourceSnapshot&) const noexcept = default;
};

//! Player ID type (0-7 for players, 8 for neutral)
using PlayerID = uint8_t;

//! Network ID for units (unique identifier)
using UnitID = int;

// C++23: Compile-time constants
inline constexpr PlayerID MAX_PLAYERS = 8;
inline constexpr PlayerID NEUTRAL_PLAYER = 8;
inline constexpr size_t MAX_UNIT_SNAPSHOT_SIZE = 32;

}  // namespace mcts
}  // namespace asc

#endif  // MCTS_DOMAIN_TYPES_H
