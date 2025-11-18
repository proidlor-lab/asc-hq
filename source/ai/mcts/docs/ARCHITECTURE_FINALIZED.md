# Architecture Finalization - Complete ✅

**Date**: 2025-11-11 22:38 UTC  
**Status**: ✅ FINALIZED - Ready for Phase 2.1 Implementation  
**Decision**: Pragmatic MVP approach with clear migration path

---

## Executive Summary

After detailed analysis and discussion, the MCTS AI architecture is **finalized** as a **2-layer MVP** for Phase 2.1, with a clear path to add a **Tactical Coordination Layer** in Phase 2.2/3.0 when requirements are better understood.

**Key Decision**: Start simple, validate core MCTS + agents integration, then add coordination complexity.

---

## Finalized Architecture

### Phase 2.1 MVP: 2-Layer System

```
┌─────────────────────────────────────────────────┐
│ Strategic Layer (Agent-Based)                   │
│ - Infrastructure building                       │
│ - Research priorities                           │
│ - Resource allocation                           │
│ - Issue orders: "Attack sector NE"              │
└────────────────┬────────────────────────────────┘
                 │ StrategicOrder
                 ↓
┌─────────────────────────────────────────────────┐
│ Combat Group Layer (MCTS + Agents)              │
│ - Unit-level action planning                    │
│ - Sequential unit planning (MVP limitation)     │
│ - MUST use MCTS for all combat                  │
│ - Agents evaluate each action                   │
│ - Service actions (repair, refuel, supply)      │
└─────────────────────────────────────────────────┘
```

**MVP Characteristics**:
- ✅ Fast to implement (4-6 weeks)
- ✅ Validates MCTS + agent integration
- ✅ Gets basic combat working
- ⚠️ Units planned sequentially (may block each other)
- ⚠️ No multi-unit choreography
- ⚠️ No repair sequences

---

### Phase 2.2/3.0 Future: Add Tactical Coordination

```
┌─────────────────────────────────────────────────┐
│ Strategic Layer (Agent-Based)                   │
└────────────────┬────────────────────────────────┘
                 │ StrategicOrder
                 ↓
┌─────────────────────────────────────────────────┐
│ Tactical Coordination Layer (TBD: MCTS/Agents) │
│ - Multi-unit choreography                       │
│ - Repair sequences (retreat → repair → attack)  │
│ - Blocker coordination (move units aside)       │
│ - Movement point budgeting                      │
│ - Service action orchestration                  │
└────────────────┬────────────────────────────────┘
                 │ CoordinatedSequence
                 ↓
┌─────────────────────────────────────────────────┐
│ Combat Group Layer (MCTS + Agents)              │
│ - Execute coordinated actions                   │
│ - Validate sequences                            │
│ - Provide feedback if not viable                │
└─────────────────────────────────────────────────┘
```

**Why Deferred**:
1. Requirements not fully understood yet
2. Need to validate MCTS + agents first
3. Adds significant complexity
4. Can be added later without breaking architecture

---

## Key Architectural Decisions

### 1. MCTS + Agents Integration ✅

**Concept**: "Agents as Requirements"

- MCTS searches action space
- Each agent evaluates actions independently
- Agents may have conflicting goals (RF avoidance vs aggression)
- Scores aggregated via weighted sum
- Result: Multi-objective optimization

**Example**:
```
Action: Move tank into enemy range
  - ReactionFireAgent: -0.7 (high RF risk)
  - AggressivenessAgent: +0.9 (gets into firing position)
  - FormationAgent: -0.3 (breaks formation)
  - Weighted Sum: (-0.7×3.0) + (0.9×1.5) + (-0.3×1.0) = -0.75
  → Action likely rejected, MCTS explores alternatives
```

### 2. Layer Responsibilities ✅

**Strategic Layer** (Agent-Based):
- High-level planning (infrastructure, research, objectives)
- Decides **WHERE** to attack (sector-level)
- NO MCTS (not efficient for long horizons)

**Combat Group Layer** (MCTS + Agents):
- Unit-level execution
- Decides **HOW** to attack (unit movements, target selection)
- MUST use MCTS for all combat
- Sequential planning in MVP

**Tactical Coordination Layer** (Future):
- Multi-unit choreography
- Repair sequences, blocker resolution
- Added in Phase 2.2/3.0

### 3. Agent Weights ✅

**Configuration**: INI file with AI profiles

```ini
[AgentWeights.Aggressive]
ReactionFire=1.5    # Accept more risk
Aggression=2.5      # High attack priority

[AgentWeights.Defensive]
ReactionFire=3.5    # Avoid danger
Aggression=0.5      # Cautious
```

**Learning**: Deferred (manual tuning for now)

### 4. Update Frequencies ✅

- **Strategic Layer**: Every turn (or on trigger)
- **Combat Groups**: Every turn (MUST run MCTS)
- **Triggers for re-planning**: Group failure, threat increase, low success probability

### 5. Communication Protocol ✅

**Domain Objects Defined**:
- `StrategicOrder`: Strategic → Combat Groups
- `TacticalStatusReport`: Combat Groups → Strategic (feedback)
- `UnitStatusReport`: Units → Groups (escalation/complaints)
- `TacticalRequest`: Escalation for reinforcements, supply, retreat

---

## Motivation: Why This Architecture?

### Problem Statement (From User)

Complex multi-unit coordination example:
```
Turn Sequence:
1. Blocker unit moves aside (3 MP)
2. Damaged tank retreats to open space (4 MP)
3. Repair truck approaches damaged tank (5 MP)
4. Repair truck repairs tank (action)
5. Tank advances back to front (4 MP)
6. Tank attacks enemy (action)
7. Repair truck moves to next damaged unit (5 MP)
```

**This requires**:
- Sequencing (who moves first?)
- Coordination (units depend on each other)
- Resource allocation (MP budgets)

**MVP Cannot Handle This** - but that's OK:
- Validates MCTS + agents for basic combat
- Foundation for coordination layer
- Requirements become clearer through implementation

---

## Phase 2.1 Implementation Scope

### In Scope ✅

1. **Agent Interfaces**
   - `IAgent` base interface
   - `AgentScore` structure
   - `AgentContext` structure
   - `UtilityAggregator` class

2. **MVP Agent Set** (5 agents)
   - `LegalMoveAgent` (veto)
   - `ReactionFireAgent` (scoring)
   - `AggressivenessAgent` (scoring)
   - `TargetPriorityAgent` (scoring)
   - `MaterialAgent` (state evaluation)

3. **MCTS Integration**
   - Agent-based action evaluation
   - Score aggregation
   - Pruning (top-N actions)
   - Fast rollout policy

4. **Service Actions**
   - `ServiceAbility` (repair, refuel, supply)
   - Service-specific agents
   - Integration with ability registry

5. **Strategic Layer** (Basic)
   - Agent-based objective planning
   - Order generation
   - Status monitoring

6. **Communication Protocol**
   - Domain objects implemented
   - Bidirectional feedback

### Out of Scope ❌

- Tactical coordination layer
- Multi-unit choreography
- Blocker resolution
- Movement point budgeting
- Compound action sequences
- Advanced strategic agents (infrastructure, research)

### Known Limitations

- Units may block each other
- No repair sequences
- Infrastructure building not optimized
- Sequential planning (suboptimal)

**Acceptable for MVP** - gets core working, validates architecture.

---

## Success Criteria

### Functional
- [ ] All 5 MVP agents implemented
- [ ] MCTS uses agents for evaluation
- [ ] Score aggregation works correctly
- [ ] Service actions functional
- [ ] Strategic orders issued and executed
- [ ] Bidirectional communication works

### Performance
- [ ] Agent evaluation <0.1ms per action
- [ ] Action generation <10ms for 20 units
- [ ] MCTS branching factor <25
- [ ] Turn time <10s for 50 units

### Quality
- [ ] Win rate >60% vs legacy AI
- [ ] Agent decisions plausible (logged)
- [ ] No crashes, no illegal moves

---

## Timeline

**Phase 2.1**: 4-6 weeks
- Week 1: Core interfaces
- Week 2-3: MVP agent set
- Week 3-4: MCTS integration
- Week 4-5: Service actions
- Week 5-6: End-to-end integration

**Phase 2.2/3.0**: 3-4 weeks (future)
- Add tactical coordination layer
- Multi-unit choreography
- Blocker resolution
- Advanced service orchestration

---

## Migration Path: MVP → Full 3-Layer

When Phase 2.1 is complete and running:

**Step 1**: Profile and measure
- Identify blocking issues
- Measure coordination impact
- Collect real-world scenarios

**Step 2**: Design coordination API
```cpp
struct CoordinatedSequence {
    std::vector<UnitActionPair> actions;
    std::vector<Dependency> dependencies;
    int totalMovementPoints;
};
```

**Step 3**: Implement coordination agents
- `SequencingAgent`: Action order
- `BlockerAgent`: Resolve blockers
- `ResourceAgent`: MP allocation
- `ServiceRoutingAgent`: Repair routing

**Step 4**: Integrate with combat groups
- Groups receive coordinated sequences
- MCTS validates viability
- Feedback loop

---

## Key References

**Detailed Specification**:
- `docs/PHASE_2.1_ARCHITECTURE_SPEC.md` (Complete spec, all interfaces defined)

**Original Design Documents**:
- `docs/hierarchical_state_design.md` (Original 3-layer vision)
- `docs/implementation_roadmap.md` (Overall project roadmap)

**Status Tracking**:
- `STATUS.md` (Current progress)
- `docs/PROJECT_CHARTER.md` (Project goals)

---

## Approval

- [x] **Architecture finalized**: 2025-11-11
- [x] **MVP scope defined**: 2-layer system
- [x] **Migration path documented**: Clear path to 3-layer
- [x] **Ready for implementation**: Phase 2.1 can begin

**Next Milestone**: Phase 2.1 Core Interfaces (Week 1)

---

**Document Owner**: MCTS AI Development Team  
**Last Updated**: 2025-11-11 22:38 UTC  
**Status**: ✅ APPROVED - BEGIN PHASE 2.1 IMPLEMENTATION
