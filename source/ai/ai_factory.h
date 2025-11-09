/***************************************************************************
 *                          ai_factory.h  -  description
 *                             -------------------
 *    begin                : 2025-11-08
 *    copyright            : (C) 2025
 *    email                : asc-hq.org
 ***************************************************************************/

/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef AI_FACTORY_H
#define AI_FACTORY_H

#include <string>
#include <vector>
#include <memory>
#include "../typen.h"
#include "../baseaiinterface.h"

class GameMap;

/**
 * @brief Factory for creating different AI implementations
 * 
 * This factory provides a centralized way to create AI instances of different types.
 * It supports the classic rule-based AI and multiple MCTS AI configurations.
 * 
 * Design Pattern: Abstract Factory with Registry
 * C++ Standard: Modern C++14/23 compatible
 */
class AIFactory {
public:
    /**
     * @brief Enumeration of available AI types
     * 
     * NOTE: Values are persisted to save files. NEVER change existing values.
     * Only append new types at the end.
     */
    enum AIType {
        AI_CLASSIC = 0,           // Original rule-based AI
        AI_MCTS_BALANCED = 1,     // MCTS with balanced parameters
        AI_MCTS_AGGRESSIVE = 2,   // MCTS tuned for aggressive play
        AI_MCTS_DEFENSIVE = 3,    // MCTS tuned for defensive play
        AI_MCTS_FAST = 4,         // MCTS with reduced computation time
        AI_MCTS_DEEP = 5,         // MCTS with deep search
        
        // Future AI types can be added here
        // AI_NEURAL = 6,
        // AI_MINIMAX = 7,
        
        AI_TYPE_COUNT             // Must always be last
    };
    
    /**
     * @brief Configuration for AI creation
     * 
     * Allows passing additional configuration to AI instances.
     */
    struct AIConfig {
        bool enableLogging = false;
        bool enableDebugOutput = false;
        int difficultyLevel = 5;  // 1-10 scale
        std::string configFilePath;
        
        static AIConfig getDefault() {
            AIConfig cfg;
            cfg.enableLogging = false;
            cfg.enableDebugOutput = false;
            cfg.difficultyLevel = 5;
            return cfg;
        }
    };
    
    /**
     * @brief Create an AI instance of the specified type
     * 
     * @param type The type of AI to create
     * @param gameMap Pointer to the game map (LEGACY: raw pointer)
     * @param playerID The player ID (0-7)
     * @param config Optional configuration
     * @return Raw pointer to BaseAI (LEGACY: caller owns the pointer)
     * 
     * LEGACY CODE ISSUE:
     * - Returns raw pointer instead of unique_ptr (required by BaseAI* in Player class)
     * - Caller must manually delete (done in Player destructor)
     * - FUTURE: Modernize Player class to use std::unique_ptr<BaseAI>
     * 
     * @throws std::invalid_argument if type is invalid or out of range
     */
    static BaseAI* createAI(AIType type, 
                           GameMap* gameMap, 
                           int playerID,
                           const AIConfig& config);
    
    // Overload without config parameter (uses default)
    static BaseAI* createAI(AIType type, 
                           GameMap* gameMap, 
                           int playerID);
    
    /**
     * @brief Parse AI type from string name
     * 
     * @param name String representation (e.g., "classic", "mcts_balanced")
     * @return Corresponding AIType enum value
     * @throws std::invalid_argument if name is not recognized
     */
    static AIType parseAIType(const std::string& name);
    
    /**
     * @brief Get string name for an AI type
     * 
     * @param type The AI type enum value
     * @return Human-readable name (e.g., "Classic AI", "MCTS Balanced")
     */
    static std::string getAITypeName(AIType type);
    
    /**
     * @brief Get short identifier for an AI type
     * 
     * @param type The AI type enum value
     * @return Short identifier (e.g., "classic", "mcts_balanced")
     */
    static std::string getAITypeIdentifier(AIType type);
    
    /**
     * @brief Get description for an AI type
     * 
     * @param type The AI type enum value
     * @return Description of the AI's characteristics
     */
    static std::string getAITypeDescription(AIType type);
    
    /**
     * @brief Get list of all available AI type names
     * 
     * @return Vector of human-readable AI names
     */
    static std::vector<std::string> getAvailableAINames();
    
    /**
     * @brief Get list of all available AI type identifiers
     * 
     * @return Vector of AI identifiers (for config files)
     */
    static std::vector<std::string> getAvailableAIIdentifiers();
    
    /**
     * @brief Check if an AI type is available (compiled in)
     * 
     * Some AI types may be disabled at compile time.
     * 
     * @param type The AI type to check
     * @return true if the AI type can be instantiated
     */
    static bool isAITypeAvailable(AIType type);
    
    /**
     * @brief Get default AI type
     * 
     * Used when no AI type is specified (e.g., loading old save files).
     * 
     * @return Default AI type (currently AI_CLASSIC for backward compatibility)
     */
    static AIType getDefaultAIType() { return AI_CLASSIC; }
    
    /**
     * @brief Validate AI type value (for save file loading)
     * 
     * @param typeValue Integer value from save file
     * @return true if value is valid and available
     */
    static bool isValidAIType(int typeValue);

private:
    // Non-instantiable utility class
    AIFactory() = delete;
    ~AIFactory() = delete;
    AIFactory(const AIFactory&) = delete;
    AIFactory& operator=(const AIFactory&) = delete;
};

#endif // AI_FACTORY_H
