# MCTS AI for ASC - Project Charter

**Date**: 2025-11-06  
**Status**: Active Development  
**Version**: 1.0  

---

## 1. Problem Statement

ASC's existing rule-based AI lacks strategic depth and tactical sophistication:

**Core Issues:**
- No multi-turn planning (cannot execute "research → build → produce" strategies)
- No fog-of-war memory (forgets enemy positions immediately)
- No reaction fire awareness (moves units into certain death)
- No group coordination (units act independently, not as combined-arms forces)
- No resource management (spends immediately, cannot save for strategic buildings)

**Impact:** AI is predictable and easily defeated by experienced players, limiting single-player appeal.

---

## 2. Project Goal

Create a **hierarchical MCTS-based AI** that:
- Executes coherent multi-turn strategies (research, infrastructure, production chains)
- Coordinates unit groups for combined-arms tactics
- Remembers and leverages fog-of-war information (enemy positions, mine locations)
- Maintains acceptable performance (<10 seconds per turn on typical maps)

**Target:** AI that challenges experienced human players and defeats legacy rule-based AI 100% without losing units.

---

## 3. Success Criteria

### Must-Have (v1.0)
- [x] Hierarchical architecture (strategic + tactical layers)
- [x] Persistent strategy memory (goals, research queue, fog-of-war tracking)
- [x] Basic tactical competence (avoids reaction fire, mines, poor terrain)
- [x] Beats legacy AI 100% win rate
- [x] Extendable agent architecture
- [ ] Turn time <10 seconds on 128x128 maps with 50 units

### Should-Have
- [x] Research tree planning
- [x] Base building strategy
- [x] Fog-of-war prediction
- [x] 60%+ scenarios: no unit losses
- [ ] Parallelization (multi-threaded tactical searches)

### Nice-to-Have (Future)
- [x] Neural network evaluation (AlphaZero-style)
- [x] Dynamic difficulty (via time allocation)
- [ ] Opponent modeling
- [ ] Post-game analysis

---

## 4. Scope

### In Scope
- **Modes**: Single-player vs AI, AI vs AI testing
- **Domains**: Tactical combat, reaction fire, logistics, strategic planning, base building, research
- **Maps**: 128x128 hexes, 50 units (stretch: 256x256, 100 units)
- **Features**: Fog-of-war, mines, height levels, terrain complexity
- **Target**: ASC 2.6 stable

### Out of Scope (v1.0)
- Neural network training pipelines
- Online learning between games
- Multiplayer/PBEM support
- Diplomacy/alliance logic
- Scenario-specific scripting
- Advanced belief state tracking (use simplified determinization)

---

## 5. Technical Approach

**Architecture:** See [docs/hierarchical_state_design.md](docs/hierarchical_state_design.md)
- Strategic layer: Group/sector level MCTS (100-200 actions)
- Tactical layer: Unit-level MCTS per group (30-50 actions)
- Persistent memory: StrategyMemory across turns

**Implementation:** See [docs/implementation_roadmap.md](docs/implementation_roadmap.md)
- Phases 0-6: Foundation → Tactical → Strategic → Memory → Optimization
- Total estimate: 16-18 weeks focused development

**Integration:** See [docs/code_structure.md](docs/code_structure.md)
- Wrapper around ASC GameMap/Player/Vehicle
- Coexist with legacy AI (player-selectable)
- Reuse ASC Action system

---

## 6. Key Decisions

### Decided
- **State representation**: Lightweight snapshots (not full GameMap clones)
- **Integration**: Coexist with legacy AI, not replace
- **Dependencies**: Pure ASC (no external MCTS libraries)
- **Testing**: AI vs AI tournaments + custom scenarios
- **Build system**: Automake (ASC native)

### TBD
- Grouping strategy: Spatial vs type-based vs heuristic (decide post-Phase 2)
- Strategic/tactical budget split: Fixed vs adaptive (tune in Phase 6)
- Parallelization approach: Thread-per-group vs work-stealing (Phase 6)

---

## 7. Timeline & Resources

**Team:** Solo development with LLM assistance  
**Expertise:** ASC gameplay knowledge available  

**Phases:** See [docs/implementation_roadmap.md](docs/implementation_roadmap.md) for details
- Phase 0: Infrastructure (2-3 weeks)
- Phase 1: Core MCTS (3-4 weeks)
- Phase 2: Tactical Domain (4-5 weeks)
- Phase 3: Strategic Layer (3-4 weeks)
- Phase 4: Memory & Coordination (3-4 weeks)
- Phase 5: Optimization (2-3 weeks)

**Estimated Total:** 4-5 months (part-time) or 2-3 months (full-time)

---

## 8. Current Status

**Active Phase:** [See STATUS.md]  
**Completed:** [Checklist in STATUS.md]  
**Next Milestone:** [See STATUS.md]

---

## 9. Risk Management

**Top Risks:** See [docs/implementation_roadmap.md](docs/implementation_roadmap.md) Section "Technische Herausforderungen"

1. **State cloning performance** → Mitigated via lightweight snapshots
2. **Hierarchical coordination complexity** → Start simple, add coordination incrementally  
3. **Scope creep** → Strict phase gates, ship after Phase 3 if functional
4. **ASC integration difficulty** → Wrapper layer, reuse existing Action system
5. **Performance insufficient** → Profile, parallelize, prune actions

---

## 10. Documentation Structure

- **[PROJECT_CHARTER.md](PROJECT_CHARTER.md)** (this file) - Project overview, goals, decisions
- **[STATUS.md](STATUS.md)** - Current progress, active work, next steps
- **[docs/game_description.md](docs/game_description.md)** - ASC mechanics, requirements
- **[docs/hierarchical_state_design.md](docs/hierarchical_state_design.md)** - Architecture design
- **[docs/implementation_roadmap.md](docs/implementation_roadmap.md)** - Phase plan, risks, deliverables
- **[docs/code_structure.md](docs/code_structure.md)** - ASC codebase analysis

---

## Approval

- [x] **Charter approved**: 2025-11-06
- [x] **Architecture finalized**: docs/hierarchical_state_design.md
- [x] **Roadmap defined**: docs/implementation_roadmap.md

**Next Review:** After Phase 2 completion (assess if architecture works in practice)
