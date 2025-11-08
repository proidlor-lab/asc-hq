/***************************************************************************
 * simple_combat_evaluator.cpp - Implementation of material-based evaluator
 ***************************************************************************/

#include "simple_combat_evaluator.h"
#include "types.h"
#include <cmath>
#include <algorithm>

namespace asc {
namespace mcts {

// ========== Main Evaluation ==========

EvaluationResult SimpleCombatEvaluator::evaluate(
    const GameStateSnapshot& snapshot,
    const EvaluationContext& context) const 
{
    EvaluationResult result;
    
    // Check terminal state first
    if (isTerminalState(snapshot, context.perspectivePlayer)) {
        // Count units to determine winner
        auto playerUnits = snapshot.getPlayerUnits(context.perspectivePlayer);
        auto enemyUnits = getEnemyUnits(snapshot, context.perspectivePlayer);
        
        if (playerUnits.empty() && enemyUnits.empty()) {
            return EvaluationResult::terminal(0.0f);  // Draw
        } else if (enemyUnits.empty()) {
            return EvaluationResult::terminal(+1.0f);  // Win
        } else {
            return EvaluationResult::terminal(-1.0f);  // Loss
        }
    }
    
    // Evaluate individual components
    result.materialScore = evaluateMaterial(snapshot, context.perspectivePlayer);
    result.positionScore = evaluatePosition(snapshot, context.perspectivePlayer);
    result.healthScore = evaluateHealth(snapshot, context.perspectivePlayer);
    result.threatScore = evaluateThreat(snapshot, context.perspectivePlayer);
    
    // Weighted combination
    float score = 0.0f;
    float totalWeight = 0.0f;
    
    if (context.materialWeight > 0.0f) {
        score += result.materialScore * context.materialWeight;
        totalWeight += context.materialWeight;
    }
    
    if (context.positionWeight > 0.0f) {
        score += result.positionScore * context.positionWeight;
        totalWeight += context.positionWeight;
    }
    
    if (context.healthWeight > 0.0f) {
        score += result.healthScore * context.healthWeight;
        totalWeight += context.healthWeight;
    }
    
    if (context.threatWeight > 0.0f) {
        score += result.threatScore * context.threatWeight;
        totalWeight += context.threatWeight;
    }
    
    // Normalize
    if (totalWeight > 0.0f) {
        result.score = score / totalWeight;
    } else {
        result.score = 0.0f;
    }
    
    // Clamp to valid range
    result.score = std::clamp(result.score, -1.0f, 1.0f);
    result.unitsEvaluated = static_cast<int>(snapshot.units.size());
    
    return result;
}

bool SimpleCombatEvaluator::isTerminalState(
    const GameStateSnapshot& snapshot,
    PlayerID player) const 
{
    // Terminal if player or all enemies have no units
    auto playerUnits = snapshot.getPlayerUnits(player);
    if (playerUnits.empty()) {
        return true;  // Player eliminated
    }
    
    auto enemyUnits = getEnemyUnits(snapshot, player);
    if (enemyUnits.empty()) {
        return true;  // All enemies eliminated
    }
    
    return false;
}

// ========== Material Evaluation ==========

float SimpleCombatEvaluator::evaluateMaterial(
    const GameStateSnapshot& snapshot,
    PlayerID player) const 
{
    float playerMaterial = 0.0f;
    float enemyMaterial = 0.0f;
    
    for (const auto& unit : snapshot.units) {
        if (unit.isDestroyed()) {
            continue;
        }
        
        // Get base unit value
        float value = getUnitValue(unit);
        
        // Weight by HP percentage (damaged units count less)
        float hpFactor = static_cast<float>(unit.getHPPercent()) / 100.0f;
        float weightedValue = value * hpFactor;
        
        if (unit.owner == player) {
            playerMaterial += weightedValue;
        } else {
            enemyMaterial += weightedValue;
        }
    }
    
    // Calculate advantage
    float totalMaterial = playerMaterial + enemyMaterial;
    if (totalMaterial < 0.01f) {
        return 0.0f;  // No units
    }
    
    // Score: -1.0 (all enemy) to +1.0 (all player)
    float advantage = (playerMaterial - enemyMaterial) / totalMaterial;
    return std::clamp(advantage, -1.0f, 1.0f);
}

// ========== Position Evaluation ==========

float SimpleCombatEvaluator::evaluatePosition(
    const GameStateSnapshot& snapshot,
    PlayerID player) const 
{
    auto playerUnits = snapshot.getPlayerUnits(player);
    if (playerUnits.empty()) {
        return -1.0f;
    }
    
    auto enemyUnits = getEnemyUnits(snapshot, player);
    if (enemyUnits.empty()) {
        return +1.0f;
    }
    
    float playerPositionScore = 0.0f;
    float enemyPositionScore = 0.0f;
    
    // Evaluate player units
    for (const auto* unit : playerUnits) {
        if (unit->isDestroyed()) continue;
        
        float score = 0.0f;
        
        // Height advantage
        score += getHeightAdvantage(*unit, snapshot) * 0.3f;
        
        // Terrain bonus
        score += getTerrainBonus(*unit, snapshot) * 0.4f;
        
        // Formation cohesion (nearby friendly units)
        int nearbyFriendlies = countNearbyEnemies(*unit, snapshot, 3);
        score += std::min(nearbyFriendlies, 3) * 0.1f;
        
        playerPositionScore += score;
    }
    
    // Evaluate enemy units
    for (const auto* unit : enemyUnits) {
        if (unit->isDestroyed()) continue;
        
        float score = 0.0f;
        score += getHeightAdvantage(*unit, snapshot) * 0.3f;
        score += getTerrainBonus(*unit, snapshot) * 0.4f;
        
        int nearbyEnemies = countNearbyEnemies(*unit, snapshot, 3);
        score += std::min(nearbyEnemies, 3) * 0.1f;
        
        enemyPositionScore += score;
    }
    
    // Average per unit
    playerPositionScore /= static_cast<float>(playerUnits.size());
    enemyPositionScore /= static_cast<float>(enemyUnits.size());
    
    // Normalize to [-1, +1]
    float advantage = playerPositionScore - enemyPositionScore;
    return normalizeScore(advantage, 2.0f);
}

// ========== Health Evaluation ==========

float SimpleCombatEvaluator::evaluateHealth(
    const GameStateSnapshot& snapshot,
    PlayerID player) const 
{
    auto playerUnits = snapshot.getPlayerUnits(player);
    if (playerUnits.empty()) {
        return -1.0f;
    }
    
    auto enemyUnits = getEnemyUnits(snapshot, player);
    if (enemyUnits.empty()) {
        return +1.0f;
    }
    
    // Calculate average HP percentage
    float playerHP = 0.0f;
    for (const auto* unit : playerUnits) {
        playerHP += static_cast<float>(unit->getHPPercent());
    }
    playerHP /= static_cast<float>(playerUnits.size());
    
    float enemyHP = 0.0f;
    for (const auto* unit : enemyUnits) {
        enemyHP += static_cast<float>(unit->getHPPercent());
    }
    enemyHP /= static_cast<float>(enemyUnits.size());
    
    // Score based on HP difference
    // If both at 100%, score = 0
    // If player at 100%, enemy at 50%, score = +0.5
    float advantage = (playerHP - enemyHP) / 100.0f;
    return std::clamp(advantage, -1.0f, 1.0f);
}

// ========== Threat Evaluation ==========

float SimpleCombatEvaluator::evaluateThreat(
    const GameStateSnapshot& snapshot,
    PlayerID player) const 
{
    auto playerUnits = snapshot.getPlayerUnits(player);
    if (playerUnits.empty()) {
        return -1.0f;
    }
    
    // Count how many player units are threatened
    int threatenedUnits = 0;
    int totalUnits = 0;
    
    for (const auto* unit : playerUnits) {
        if (unit->isDestroyed()) continue;
        
        totalUnits++;
        
        if (isInReactionFireZone(*unit, snapshot)) {
            threatenedUnits++;
        }
    }
    
    if (totalUnits == 0) {
        return -1.0f;
    }
    
    // Lower score = more threatened
    // 0 threatened = +1.0, all threatened = -1.0
    float threatRatio = static_cast<float>(threatenedUnits) / static_cast<float>(totalUnits);
    return 1.0f - (2.0f * threatRatio);
}

// ========== Utility Methods ==========

float SimpleCombatEvaluator::getUnitValue(const UnitSnapshot& unit) {
    // Simplified MVP: estimate value based on type characteristics
    // Post-MVP: use actual VehicleType->productionCost or combat stats
    
    if (unit.type == nullptr) {
        return 100.0f;  // Default value
    }
    
    // Rough heuristic based on typical ASC unit values
    // Light units: 100-300
    // Medium units: 300-600
    // Heavy units: 600-1000
    
    // For MVP, return constant value (will be improved post-MVP)
    // TODO: Integrate actual unit costs from VehicleType
    return 500.0f;
}

bool SimpleCombatEvaluator::isInReactionFireZone(
    const UnitSnapshot& unit,
    const GameStateSnapshot& snapshot) 
{
    // Simplified MVP: check if enemies within 10 hexes
    // Post-MVP: use actual weapon ranges and RF rules
    
    const int RF_RANGE = 10;
    
    for (const auto& enemy : snapshot.units) {
        if (enemy.owner == unit.owner || enemy.isDestroyed()) {
            continue;
        }
        
        // Calculate hex distance
        int dx = std::abs(enemy.x - unit.x);
        int dy = std::abs(enemy.y - unit.y);
        int distance = std::max(dx, dy);  // Hex distance (approximate)
        
        if (distance <= RF_RANGE) {
            return true;
        }
    }
    
    return false;
}

float SimpleCombatEvaluator::getHeightAdvantage(
    const UnitSnapshot& unit,
    const GameStateSnapshot& snapshot) 
{
    // Height advantage: higher is better (for visibility and range)
    // Normalize to 0.0 (low) to 1.0 (high)
    
    // ASC height levels: 0-7 (typically)
    constexpr float MAX_HEIGHT = 7.0f;
    return static_cast<float>(unit.height) / MAX_HEIGHT;
}

float SimpleCombatEvaluator::getTerrainBonus(
    const UnitSnapshot& unit,
    const GameStateSnapshot& snapshot) 
{
    // Check terrain at unit position
    const auto* field = snapshot.getTerrainAt(unit.getPosition());
    
    if (field == nullptr || field->terrain == nullptr) {
        return 0.0f;  // No terrain data
    }
    
    // Simplified MVP: assume some terrains give defensive bonus
    // Post-MVP: use actual TerrainType->defenseBonus or similar
    
    // For now, just return neutral score
    // TODO: Integrate actual terrain properties
    return 0.0f;
}

int SimpleCombatEvaluator::countNearbyEnemies(
    const UnitSnapshot& unit,
    const GameStateSnapshot& snapshot,
    int range) 
{
    int count = 0;
    
    for (const auto& other : snapshot.units) {
        if (other.owner == unit.owner || other.isDestroyed()) {
            continue;  // Skip friendlies and destroyed units
        }
        
        // Calculate hex distance
        int dx = std::abs(other.x - unit.x);
        int dy = std::abs(other.y - unit.y);
        int distance = std::max(dx, dy);
        
        if (distance <= range) {
            count++;
        }
    }
    
    return count;
}

std::vector<const UnitSnapshot*> SimpleCombatEvaluator::getEnemyUnits(
    const GameStateSnapshot& snapshot,
    PlayerID player) 
{
    std::vector<const UnitSnapshot*> enemies;
    
    for (const auto& unit : snapshot.units) {
        if (unit.owner != player && !unit.isDestroyed()) {
            enemies.push_back(&unit);
        }
    }
    
    return enemies;
}

} // namespace mcts
} // namespace asc
