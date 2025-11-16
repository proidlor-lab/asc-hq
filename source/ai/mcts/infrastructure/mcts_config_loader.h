/***************************************************************************
 *                   mcts_config_loader.h  -  description
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

#ifndef MCTS_CONFIG_LOADER_H
#define MCTS_CONFIG_LOADER_H

#include <string>
#include <map>
#include <memory>
#include <vector>
#include "core/mcts_config.h"

// Forward declaration no longer needed - MCTSConfig included above
namespace asc {
namespace mcts {

/**
 * @brief Configuration loader for MCTS AI profiles
 *
 * Loads MCTS AI configuration from various sources:
 * - Built-in presets (balanced, aggressive, etc.)
 * - Configuration files (INI format)
 * - Runtime overrides (command line, console commands)
 *
 * Design Pattern: Strategy + Builder
 *
 * CONFIGURATION HIERARCHY:
 * 1. Built-in defaults (always available)
 * 2. Config file overrides (optional)
 * 3. Runtime overrides (highest priority)
 */
class MCTSConfigLoader {
  public:
   /**
    * @brief Extended profile config (search + evaluation/agent profile)
    */
   struct ProfileConfig {
      // Search
      int maxIterations{200};
      int maxTimeMs{2000};
      int rolloutDepthLimit{10};
      double explorationConstant{1.414};
      double earlyTerminationThreshold{0.95};

      // Evaluation weights
      double materialWeight{2.0};
      double positionWeight{1.0};
      double healthWeight{1.5};
      double threatWeight{1.2};

      // Agent weight profile name
      std::string agentProfile{"Balanced"};

      // Logging
      bool enableLogging{false};
      bool enableDebugOutput{false};
   };

   /**
    * @brief Configuration source
    */
   enum class Source {
      BUILTIN,      // Built-in preset
      CONFIG_FILE,  // Loaded from file
      RUNTIME       // Set at runtime
   };

   /**
    * @brief Configuration entry with metadata
    */
   struct ConfigEntry {
      std::string name;
      MCTSConfig config;
      Source source;
      std::string description;
   };

   /**
    * @brief Load configuration by name
    *
    * Searches in order: runtime -> config file -> built-in
    *
    * @param name Configuration name (e.g., "balanced", "aggressive")
    * @return Configuration object
    * @throws std::runtime_error if configuration not found
    */
   static MCTSConfig loadConfig(const std::string& name);

   /**
    * @brief Load profile config (search + evaluation weights)
    *
    * Searches runtime -> config file -> built-in. Safe default always returned.
    */
   static ProfileConfig loadProfile(const std::string& name);

   /**
    * @brief Load configuration from file
    *
    * File format: INI-style key-value pairs
    *
    * Example:
    * ```
    * [mcts_balanced]
    * maxIterations=200
    * maxTimeMs=2000
    * rolloutDepthLimit=10
    * explorationConstant=1.414
    * ```
    *
    * @param filepath Path to configuration file
    * @return Map of profile name -> configuration
    * @throws std::runtime_error if file cannot be read
    */
   static std::map<std::string, ConfigEntry> loadConfigFile(const std::string& filepath);

   /**
    * @brief Register custom configuration at runtime
    *
    * @param name Profile name
    * @param config Configuration object
    * @param description Optional description
    */
   static void registerConfig(const std::string& name, const MCTSConfig& config,
                              const std::string& description = "");

   /**
    * @brief Get all available configuration names
    *
    * @return Vector of profile names (built-in + loaded + runtime)
    */
   static std::vector<std::string> getAvailableConfigs();

   /**
    * @brief Get configuration metadata
    *
    * @param name Configuration name
    * @return Configuration entry with metadata
    * @throws std::runtime_error if not found
    */
   static ConfigEntry getConfigEntry(const std::string& name);

   /**
    * @brief Check if configuration exists
    *
    * @param name Configuration name
    * @return true if configuration is registered
    */
   static bool hasConfig(const std::string& name);

   /**
    * @brief Clear all runtime configurations
    *
    * Built-in configurations are not affected.
    */
   static void clearRuntimeConfigs();

   /**
    * @brief Reset to defaults
    *
    * Clears all loaded and runtime configurations.
    */
   static void reset();

  private:
   // Non-instantiable utility class
   MCTSConfigLoader() = delete;

   // Internal registry (singleton pattern)
   static std::map<std::string, ConfigEntry>& getRuntimeRegistry();
   static std::map<std::string, ConfigEntry>& getFileRegistry();

   // Built-in configuration creators
   static ConfigEntry createBalancedConfig();
   static ConfigEntry createAggressiveConfig();
   static ConfigEntry createDefensiveConfig();
   static ConfigEntry createFastConfig();
   static ConfigEntry createDeepConfig();

   // INI file parser helpers
   static std::string trim(const std::string& str);
   static std::pair<std::string, std::string> parseLine(const std::string& line);
};

}  // namespace mcts
}  // namespace asc

#endif  // MCTS_CONFIG_LOADER_H
