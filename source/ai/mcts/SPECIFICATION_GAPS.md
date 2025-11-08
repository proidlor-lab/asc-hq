# MCTS AI - Specification Gaps & Future Work

**Last Updated**: 2025-11-08  
**Purpose**: Track MVP simplifications, missing specifications, and post-MVP improvements

---

## Overview

This document tracks areas where the MCTS AI implementation uses simplified heuristics or placeholder logic instead of full ASC game mechanics integration. These are deliberate MVP choices to prioritize architecture over completeness.

**Strategy**: Build a clean, extensible architecture first. Integrate actual ASC mechanics post-MVP.

---

## Phase 0.1: Game State Cloning

### ✅ Complete - No significant gaps

Minor future improvements:
- **Terrain data**: Currently sparse storage. Could add more terrain properties if needed for advanced tactics
- **Building integration**: Not yet captured in snapshots (deferred to Phase 2)

---

## Phase 0.2: Action Execution

### Simplified Combat Mechanics

**Current State**: Deterministic damage calculation with fixed values

**Gap**: Not using actual ASC combat formulas from `VehicleType`

**Actual ASC Integration Needed**:
```cpp
// TODO: Use VehicleType weapon stats
const Weapontype* weapon = attacker.type->getWeapon(target);
if (weapon) {
    int damage = weapon->calculateDamage(attacker, defender, terrain, distance);
    // Apply terrain modifiers, armor, experience bonuses
}
```

**Impact**: MVP combat is less realistic but sufficient for testing MCTS architecture

**Priority**: Medium (post-MVP Phase 1.4)

---

### Simplified Pathfinding

**Current State**: Only generates moves to 6 neighbor hexes

**Gap**: No A* pathfinding with full movement budget

**Actual ASC Integration Needed**:
```cpp
// TODO: Full pathfinding
PathFinder pathfinder(snapshot);
auto reachable = pathfinder.findReachableFields(unit, unit.movement);
for (auto& field : reachable) {
    actions.push_back(MoveAction{unit.networkID, field.coord});
}
```

**Impact**: Limits tactical options. MCTS can't plan multi-hex moves.

**Priority**: High (post-MVP Phase 1.3)

---

### Simplified Reaction Fire

**Current State**: Assumes RF always hits with fixed damage

**Gap**: Not using actual ASC RF rules, hit chances, or weapon specifics

**Actual ASC Integration Needed**:
```cpp
// TODO: Actual RF simulation
ReactionFireHandler rfHandler(snapshot);
auto rfResults = rfHandler.simulateReactionFire(move);
for (auto& rf : rfResults) {
    if (rf.hits(rng)) {
        applyDamage(rf.target, rf.damage);
    }
}
```

**Impact**: RF evaluation is approximate. May miss edge cases.

**Priority**: Medium (post-MVP Phase 1.4)

---

### Missing Action Types

**Current State**: Only Move, Attack, Wait

**Gap**: No support for:
- **Service actions**: Repair, refuel, supply
- **Build actions**: Construct buildings, deploy units
- **Research actions**: Tech tree progression
- **Cargo actions**: Load/unload units
- **Special abilities**: Minelaying, healing, etc.

**Integration Needed**: Extend `Action` variant with new types

**Impact**: MCTS can only handle basic combat. No base-building or logistics.

**Priority**: 
- Service actions: High (Phase 2)
- Build/Research: Medium (Phase 3-4)
- Cargo/Special: Low (Phase 5+)

---

### No Ammo Tracking

**Current State**: `ammoMask` bitmask only (has ammo yes/no)

**Gap**: No per-weapon ammo counts

**Actual ASC Integration Needed**:
```cpp
struct UnitSnapshot {
    int16_t ammo[16];  // Per-weapon ammo (instead of just mask)
};
```

**Impact**: Can't model ammo scarcity or resupply needs.

**Priority**: Low-Medium (post-MVP if logistics become critical)

---

## Phase 0.3: State Evaluation

### Fixed Unit Values

**Current State**: All units worth 500 points

**Gap**: Not using actual `VehicleType->productionCost` or combat stats

**Actual ASC Integration Needed**:
```cpp
float SimpleCombatEvaluator::getUnitValue(const UnitSnapshot& unit) {
    if (unit.type == nullptr) return 100.0f;
    
    // Use actual production cost as base value
    float baseValue = unit.type->productionCost;
    
    // Could also factor in combat stats
    float combatValue = evaluateCombatStats(unit.type);
    
    return baseValue * 0.7f + combatValue * 0.3f;
}
```

**Impact**: Material evaluation doesn't reflect actual unit worth. May overvalue weak units.

**Priority**: High (Phase 1.2 - needed for realistic evaluation)

---

### Fixed RF Range

**Current State**: 10-hex RF zone for all units

**Gap**: Not using actual weapon ranges from `VehicleType`

**Actual ASC Integration Needed**:
```cpp
bool isInReactionFireZone(const UnitSnapshot& unit, const GameStateSnapshot& snapshot) {
    for (const auto& enemy : snapshot.units) {
        if (enemy.owner == unit.owner) continue;
        
        // Get actual weapon range
        int rfRange = enemy.type->getMaxWeaponRange();
        int distance = hexDistance(unit.getPosition(), enemy.getPosition());
        
        if (distance <= rfRange) {
            return true;
        }
    }
    return false;
}
```

**Impact**: RF evaluation is inaccurate. Long-range units treated same as short-range.

**Priority**: High (Phase 1.2)

---

### No Terrain Bonuses

**Current State**: `getTerrainBonus()` returns 0.0

**Gap**: Not using `TerrainType->defenseBonus` or similar

**Actual ASC Integration Needed**:
```cpp
float getTerrainBonus(const UnitSnapshot& unit, const GameStateSnapshot& snapshot) {
    const auto* field = snapshot.getTerrainAt(unit.getPosition());
    if (!field || !field->terrain) return 0.0f;
    
    // Use actual terrain defensive bonus
    float defensiveBonus = field->terrain->getDefensiveBonus(unit.type);
    
    // Normalize to 0.0-1.0 range
    return std::min(defensiveBonus / 100.0f, 1.0f);
}
```

**Impact**: Position evaluation ignores tactical terrain value (forests, buildings, etc.)

**Priority**: Medium (Phase 1.3 - improves tactical awareness)

---

### No Flanking Detection

**Current State**: No detection of flanking or encirclement

**Gap**: Advanced tactical patterns not evaluated

**Needed**:
```cpp
// Detect if unit is surrounded by enemies
bool isEncircled(const UnitSnapshot& unit, const GameStateSnapshot& snapshot) {
    int enemyNeighbors = countNearbyEnemies(unit, snapshot, 1);
    int totalNeighbors = 6;  // Hex grid
    return enemyNeighbors >= 4;  // 4+ of 6 neighbors are enemies
}

// Detect flanking opportunities
bool canFlank(const UnitSnapshot& unit, const UnitSnapshot& target, ...) {
    // Check if attacking from rear/side
    int relativeDirection = calculateRelativeDirection(unit, target);
    return relativeDirection > 90;  // Side or rear attack
}
```

**Impact**: Misses advanced tactical opportunities

**Priority**: Low-Medium (Phase 2 - nice-to-have heuristic)

---

### No Objective-Based Scoring

**Current State**: Only combat-focused evaluation (kill enemy units)

**Gap**: No scoring for strategic objectives:
- Control points / sectors
- Resource nodes
- Victory conditions (king of the hill, etc.)

**Needed**:
```cpp
// Add to EvaluationResult
float objectiveScore{0.0f};

// Evaluate control of key map locations
float evaluateObjectives(const GameStateSnapshot& snapshot, PlayerID player) {
    float score = 0.0f;
    
    for (auto& objective : snapshot.objectives) {
        if (objective.controlledBy == player) {
            score += objective.value;
        } else if (objective.controlledBy != NEUTRAL) {
            score -= objective.value;
        }
    }
    
    return normalizeScore(score, maxObjectiveValue);
}
```

**Impact**: AI only fights, doesn't pursue map objectives

**Priority**: High (Phase 3 - needed for strategic play)

---

## Phase 1: Core MCTS Engine (Future)

### To Be Specified

**Areas needing design decisions:**

1. **Exploration Constant (C)**: What UCB1 constant to use?
   - Standard: C = √2
   - Needs tuning for ASC tactics
   - May need dynamic adjustment

2. **Rollout Depth**: How many moves to simulate?
   - Shallow: 5-10 moves (faster, less accurate)
   - Deep: 20-30 moves (slower, more accurate)
   - Adaptive depth based on game phase?

3. **Time Budget**: How long to think per turn?
   - Tactical: 1-5 seconds (fast response)
   - Strategic: 10-30 seconds (deeper planning)
   - Anytime algorithm (return best so far when time out)?

4. **Tree Pruning**: When to prune unpromising branches?
   - Visit threshold: Prune if visited < N times?
   - Score threshold: Prune if UCB1 value too low?
   - Memory limit: Prune oldest/worst nodes?

**Priority**: High (Phase 1.1 implementation)

---

## Post-MVP Integration Priorities

### High Priority (Phase 1-2)
1. ✅ **Actual unit values** - Use VehicleType->productionCost
2. ✅ **Actual weapon ranges** - Use VehicleType weapon stats
3. **Full pathfinding** - A* with movement budget
4. **Service actions** - Repair, refuel, supply
5. **Objective scoring** - Map control, victory conditions

### Medium Priority (Phase 2-3)
1. **Actual combat formulas** - Terrain modifiers, armor, experience
2. **Actual RF rules** - Hit chances, weapon selection
3. **Terrain bonuses** - Defensive positions, cover
4. **Building integration** - Capture, production, tech
5. **Ammo tracking** - Per-weapon ammo counts

### Low Priority (Phase 3+)
1. **Cargo system** - Load/unload transports
2. **Special abilities** - Minelaying, healing, etc.
3. **Flanking detection** - Advanced tactical awareness
4. **Fog of War** - Visibility, scouting, intelligence
5. **Research system** - Tech tree, upgrades

---

## Integration Strategy

**Two-Phase Approach:**

**Phase A: MVP Architecture** (Current)
- Clean interfaces (IActionExecutor, ITacticalEvaluator)
- Simplified heuristics (good enough for testing)
- Focus: Prove MCTS architecture works

**Phase B: Full ASC Integration** (Post-MVP)
- Replace simplified logic with actual ASC mechanics
- Incrementally improve evaluation accuracy
- Focus: Production-quality AI

**Validation**: Compare MCTS AI performance before/after integration
- Should see steady improvement as we add real mechanics
- If performance degrades, indicates architectural issues

---

## How to Use This Document

**When implementing new features:**
1. Check if there's a specification gap
2. Decide: MVP simplification or full integration?
3. If MVP: Document the gap here
4. If full: Update this document as "Resolved"

**When improving existing code:**
1. Find relevant gap in this document
2. Implement actual ASC integration
3. Update tests to verify correctness
4. Mark gap as "✅ Resolved"

**When planning phases:**
- Review gaps by priority
- Schedule high-priority integrations first
- Track progress in STATUS.md

---

## Summary Statistics

**Total Specification Gaps**: 15

**By Priority:**
- High: 5 gaps
- Medium: 6 gaps
- Low: 4 gaps

**By Phase:**
- Phase 0.1: 0 gaps (complete)
- Phase 0.2: 5 gaps (combat, pathfinding, actions)
- Phase 0.3: 5 gaps (evaluation heuristics)
- Phase 1+: 5 gaps (TBD - engine parameters)

**MVP Architecture Complete**: 30%  
**Full ASC Integration**: 0% (post-MVP)

---

**Note**: This is a living document. Update as gaps are discovered or resolved.
