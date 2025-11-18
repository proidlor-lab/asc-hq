# Phase 2.1 Architecture Clarifications

**Date**: 2025-11-12 09:28 UTC  
**Status**: ✅ ALL QUESTIONS RESOLVED - Ready for Week 1 Implementation

---

## ✅ Resolved Questions (Ready for Implementation)

### Q1: Service Action Agent Design
**Resolution**: Add **6th MVP agent** - `ServiceUtilityAgent`
- Evaluates repair/refuel/supply actions based on unit need urgency
- Weight: 2.5 (HIGH for service units, LOW for combat units)
- **Unit Role Types** (to be defined in Week 1):
  - **Service-Primary**: Only service actions, no combat
  - **Combat-Primary**: Only combat actions
  - **Hybrid**: Context-dependent

### Q2: Reaction Fire Zone Computation
**Resolution**: Global cache per turn with intelligent caching rules
- **Implementation**: `ReactionFireDetector` utility class (Week 1)
- **Caching Strategy**:
  - Standard units: Re-compute each turn
  - Static units (turrets): Cache until destroyed
  - Triggered RF: Remove from cache (threat gone for that turn)
- **Storage**: `std::map<MapCoordinate, std::vector<RFThreat>>` in `AgentContext`
- **Performance Target**: <10ms per turn

### Q3: Action Pruning Configuration
**Resolution**: Configurable via properties with reasonable defaults
```cpp
struct MCTSConfig {
    int maxActionsExpansion = 20;    // Full tree expansion
    int maxActionsRollout = 5;        // Fast rollout
    float vetoThreshold = -0.9f;      // Hard veto cutoff
    float pruneThreshold = -0.5f;     // Soft prune cutoff
};
```

### Q4: Strategic Layer MVP Scope
**Resolution**: 3 agents - 1 real + 2 stubs
1. **GlobalGoalAgent** (REAL) - Static goal: "Capture enemy HQ"
   - Uses world map information to locate HQ
   - Issues ATTACK orders toward HQ
2. **InfrastructureAgent** (STUB) - Dummy, no implementation
3. **ResearchAgent** (STUB) - Dummy, no implementation

**Rationale**: Focus on combat testing, defer economy/research to Phase 3

### Q5: Unit Planning Order
**Resolution**: Static priority-based order
1. **Long-range attacks** (range > 10) - execute first
2. **Close-range attacks** (range ≤ 10) - execute second
3. **Service actions** (repair, refuel, supply) - execute last

**Critical Rule**: Units with service actions available should **NOT** fight

**Rationale**: Long-range units soften targets → close-range finish → service cleans up

### Q6: Agent Weight Configuration
**Resolution**: `AgentWeightConfig` class (Week 1)
- Loads profiles from INI file
- Reuses ASC's existing config infrastructure
- Default profile: "Balanced"

### Q7: GameMap Propagation
**Resolution**: Read-only access for all agents
- Pass pointer to all agents (cheap, even if only few use it)
- **Current Usage**: PathfindingAdapter
- **Future Usage**: TerrainAdvantageAgent, FormationAgent
- **MVP**: Most agents won't need it (RF zones pre-computed)

### Q8: Strategic Order Completion Criteria
**Resolution**: Two completion triggers
1. **No more actions available** (all units exhausted)
2. **OR no more positive actions** (all actions score ≤ 0)

**Rationale**: Stop planning when nothing useful left to do

---

## ✅ Final Resolutions (2025-11-12)

### RQ1: Unit Role Type Detection ✅ RESOLVED
**Decision**: Check VehicleType capabilities dynamically (Option B approved)

**Implementation**:
```cpp
UnitRole detectRole(const VehicleType* type) {
    if (type->hasRepairAbility() || type->hasRefuelAbility() || type->hasSupplyAbility()) {
        return UnitRole::SERVICE_PRIMARY;
    }
    if (type->hasWeapons()) {
        return UnitRole::COMBAT_PRIMARY;
    }
    return UnitRole::HYBRID;
}
```

**Rationale**: Dynamic detection is more flexible than hard-coded lists

---

### RQ2: ReactionFireDetector Implementation Details ✅ RESOLVED
**Decisions**:
1. **Static Detection**: `vehicleType->Movement < 2` (turrets, fixed defenses)
   - Changed from Movement == 0 to Movement < 2 per user input
2. **Triggered RF**: Don't track for MVP (re-compute each turn, conservative approach)
3. **Damage Calculation**: Use **REAL combat formulas** via interface
   - **Critical Change**: No simplified estimates - use actual ASC combat system
   - **Implementation**: Create `ICombatCalculator` interface adapter

**ICombatCalculator Interface**:
```cpp
class ICombatCalculator {
public:
    virtual ~ICombatCalculator() = default;
    
    virtual float calculateExpectedDamage(
        const UnitSnapshot& attacker,
        const UnitSnapshot& target,
        const MapCoordinate& attackFrom
    ) const = 0;
    
    // If not directly accessible, implement adapter to ASC combat system
};
```

**Rationale**: Accurate threat assessment is critical for RF avoidance decisions

---

### RQ3: INI File Location & Format ✅ RESOLVED
**Decisions**:
- **Location**: `source/ai/mcts/mcts_agents.ini` (WITHIN MCTS module, not data/)
- **Parser**: Reuse ASC's `SimpleIniFile` class (existing infrastructure)
- **Profile Names**: Same as AI profiles (Balanced, Aggressive, Defensive, Fast, Deep)

**Example Format**:
```ini
[AgentWeights.Balanced]
ReactionFire=3.0
Aggressiveness=1.5
TargetPriority=1.2
Material=2.0
ServiceUtility=2.5

[AgentWeights.Aggressive]
ReactionFire=1.5
Aggressiveness=2.5
TargetPriority=1.5
Material=2.0
ServiceUtility=1.0
```

---

### RQ4: Service Action Ability Detection ✅ RESOLVED
**Decision**: Iterate units within service range

**Implementation** (MVP):
- Query all friendly units within service range (from VehicleType)
- Score based on urgency: `(1.0 - HP%) × unitValue`
- Cache "service target candidates" in AgentContext

**TODO**: Optimize with spatial indexing or pre-computed neediest units list (Phase 2.2+)

---

### RQ5: Unit Planning Priority Edge Cases ✅ RESOLVED
**Decision**: Simple categorization for MVP

**Implementation**:
- **Hybrid units**: If has service ability → service category (primary role)
- **Multiple weapons**: Use maximum weapon range for categorization
- **Planning order**: Batch by category (all long-range → all close-range → all service)

**TODO**: Refine hybrid unit handling based on tactical context (Phase 2.2+)
- Example: Allow service unit to use weapon if no service targets available

---

### RQ6: AgentContext Memory Footprint ✅ DEFERRED
**Decision**: Not a critical concern, profile during Week 2-3

**Expected Size**: <10 KB (RF zones ~3 KB, pointers only)

**Action**: Monitor during profiling, optimize if necessary

**Rationale**: Size is theoretical concern, empirical data will guide optimization

---

### RQ7: Strategic Order Priority Resolution ✅ DEFERRED
**Decision**: Address in Phase 3 (strategic planning)

**Phase 2.1 MVP**: Simple order: "ATTACK toward HQ" (no override logic)

**Future**: Add tactical situation assessment, order modification based on unit status

**Rationale**: MVP focuses on combat execution, strategic complexity comes later

---

## 🟢 Low Priority (Monitor & Profile)

### RQ8: Action Generation Performance
- Profile in Week 2-3
- Optimize if >10ms target exceeded

### RQ9: MCTS Tree Memory Management
- AgentScore is lightweight (~24 bytes)
- Monitor during integration testing

### RQ10: Multi-Player Turn Order
- ASC's AI runs per player (should be isolated)
- Test in Phase 3

---

## Updated Phase 2.1 Scope

### MVP Agent Set (6 agents)
1. **LegalMoveAgent** (veto)
2. **ReactionFireAgent** (scoring)
3. **AggressivenessAgent** (scoring)
4. **TargetPriorityAgent** (scoring)
5. **MaterialAgent** (state evaluation)
6. **ServiceUtilityAgent** (service action evaluation) ⭐ **NEW**

### Week 1 Deliverables (Updated)
- Agent interfaces (IAgent, IVetoAgent, IUtilityAgent)
- Communication domain objects (StrategicOrder, TacticalStatusReport, etc.)
- AgentContext and AgentScore structures
- UtilityAggregator (score aggregation logic)
- **ReactionFireDetector** utility class (RF zone caching) ⭐
- **AgentWeightConfig** class (INI profile loading) ⭐
- **MCTSConfig** structure (pruning parameters) ⭐
- **Unit Role Type** enum and detection logic ⭐

### Success Criteria (Updated)
- [ ] All **6 MVP agents** implemented and tested
- [ ] RF zone caching works (static units, triggered RF)
- [ ] Unit planning order correct (long-range → close-range → service)
- [ ] Service units don't engage in combat
- [ ] Order completion criteria working (no actions OR no positive actions)

---

## Implementation Readiness

### ✅ ALL QUESTIONS RESOLVED (2025-11-12)

**Critical Decisions Finalized**:
- ✅ RQ1: Unit role type detection (dynamic capability checking)
- ✅ RQ2: ReactionFireDetector (Movement < 2, real combat via ICombatCalculator)
- ✅ RQ3: INI file (`source/ai/mcts/mcts_agents.ini`, reuse ASC parser)
- ✅ RQ4: Service action detection (units in range, TODO for optimization)
- ✅ RQ5: Planning edge cases (simple categorization, TODO for refinement)
- ✅ RQ6: AgentContext size (profile during implementation)
- ✅ RQ7: Strategic order conflicts (defer to Phase 3)

**Key Technical Requirements**:
1. **ICombatCalculator Interface**: Adapter for ASC's real combat formulas (Week 1)
2. **Unit Role Detection**: Dynamic checking of repair/refuel/supply abilities
3. **Static Units**: Detection via Movement < 2 (not Movement == 0)
4. **INI Location**: Within MCTS module, not in data/ directory
5. **TODOs Documented**: Service optimization, hybrid unit refinement (Phase 2.2+)

### 📊 Success Metrics
- **Week 1 Completion**: All infrastructure classes implemented (includes ICombatCalculator)
- **Week 3 Completion**: All 6 agents functional
- **Week 6 Completion**: End-to-end combat test passing

---

## NO BLOCKING ISSUES - Ready for Immediate Implementation

**Next Action**: Begin Week 1 implementation
1. Implement agent interfaces (IAgent, IVetoAgent, IUtilityAgent)
2. Create **ICombatCalculator** interface for real damage calculations
3. Implement ReactionFireDetector with Movement < 2 detection
4. Create AgentWeightConfig loading from `source/ai/mcts/mcts_agents.ini`
5. Implement unit role detection with dynamic capability checking

**Document Updated**: 2025-11-12 09:28 UTC  
**Status**: ✅ FINAL - All architectural questions resolved, implementation can proceed
