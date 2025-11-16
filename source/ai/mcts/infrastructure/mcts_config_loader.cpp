/***************************************************************************
 *                   mcts_config_loader.cpp  -  implementation
 ***************************************************************************/

#include "mcts_config_loader.h"
#include "../core/mcts_config.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace asc {
namespace mcts {

namespace {

constexpr const char* kDefaultConfigPath = "source/ai/mcts/config/mcts_ai_profiles.ini";
constexpr const char* kEnvConfigPath = "MCTS_PROFILE_FILE";

std::string toLower(std::string s) {
   std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
   return s;
}

// ---------- Built-in profiles ----------

MCTSConfigLoader::ProfileConfig builtinBalanced() {
   return MCTSConfigLoader::ProfileConfig{};
}

MCTSConfigLoader::ProfileConfig builtinAggressive() {
   auto p = builtinBalanced();
   p.maxIterations = 250;
   p.materialWeight = 3.0;
   p.positionWeight = 0.5;
   p.threatWeight = 0.8;
   p.agentProfile = "Aggressive";
   return p;
}

MCTSConfigLoader::ProfileConfig builtinDefensive() {
   auto p = builtinBalanced();
   p.materialWeight = 1.5;
   p.positionWeight = 1.5;
   p.healthWeight = 2.0;
   p.threatWeight = 2.5;
   p.agentProfile = "Defensive";
   return p;
}

// ---------- INI parsing helpers ----------

struct ParsedProfile {
   MCTSConfigLoader::ProfileConfig cfg;
   MCTSConfigLoader::Source source{MCTSConfigLoader::Source::BUILTIN};
};

bool parseBool(const std::string& v, bool def) {
   const auto l = toLower(v);
   if (l == "1" || l == "true" || l == "yes" || l == "on")
      return true;
   if (l == "0" || l == "false" || l == "no" || l == "off")
      return false;
   return def;
}

template <typename T>
T parseNumber(const std::string& v, T def) {
   std::istringstream iss(v);
   T out{};
   if (iss >> out)
      return out;
   return def;
}

void applyKeyValue(const std::string& key, const std::string& value, ParsedProfile& profile) {
   auto& cfg = profile.cfg;
   const auto k = toLower(key);
   if (k == "maxiterations")
      cfg.maxIterations = parseNumber<int>(value, cfg.maxIterations);
   else if (k == "maxtimems")
      cfg.maxTimeMs = parseNumber<int>(value, cfg.maxTimeMs);
   else if (k == "rolloutdepthlimit")
      cfg.rolloutDepthLimit = parseNumber<int>(value, cfg.rolloutDepthLimit);
   else if (k == "explorationconstant")
      cfg.explorationConstant = parseNumber<double>(value, cfg.explorationConstant);
   else if (k == "earlyterminationthreshold")
      cfg.earlyTerminationThreshold = parseNumber<double>(value, cfg.earlyTerminationThreshold);
   else if (k == "materialweight")
      cfg.materialWeight = parseNumber<double>(value, cfg.materialWeight);
   else if (k == "positionweight")
      cfg.positionWeight = parseNumber<double>(value, cfg.positionWeight);
   else if (k == "healthweight")
      cfg.healthWeight = parseNumber<double>(value, cfg.healthWeight);
   else if (k == "threatweight")
      cfg.threatWeight = parseNumber<double>(value, cfg.threatWeight);
   else if (k == "agentprofile")
      cfg.agentProfile = value;
   else if (k == "enablelogging")
      cfg.enableLogging = parseBool(value, cfg.enableLogging);
   else if (k == "enabledebugoutput")
      cfg.enableDebugOutput = parseBool(value, cfg.enableDebugOutput);
}

std::map<std::string, ParsedProfile> loadProfilesFromFile(const std::string& path) {
   std::ifstream in(path);
   if (!in) {
      return {};
   }

   std::map<std::string, ParsedProfile> profiles;
   std::string line;
   std::string currentSection;
   while (std::getline(in, line)) {
      // Trim
      auto posComment = line.find_first_of("#;");
      if (posComment != std::string::npos)
         line = line.substr(0, posComment);
      auto trim = [](const std::string& s) {
         const auto begin = s.find_first_not_of(" \t\r\n");
         if (begin == std::string::npos)
            return std::string();
         const auto end = s.find_last_not_of(" \t\r\n");
         return s.substr(begin, end - begin + 1);
      };
      line = trim(line);
      if (line.empty())
         continue;

      if (line.front() == '[' && line.back() == ']') {
         currentSection = toLower(line.substr(1, line.size() - 2));
         profiles[currentSection] = ParsedProfile{};
         profiles[currentSection].source = MCTSConfigLoader::Source::CONFIG_FILE;
         continue;
      }

      auto eq = line.find('=');
      if (eq == std::string::npos || currentSection.empty())
         continue;
      auto key = trim(line.substr(0, eq));
      auto value = trim(line.substr(eq + 1));
      applyKeyValue(key, value, profiles[currentSection]);
   }
   return profiles;
}

}  // namespace

// ---------- Public API ----------

MCTSConfig MCTSConfigLoader::loadConfig(const std::string& name) {
   // For now, delegate to loadProfile and map fields into MCTSConfig
   const auto p = loadProfile(name);
   MCTSConfig cfg;
   cfg.maxIterations = p.maxIterations;
   cfg.maxTimeMs = p.maxTimeMs;
   cfg.rolloutDepthLimit = p.rolloutDepthLimit;
   cfg.explorationConstant = p.explorationConstant;
   cfg.earlyTerminationThreshold = p.earlyTerminationThreshold;
   cfg.agentProfile = p.agentProfile;
   return cfg;
}

MCTSConfigLoader::ProfileConfig MCTSConfigLoader::loadProfile(const std::string& name) {
   const auto key = toLower(name);

   // Builtin base
   ProfileConfig base = builtinBalanced();
   if (key == "aggressive" || key == "mcts_aggressive")
      base = builtinAggressive();
   else if (key == "defensive" || key == "mcts_defensive")
      base = builtinDefensive();

   // File overrides
   static std::map<std::string, ParsedProfile> cache;
   static bool loaded = false;
   if (!loaded) {
      std::string path = kDefaultConfigPath;
      if (const char* env = std::getenv(kEnvConfigPath)) {
         path = env;
      }
      cache = loadProfilesFromFile(path);
      loaded = true;
   }

   const auto it = cache.find(key);
   if (it != cache.end()) {
      const auto& overrideCfg = it->second.cfg;
      auto apply = [&](auto& field, const auto& val) { field = val; };
      apply(base.maxIterations, overrideCfg.maxIterations);
      apply(base.maxTimeMs, overrideCfg.maxTimeMs);
      apply(base.rolloutDepthLimit, overrideCfg.rolloutDepthLimit);
      apply(base.explorationConstant, overrideCfg.explorationConstant);
      apply(base.earlyTerminationThreshold, overrideCfg.earlyTerminationThreshold);
      apply(base.materialWeight, overrideCfg.materialWeight);
      apply(base.positionWeight, overrideCfg.positionWeight);
      apply(base.healthWeight, overrideCfg.healthWeight);
      apply(base.threatWeight, overrideCfg.threatWeight);
      apply(base.agentProfile, overrideCfg.agentProfile);
      apply(base.enableLogging, overrideCfg.enableLogging);
      apply(base.enableDebugOutput, overrideCfg.enableDebugOutput);
   }

   return base;
}

// The remaining methods are placeholders for fuller config management
std::map<std::string, MCTSConfigLoader::ConfigEntry>
MCTSConfigLoader::loadConfigFile(const std::string&) {
   return {};
}

void MCTSConfigLoader::registerConfig(const std::string&, const MCTSConfig&, const std::string&) {}
std::vector<std::string> MCTSConfigLoader::getAvailableConfigs() {
   return {};
}
MCTSConfigLoader::ConfigEntry MCTSConfigLoader::getConfigEntry(const std::string&) {
   return {};
}
bool MCTSConfigLoader::hasConfig(const std::string&) {
   return false;
}
void MCTSConfigLoader::clearRuntimeConfigs() {}
void MCTSConfigLoader::reset() {}
std::map<std::string, MCTSConfigLoader::ConfigEntry>& MCTSConfigLoader::getRuntimeRegistry() {
   static std::map<std::string, ConfigEntry> dummy;
   return dummy;
}
std::map<std::string, MCTSConfigLoader::ConfigEntry>& MCTSConfigLoader::getFileRegistry() {
   static std::map<std::string, ConfigEntry> dummy;
   return dummy;
}
std::string MCTSConfigLoader::trim(const std::string& s) {
   return s;
}
std::pair<std::string, std::string> MCTSConfigLoader::parseLine(const std::string& line) {
   return {line, {}};
}

}  // namespace mcts
}  // namespace asc
