/***************************************************************************
 * i_game_state.h - Abstract game state interface for MCTS
 *
 * Purpose: Allow different state representations (tactical, strategic,
 *          compressed, memory-backed) to plug into the MCTS pipeline.
 *
 * Part of: ASC MCTS AI
 ***************************************************************************/

#ifndef MCTS_I_GAME_STATE_H
#define MCTS_I_GAME_STATE_H

#include "types.h"
#include "unit_snapshot.h"

#include <cstddef>
#include <memory>
#include <vector>

class TerrainType;

namespace asc {
namespace mcts {

/**
 * Sparse terrain snapshot describing a single field.
 *
 * Stored separately from concrete game state implementations so the
 * interface can expose read-only access without revealing internal layout.
 */
struct FieldSnapshot {
   const TerrainType* terrain;
   uint16_t visibilityMask;
   UnitID unitID;
   bool hasMine;
   uint8_t _padding[3];

   constexpr FieldSnapshot() noexcept
      : terrain(nullptr), visibilityMask(0), unitID(-1), hasMine(false), _padding{0, 0, 0} {}
};

/**
 * Lightweight statistics useful for profiling different state backends.
 */
struct GameStateStats {
   size_t unitCount;
   size_t terrainFieldCount;
   size_t memoryBytes;
};

/**
 * Abstract interface representing the mutable state used by MCTS.
 *
 * Concrete implementations (e.g. GameStateSnapshot, future strategic
 * representations) should expose the same API so higher layers remain
 * agnostic of the underlying storage.
 */
class IGameState {
  public:
   virtual ~IGameState() = default;

   /// Clone the complete state (used heavily by MCTS).
   [[nodiscard]] virtual std::unique_ptr<IGameState> clone() const = 0;

   // --- Metadata ---------------------------------------------------------
   virtual int getMapWidth() const = 0;
   virtual int getMapHeight() const = 0;

   virtual PlayerID getCurrentPlayer() const = 0;
   virtual void setCurrentPlayer(PlayerID player) = 0;

   virtual PlayerID getPerspective() const = 0;
   virtual void setPerspective(PlayerID player) = 0;

   // --- Unit access ------------------------------------------------------
   virtual void addUnit(const UnitSnapshot& unit) = 0;
   virtual const UnitSnapshot* findUnit(UnitID id) const = 0;
   virtual UnitSnapshot* findUnitMutable(UnitID id) = 0;
   virtual const UnitSnapshot* getUnitAt(const MapCoordinate& pos) const = 0;
   virtual std::vector<const UnitSnapshot*> getPlayerUnits(PlayerID player) const = 0;

   virtual const std::vector<UnitSnapshot>& getUnits() const = 0;
   virtual std::vector<UnitSnapshot>& getUnitsMutable() = 0;

   // --- Terrain / validity ----------------------------------------------
   virtual const FieldSnapshot* getTerrainAt(const MapCoordinate& pos) const = 0;
   [[nodiscard]] virtual bool isValidCoordinate(const MapCoordinate& pos) const noexcept = 0;

   // --- Resources --------------------------------------------------------
   virtual ResourceSnapshot getPlayerResources(PlayerID player) const = 0;
   virtual void setPlayerResources(PlayerID player, const ResourceSnapshot& resources) = 0;

   // --- Profiling --------------------------------------------------------
   virtual GameStateStats getStats() const = 0;
};

}  // namespace mcts
}  // namespace asc

#endif  // MCTS_I_GAME_STATE_H
