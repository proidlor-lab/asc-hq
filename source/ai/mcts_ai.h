/***************************************************************************
 *                          mcts_ai.h  -  description
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

#ifndef MCTS_AI_H
#define MCTS_AI_H

#include "../typen.h"
#include "../baseaiinterface.h"
#include "ai_factory.h"
#include <memory>
#include <string>
#include <vector>

// Forward declarations to avoid including entire MCTS headers
namespace asc {
namespace mcts {
    class MCTSSearch;
    class ITacticalEvaluator;
    struct MCTSConfig;
    class IGameStateReader;
    class IActionExecutor;
    class ILegacyGameInterface;
}
}

class GameMap;
class Vehicle;

/**
 * @brief MCTS-based AI implementation
 * 
 * This class bridges the legacy ASC AI interface (BaseAI) with the modern
 * MCTS implementation. It handles the conversion between ASC's game state
 * and MCTS's snapshot-based representation.
 * 
 * Design Patterns:
 * - Adapter: Adapts MCTS engine to BaseAI interface
 * - Bridge: Separates legacy interface from modern implementation
 * - Dependency Injection: Uses interfaces for MCTS components
 * 
 * LEGACY CODE INTEGRATION:
 * - Implements BaseAI interface (raw pointers, no const-correctness)
 * - Receives GameMap* raw pointer (not owned)
 * - Must implement read/write for save file compatibility
 * 
 * MODERN C++ USAGE:
 * - Internal MCTS components use std::unique_ptr
 * - Clean separation via interfaces
 * - RAII for resource management
 */
class MCTS_AI : public BaseAI {
public:
    /**
     * @brief Configuration profile for MCTS AI
     */
    struct Profile {
        std::string name;
        int maxIterations;
        int maxTimeMs;
        int rolloutDepthLimit;
        double explorationConstant;
        double earlyTerminationThreshold;
        
        // Evaluation weights
        double materialWeight;
        double positionWeight;
        double healthWeight;
        double threatWeight;
        
        bool enableLogging;
        bool enableDebugOutput;
    };
    
    /**
     * @brief Construct MCTS AI with custom profile
     * 
     * @param gameMap Pointer to game map (LEGACY: not owned)
     * @param playerID Player ID (0-7)
     * @param profile Configuration profile
     * @param config Additional configuration
     */
    MCTS_AI(GameMap* gameMap, 
            int playerID, 
            const Profile& profile,
            const AIFactory::AIConfig& config = AIFactory::AIConfig{});
    
    /**
     * @brief Destructor - cleanup MCTS resources
     */
    ~MCTS_AI() override;
    
    // ===== BaseAI Interface Implementation =====
    
    /**
     * @brief Run AI turn
     * 
     * LEGACY INTERFACE:
     * - Takes raw pointer MapDisplayInterface*
     * - Must execute actions directly on GameMap
     * - Blocking call (AI thinks and executes in one go)
     * 
     * @param mapDisplay Display interface for visualization (may be nullptr)
     */
    void run(MapDisplayInterface* mapDisplay) override;
    
    /**
     * @brief Check if AI is currently running
     * 
     * @return true if AI is executing a turn
     */
    bool isRunning() override { return running; }
    
    /**
     * @brief Get AI vision mode
     * 
     * @return Vision state (typically VISION_COMPLETE for AI)
     */
    VisibilityStates getVision() override { return vision; }
    
    /**
     * @brief Load AI state from save file
     * 
     * LEGACY INTERFACE:
     * - Uses tnstream& (ASC's custom stream class)
     * - Must maintain backward compatibility
     * 
     * @param stream Input stream
     */
    void read(tnstream& stream) override;
    
    /**
     * @brief Save AI state to save file
     * 
     * LEGACY INTERFACE:
     * - Uses tnstream& (ASC's custom stream class)
     * - Must maintain backward compatibility
     * 
     * @param stream Output stream
     */
    void write(tnstream& stream) const override;
    
    // ===== Factory Methods for Preset Profiles =====
    
    /**
     * @brief Create balanced MCTS AI
     * 
     * Balanced parameters suitable for general gameplay.
     */
    static MCTS_AI* createBalanced(GameMap* gameMap, int playerID, 
                                   const AIFactory::AIConfig& config);
    
    /**
     * @brief Create aggressive MCTS AI
     * 
     * Tuned for offensive play, higher aggression weight.
     */
    static MCTS_AI* createAggressive(GameMap* gameMap, int playerID, 
                                     const AIFactory::AIConfig& config);
    
    /**
     * @brief Create defensive MCTS AI
     * 
     * Tuned for defensive play, higher reaction fire avoidance.
     */
    static MCTS_AI* createDefensive(GameMap* gameMap, int playerID, 
                                    const AIFactory::AIConfig& config);
    
    /**
     * @brief Create fast MCTS AI
     * 
     * Reduced iterations for faster turn times.
     */
    static MCTS_AI* createFast(GameMap* gameMap, int playerID, 
                               const AIFactory::AIConfig& config);
    
    /**
     * @brief Create deep MCTS AI
     * 
     * Increased iterations for better quality decisions.
     */
    static MCTS_AI* createDeep(GameMap* gameMap, int playerID, 
                               const AIFactory::AIConfig& config);
    
    // ===== Configuration Access =====
    
    /**
     * @brief Get current profile
     */
    const Profile& getProfile() const { return profile; }
    
    /**
     * @brief Update profile at runtime
     * 
     * Note: Recreates MCTS components with new configuration.
     * Should not be called during run().
     */
    void setProfile(const Profile& newProfile);
    
    /**
     * @brief Get predefined profile by name
     */
    static Profile getProfileByName(const std::string& name);

private:
    // ===== Internal State =====
    
    // LEGACY CODE INTERFACE:
    GameMap* gameMap;              // Not owned, raw pointer from legacy code
    int playerID;                  // Player ID (0-7)
    
    // Configuration
    Profile profile;
    AIFactory::AIConfig config;
    
    // Runtime state
    bool running;
    VisibilityStates vision;
    bool initialized;
    
    // MODERN C++ COMPONENTS (using smart pointers internally):
    // MVP NOTE: Commented out for Phase 1.1b - using heuristics instead of MCTS
    // Phase 1.2+ will uncomment and implement actual MCTS
    // std::unique_ptr<asc::mcts::IGameStateReader> stateReader;
    // std::unique_ptr<asc::mcts::ITacticalEvaluator> evaluator;
    // std::unique_ptr<asc::mcts::IActionExecutor> actionExecutor;
    // std::unique_ptr<asc::mcts::MCTSSearch> searchEngine;
    
    // ===== Internal Methods =====
    
    /**
     * @brief Initialize MCTS components (lazy initialization)
     */
    void initialize();
    
    /**
     * @brief Create MCTS configuration from profile
     * MVP: Not used in Phase 1.1b
     */
    // std::unique_ptr<asc::mcts::MCTSConfig> createMCTSConfig() const;
    
    /**
     * @brief Process all player units
     * 
     * Main AI loop: iterate through units, run MCTS, execute actions.
     */
    void processUnits(MapDisplayInterface* mapDisplay);
    
    /**
     * @brief Process a single unit
     * 
     * MVP: Uses simple heuristics for Phase 1.1b testing
     * FUTURE: Replace with full MCTS search (Phase 1.2+)
     * 
     * @param unit Unit to process
     * @param legacyInterface Interface for executing actions
     * @param mapDisplay Display interface (optional)
     * @return true if unit took an action
     */
    bool processUnit(class Vehicle* unit, 
                    asc::mcts::ILegacyGameInterface* legacyInterface,
                    MapDisplayInterface* mapDisplay);
    
    /**
     * @brief Find nearby enemy units (simple heuristic)
     * 
     * @param unit Unit to search from
     * @return Vector of enemy unit IDs
     */
    std::vector<int> findNearbyEnemies(class Vehicle* unit);
    
    /**
     * @brief Execute best action found by MCTS
     * 
     * LEGACY CODE INTEGRATION:
     * - Must use ASC's Command system to execute on real GameMap
     * - Cannot use simulation executor here (needs real game effects)
     * 
     * @param unitID Unit to command
     * @param action Action to execute
     * @return true if action executed successfully
     */
    bool executeAction(int unitID, const std::string& action);
    
    /**
     * @brief Logging helper
     */
    void log(const std::string& message) const;
    
    /**
     * @brief Debug output helper
     */
    void debug(const std::string& message) const;
    
    // ===== Preset Profiles =====
    static Profile createBalancedProfile();
    static Profile createAggressiveProfile();
    static Profile createDefensiveProfile();
    static Profile createFastProfile();
    static Profile createDeepProfile();
};

#endif // MCTS_AI_H
