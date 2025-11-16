/***************************************************************************
 *                          ai_factory.cpp  -  description
 *                             -------------------
 *    begin                : 2025-11-08
 *    copyright            : (C) 2025
 *    email                : asc-hq.org
 ***************************************************************************/

#include "ai_factory.h"
#include "ai.h"         // Classic AI
#include "mcts_ai.h"    // MCTS AI implementations
#include "../errors.h"  // For displayLogMessage
#include <stdexcept>
#include <algorithm>

// Overload without config (uses default)
BaseAI* AIFactory::createAI(AIType type, GameMap* gameMap, int playerID) {
   return createAI(type, gameMap, playerID, AIConfig::getDefault());
}

BaseAI* AIFactory::createAI(AIType type, GameMap* gameMap, int playerID, const AIConfig& config) {
   // Validate parameters
   if (!gameMap) {
      throw std::invalid_argument("AIFactory::createAI - gameMap cannot be null");
   }

   if (playerID < 0 || playerID >= 8) {
      throw std::invalid_argument("AIFactory::createAI - playerID must be 0-7");
   }

   // Validate AI type
   if (!isValidAIType(static_cast<int>(type))) {
      throw std::invalid_argument("AIFactory::createAI - invalid AI type: " + std::to_string(type));
   }

   // Check availability
   if (!isAITypeAvailable(type)) {
      // Fallback to classic AI if requested type unavailable
      type = AI_CLASSIC;
   }

   // LEGACY CODE INTERFACE:
   // - Must return raw pointer (Player class stores BaseAI*)
   // - Caller (Player) owns the pointer and will delete it
   // - Cannot use smart pointers here without refactoring Player class

   switch (type) {
      case AI_CLASSIC:
         // Classic rule-based AI
         displayLogMessage(1, "AIFactory: Creating Classic AI for player " +
                                 ASCString::toString(playerID) + "\n");
         return new AI(gameMap, playerID);

      case AI_MCTS_BALANCED:
         displayLogMessage(1, "AIFactory: Creating MCTS Balanced AI for player " +
                                 ASCString::toString(playerID) + "\n");
         return MCTS_AI::createBalanced(gameMap, playerID, config);

      case AI_MCTS_AGGRESSIVE:
         displayLogMessage(1, "AIFactory: Creating MCTS Aggressive AI for player " +
                                 ASCString::toString(playerID) + "\n");
         return MCTS_AI::createAggressive(gameMap, playerID, config);

      case AI_MCTS_DEFENSIVE:
         displayLogMessage(1, "AIFactory: Creating MCTS Defensive AI for player " +
                                 ASCString::toString(playerID) + "\n");
         return MCTS_AI::createDefensive(gameMap, playerID, config);

      case AI_MCTS_FAST:
         displayLogMessage(1, "AIFactory: Creating MCTS Fast AI for player " +
                                 ASCString::toString(playerID) + "\n");
         return MCTS_AI::createFast(gameMap, playerID, config);

      case AI_MCTS_DEEP:
         displayLogMessage(1, "AIFactory: Creating MCTS Deep AI for player " +
                                 ASCString::toString(playerID) + "\n");
         return MCTS_AI::createDeep(gameMap, playerID, config);

      default:
         // Should never reach here due to validation above
         throw std::logic_error("AIFactory::createAI - unhandled AI type");
   }
}

AIFactory::AIType AIFactory::parseAIType(const std::string& name) {
   std::string lowerName = name;
   std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);

   if (lowerName == "classic" || lowerName == "ai_classic") {
      return AI_CLASSIC;
   } else if (lowerName == "mcts_balanced" || lowerName == "balanced") {
      return AI_MCTS_BALANCED;
   } else if (lowerName == "mcts_aggressive" || lowerName == "aggressive") {
      return AI_MCTS_AGGRESSIVE;
   } else if (lowerName == "mcts_defensive" || lowerName == "defensive") {
      return AI_MCTS_DEFENSIVE;
   } else if (lowerName == "mcts_fast" || lowerName == "fast") {
      return AI_MCTS_FAST;
   } else if (lowerName == "mcts_deep" || lowerName == "deep") {
      return AI_MCTS_DEEP;
   } else {
      throw std::invalid_argument("Unknown AI type name: " + name);
   }
}

std::string AIFactory::getAITypeName(AIType type) {
   switch (type) {
      case AI_CLASSIC:
         return "Classic AI";
      case AI_MCTS_BALANCED:
         return "MCTS Balanced";
      case AI_MCTS_AGGRESSIVE:
         return "MCTS Aggressive";
      case AI_MCTS_DEFENSIVE:
         return "MCTS Defensive";
      case AI_MCTS_FAST:
         return "MCTS Fast";
      case AI_MCTS_DEEP:
         return "MCTS Deep";
      default:
         return "Unknown AI";
   }
}

std::string AIFactory::getAITypeIdentifier(AIType type) {
   switch (type) {
      case AI_CLASSIC:
         return "classic";
      case AI_MCTS_BALANCED:
         return "mcts_balanced";
      case AI_MCTS_AGGRESSIVE:
         return "mcts_aggressive";
      case AI_MCTS_DEFENSIVE:
         return "mcts_defensive";
      case AI_MCTS_FAST:
         return "mcts_fast";
      case AI_MCTS_DEEP:
         return "mcts_deep";
      default:
         return "unknown";
   }
}

std::string AIFactory::getAITypeDescription(AIType type) {
   switch (type) {
      case AI_CLASSIC:
         return "Original rule-based AI. Fast and predictable.";

      case AI_MCTS_BALANCED:
         return "MCTS AI with balanced parameters. Good general-purpose AI "
                "using Monte Carlo Tree Search for tactical decisions.";

      case AI_MCTS_AGGRESSIVE:
         return "MCTS AI tuned for aggressive play. Prioritizes attacks "
                "and territorial expansion over defensive positioning.";

      case AI_MCTS_DEFENSIVE:
         return "MCTS AI tuned for defensive play. Minimizes risks, "
                "avoids reaction fire, maintains strong defensive positions.";

      case AI_MCTS_FAST:
         return "MCTS AI with reduced computation time. Makes faster "
                "decisions with shorter search depth.";

      case AI_MCTS_DEEP:
         return "MCTS AI with deep search. Takes longer per turn but "
                "makes more sophisticated strategic decisions.";

      default:
         return "Unknown AI type";
   }
}

std::vector<std::string> AIFactory::getAvailableAINames() {
   std::vector<std::string> names;

   for (int i = 0; i < AI_TYPE_COUNT; ++i) {
      AIType type = static_cast<AIType>(i);
      if (isAITypeAvailable(type)) {
         names.push_back(getAITypeName(type));
      }
   }

   return names;
}

std::vector<std::string> AIFactory::getAvailableAIIdentifiers() {
   std::vector<std::string> identifiers;

   for (int i = 0; i < AI_TYPE_COUNT; ++i) {
      AIType type = static_cast<AIType>(i);
      if (isAITypeAvailable(type)) {
         identifiers.push_back(getAITypeIdentifier(type));
      }
   }

   return identifiers;
}

bool AIFactory::isAITypeAvailable(AIType type) {
   // Classic AI always available
   if (type == AI_CLASSIC) {
      return true;
   }

   // MCTS AI types are now always compiled in (Phase 1.1b+)
   return (type >= AI_MCTS_BALANCED && type <= AI_MCTS_DEEP);
}

bool AIFactory::isValidAIType(int typeValue) {
   return (typeValue >= 0 && typeValue < AI_TYPE_COUNT);
}
