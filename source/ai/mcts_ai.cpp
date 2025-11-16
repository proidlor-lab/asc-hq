/***************************************************************************
 *                          mcts_ai.cpp  -  description
 *                             -------------------
 *    begin                : 2025-11-08
 *    copyright            : (C) 2025
 *    email                : asc-hq.org
 ***************************************************************************/

#include "mcts_ai.h"
#include "../gamemap.h"
#include "../player.h"
#include "../vehicle.h"
#include "../mapfield.h"
#include "mcts/domain/i_game_state_reader.h"
#include "mcts/domain/i_action_executor.h"
#include "mcts/domain/i_tactical_evaluator.h"
#include "mcts/core/mcts_search.h"
#include "mcts/infrastructure/legacy_game_interface.h"
#include "mcts/infrastructure/mcts_config_loader.h"
#include <iostream>
#include <algorithm>

namespace {

constexpr int kTacticalRadius = 12;

MapCoordinate toLegacyCoordinate(const asc::mcts::MapCoordinate& coord) {
   return MapCoordinate(coord.x, coord.y);
}

std::string toLowerCopy(const std::string& input) {
   std::string result = input;
   std::transform(result.begin(), result.end(), result.begin(), ::tolower);
   return result;
}

}  // namespace

// ===== Constructor =====

MCTS_AI::MCTS_AI(GameMap* gameMap, int playerID, const Profile& profile,
                 const AIFactory::AIConfig& config)
   : gameMap(gameMap),
     playerID(playerID),
     profile(profile),
     config(config),
     running(false),
     vision(visible_all)  // AI has full vision
     ,
     initialized(false),
     stateReader(nullptr),
     searchEngine(nullptr) {
   if (!gameMap) {
      throw std::invalid_argument("MCTS_AI: gameMap cannot be null");
   }

   if (playerID < 0 || playerID >= 8) {
      throw std::invalid_argument("MCTS_AI: playerID must be 0-7");
   }

   log("MCTS_AI created for player " + std::to_string(playerID) + " with profile: " + profile.name);
}

MCTS_AI::~MCTS_AI() {
   log("MCTS_AI destroyed");
}

// ===== BaseAI Interface Implementation =====

void MCTS_AI::run(MapDisplayInterface* mapDisplay) {
   if (running) {
      log("WARNING: run() called while already running");
      return;
   }

   running = true;

   try {
      // Lazy initialization
      if (!initialized) {
         initialize();
      }

      log("=== Starting MCTS AI turn for player " + std::to_string(playerID) + " ===");
      log(">>> AI Profile: " + profile.name +
          " (explorationConstant=" + std::to_string(profile.explorationConstant) +
          ", iterations=" + std::to_string(profile.maxIterations) + ")");
      debug(">>> Using wired MCTS search engine (agents profile: " + profile.agentProfile + ")");

      // Process all units
      processUnits(mapDisplay);

      log("=== MCTS AI turn complete for profile: " + profile.name + " ===");

   } catch (const std::exception& e) {
      log("ERROR: Exception during MCTS AI turn: " + std::string(e.what()));
      // Don't re-throw - AI should fail gracefully
   }

   running = false;
}

void MCTS_AI::read(tnstream& stream) {
   // LEGACY SAVE FILE FORMAT:
   // For now, MCTS AI doesn't persist any state between turns
   // Future: Could save memory, learned strategies, etc.

   log("Loading MCTS AI state from save file");

   // Read version marker for future compatibility
   int version = stream.readInt();

   if (version == 1) {
      // Version 1 format: just version marker, no additional state
      // Future versions can add more fields here
   }

   // IMPORTANT: Must match write() exactly
}

void MCTS_AI::write(tnstream& stream) const {
   // LEGACY SAVE FILE FORMAT:
   // Write minimal state for now

   log("Saving MCTS AI state to save file");

   // Write version marker
   const int version = 1;
   stream.writeInt(version);

   // Future: Add persistent state (memory, strategies, etc.)

   // IMPORTANT: Must match read() exactly
}

// ===== Factory Methods =====

MCTS_AI* MCTS_AI::createBalanced(GameMap* gameMap, int playerID,
                                 const AIFactory::AIConfig& config) {
   return new MCTS_AI(gameMap, playerID, createBalancedProfile(), config);
}

MCTS_AI* MCTS_AI::createAggressive(GameMap* gameMap, int playerID,
                                   const AIFactory::AIConfig& config) {
   // Load profile from config loader to allow runtime overrides
   auto loaded = asc::mcts::MCTSConfigLoader::loadProfile("aggressive");
   Profile p = createAggressiveProfile();
   p.maxIterations = loaded.maxIterations;
   p.maxTimeMs = loaded.maxTimeMs;
   p.rolloutDepthLimit = loaded.rolloutDepthLimit;
   p.explorationConstant = loaded.explorationConstant;
   p.earlyTerminationThreshold = loaded.earlyTerminationThreshold;
   p.materialWeight = loaded.materialWeight;
   p.positionWeight = loaded.positionWeight;
   p.healthWeight = loaded.healthWeight;
   p.threatWeight = loaded.threatWeight;
   p.agentProfile = loaded.agentProfile;
   return new MCTS_AI(gameMap, playerID, p, config);
}

MCTS_AI* MCTS_AI::createDefensive(GameMap* gameMap, int playerID,
                                  const AIFactory::AIConfig& config) {
   auto loaded = asc::mcts::MCTSConfigLoader::loadProfile("defensive");
   Profile p = createDefensiveProfile();
   p.maxIterations = loaded.maxIterations;
   p.maxTimeMs = loaded.maxTimeMs;
   p.rolloutDepthLimit = loaded.rolloutDepthLimit;
   p.explorationConstant = loaded.explorationConstant;
   p.earlyTerminationThreshold = loaded.earlyTerminationThreshold;
   p.materialWeight = loaded.materialWeight;
   p.positionWeight = loaded.positionWeight;
   p.healthWeight = loaded.healthWeight;
   p.threatWeight = loaded.threatWeight;
   p.agentProfile = loaded.agentProfile;
   return new MCTS_AI(gameMap, playerID, p, config);
}

MCTS_AI* MCTS_AI::createFast(GameMap* gameMap, int playerID, const AIFactory::AIConfig& config) {
   return new MCTS_AI(gameMap, playerID, createFastProfile(), config);
}

MCTS_AI* MCTS_AI::createDeep(GameMap* gameMap, int playerID, const AIFactory::AIConfig& config) {
   return new MCTS_AI(gameMap, playerID, createDeepProfile(), config);
}

// ===== Configuration =====

void MCTS_AI::setProfile(const Profile& newProfile) {
   if (running) {
      throw std::logic_error("Cannot change profile while AI is running");
   }

   profile = newProfile;
   initialized = false;  // Force re-initialization with new profile

   log("Profile changed to: " + newProfile.name);
}

MCTS_AI::Profile MCTS_AI::getProfileByName(const std::string& name) {
   std::string lowerName = toLowerCopy(name);

   if (lowerName == "balanced")
      return createBalancedProfile();
   if (lowerName == "aggressive")
      return createAggressiveProfile();
   if (lowerName == "defensive")
      return createDefensiveProfile();
   if (lowerName == "fast")
      return createFastProfile();
   if (lowerName == "deep")
      return createDeepProfile();

   throw std::invalid_argument("Unknown profile name: " + name);
}

// ===== Internal Methods =====

void MCTS_AI::initialize() {
   log("Initializing MCTS components...");

   auto cfg = createMCTSConfig();
   mctsConfig = *cfg;

   stateReader = asc::mcts::createGameStateReader(gameMap);
   auto evaluator = asc::mcts::EvaluatorFactory::createDefault();
   searchEngine = std::make_unique<asc::mcts::MCTSSearch>(std::move(evaluator), mctsConfig);

   initialized = true;
   log("MCTS components initialized");
}

std::unique_ptr<asc::mcts::MCTSConfig> MCTS_AI::createMCTSConfig() const {
   auto config = std::make_unique<asc::mcts::MCTSConfig>();
   config->maxIterations = profile.maxIterations;
   config->maxTimeMs = profile.maxTimeMs;
   config->rolloutDepthLimit = profile.rolloutDepthLimit;
   config->explorationConstant = profile.explorationConstant;
   config->earlyTerminationThreshold = profile.earlyTerminationThreshold;
   config->agentProfile = profile.agentProfile;
   return config;
}

void MCTS_AI::processUnits(MapDisplayInterface* mapDisplay) {
   log("Processing units for player " + std::to_string(playerID));

   // Create legacy game interface for action execution
   auto legacyInterface = asc::mcts::createLegacyGameInterface(gameMap);

   // Get all player units
   // LEGACY CODE: Iterate raw pointer list
   Player& player = gameMap->player[playerID];

   if (player.vehicleList.empty()) {
      log("No units to process");
      return;
   }

   // Process each unit
   int unitsProcessed = 0;
   int actionsExecuted = 0;

   for (auto it = player.vehicleList.begin(); it != player.vehicleList.end(); ++it) {
      Vehicle* unit = *it;
      if (!unit) {
         continue;  // Skip null pointers
      }

      // Check if unit has already finished its turn
      if (legacyInterface->hasUnitFinishedTurn(unit->networkid)) {
         debug("Unit " + std::to_string(unit->networkid) + " already finished");
         continue;
      }

      // Process this unit
      try {
         bool actionTaken = processUnit(unit, legacyInterface.get(), mapDisplay);
         if (actionTaken) {
            actionsExecuted++;
         }
         unitsProcessed++;
      } catch (const std::exception& e) {
         log("ERROR: Exception processing unit " + std::to_string(unit->networkid) + ": " +
             e.what());
      }
   }

   log("Processed " + std::to_string(unitsProcessed) + " units, " +
       std::to_string(actionsExecuted) + " actions executed");
}

bool MCTS_AI::processUnit(Vehicle* unit, asc::mcts::ILegacyGameInterface* legacyInterface,
                          MapDisplayInterface* mapDisplay) {
   debug("Processing unit " + std::to_string(unit->networkid) + " at (" +
         std::to_string(unit->xpos) + "," + std::to_string(unit->ypos) + ")");

   if (!stateReader || !searchEngine) {
      debug("MCTS components not initialized; waiting");
      legacyInterface->executeWait(unit->networkid);
      return false;
   }

   // Build tactical snapshot around the acting unit
   std::vector<int> unitIDs{unit->networkid};
   asc::mcts::MapCoordinate center(static_cast<int16_t>(unit->xpos),
                                   static_cast<int16_t>(unit->ypos));
   auto snapshot = stateReader->createTacticalSnapshot(unitIDs, center, kTacticalRadius);

   if (!snapshot) {
      debug("Snapshot creation failed; waiting");
      legacyInterface->executeWait(unit->networkid);
      return false;
   }

   snapshot->currentPlayer = static_cast<asc::mcts::PlayerID>(playerID);
   snapshot->perspective = static_cast<asc::mcts::PlayerID>(playerID);

   auto result = searchEngine->search(*snapshot, static_cast<asc::mcts::PlayerID>(playerID));

   if (!result.bestAction.has_value()) {
      debug("MCTS returned no action; waiting");
      legacyInterface->executeWait(unit->networkid);
      return false;
   }

   const auto& action = *result.bestAction;
   bool executed = executeAction(action, legacyInterface, mapDisplay);

   if (!executed) {
      debug("Failed to execute action; waiting");
      legacyInterface->executeWait(unit->networkid);
      return false;
   }

   log("Executed " + asc::mcts::getActionTypeName(action) + " for unit " +
       std::to_string(unit->networkid));
   return true;
}

std::vector<int> MCTS_AI::findNearbyEnemies(Vehicle* unit) {
   std::vector<int> enemies;

   if (!unit || !gameMap) {
      return enemies;
   }

   // Simple search: Check fields within 10 hexes
   const int searchRadius = 10;

   for (int dx = -searchRadius; dx <= searchRadius; ++dx) {
      for (int dy = -searchRadius; dy <= searchRadius; ++dy) {
         int checkX = unit->xpos + dx;
         int checkY = unit->ypos + dy;

         // Validate coordinates
         if (checkX < 0 || checkX >= gameMap->xsize || checkY < 0 || checkY >= gameMap->ysize) {
            continue;
         }

         // LEGACY CODE: Get field
         MapField* field = gameMap->getField(checkX, checkY);
         if (!field || !field->vehicle) {
            continue;
         }

         // Check if enemy
         Vehicle* other = field->vehicle;
         if (other->getOwner() != unit->getOwner() && other->getOwner() != 8) {  // 8 = neutral
            enemies.push_back(other->networkid);
         }
      }
   }

   return enemies;
}

bool MCTS_AI::executeAction(const asc::mcts::Action& action,
                            asc::mcts::ILegacyGameInterface* legacyInterface,
                            MapDisplayInterface* mapDisplay) {
   if (!legacyInterface) {
      return false;
   }

   return std::visit(
      asc::mcts::overloaded{
         [this, legacyInterface, mapDisplay](const asc::mcts::MoveAction& move) {
            MapCoordinate dest = toLegacyCoordinate(move.destination);
            if (!legacyInterface->canMove(move.unitID, dest)) {
               debug("Move not legal for unit " + std::to_string(move.unitID));
               return false;
            }
            return legacyInterface->executeMove(move.unitID, dest, mapDisplay);
         },
         [this, legacyInterface, mapDisplay](const asc::mcts::AttackAction& attack) {
            MapCoordinate targetPos = toLegacyCoordinate(attack.target);
            const int targetID = legacyInterface->getUnitAt(targetPos);
            if (targetID < 0) {
               debug("No target at attack position for unit " + std::to_string(attack.attackerID));
               return false;
            }

            if (!legacyInterface->canAttack(attack.attackerID, targetID)) {
               debug("Attack not legal for unit " + std::to_string(attack.attackerID) +
                     " -> target " + std::to_string(targetID));
               return false;
            }

            return legacyInterface->executeAttack(attack.attackerID, targetID, mapDisplay);
         },
         [legacyInterface](const asc::mcts::WaitAction& wait) {
            return legacyInterface->executeWait(wait.unitID);
         }},
      action);
}

void MCTS_AI::log(const std::string& message) const {
   if (profile.enableLogging || config.enableLogging) {
      std::cout << "[MCTS_AI P" << playerID << "] " << message << std::endl;
   }
}

void MCTS_AI::debug(const std::string& message) const {
   if (profile.enableDebugOutput || config.enableDebugOutput) {
      std::cout << "[MCTS_AI DEBUG P" << playerID << "] " << message << std::endl;
   }
}

// ===== Preset Profiles =====

MCTS_AI::Profile MCTS_AI::createBalancedProfile() {
   Profile p;
   p.name = "Balanced";
   p.maxIterations = 200;
   p.maxTimeMs = 2000;
   p.rolloutDepthLimit = 10;
   p.explorationConstant = 1.414;  // sqrt(2) - standard UCB1
   p.earlyTerminationThreshold = 0.95;

   // Evaluation weights
   p.materialWeight = 2.0;
   p.positionWeight = 1.0;
   p.healthWeight = 1.5;
   p.threatWeight = 1.2;

   p.enableLogging = false;
   p.enableDebugOutput = false;
   p.agentProfile = "Balanced";

   return p;
}

MCTS_AI::Profile MCTS_AI::createAggressiveProfile() {
   Profile p = createBalancedProfile();
   p.name = "Aggressive";

   // More iterations for deeper search
   p.maxIterations = 250;

   // Higher material weight (value kills)
   p.materialWeight = 3.0;

   // Lower position/defensive weights
   p.positionWeight = 0.5;
   p.threatWeight = 0.8;  // Less concerned about RF

   p.agentProfile = "Aggressive";
   return p;
}

MCTS_AI::Profile MCTS_AI::createDefensiveProfile() {
   Profile p = createBalancedProfile();
   p.name = "Defensive";

   // More conservative search
   p.maxIterations = 200;

   // Lower material weight (preserving units more important)
   p.materialWeight = 1.5;

   // Higher position and threat weights
   p.positionWeight = 1.5;
   p.healthWeight = 2.0;
   p.threatWeight = 2.5;  // Very concerned about RF

   p.agentProfile = "Defensive";
   return p;
}

MCTS_AI::Profile MCTS_AI::createFastProfile() {
   Profile p = createBalancedProfile();
   p.name = "Fast";

   // Reduced iterations for speed
   p.maxIterations = 100;
   p.maxTimeMs = 1000;
   p.rolloutDepthLimit = 5;

   p.agentProfile = "Balanced";
   return p;
}

MCTS_AI::Profile MCTS_AI::createDeepProfile() {
   Profile p = createBalancedProfile();
   p.name = "Deep";

   // Increased iterations and depth
   p.maxIterations = 500;
   p.maxTimeMs = 5000;
   p.rolloutDepthLimit = 20;

   p.agentProfile = "Balanced";
   return p;
}
