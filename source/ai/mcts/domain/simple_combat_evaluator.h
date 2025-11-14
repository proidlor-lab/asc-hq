/***************************************************************************
 * simple_combat_evaluator.h - Material-based tactical evaluation
 * 
 * Purpose: Simple but effective evaluation function for combat scenarios
 *          Based on unit values, HP, positions, and tactical heuristics
 * 
 * Part of: ASC MCTS AI (Phase 0.3 - Basic Evaluation Function)
 * 
 * Evaluation Components:
 * 1. Material Score: Sum of unit values (weighted by HP)
 * 2. Position Score: Terrain advantage, height, defensive positions
 * 3. Threat Score: RF zones, concentration of fire
 * 4. Health Score: Overall army health
 * 
 * Performance: Designed for <1ms execution
 ***************************************************************************/

#ifndef MCTS_SIMPLE_COMBAT_EVALUATOR_H
#define MCTS_SIMPLE_COMBAT_EVALUATOR_H

#include "i_tactical_evaluator.h"
#include <vector>

namespace asc {
namespace mcts {

/**
 * Simple material-based evaluator for combat scenarios
 * 
 * Philosophy:
 * - Material is fundamental (unit values matter most)
 * - HP-weighted (damaged units count less)
 * - Position matters (height, cover, formations)
 * - Threat awareness (RF zones, concentration)
 * 
 * Design: Stateless, fast, tunable via EvaluationContext
 */
class SimpleCombatEvaluator : public ITacticalEvaluator {
public:
    SimpleCombatEvaluator() = default;
    ~SimpleCombatEvaluator() override = default;
    
    // ========== ITacticalEvaluator Interface ==========
    
    EvaluationResult evaluate(
        const IGameState& snapshot,
        const EvaluationContext& context) const override;
    
    bool isTerminalState(
        const IGameState& snapshot,
        PlayerID player) const override;
    
    std::unique_ptr<ITacticalEvaluator> clone() const override {
        return std::make_unique<SimpleCombatEvaluator>();
    }
    
    const char* getName() const noexcept override {
        return "SimpleCombatEvaluator";
    }
    
private:
    // ========== Evaluation Sub-Components ==========
    
    /**
     * Calculate material advantage
     * 
     * Sum of (unit_value * hp_percentage) for player minus enemies
     * 
     * @param snapshot Game state
     * @param player Perspective player
     * @return Score (-1.0 to +1.0)
     */
    float evaluateMaterial(
        const IGameState& snapshot,
        PlayerID player) const;
    
    /**
     * Calculate positional advantage
     * 
     * Considers:
     * - Height advantage
     * - Defensive terrain (cover, buildings)
     * - Formation cohesion
     * - Map control (key positions)
     * 
     * @param snapshot Game state
     * @param player Perspective player
     * @return Score (-1.0 to +1.0)
     */
    float evaluatePosition(
        const IGameState& snapshot,
        PlayerID player) const;
    
    /**
     * Calculate health advantage
     * 
     * Average HP percentage of army
     * 
     * @param snapshot Game state
     * @param player Perspective player
     * @return Score (-1.0 to +1.0)
     */
    float evaluateHealth(
        const IGameState& snapshot,
        PlayerID player) const;
    
    /**
     * Calculate threat level
     * 
     * Considers:
     * - RF zones (how many units in danger)
     * - Fire concentration (enemies targeting same units)
     * - Flanking threats
     * 
     * @param snapshot Game state
     * @param player Perspective player
     * @return Score (-1.0 to +1.0, lower = more threatened)
     */
    float evaluateThreat(
        const IGameState& snapshot,
        PlayerID player) const;
    
    // ========== Utility Methods ==========
    
    /**
     * Get approximate unit value
     * 
     * Simplified for MVP: uses rough heuristic
     * Post-MVP: integrate actual VehicleType cost/stats
     * 
     * @param unit Unit snapshot
     * @return Approximate value (0-1000)
     */
    static float getUnitValue(const UnitSnapshot& unit);
    
    /**
     * Check if unit is in enemy reaction fire range
     * 
     * Simplified for MVP: 10-hex range
     * Post-MVP: use actual weapon ranges
     * 
     * @param unit Unit to check
     * @param snapshot Game state
     * @return true if in RF danger
     */
    static bool isInReactionFireZone(
        const UnitSnapshot& unit,
        const IGameState& snapshot);
    
    /**
     * Get height advantage for unit
     * 
     * @param unit Unit
     * @param snapshot Game state
     * @return Height score (0.0 to 1.0)
     */
    static float getHeightAdvantage(
        const UnitSnapshot& unit,
        const IGameState& snapshot);
    
    /**
     * Get defensive terrain bonus
     * 
     * @param unit Unit
     * @param snapshot Game state
     * @return Terrain score (0.0 to 1.0)
     */
    static float getTerrainBonus(
        const UnitSnapshot& unit,
        const IGameState& snapshot);
    
    /**
     * Count enemies within range of unit
     * 
     * @param unit Unit
     * @param snapshot Game state
     * @param range Range in hexes
     * @return Count of nearby enemies
     */
    static int countNearbyEnemies(
        const UnitSnapshot& unit,
        const IGameState& snapshot,
        int range);
    
    /**
     * Get all enemy units for a player
     * 
     * @param snapshot Game state
     * @param player Player ID
     * @return Vector of enemy units
     */
    static std::vector<const UnitSnapshot*> getEnemyUnits(
        const IGameState& snapshot,
        PlayerID player);
    
    /**
     * Normalize score to [-1.0, +1.0] range
     * 
     * Uses tanh-like function for smooth saturation
     * 
     * @param rawScore Unnormalized score
     * @param scale Scaling factor
     * @return Normalized score
     */
    static constexpr float normalizeScore(float rawScore, float scale = 1.0f) noexcept {
        // Simple normalization: clamp to [-1, +1]
        float normalized = rawScore / scale;
        if (normalized > 1.0f) return 1.0f;
        if (normalized < -1.0f) return -1.0f;
        return normalized;
    }
};

} // namespace mcts
} // namespace asc

#endif // MCTS_SIMPLE_COMBAT_EVALUATOR_H
