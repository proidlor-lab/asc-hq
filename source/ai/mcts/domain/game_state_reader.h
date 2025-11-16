/***************************************************************************
 * game_state_reader.h - Concrete implementation of IGameStateReader
 *
 * Purpose: Adapter that wraps legacy GameMap and provides clean interface
 *
 * Part of: ASC MCTS AI (Phase 0.1 - Game State Cloning)
 ***************************************************************************/

#ifndef MCTS_GAME_STATE_READER_H
#define MCTS_GAME_STATE_READER_H

#include "i_game_state_reader.h"

namespace asc {
namespace mcts {

/**
 * Concrete implementation of IGameStateReader
 *
 * Wraps ASC's GameMap and provides clean, dependency-injected interface
 *
 * Design:
 * - Read-only access to GameMap (no modifications)
 * - Hides legacy pointer hierarchies and complexity
 * - Provides efficient snapshot creation
 *
 * Usage via DI:
 *   unique_ptr<IGameStateReader> reader = createGameStateReader(gameMap);
 *   auto snapshot = reader->createTacticalSnapshot(...);
 */
class GameStateReader : public IGameStateReader {
  private:
   const GameMap* map;  // Non-owning pointer (GameMap owned by game engine)

  public:
   /**
    * Constructor
    *
    * @param gameMap Pointer to ASC GameMap (must outlive this object)
    */
   explicit GameStateReader(const GameMap* gameMap);

   // Destructor
   virtual ~GameStateReader() = default;

   // ========== IGameStateReader Implementation ==========

   int getMapWidth() const override;
   int getMapHeight() const override;

   PlayerID getCurrentPlayer() const override;

   std::vector<const Vehicle*> getPlayerUnits(PlayerID player) const override;
   const Vehicle* getUnitByID(UnitID networkID) const override;
   const Vehicle* getUnitAt(const MapCoordinate& pos) const override;
   std::vector<const Vehicle*> getUnitsInRange(const MapCoordinate& center,
                                               int range) const override;

   const TerrainType* getTerrainAt(const MapCoordinate& pos) const override;
   bool isFieldVisible(const MapCoordinate& pos, PlayerID observer) const override;
   uint16_t getVisibilityMask(const MapCoordinate& pos) const override;

   ResourceSnapshot getPlayerResources(PlayerID player) const override;

   std::unique_ptr<GameStateSnapshot> createFullSnapshot(PlayerID perspective) const override;

   std::unique_ptr<GameStateSnapshot> createTacticalSnapshot(const std::vector<UnitID>& unitIDs,
                                                             const MapCoordinate& center,
                                                             int radius) const override;

   bool isValidCoordinate(const MapCoordinate& pos) const override;
   const GameMap* getGameMap() const override;

  private:
   // ========== Helper Methods ==========

   /**
    * Check if coordinate is within range of center
    */
   bool isInRange(const MapCoordinate& pos, const MapCoordinate& center, int range) const;

   /**
    * Calculate hex distance between two coordinates
    */
   int hexDistance(const MapCoordinate& a, const MapCoordinate& b) const;

   /**
    * Add unit to snapshot (helper for snapshot creation)
    */
   void addUnitToSnapshot(GameStateSnapshot* snapshot, const Vehicle* vehicle) const;

   /**
    * Add terrain field to snapshot (helper)
    */
   void addTerrainToSnapshot(GameStateSnapshot* snapshot, const MapCoordinate& pos) const;
};

}  // namespace mcts
}  // namespace asc

#endif  // MCTS_GAME_STATE_READER_H
