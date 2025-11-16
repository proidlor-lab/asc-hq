/***************************************************************************
 * agent_weight_config.cpp
 ***************************************************************************/

#include "agent_weight_config.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>

namespace asc {
namespace mcts {

namespace {

std::string trim(std::string value) {
    const auto notSpace = [](unsigned char ch) {
        return !std::isspace(ch);
    };
    value.erase(value.begin(), std::find_if(value.begin(), value.end(), notSpace));
    value.erase(std::find_if(value.rbegin(), value.rend(), notSpace).base(), value.end());
    return value;
}

std::string normalizeSectionName(const std::string& input) {
    if (input.rfind("AgentWeights.", 0) == 0) {
        return input;
    }
    return "AgentWeights." + input;
}

} // namespace

bool AgentWeightConfig::loadFromFile(const std::string& path) {
    std::ifstream input(path);
    if (!input.is_open()) {
        return false;
    }

    profiles_.clear();

    std::string line;
    std::string currentSection = activeProfile_;

    while (std::getline(input, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#'
            || (line.size() >= 2 && line[0] == '/' && line[1] == '/')) {
            continue;
        }

        if (line.front() == '[' && line.back() == ']') {
            const std::string section = line.substr(1, line.size() - 2);
            currentSection = normalizeSectionName(trim(section));
            continue;
        }

        const auto equalsPos = line.find('=');
        if (equalsPos == std::string::npos) {
            continue;
        }

        std::string key = trim(line.substr(0, equalsPos));
        std::string value = trim(line.substr(equalsPos + 1));

        try {
            const float weight = std::stof(value);
            profiles_[currentSection][key] = weight;
        } catch (const std::exception&) {
            // Ignore malformed weights
        }
    }

    return true;
}

void AgentWeightConfig::selectProfile(const std::string& profile) {
    const std::string normalized = normalizeSectionName(profile);
    if (hasProfile(normalized)) {
        activeProfile_ = normalized;
    }
}

bool AgentWeightConfig::hasProfile(const std::string& profile) const {
    return profiles_.find(normalizeSectionName(profile)) != profiles_.end();
}

float AgentWeightConfig::getWeight(const std::string& agentName) const {
    const auto profileIt = profiles_.find(activeProfile_);
    if (profileIt == profiles_.end()) {
        return 1.0f;
    }
    const auto weightIt = profileIt->second.find(agentName);
    if (weightIt == profileIt->second.end()) {
        return 1.0f;
    }
    return weightIt->second;
}

} // namespace mcts
} // namespace asc
