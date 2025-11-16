# Phase 2.1 Architecture Specification

**Date**: 2025-11-11  
**Status**: ✅ FINALIZED - Ready for Implementation  
**Purpose**: Detailed specification for utility-agent framework with MCTS integration  
**Architecture**: 3-layer intent (Strategic agent-only, Coordination MCTS for multi-unit, Execution/tactical replay). MVP currently runs MCTS at per-unit combat layer (misaligned; see alignment note below). Coordination layer is deferred to Phase 2.2/3.0 but should host MCTS.

---

## Executive Summary

Phase 2.1 implements a **multi-layer agent-MCTS hybrid architecture** where:
- **Strategic Layer**: Agent-based high-level planning (infrastructure, research, objectives)
- **Tactical Layer**: Coordination and monitoring (**needs clarification**)
- **Combat Group Layer**: MCTS + Agents for unit-level combat decisions

**Key Innovation**: Agents act as "requirements" - MCTS searches action space, agents evaluate actions from multiple (potentially conflicting) perspectives, scores are aggregated.

---

## 1. Layer Architecture

### Architecture Decision: MVP with corrected layer roles

**Intended target**:
- **Strategic Layer**: Agent-only aggregation (no MCTS) producing orders/goals.
- **Coordination Layer**: MCTS over multi-unit plans (ordering, resource budgets, deconfliction).
- **Execution Layer**: Replay plan per unit with safety/legality checks.

**Current MVP (misaligned)**:
- MCTS runs per-unit in the combat layer (sequential unit planning).
- Coordination layer is deferred; no multi-unit choreography.
- Strategic layer is stubbed (no real agent aggregation feeding plans).

**Adjustment needed**:
- Move MCTS up to the coordination layer when it lands (Phase 2.2).
- Keep strategic agent-only; feed Coordination MCTS with orders/context.
- Reduce tactical to plan replay + local safety checks.

**Motivation for Future Coordination Layer**:

Complex multi-unit sequences require coordination:
```
Example: Repair sequence
1. Blocker unit moves aside (3 MP)
2. Damaged tank retreats to open space (4 MP)
3. Repair truck approaches damaged tank (5 MP)
4. Repair truck repairs tank (action)
5. Tank advances back to front (4 MP)
6. Tank attacks enemy (action)
7. Repair truck moves to next damaged unit (5 MP)
```

This requires:
- Sequencing (who moves first?)
- Coordination (units depend on each other)
- Resource allocation (MP budget management)

MVP will handle simpler cases; coordination added later when requirements are clearer.

---

### 1.1 Strategic Layer (Agent-Based, NO MCTS)

**Responsibility**: High-level planning
- Infrastructure building (factories, depots, roads)
- Research prioritization
- Resource allocation
- Sector-level objectives (where to attack/defend)

**Decision Method**: Agent-based heuristics (NO MCTS)
- **Rationale**: Long time horizon, discrete decisions, MCTS not efficient here

**Agents**:
- `InfrastructureAgent`: Building placement and timing
- `ResearchAgent`: Tech tree navigation
- `MapAnalysisAgent`: Terrain and sector analysis
- `ObjectiveAgent`: Victory condition tracking
- `ResourceAgent`: Budget allocation

**Output**: Strategic orders for tactical layer

**Update Frequency**: 
- Every turn for cheap decisions (objective updates)
- On trigger for expensive decisions (major re-planning)

**Triggers for Re-planning**:
```cpp
bool shouldReplanStrategic() {
    if (anyGroupFailed()) return true;
    if (threatLevelIncreased(2)) return true;
    if (successProbability < 0.3f) return true;
    if (turnsSinceLastPlan > 5) return true;
    return false;
}
```

---

### 1.2 Coordination Layer (DEFERRED to Phase 2.2/3.0) — TARGET HOST FOR MCTS

**Status**: NOT IMPLEMENTED in Phase 2.1 MVP (currently empty; MCTS sits in combat layer and must be lifted here).

**Responsibility**: Multi-unit choreography and coordination
- Plan multi-step sequences (repair → attack)
- Coordinate movement (resolve blocker units)
- Allocate movement/action point budgets
- Orchestrate service actions and block/unblock sequences
- Produce ordered plans for execution

**Decision Method**: MCTS over multi-unit action sets with agent scoring/pruning.

**Phase 2.1 Workaround (current)**: Sequential per-unit planning (MCTS in combat layer) — causes self-blocking risk; to be replaced.

**Communication (target)**:
- Input: `StrategicOrder` (from strategic agent layer)
- Output: `CoordinationPlan` (ordered multi-unit actions) to execution
- Feedback: `TacticalStatusReport` upstream to strategic

---

### 1.3 Combat Group Layer (MCTS + Agents)

**Responsibility**: Unit-level combat execution

**Status**: ✅ PHASE 2.1 MVP SCOPE

**Decision Method**: MCTS with agent evaluation
- MCTS explores action combinations
- Agents evaluate each action
- Aggregate scores guide MCTS selection
- **MVP**: Sequential unit planning (no inter-unit coordination)

**Agents** (MVP Set):
- `LegalMoveAgent` (Veto)
- `ReactionFireAgent` (Scoring)
- `AggressivenessAgent` (Scoring)
- `TargetPriorityAgent` (Scoring)
- `MaterialAgent` (State Evaluation)

**Update Frequency**: Every turn (MUST run MCTS for combat)

**MVP Limitations**:
- Units planned sequentially (Unit 1, then Unit 2, then Unit 3)
- No lookahead for coordination ("will Unit 2 need this space?")
- May result in suboptimal blocking
- **Mitigation**: FormationAgent (future) can reduce blocking issues

**Future Enhancement** (Phase 2.2/3.0):
- Receives coordinated action sequences from Tactical Layer
- Executes multi-unit choreography
- Validates coordination plan viability

---

## 2. Communication Protocol

### 2.1 Strategic → Tactical: StrategicOrder

```cpp
struct StrategicOrder {
    OrderType type;           // ATTACK, DEFEND, HOLD, FALLBACK, etc.
    MapCoordinate objective;  // Target location/sector
    Priority priority;        // CRITICAL, HIGH, MEDIUM, LOW
    
    // Constraints
    int maxTurnsToComplete;
    int maxAcceptableLosses;  // Unit value threshold
    
    // Resources
    int fuelBudget;
    int ammoBudget;
    
    // Context
    std::string reason;
    std::vector<UnitGroupID> assignedGroups;
};

enum class OrderType {
    ATTACK, DEFEND, HOLD, FALLBACK, REGROUP,
    SCOUT, SUPPORT, BUILD_INFRASTRUCTURE
};

enum class Priority {
    CRITICAL = 4,
    HIGH = 3,
    MEDIUM = 2,
    LOW = 1
};
```

### 2.2 Tactical → Strategic: TacticalStatusReport

```cpp
struct TacticalStatusReport {
    UnitGroupID groupId;
    StrategicOrder currentOrder;
    
    // Progress
    ExecutionStatus status;
    float progressPercent;      // 0.0 - 1.0
    int turnsElapsed;
    int estimatedTurnsRemaining;
    
    // Situation
    int currentStrength;
    int strengthLost;
    float averageHealth;
    
    // Enemy
    int enemyStrengthEstimate;
    ThreatLevel threatLevel;
    
    // Resources
    float fuelLevel;
    float ammoLevel;
    
    // Requests (escalation/complaints)
    std::vector<TacticalRequest> requests;
    
    // Confidence
    float successProbability;
};

enum class ExecutionStatus {
    IN_PROGRESS,
    COMPLETED,
    FAILED,
    BLOCKED,
    AWAITING_SUPPORT
};

enum class ThreatLevel {
    NONE = 0, LOW = 1, MEDIUM = 2, HIGH = 3, CRITICAL = 4
};
```

### 2.3 Tactical Request (Escalation)

```cpp
struct TacticalRequest {
    RequestType type;
    Priority urgency;
    std::string description;
    
    union {
        ReinforcementRequest reinforcement;
        SupplyRequest supply;
        RetreatRequest retreat;
        SupportRequest support;
    };
};

enum class RequestType {
    REQUEST_REINFORCEMENT,
    REQUEST_SUPPLY,
    REQUEST_RETREAT,
    REQUEST_AIR_SUPPORT,
    REQUEST_ARTILLERY,
    REQUEST_ORDER_CHANGE,
    REPORT_OPPORTUNITY
};
```

### 2.4 Unit → Tactical: UnitStatusReport

```cpp
struct UnitStatusReport {
    UnitID unitId;
    MapCoordinate position;
    float health;
    float fuel;
    float ammo;
    
    bool isUnderFire;
    int enemiesInRange;
    bool hasLoS;
    
    std::vector<UnitIssue> issues;
};

struct UnitIssue {
    IssueType type;    // LOW_FUEL, LOW_AMMO, DAMAGED, BLOCKED, PINNED_BY_RF
    Severity severity; // INFO, WARNING, CRITICAL
    std::string details;
};
```

---

## 3. MCTS-Agent Integration

### 3.1 Core Concept: "Agents as Requirements"

**User Description**: "Agents are requirements. MCTS searches for actions. Each agent validates action with its own goal. Goals may conflict. Merge into ONE result optimized for maximum outcome."

**Implementation**: Multi-objective MCTS

```
MCTS Iteration:
  1. Select node (UCB1)
  2. Generate candidate actions (AbilityRegistry)
  3. FOR EACH action:
       - Each agent evaluates independently
       - Agents return scores [-1.0, +1.0]
       - Scores may conflict (RF=-0.7, Aggression=+0.9)
       - Aggregate scores (weighted sum)
  4. Create child nodes for high-scoring actions only
  5. Simulate with fast agent subset
  6. Backpropagate
```

### 3.2 Agent Interface

```cpp
class IAgent {
public:
    virtual ~IAgent() = default;
    
    virtual AgentScore evaluate(
        const Action& action,
        const AgentContext& context
    ) const = 0;
    
    virtual std::string getName() const = 0;
    virtual float getWeight() const = 0;
    virtual void setWeight(float weight) = 0;
    virtual AgentCategory getCategory() const = 0;
};

struct AgentScore {
    float utility;          // [-1.0, +1.0]
    float confidence;       // [0.0, 1.0]
    bool isVeto;           // Hard constraint violation
    std::string reasoning;  // For debugging
};

struct AgentContext {
    const GameStateSnapshot& state;
    const UnitSnapshot* actingUnit;
    PlayerID playerId;
    const GameMap* gameMap;  // Optional, for pathfinding
    const std::vector<MapCoordinate>& rfZones;
    bool isFastRollout;      // Simplified evaluation hint
    int searchDepth;
};

enum class AgentCategory {
    SURVIVAL,      // Prevent death (RF, damage)
    TACTICAL,      // Combat effectiveness
    STRATEGIC,     // Long-term positioning
    RESOURCE,      // Fuel/ammo conservation
    COORDINATION   // Group cohesion
};
```

### 3.3 Score Aggregation

**Confirmed**: Weighted sum with veto threshold

```cpp
float aggregateScores(const std::vector<IAgent*>& agents,
                     const Action& action,
                     const AgentContext& context) {
    float totalScore = 0.0f;
    float totalWeight = 0.0f;
    
    for (auto& agent : agents) {
        AgentScore score = agent->evaluate(action, context);
        
        // Hard veto
        if (score.isVeto) {
            return -1.0f;
        }
        
        totalScore += score.utility * agent->getWeight();
        totalWeight += agent->getWeight();
    }
    
    return totalScore / totalWeight;
}
```

**Advanced**: Category-based priority (future enhancement)
```cpp
// SURVIVAL agents can override TACTICAL agents
if (survivalScore < -0.5f) {
    return survivalScore;  // Survival takes priority
}
```

### 3.4 Integration Points

**During Expansion**:
```cpp
std::vector<Action> candidateActions = generator->generateAllActions(state);

std::vector<ScoredAction> scoredActions;
for (const auto& action : candidateActions) {
    float score = aggregateScores(agents, action, context);
    if (score > VETO_THRESHOLD) {
        scoredActions.push_back({action, score});
    }
}

// Sort and keep top N
std::sort(scoredActions, [](auto& a, auto& b) { return a.score > b.score; });
scoredActions.resize(MAX_ACTIONS_PER_NODE);

// Create child nodes
for (const auto& scored : scoredActions) {
    addChild(scored.action);
}
```

**During Rollout**:
```cpp
// Use reduced agent set for speed
std::vector<IAgent*> fastAgents = {rfAgent, aggressionAgent};

Action selectRolloutAction(const GameState& state) {
    auto actions = generator->generateActions(state);
    
    float bestScore = -999.0f;
    Action bestAction;
    
    for (const auto& action : actions) {
        float score = aggregateScores(fastAgents, action, fastContext);
        if (score > bestScore) {
            bestScore = score;
            bestAction = action;
        }
    }
    
    return bestAction;
}
```

**During Terminal Evaluation**:
```cpp
// Use full evaluation agent set
std::vector<IStateEvaluationAgent*> evalAgents = {
    materialAgent, positionAgent, threatAgent, formationAgent
};

float evaluateTerminalState(const GameState& state) {
    // Aggregate state evaluation scores
    return aggregateStateScores(evalAgents, state);
}
```

---

## 4. Agent Weight Configuration

**Confirmed**: Config-based with AI profile support

### 4.1 Configuration File

```ini
[AgentWeights.Default]
ReactionFire=3.0
Aggression=1.5
TargetPriority=1.2
Formation=1.0
FuelConservation=0.5

[AgentWeights.Aggressive]
ReactionFire=1.5    # Lower (accept more risk)
Aggression=2.5      # Higher (attack priority)
TargetPriority=1.5
Formation=0.5
FuelConservation=0.3

[AgentWeights.Defensive]
ReactionFire=3.5    # Higher (avoid danger)
Aggression=0.5      # Lower (cautious)
TargetPriority=1.0
Formation=2.0       # Higher (maintain cohesion)
FuelConservation=1.0
```

### 4.2 Loading Weights

```cpp
class AgentWeightConfig {
public:
    void loadProfile(const std::string& profileName);
    float getWeight(const std::string& agentName) const;
    
private:
    std::map<std::string, float> weights_;
};

// Usage
AgentWeightConfig config;
config.loadProfile("Aggressive");
rfAgent->setWeight(config.getWeight("ReactionFire"));
```

### 4.3 Dynamic Adjustment (Future)

**Deferred**: Learning-based weight adjustment

**Future Enhancement**: Strategic context adjusts weights
```cpp
// If strategic order is ATTACK
if (order.type == ATTACK) {
    aggressionAgent->setWeight(2.5);
} else if (order.type == DEFEND) {
    rfAgent->setWeight(3.5);
}
```

---

## 5. MVP Agent Set (Phase 2.1)

### 5.1 Veto Layer

**LegalMoveAgent**
- **Purpose**: Block illegal actions
- **Checks**: Out of bounds, blocked terrain, no movement points
- **Weight**: N/A (hard veto)
- **Implementation**: ~50 LOC

### 5.2 Scoring Layer

**ReactionFireAgent**
- **Purpose**: Avoid reaction fire zones
- **Score**: `-1.0 * (expectedDamage / unitMaxHP)`
- **Weight**: 3.0 (HIGH)
- **Implementation**: ~150 LOC (RF zone calculation)

**AggressivenessAgent**
- **Purpose**: Prefer attacks over waiting
- **Score**: `+1.0` for attacks, `0.0` for move, `-0.3` for wait
- **Weight**: 1.5 (MEDIUM, profile-adjustable)
- **Implementation**: ~80 LOC

**TargetPriorityAgent**
- **Purpose**: Prefer weak/valuable targets
- **Score**: `+1.0` for low-HP high-value, `-0.5` for strong targets
- **Weight**: 1.2 (MEDIUM)
- **Implementation**: ~100 LOC

### 5.3 State Evaluation Layer

**MaterialAgent**
- **Purpose**: Track unit value balance
- **Score**: `(ownValue - enemyValue) / totalValue`
- **Weight**: 2.0 (HIGH)
- **Implementation**: ~60 LOC

### 5.4 Service Action Layer

**ServiceUtilityAgent** ✅ ADDED (6th MVP agent)
- **Purpose**: Evaluate repair/refuel/supply actions based on unit need
- **Score**: `+1.0` for critical need (unit <20% HP/fuel), `0.0` for no need
- **Weight**: 2.5 (HIGH for service units, LOW for combat units)
- **Implementation**: ~120 LOC
- **Unit Role Types** (to be defined in Week 1):
  - **Service-Primary**: Repair trucks, supply vehicles (only service, no combat)
  - **Combat-Primary**: Tanks, infantry (only combat actions)
  - **Hybrid**: Some units can do both (context-dependent)

**Note**: Units with available service actions should **NOT** engage in combat (enforced by unit role type).

---

## 6. Architecture Decisions (Resolved)

### Critical Questions - RESOLVED ✅

**Q1: Tactical Layer Role** ✅ RESOLVED
- **Phase 2.1**: No tactical coordination layer (2-layer MVP)
- **Phase 2.2/3.0**: Add coordination layer for multi-unit choreography
- **Rationale**: Validate MCTS + agents first, add complexity later

**Q2: Layer Count** ✅ RESOLVED
- **Phase 2.1 MVP**: 2 layers (Strategic + Combat Groups)
- **Future**: 3 layers (Strategic + Tactical Coordination + Combat Groups)
- Tactical coordination deferred until requirements clearer

**Q3: Strategic Order Flow** ✅ RESOLVED
```
Phase 2.1 MVP:
Strategic: "Attack sector NE"
   ↓ (direct assignment)
Combat Groups: MCTS for unit actions (sequential)

Phase 2.2/3.0 Future:
Strategic: "Attack sector NE"
   ↓
Tactical Coordination: Plan multi-unit sequences
   ↓
Combat Groups: Execute coordinated actions
```

### Implementation Questions (2025-11-11 Update)

**Q4: Multi-Unit Coordination** ✅ RESOLVED
- Deferred to Phase 2.2/3.0 (confirmed)
- MVP: Sequential unit planning with static priority order (see Q5 below)
- Future: Compound actions or tactical-layer coordination

**Q5: Unit Planning Order** ✅ RESOLVED (2025-11-11)
- **Static Priority Order**:
  1. **Long-range attacks** (range > 10) - execute first
  2. **Close-range attacks** (range ≤ 10) - execute second
  3. **Service actions** (repair, refuel, supply) - execute last
- **Rule**: Units with service actions available should NOT fight
- **Rationale**: Long-range units soften targets, close-range units finish, service units clean up

**Q6: Performance Budget Split** ✅ RESOLVED
- **Phase 2.1**: Strategic 10%, Combat 90% (no tactical layer)
- **Phase 2.2/3.0**: Strategic 10%, Tactical 20%, Combat 70%
- Will measure empirically and adjust

**Q7: Service Action Agent Design** ✅ RESOLVED (2025-11-11)
- **Add 6th agent**: `ServiceUtilityAgent` to MVP agent set
- Evaluates service actions based on unit need urgency
- Unit role types will be defined (service-primary, combat-primary, hybrid)
- See Section 5.4 for details

**Q8: Reaction Fire Zone Computation** ✅ RESOLVED (2025-11-11)
- **Strategy**: Global cache per turn, pre-computed before action generation
- **Implementation**: `ReactionFireDetector` utility class (Week 1)
- **Caching Rules**:
  - Standard units: Re-compute each turn
  - Static units (turrets): Cache until unit destroyed
  - Triggered RF: Remove from cache (threat gone)
- **Storage**: `std::map<MapCoordinate, std::vector<RFThreat>>` in `AgentContext`
- **Performance Target**: <10ms per turn for RF zone computation

**Q9: Action Pruning Configuration** ✅ RESOLVED (2025-11-11)
- **Configurable via properties** (Week 1)
- **Reasonable Defaults**:
  ```cpp
  struct MCTSConfig {
      int maxActionsExpansion = 20;    // Full tree expansion
      int maxActionsRollout = 5;        // Fast rollout
      float vetoThreshold = -0.9f;      // Hard veto cutoff
      float pruneThreshold = -0.5f;     // Soft prune cutoff
  };
  ```
- Different values for expansion (quality) vs rollout (speed)

**Q10: Strategic Layer MVP Scope** ✅ RESOLVED (2025-11-11)
- **3 Agents for Phase 2.1**:
  1. **GlobalGoalAgent** (REAL) - Static goal: "Capture enemy HQ"
     - Uses world map information to locate HQ
     - Issues ATTACK orders toward HQ
  2. **InfrastructureAgent** (STUB) - Dummy, no implementation
  3. **ResearchAgent** (STUB) - Dummy, no implementation
- **Rationale**: Focus on combat testing, defer economy/research to later phases
- **Option B** (MapAnalysisAgent, ObjectiveAgent) deferred to Phase 3

**Q11: Agent Weight Configuration** ✅ RESOLVED (2025-11-11)
- **Implementation**: `AgentWeightConfig` class (Week 1)
- Loads profiles from INI file (reuse ASC's config system)
- Default profile: "Balanced" (all weights from Section 4.1)

**Q12: GameMap Propagation** ✅ RESOLVED (2025-11-11)
- **Read-only access** for all agents
- **Pass pointer to all agents** (cheap, even if only few use it)
- **Current Usage**:
  - PathfindingAdapter (already implemented)
  - Future: TerrainAdvantageAgent, FormationAgent
- **MVP agents**: Likely don't need GameMap (RF zones pre-computed)

**Q13: Strategic Order Completion Criteria** ✅ RESOLVED (2025-11-11)
- **Completion Triggers**:
  1. No more actions available (all units exhausted)
  2. OR no more positive actions available (all actions score ≤ 0)
- **Rationale**: Stop planning when nothing useful left to do
- **Status Reporting**: Combat groups report `ExecutionStatus::COMPLETED` when criteria met

---

## 7. Implementation Plan

### Phase 2.1 MVP Scope

**Core Goal**: MCTS + Agent integration for basic combat

**In Scope**:
- ✅ Agent interfaces and evaluation
- ✅ MCTS with agent scoring
- ✅ **6 MVP agents** (added ServiceUtilityAgent)
- ✅ Sequential unit planning (priority-based order)
- ✅ Service actions (repair, refuel, supply)
- ✅ Strategic layer (GlobalGoalAgent + 2 stubs)
- ✅ ReactionFireDetector utility (RF zone caching)
- ✅ AgentWeightConfig (INI-based profiles)
- ✅ MCTSConfig (configurable pruning parameters)
- ✅ Unit role type system (service/combat/hybrid)

**Out of Scope** (Deferred to Phase 2.2/3.0):
- ❌ Tactical coordination layer
- ❌ Multi-unit choreography
- ❌ Movement point budgeting
- ❌ Blocker coordination
- ❌ Compound action sequences

**Known Limitations**:
- Units may block each other (sequential planning)
- No repair sequences (repair truck just repairs, doesn't coordinate retreat)
- Infrastructure building not optimized (builders act independently)

### Phase 2.1 Implementation Plan (Updated 2025-11-11)

**Week 1**: Core interfaces + Infrastructure
- Agent interfaces (IAgent, IVetoAgent, IUtilityAgent)
- Communication domain objects (StrategicOrder, TacticalStatusReport, etc.)
- AgentContext and AgentScore structures
- UtilityAggregator (score aggregation logic)
- **ReactionFireDetector** utility class (RF zone caching, Movement < 2 for static)
- **ICombatCalculator** interface (adapter for ASC combat system, real damage formulas)
- **AgentWeightConfig** class (INI from `source/ai/mcts/mcts_agents.ini`)
- **MCTSConfig** structure (pruning parameters)
- **Unit Role Type** enum and detection logic (dynamic capability checking)

**Week 2-3**: MVP Agent Set
- Implement **6 agents**:
  1. LegalMoveAgent (veto)
  2. ReactionFireAgent (scoring)
  3. AggressivenessAgent (scoring)
  4. TargetPriorityAgent (scoring)
  5. MaterialAgent (state evaluation)
  6. **ServiceUtilityAgent** (service action evaluation)
- Unit tests for each agent
- Integration with action generator
- Unit role type assignments (service-primary, combat-primary, hybrid)

**Week 3-4**: MCTS Integration
- Modify MCTSSearch to use agents
- Expansion with agent scoring
- Rollout with fast agents
- Terminal eval with state agents

**Week 4-5**: Service Actions
- ServiceAbility implementation
- Service-specific agents
- Integration tests

**Week 5-6**: End-to-End Integration
- Combat group controller (sequential unit planning with priority order)
- Strategic planner (GlobalGoalAgent + stubs)
- Full turn execution test
- Order completion detection (no actions OR no positive actions)
- Communication protocol validation (StrategicOrder ↔ TacticalStatusReport)

---

## 8. Success Criteria

### Functional
- [ ] All **6 MVP agents** implemented and tested
- [ ] MCTS uses agents for action evaluation (expansion + rollout)
- [ ] Agent scores aggregate correctly (weighted sum + veto)
- [ ] Config-based weight loading works (INI profiles)
- [ ] Communication protocol functional (bidirectional)
- [ ] RF zone caching works (static units, triggered RF)
- [ ] Unit planning order correct (long-range → close-range → service)
- [ ] Service units don't engage in combat
- [ ] Order completion criteria working (no actions OR no positive actions)

### Performance
- [ ] Agent evaluation <0.1ms per action
- [ ] Action generation + scoring <10ms for 20 units
- [ ] MCTS branching factor <25 (pruning effective)
- [ ] Full turn time <10s for 50 units

### Quality
- [ ] Agent decisions are plausible (logged and reviewable)
- [ ] Win rate >60% vs legacy AI
- [ ] No crashes, no illegal moves

---

## 9. Remaining Open Questions & Design Decisions

### 🔴 Critical - Must Define in Week 1

**RQ1: Unit Role Type Detection** ✅ RESOLVED (2025-11-12)
- **Decision**: Check VehicleType capabilities dynamically (Option B)
- **Implementation**:
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
- **Rationale**: Dynamic detection is more flexible than hard-coded lists

**RQ2: ReactionFireDetector Implementation Details** ✅ RESOLVED (2025-11-12)
- **Static Unit Detection**: `vehicleType->Movement < 2` (turrets, fixed defenses)
- **Triggered RF Tracking**: Don't track for MVP (re-compute each turn, conservative approach)
- **RF Damage Calculation**: Use **real combat formulas** via interface
  - **Design**: `ICombatCalculator` interface to access ASC's combat system
  - **Rationale**: Accurate threat assessment critical for RF avoidance
  - **Implementation**: If not directly accessible, create adapter interface
  ```cpp
  class ICombatCalculator {
  public:
      virtual float calculateExpectedDamage(
          const UnitSnapshot& attacker,
          const UnitSnapshot& target,
          const MapCoordinate& attackFrom
      ) const = 0;
  };
  ```
- **Caching Strategy**: 
  - Standard units: Re-compute each turn
  - Static units (Movement < 2): Cache until unit destroyed
  - Triggered RF: Conservative (assume always active for MVP)

**RQ3: INI File Location & Format** ✅ RESOLVED (2025-11-12)
- **Location**: `source/ai/mcts/mcts_agents.ini` (within MCTS module)
- **Parser**: Reuse ASC's `SimpleIniFile` class (existing infrastructure)
- **Profile Names**: Same as AI profiles (Balanced, Aggressive, Defensive, Fast, Deep)
- **Example Format**:
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

### 🟡 Important - Clarify During Implementation

**RQ4: Service Action Ability Detection** ✅ RESOLVED (2025-11-12)
- **Decision**: Iterate units within service range
- **Implementation** (MVP):
  - Query all friendly units within service range (from VehicleType)
  - Score based on urgency: `(1.0 - HP%) × unitValue`
  - Cache "service target candidates" in AgentContext
- **TODO**: Optimize with spatial indexing or pre-computed neediest units list (Phase 2.2+)

**RQ5: Unit Planning Priority Edge Cases** ✅ RESOLVED (2025-11-12)
- **Decision**: Simple categorization for MVP
- **Implementation**:
  - **Hybrid units**: If has service ability → service category (primary role)
  - **Multiple weapons**: Use maximum weapon range for categorization
  - **Planning order**: Batch by category (all long-range → all close-range → all service)
- **TODO**: Refine hybrid unit handling based on tactical context (Phase 2.2+)
  - Example: Allow service unit to use weapon if no service targets available

**RQ6: AgentContext Memory Footprint** ✅ DEFERRED
- **Decision**: Profile during Week 2-3 implementation
- **Expected size**: <10 KB (RF zones ~3 KB, pointers only)
- **Action**: Monitor during profiling, optimize if necessary
- **Not critical**: Size is theoretical concern, empirical data will guide optimization

**RQ7: Strategic Order Priority Resolution** ✅ DEFERRED
- **Decision**: Address in Phase 3 (strategic planning)
- **Phase 2.1 MVP**: Simple order: "ATTACK toward HQ" (no override logic)
- **Future**: Add tactical situation assessment, order modification based on unit status
- **Not blocking**: MVP focuses on combat execution, strategic complexity comes later

---

### 🟢 Low Priority - Defer or Monitor

**RQ8: Action Generation Performance** 🟢 LOW
- **Question**: Is current action generation fast enough with 6 agents scoring?
- **Mitigation**: Profile in Week 2-3, optimize if >10ms target exceeded
- **Defer**: Wait for empirical data

**RQ9: MCTS Tree Memory Management** 🟢 LOW
- **Question**: Does adding 6 agent scores per node increase memory significantly?
- **Mitigation**: AgentScore is lightweight (~24 bytes), should be fine
- **Defer**: Monitor during integration testing

**RQ10: Multi-Player Turn Order** 🟢 LOW
- **Question**: Does sequential unit planning work correctly in multi-player?
- **Note**: ASC's AI runs per player, should be isolated
- **Defer**: Test in Phase 3 integration tests

---

### Design Decisions Summary

**Resolved Session 1 (2025-11-11)**:
- ✅ 6th agent (ServiceUtilityAgent) added to MVP
- ✅ Unit planning order (long-range → close-range → service)
- ✅ RF zone caching strategy
- ✅ Action pruning configuration
- ✅ Strategic layer scope (GlobalGoalAgent + stubs)
- ✅ Order completion criteria
- ✅ GameMap propagation approach

**Resolved Session 2 (2025-11-12)** - ALL CRITICAL QUESTIONS RESOLVED ✅:
- ✅ **RQ1**: Unit role type detection (dynamic capability checking)
- ✅ **RQ2**: ReactionFireDetector (Movement < 2, real combat formulas, ICombatCalculator interface)
- ✅ **RQ3**: INI file location (`source/ai/mcts/mcts_agents.ini`, reuse ASC parser)
- ✅ **RQ4**: Service action detection (units in range, TODO for optimization)
- ✅ **RQ5**: Planning edge cases (simple categorization, TODO for refinement)
- ✅ **RQ6**: AgentContext size (profile during implementation, not critical)
- ✅ **RQ7**: Strategic order conflicts (defer to Phase 3)

**Monitor During Implementation**:
- 🟢 RQ8-RQ10: Performance metrics, memory usage, multi-player (profile and measure)

**NO BLOCKING ISSUES REMAINING** - Ready for Week 1 implementation!

---

## 10. Migration Path to Full 3-Layer System

### Phase 2.1 → Phase 2.2 Transition

When ready to add tactical coordination:

**Step 1: Identify Coordination Needs**
- Profile combat scenarios
- Identify common blocking issues
- Measure impact of sequential planning

**Step 2: Design Coordination API**
```cpp
struct CoordinatedSequence {
    std::vector<UnitActionPair> actions;  // Ordered list
    std::vector<Dependency> dependencies; // "Unit A must move before Unit B"
    int totalMovementPoints;              // MP budget
};

class TacticalCoordinator {
    CoordinatedSequence planSequence(
        const std::vector<Unit*>& units,
        const StrategicOrder& order
    );
};
```

**Step 3: Implement Coordination Agents**
- `SequencingAgent`: Determines action order
- `BlockerAgent`: Identifies and resolves blockers
- `ResourceAgent`: Allocates MP budgets
- `ServiceRoutingAgent`: Plans repair truck routes

**Step 4: Integrate with Combat Groups**
- Combat groups receive coordinated sequences
- MCTS validates sequence viability
- Feedback if sequence not executable

**Estimated Effort**: 3-4 weeks (Phase 2.2 scope)

---

**Document Status**: ✅ FINAL (2025-11-12 09:28 UTC) - ALL Questions Resolved  
**Architecture**: 2-layer MVP with 6 agents, priority-based unit planning, RF with real combat formulas  
**Ready for Implementation**: Week 1 can begin immediately - NO BLOCKERS  
**Next Update**: After Week 1 completion (validate infrastructure, begin agent implementation)
