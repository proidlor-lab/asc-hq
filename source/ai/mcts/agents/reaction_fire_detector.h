/***************************************************************************
 * reaction_fire_detector.h - Pre-compute reaction fire threat zones
 ***************************************************************************/

#ifndef MCTS_REACTION_FIRE_DETECTOR_H
#define MCTS_REACTION_FIRE_DETECTOR_H

#include "../domain/game_state_snapshot.h"
#include "../domain/i_combat_calculator.h"

#include <map>
#include <vector>

class VehicleType;

namespace asc {
namespace mcts {

/**
 * Represents a reaction-fire threat emitted by a hostile unit.
 */
struct ReactionFireThreat {
    UnitID attackerId{0};
    MapCoordinate attackPosition{};
    int maxRange{0};               ///< Range in hexes
    bool isStatic{false};          ///< True for turrets/fixed guns
    const UnitSnapshot* attacker{nullptr};
};

/**
 * Computes and caches reaction-fire zones for the current state.
 */
class ReactionFireDetector {
public:
    ReactionFireDetector() = default;

    void initialize(const GameStateSnapshot* state,
                    PlayerID friendlyPlayer,
                    const ICombatCalculator* calculator);

    void rebuild();

    [[nodiscard]] const std::vector<ReactionFireThreat>&
    getThreatsAt(const MapCoordinate& coordinate) const;

    void removeThreat(UnitID attackerId);

    [[nodiscard]] bool hasThreats(const MapCoordinate& coordinate) const;

private:
    [[nodiscard]] static int hexDistance(const MapCoordinate& a, const MapCoordinate& b) noexcept;
    [[nodiscard]] bool isStaticUnit(const UnitSnapshot& unit) const;
    [[nodiscard]] bool supportsReactionFire(const VehicleType* type) const;
    [[nodiscard]] int getMaxReactionRange(const VehicleType* type) const;
    void addThreatArea(const UnitSnapshot& unit);

    const GameStateSnapshot* state_{nullptr};
    const ICombatCalculator* calculator_{nullptr};
    PlayerID friendlyPlayer_{0};
    std::map<MapCoordinate, std::vector<ReactionFireThreat>> threatMap_;
    std::vector<ReactionFireThreat> emptyThreats_;
};

} // namespace mcts
} // namespace asc

#endif // MCTS_REACTION_FIRE_DETECTOR_H
