/***************************************************************************
 * combat_calculator.cpp - Implementation of combat calculations
 ***************************************************************************/

#include "combat_calculator.h"
#include "../../../vehicletype.h"
#include "../../../typen.h"
#include <cmath>
#include <algorithm>

namespace asc {
namespace mcts {

int CombatCalculator::calculateDamage(
    const UnitSnapshot& attacker,
    const UnitSnapshot& defender,
    int weaponIndex,
    int distance,
    int terrainDefenseBonus,
    float hemmingFactor
) {
    // Validate inputs
    if (weaponIndex < 0 || weaponIndex >= 16) {
        return 0;  // Invalid weapon
    }
    
    // Fallback for testing: if no type data, use simplified calculation
    if (attacker.type == nullptr || weaponIndex >= attacker.type->weapons.count) {
        int baseDamage = 30;
        int expBonus = attacker.experienceOffensive / 100;
        int defReduction = defender.experienceDefensive / 200;
        int totalDamage = baseDamage + expBonus - defReduction;
        return std::clamp(totalDamage, 5, 100);
    }
    
    const auto& weapon = attacker.type->weapons.weapon[weaponIndex];
    
    // Check if weapon can target this unit type
    if (defender.type != nullptr) {
        auto targetCheck = canWeaponTarget(weapon, attacker.height, defender.height, 
                                          defender.type->movemalustyp);
        if (!targetCheck.canTarget || targetCheck.effectiveness <= 0) {
            return 0;  // Cannot damage this target
        }
    }
    
    // Get base weapon strength at this distance
    int baseStrength = getWeaponStrength(weapon, distance);
    if (baseStrength <= 0) {
        return 0;  // Out of range or no damage
    }
    
    // ========== ASC Combat Formula (from attack.cpp::tfight::calc) ==========
    
    // Calculate absolute strength
    // absstrength = strength * (1 + exp + attackbonus) * damage_factor * hemming
    float expOffensiveBonus = calculateExperienceBonus(attacker.experienceOffensive);
    float damageStateFactor = calculateDamageStateFactor(attacker.damage);
    
    // attackbonus: simplified (ASC uses terrain-based attack bonus)
    float attackBonus = 0.0f;  // TODO: Add terrain attack bonus if needed
    
    float absStrength = static_cast<float>(baseStrength) 
                       * (1.0f + expOffensiveBonus + attackBonus)
                       * damageStateFactor
                       * hemmingFactor;
    
    // Calculate absolute defense
    // absdefense = (armor/5) * (1 + defensebonus + exp)
    int armor = 0;
    if (defender.type != nullptr) {
        armor = defender.type->armor;
    } else {
        armor = 10;  // Default armor for testing
    }
    
    const float armorDivisor = 5.0f;
    float expDefensiveBonus = calculateExperienceBonus(defender.experienceDefensive);
    
    // defensebonus: terrain defense bonus (0-100 scale, divide by 8 per ASC formula)
    float defenseBonus = static_cast<float>(terrainDefenseBonus) / 8.0f;
    
    float absDefense = (static_cast<float>(armor) / armorDivisor)
                      * (1.0f + defenseBonus + expDefensiveBonus);
    
    // Prevent division by zero
    if (absDefense < 0.1f) {
        absDefense = 0.1f;
    }
    
    // Calculate damage
    // damage = ceil(current_damage + absstrength / absdefense * 1000 / damagefactor)
    // damagefactor is a game parameter (typically 10)
    const int damageFactor = 10;  // TODO: Read from game parameters if available
    
    float damageIncrease = absStrength / absDefense * 1000.0f / static_cast<float>(damageFactor);
    int newDamage = static_cast<int>(std::ceil(static_cast<float>(defender.damage) + damageIncrease));
    
    // Clamp to valid range
    newDamage = std::clamp(newDamage, defender.damage + 1, 100);
    
    // Return damage dealt (not total damage)
    int damageDealt = newDamage - defender.damage;
    
    // Apply weapon effectiveness against unit type
    if (defender.type != nullptr && defender.type->movemalustyp < cmovemalitypenum) {
        int effectiveness = weapon.targetingAccuracy[defender.type->movemalustyp];
        if (effectiveness > 0 && effectiveness < 100) {
            damageDealt = (damageDealt * effectiveness) / 100;
        }
    }
    
    return std::clamp(damageDealt, 1, 100);
}

WeaponTargetCheck CombatCalculator::canWeaponTarget(
    const ::SingleWeapon& weapon,
    int attackerHeight,
    int defenderHeight,
    int defenderUnitType
) {
    WeaponTargetCheck result;
    result.canTarget = false;
    result.effectiveness = 0;
    result.reason = "Unknown";
    
    // Check 1: Can weapon shoot from attacker's height?
    if (!(weapon.sourceheight & attackerHeight)) {
        result.reason = "Cannot shoot from this height";
        return result;
    }
    
    // Check 2: Can weapon target defender's height?
    if (!(weapon.targ & defenderHeight)) {
        result.reason = "Cannot target this height";
        return result;
    }
    
    // Check 3: Weapon effectiveness against unit type
    if (defenderUnitType >= 0 && defenderUnitType < cmovemalitypenum) {
        int effectiveness = weapon.targetingAccuracy[defenderUnitType];
        if (effectiveness <= 0) {
            result.reason = "Weapon ineffective against this unit type";
            return result;
        }
        result.effectiveness = effectiveness;
    } else {
        result.effectiveness = 100;  // Default effectiveness
    }
    
    // Check 4: Height difference efficiency (optional, not critical for basic targeting)
    // The efficiency array has 13 elements for height differences from -6 to +6
    // We can add this check later if needed
    
    result.canTarget = true;
    result.reason = "Can target";
    return result;
}

int CombatCalculator::getWeaponStrength(
    const ::SingleWeapon& weapon,
    int distance
) {
    // Convert weapon ranges from storage format (multiples of 10)
    int minRange = (weapon.mindistance + 9) / 10;  // Round up
    int maxRange = weapon.maxdistance / 10;
    
    // Check if in range
    if (distance < minRange || distance > maxRange) {
        return 0;  // Out of range
    }
    
    // Linear interpolation between max and min strength
    if (distance <= minRange) {
        return weapon.maxstrength;
    } else if (distance >= maxRange) {
        return weapon.minstrength;
    } else {
        int range = maxRange - minRange;
        if (range > 0) {
            float ratio = static_cast<float>(distance - minRange) / static_cast<float>(range);
            return static_cast<int>(
                weapon.maxstrength - ratio * (weapon.maxstrength - weapon.minstrength)
            );
        } else {
            return weapon.maxstrength;
        }
    }
}

float CombatCalculator::calculateExperienceBonus(
    int experience,
    float maxBonus,
    float experienceAt90
) {
    if (experience < 0) {
        return 0.0f;
    }
    
    // ASC formula: maxBonus * (1.0 - pow(pow(0.1, 1.0/ninety), experience))
    float expBonus = maxBonus * (1.0f - std::pow(std::pow(0.1f, 1.0f / experienceAt90), 
                                                  static_cast<float>(experience)));
    return expBonus;
}

float CombatCalculator::calculateDamageStateFactor(int damage) {
    // ASC formula: 1 - (2.0 * damage / 300.0)
    // This means at 100% damage, unit still has 33% of attack strength
    return 1.0f - (2.0f * static_cast<float>(damage) / 300.0f);
}

} // namespace mcts
} // namespace asc
