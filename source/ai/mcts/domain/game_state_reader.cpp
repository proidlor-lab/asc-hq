/***************************************************************************
 * game_state_reader.cpp - Implementation of GameStateReader
 *
 * Part of: ASC MCTS AI (Phase 0.1 - Game State Cloning)
 ***************************************************************************/

#include "game_state_reader.h"
#include "game_state_snapshot.h"
#include "gamemap.h"
#include "mapfield.h"
#include "player.h"
#include "vehicle.h"
#include "terraintype.h"
#include "mapalgorithms.h"
#include <algorithm>
#include <cmath>

namespace asc {
namespace mcts {

// ========== Factory Function ==========

std::unique_ptr<IGameStateReader> createGameStateReader(const GameMap* gameMap) {
   return std::make_unique<GameStateReader>(gameMap);
}

// ========== Constructor ==========

GameStateReader::GameStateReader(const GameMap* gameMap) : map(gameMap) {
   if (!map) {
      throw std::invalid_argument("GameStateReader: gameMap cannot be null");
   }
}

// ========== Map Dimensions ==========

int GameStateReader::getMapWidth() const {
   return map->xsize;
}

int GameStateReader::getMapHeight() const {
   return map->ysize;
}

// ========== Current Player ==========

PlayerID GameStateReader::getCurrentPlayer() const {
   return static_cast<PlayerID>(map->actplayer);
}

// ========== Unit Queries ==========

std::vector<const Vehicle*> GameStateReader::getPlayerUnits(PlayerID player) const {
   std::vector<const Vehicle*> result;

   if (player >= 8) {
      return result;  // Invalid player ID
   }

   const Player& p = map->player[player];

   // Iterate through player's vehicle list
   for (const Vehicle* vehicle : p.vehicleList) {
      if (vehicle) {
         result.push_back(vehicle);
      }
   }

   return result;
}

const Vehicle* GameStateReader::getUnitByID(UnitID networkID) const {
   return map->getUnit(networkID);
}

const Vehicle* GameStateReader::getUnitAt(const MapCoordinate& pos) const {
   if (!isValidCoordinate(pos)) {
      return nullptr;
   }

   const MapField* field = map->getField(pos.x, pos.y);
   return field ? field->vehicle : nullptr;
}

std::vector<const Vehicle*> GameStateReader::getUnitsInRange(const MapCoordinate& center,
                                                             int range) const {
   std::vector<const Vehicle*> result;

   // Iterate through all players' units
   for (int p = 0; p < 8; ++p) {
      auto units = getPlayerUnits(static_cast<PlayerID>(p));
      for (const Vehicle* unit : units) {
         MapCoordinate unitPos(unit->xpos, unit->ypos);
         if (isInRange(unitPos, center, range)) {
            result.push_back(unit);
         }
      }
   }

   return result;
}

// ========== Field/Terrain Queries ==========

const TerrainType* GameStateReader::getTerrainAt(const MapCoordinate& pos) const {
   if (!isValidCoordinate(pos)) {
      return nullptr;
   }

   const MapField* field = map->getField(pos.x, pos.y);
   if (!field) {
      return nullptr;
   }

   // Use getTerrainType() method to get TerrainType pointer from Weather
   return field->getTerrainType();
}

bool GameStateReader::isFieldVisible(const MapCoordinate& pos, PlayerID observer) const {
   if (!isValidCoordinate(pos) || observer >= 8) {
      return false;
   }

   const MapField* field = map->getField(pos.x, pos.y);
   if (!field) {
      return false;
   }

   // Check visibility bitmask (bit N = player N can see)
   return (field->visible & (1 << observer)) != 0;
}

uint16_t GameStateReader::getVisibilityMask(const MapCoordinate& pos) const {
   if (!isValidCoordinate(pos)) {
      return 0;
   }

   const MapField* field = map->getField(pos.x, pos.y);
   return field ? field->visible : 0;
}

// ========== Resource Queries ==========

ResourceSnapshot GameStateReader::getPlayerResources(PlayerID player) const {
   if (player >= 8) {
      return ResourceSnapshot();
   }

   // Resources are stored in GameMap::bi_resource array
   const Resources& res = map->bi_resource[player];

   return ResourceSnapshot(static_cast<int16_t>(res.energy), static_cast<int16_t>(res.material),
                           static_cast<int16_t>(res.fuel));
}

// ========== Snapshot Creation ==========

std::unique_ptr<GameStateSnapshot> GameStateReader::createFullSnapshot(PlayerID perspective) const {
   auto snapshot = std::make_unique<GameStateSnapshot>();

   // Metadata
   snapshot->mapWidth = map->xsize;
   snapshot->mapHeight = map->ysize;
   snapshot->currentPlayer = static_cast<PlayerID>(map->actplayer);
   snapshot->perspective = perspective;

   // Add all units from all players
   for (int p = 0; p < 8; ++p) {
      auto units = getPlayerUnits(static_cast<PlayerID>(p));
      for (const Vehicle* unit : units) {
         if (unit) {
            addUnitToSnapshot(snapshot.get(), unit);
         }
      }
   }

   // Add terrain (full map - can be expensive!)
   // For MVP, we might want to limit this or use sparse representation
   for (int y = 0; y < map->ysize; ++y) {
      for (int x = 0; x < map->xsize; ++x) {
         addTerrainToSnapshot(snapshot.get(), MapCoordinate(x, y));
      }
   }

   // Add resources
   for (int p = 0; p < 8; ++p) {
      snapshot->playerResources[p] = getPlayerResources(static_cast<PlayerID>(p));
   }

   return snapshot;
}

std::unique_ptr<GameStateSnapshot>
GameStateReader::createTacticalSnapshot(const std::vector<UnitID>& unitIDs,
                                        const MapCoordinate& center, int radius) const {
   auto snapshot = std::make_unique<GameStateSnapshot>();

   // Metadata
   snapshot->mapWidth = map->xsize;
   snapshot->mapHeight = map->ysize;
   snapshot->currentPlayer = static_cast<PlayerID>(map->actplayer);
   snapshot->perspective = getCurrentPlayer();  // Assume current player perspective

   // Add specified units
   for (UnitID id : unitIDs) {
      const Vehicle* unit = getUnitByID(id);
      if (unit) {
         addUnitToSnapshot(snapshot.get(), unit);
      }
   }

   // Add nearby units (within radius + buffer for reaction fire detection)
   const int rfBuffer = 10;  // Extra range for RF-capable units
   auto nearbyUnits = getUnitsInRange(center, radius + rfBuffer);
   for (const Vehicle* unit : nearbyUnits) {
      // Check if not already added
      if (snapshot->findUnit(unit->networkid) == nullptr) {
         addUnitToSnapshot(snapshot.get(), unit);
      }
   }

   // Add terrain for tactical area (sparse)
   int minX = std::max(0, center.x - radius);
   int maxX = std::min(map->xsize - 1, center.x + radius);
   int minY = std::max(0, center.y - radius);
   int maxY = std::min(map->ysize - 1, center.y + radius);

   for (int y = minY; y <= maxY; ++y) {
      for (int x = minX; x <= maxX; ++x) {
         MapCoordinate pos(x, y);
         if (isInRange(pos, center, radius)) {
            addTerrainToSnapshot(snapshot.get(), pos);
         }
      }
   }

   // Add resources (all players - needed for production decisions)
   for (int p = 0; p < 8; ++p) {
      snapshot->playerResources[p] = getPlayerResources(static_cast<PlayerID>(p));
   }

   return snapshot;
}

// ========== Utility ==========

bool GameStateReader::isValidCoordinate(const MapCoordinate& pos) const {
   return pos.x >= 0 && pos.x < map->xsize && pos.y >= 0 && pos.y < map->ysize;
}

const GameMap* GameStateReader::getGameMap() const {
   return map;
}

// ========== Helper Methods ==========

bool GameStateReader::isInRange(const MapCoordinate& pos, const MapCoordinate& center,
                                int range) const {
   return hexDistance(pos, center) <= range;
}

int GameStateReader::hexDistance(const MapCoordinate& a, const MapCoordinate& b) const {
   // beeline has an overload that takes x,y coordinates directly
   return beeline(a.x, a.y, b.x, b.y);
}

void GameStateReader::addUnitToSnapshot(GameStateSnapshot* snapshot, const Vehicle* vehicle) const {
   if (!vehicle)
      return;

   UnitSnapshot unitSnap = UnitSnapshot::fromVehicle(vehicle);
   snapshot->addUnit(unitSnap);
}

void GameStateReader::addTerrainToSnapshot(GameStateSnapshot* snapshot,
                                           const MapCoordinate& pos) const {
   if (!isValidCoordinate(pos))
      return;

   const MapField* field = map->getField(pos.x, pos.y);
   if (!field)
      return;

   FieldSnapshot fieldSnap;
   fieldSnap.terrain = field->getTerrainType();  // Use getTerrainType() method
   fieldSnap.visibilityMask = field->visible;
   fieldSnap.unitID = field->vehicle ? field->vehicle->networkid : -1;
   fieldSnap.hasMine = !field->mines.empty();  // Simplified: just check if any mine

   snapshot->terrain[pos] = fieldSnap;
}

}  // namespace mcts
}  // namespace asc
