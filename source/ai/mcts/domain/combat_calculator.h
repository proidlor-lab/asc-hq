/***************************************************************************
 * combat_calculator.h - Reusable combat calculation for MCTS
 *
 * This interface wraps ASC's AttackFormula to provide combat calculations
 * for MCTS simulation while maintaining consistency with the actual game.
 ***************************************************************************/

#ifndef MCTS_COMBAT_CALCULATOR_H
#define MCTS_COMBAT_CALCULATOR_H

#include "unit_snapshot.h"

// Forward declarations from ASC
class SingleWeapon;

namespace asc {
namespace mcts {

/**
 * @brief Result of a weapon target check
 */
struct WeaponTargetCheck {
   bool canTarget;      ///< Can this weapon hit the target?
   int effectiveness;   ///< Weapon effectiveness (0-100%)
   const char* reason;  ///< Reason if cannot target
};

/**
 * @brief Combat calculator using ASC's actual combat formulas
 *
 * This class provides combat calculations for MCTS by wrapping or
 * reimplementing ASC's AttackFormula logic. It ensures that MCTS
 * simulations use the same combat mechanics as the actual game.
 */
class CombatCalculator {
  public:
   /**
    * @brief Calculate damage using ASC's actual combat formula
    *
    * Formula from attack.cpp::tfight::calc():
    * absstrength = strength * (1 + exp_bonus + attack_bonus) * damage_factor * hemming
    * absdefense = (armor/5) * (1 + defense_bonus + exp_bonus)
    * damage = ceil(current_damage + absstrength / absdefense * 1000 / damagefactor)
    *
    * @param attacker Attacking unit
    * @param defender Defending unit
    * @param weaponIndex Which weapon to use
    * @param distance Distance in hex fields
    * @param terrainDefenseBonus Terrain defense bonus (0-100)
    * @param hemmingFactor Flanking/hemming factor (1.0 = no hemming, 1.4 = max)
    * @return Damage dealt (0-100)
    */
   static int calculateDamage(const UnitSnapshot& attacker, const UnitSnapshot& defender,
                              int weaponIndex, int distance, int terrainDefenseBonus = 0,
                              float hemmingFactor = 1.0f);

   /**
    * @brief Check if a weapon can target a specific unit
    *
    * Checks:
    * 1. Height targeting (weapon.targ & defender.height)
    * 2. Unit type effectiveness (weapon.targetingAccuracy[defender.movemalustyp])
    * 3. Height difference efficiency (weapon.efficiency[heightDiff+6])
    *
    * @param weapon Weapon to check
    * @param attackerHeight Attacker's height (bitmapped)
    * @param defenderHeight Defender's height (bitmapped)
    * @param defenderUnitType Defender's MoveMalusType
    * @return Check result with effectiveness percentage
    */
   static WeaponTargetCheck canWeaponTarget(const ::SingleWeapon& weapon, int attackerHeight,
                                            int defenderHeight, int defenderUnitType);

   /**
    * @brief Calculate weapon strength at a given distance
    *
    * Uses linear interpolation between min and max strength based on distance.
    * Weapon ranges are stored as multiples of 10 (e.g., 100 = 10 hexes)
    *
    * @param weapon Weapon to calculate strength for
    * @param distance Distance in hex fields
    * @return Base weapon strength (before modifiers)
    */
   static int getWeaponStrength(const ::SingleWeapon& weapon, int distance);

   /**
    * @brief Calculate experience bonus for attack
    *
    * Formula from attack.cpp: AttackFormula::strength_experience()
    * maxExpBonus * (1.0 - pow(pow(0.1, 1.0/ninety), experience))
    *
    * @param experience Unit experience (0-500)
    * @param maxBonus Maximum bonus percentage (default: 100% = factor 1.0)
    * @param experienceAt90 Experience needed for 90% of max bonus (default: 250)
    * @return Experience bonus factor (e.g., 0.5 = +50%)
    */
   static float calculateExperienceBonus(int experience, float maxBonus = 1.0f,
                                         float experienceAt90 = 250.0f);

   /**
    * @brief Calculate damage reduction from unit damage state
    *
    * Formula from attack.cpp: AttackFormula::strength_damage()
    * 1 - (2.0 * damage / 300.0)
    *
    * @param damage Current damage (0-100)
    * @return Damage reduction factor (1.0 = full strength, 0.33 = 100% damaged)
    */
   static float calculateDamageStateFactor(int damage);
};

}  // namespace mcts
}  // namespace asc

#endif  // MCTS_COMBAT_CALCULATOR_H
