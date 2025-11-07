# MCTS AI Implementierungs-Roadmap

## Ziel: Schneller Durchstich (Tactical Combat MVP)

Diese Roadmap fokussiert sich auf einen **minimalen funktionalen Durchstich** der MCTS-Architektur mit Schwerpunkt auf der **taktischen Ebene mit Kampf**. Alle anderen Komponenten werden architektonisch vorbereitet, aber nur als Dummy-Implementierungen bereitgestellt.

---

## Phase 0: Infrastruktur & Game State Foundation
**Dauer**: 2-3 Wochen  
**Priorität**: CRITICAL (Blocker für alles weitere)

### 0.1 Game State Cloning & Snapshots ✅ COMPLETE

**Status**: ✅ **Implemented (2025-11-07)**

**Deliverables**:
- [x] `GameStateSnapshot` Klasse (domain/game_state_snapshot.h)
  - Kann relevante Teile von `GameMap` extrahieren (Units, Terrain, Ressourcen)
  - Fast cloning: 0.019ms per clone (50× better than target!)
  - Lightweight: 4.6 KB for 20 units (vs. target <20 KB)
- [x] `GameStateReader` Adapter (domain/game_state_reader.h/cpp)
  - Dependency Injection pattern (IGameStateReader interface)
  - Read-only wrapper around GameMap
  - No modifications to legacy code
- [x] C++23 Modernization
  - constexpr optimizations for compile-time evaluation
  - Three-way comparison operators (operator<=>)
  - fast_map compatibility layer (ready for std::flat_map)
  - Modern type safety (noexcept, constexpr)

**Technische Herausforderungen**:
- ASC's `GameMap` ist groß und komplex → selektives Kopieren nötig
- Pointer-Hierarchien müssen korrekt kopiert werden
- Shared Resources (z.B. `GameMap::terrainmap`) dürfen nicht dupliziert werden

**Success Criteria**: ✅ **ALL ACHIEVED**
- ✅ Unit-Tests: 100 clones in 3.3ms (150× better than target!)
- ✅ Snapshot-Size: 4.6 KB for 20 units (10× better than target!)
- ✅ All tests passing (see: snapshot_test)

---

### 0.2 Action Execution Interface (Woche 1-2)

**Ziel**: Einheitliches Interface zum Ausführen von Aktionen auf simuliertem State

**Deliverables**:
- [ ] `IActionExecutor` Interface
  - `executeMove(unit, targetPosition)`: Bewegt Unit
  - `executeAttack(attacker, defender)`: Führt Angriff aus
  - `undoAction()`: Rollback für Simulation (optional, falls nötig)
- [ ] `SimulationActionExecutor` Implementierung
  - Führt Aktionen auf `GameStateSnapshot` aus (nicht auf Original-Map)
  - Nutzt bestehende ASC `Command`-System wo möglich
  - Optimiert für Performance (keine GUI-Updates, kein Netzwerk)
- [ ] `RealGameActionExecutor` Implementierung
  - Führt Aktionen auf echter `GameMap` aus (für finale KI-Entscheidung)
  - Integration mit ASC's Action-Queue

**Technische Herausforderungen**:
- ASC's Command-System ist für echte Spiel-Ausführung designed, nicht für Simulationen
- Reaction Fire muss korrekt simuliert werden
- Fuel/Ammo-Konsum muss getrackt werden

**Success Criteria**:
- Unit-Tests: Move + Attack korrekt simuliert
- Performance: 1000 Aktionen/Sekunde auf Snapshot
- Korrektheit: Simulation-Ergebnis == Real-Game-Ergebnis

---

### 0.3 Basic Evaluation Function (Woche 2)

**Ziel**: Bewertung von Spielzuständen für MCTS

**Deliverables**:
- [ ] `ITacticalEvaluator` Interface
  - `evaluate(GameStateSnapshot) -> float`: Gibt Score zurück (-1 bis +1)
- [ ] `SimpleCombatEvaluator` Implementierung
  - Material-basierte Bewertung: Summe eigener Unit-Werte minus Gegner-Unit-Werte
  - HP-Gewichtung: Units mit niedriger HP zählen weniger
  - Position-Bonus: Units in vorteilhafter Position (Cover, Höhe) zählen mehr
- [ ] Heuristiken für Combat
  - Reaktion Fire vermeiden (Malus für gefährliche Moves)
  - Konzentration von Feuer (Bonus wenn mehrere Units einen Gegner angreifen)
  - Defensive Position (Bonus für Units in Cover/Gebäuden)

**Success Criteria**:
- Evaluation korreliert mit tatsächlichem Combat-Erfolg (Validierung durch Tests)
- Performance: <1ms pro Evaluation
- Plausibilität: Menschliche Spieler stimmen mit Evaluation überein (qualitativ)

---

## Phase 1: Tactical MCTS Core (Combat-Fokus)
**Dauer**: 3-4 Wochen  
**Priorität**: HIGH (Der eigentliche Durchstich)

---

### Architektur-Konzept: Modulare Utility-Based Agents

**Vision**: Taktisches Verhalten wird durch **kombinierbare Agent-Module** definiert, nicht durch monolithische Heuristiken. Jeder Agent bewertet Aktionen aus einer spezifischen Perspektive (RF-Vermeidung, Angriffslust, Support-Rollen, Rückzug, etc.) und gibt einen Utility-Score zurück. Die gewichtete Kombination aller Agents bestimmt die finale Action-Bewertung.

**Drei-Schicht-Architektur**:

1. **Veto-Layer (Action Feasibility)**: Agents können Aktionen komplett verwerfen
2. **Scoring-Layer (Action Utility)**: Agents bewerten erlaubte Aktionen mit Scores
3. **Evaluation-Layer (State Evaluation)**: Agents bewerten resultierende States nach Action-Ausführung

Diese Architektur wird **progressiv** implementiert: MVP startet mit wenigen Agents, weitere werden iterativ hinzugefügt.

---

### 1.1 MCTS-Kern-Algorithmus (Woche 3)

**Ziel**: Funktionierender MCTS für einzelne Unit-Gruppe

**Deliverables**:
- [ ] `MCTSNode` Klasse
  - Speichert State, Parent, Children, Statistiken (Visits, Wins)
  - UCB1-Formel für Selection
- [ ] `MCTS` Algorithmus
  - Selection: Wähle vielversprechendsten Node (UCB1)
  - Expansion: Generiere neue Child-Nodes (mögliche Aktionen)
  - Simulation: Random Rollout bis Terminal-State oder Depth-Limit
  - Backpropagation: Update Statistiken
- [ ] Iteration-Budget: 500-2000 Iterationen
- [ ] Best-Action-Selection: Wähle meistbesuchten Child

**Success Criteria**:
- MCTS findet offensichtlich beste Züge in einfachen Szenarien
- Performance: 500 Iterationen in <1 Sekunde
- Code-Qualität: Gut testbar, modular

---

### 1.2 Utility-Agent Framework (Woche 3-4)

**Ziel**: Modulares System für taktische Entscheidungs-Agents

#### 1.2.1 Core Interfaces

**Deliverables**:
- [ ] **`IActionVetoAgent`** Interface (Veto-Layer)
  ```
  bool canExecute(Action, GameStateSnapshot, Context) -> bool
  string getVetoReason() -> string  // für Debugging
  ```
  - Verwirft illegale oder extrem schädliche Aktionen
  - Beispiel: "SuicidePreventionAgent", "LegalMoveAgent"

- [ ] **`IActionUtilityAgent`** Interface (Scoring-Layer)
  ```
  float evaluateAction(Action, GameStateSnapshot, Context) -> float [-1.0, +1.0]
  float getWeight() -> float  // Gewichtung dieses Agents
  ```
  - Bewertet erlaubte Aktionen aus spezifischer Perspektive
  - Beispiel: "ReactionFireAgent", "AggressivenessAgent", "SupportAgent"

- [ ] **`IStateEvaluationAgent`** Interface (Evaluation-Layer)
  ```
  float evaluateState(GameStateSnapshot, Context) -> float [-1.0, +1.0]
  float getWeight() -> float
  ```
  - Bewertet gesamten Spielzustand nach Action-Ausführung
  - Beispiel: "MaterialAgent", "PositionAgent", "ThreatAgent"

- [ ] **`UtilityAggregator`** Klasse
  ```
  combineScores(List<AgentScore>, List<Weight>) -> float
  ```
  - Gewichtete Summe mit Normalisierung
  - Optional: Veto-Threshold (Score < -0.9 → Action verworfen)

#### 1.2.2 MVP Agent-Set (Combat-Fokus)

**Deliverables**: Implementiere zunächst **5 essenzielle Agents**

**Veto-Layer**:
- [ ] **`LegalMoveAgent`** (Veto)
  - Verwirft illegale Moves (außerhalb Map, blockiertes Terrain, etc.)
  - Weight: N/A (hard veto)

**Scoring-Layer**:
- [ ] **`ReactionFireAgent`** (Score: -1.0 bis 0.0)
  - Bewertet RF-Risk basierend auf Gegner-Positionen und Reichweiten
  - Formel: `-1.0 * (expected_damage / unit_max_hp)`
  - Weight: **HOCH** (z.B. 3.0) → RF ist extrem wichtig
  
- [ ] **`AggressivenessAgent`** (Score: 0.0 bis +1.0)
  - Bevorzugt Angriffe, bestraft Rückzug/Wait
  - Formel: `+1.0` für Attacks, `+0.0` für Move, `-0.3` für Wait
  - Weight: **MITTEL** (z.B. 1.5) → variabel basierend auf "Global Goal"

- [ ] **`TargetPriorityAgent`** (Score: -0.5 bis +1.0)
  - Bevorzugt Angriffe auf schwache/wertvolle Gegner
  - Formel: `+1.0` für low-HP high-value Targets, `-0.5` für starke Targets
  - Weight: **MITTEL** (z.B. 1.2)

**Evaluation-Layer**:
- [ ] **`MaterialEvaluationAgent`** (State-Score: -1.0 bis +1.0)
  - Summe eigener Unit-Werte minus Gegner-Unit-Werte, normalisiert
  - Weight: **HOCH** (z.B. 2.0) → Material ist fundamental

#### 1.2.3 Action Generation & Utility-Scoring

**Deliverables**:
- [ ] **`UtilityBasedActionGenerator`** Klasse
  - Generiert alle legalen Aktionen (Move, Attack, Wait)
  - Filtert durch Veto-Layer
  - Scored durch Scoring-Layer
  - Sortiert nach Combined-Utility
  - Pruning: Behalte Top-N Actions (z.B. Top-20)

- [ ] **Integration mit MCTS**
  - MCTS-Expansion nutzt `UtilityBasedActionGenerator`
  - Nur Top-N Actions werden als Child-Nodes erstellt
  - State-Evaluation nutzt `IStateEvaluationAgent`s

**Technische Herausforderungen**:
- Pathfinding muss effizient sein (A* auf lokaler Map)
- RF-Prediction muss korrekt sein (nutzt ASC's RF-Regeln)
- Performance: Agent-Calls müssen <0.1ms sein (caching wenn nötig)

**Success Criteria**:
- Alle 5 MVP-Agents implementiert und getestet
- Generiert alle legalen Moves korrekt
- Pruning reduziert Branching-Factor auf <25
- Performance: <10ms für Action-Generation + Scoring (20 Units)
- **Plausibilität**: Agent-Scores sind nachvollziehbar (Logging/Debugging)

---

### 1.3 Rollout-Policy mit Agents (Woche 4-5)

**Ziel**: Schnelle Simulation für MCTS Rollouts, nutzt Utility-Agents

**Deliverables**:
- [ ] **`ISimulationPolicy`** Interface
  - `selectAction(GameStateSnapshot, Unit) -> Action`
  
- [ ] **`UtilityBasedRolloutPolicy`** Implementierung
  - Nutzt **reduzierte Agent-Set** für Performance (z.B. nur RF + Aggressiveness)
  - Wählt Action mit höchstem Combined-Score
  - Schnellere Evaluation als volle Agent-Pipeline (Target: <0.5ms pro Action)
  - **Vereinfachungen für Speed**:
    - Nur Top-5 Actions evaluieren (statt Top-20)
    - Simplified RF-Check (keine exakte Damage-Berechnung, nur Boolean)
    - Keine Pathfinding-Suche, nur immediate Neighbors
  
- [ ] **`RandomPolicy`** (als Baseline)
  - Wählt zufällige legale Aktion
  - Für Performance-Vergleich
  
- [ ] **Depth-Limit** für Rollouts
  - 5-10 Züge (konfigurierbar)
  - Terminal-State Detection (alle Gegner tot / alle eigenen Units tot)

**Success Criteria**:
- Utility-Policy ist deutlich besser als Random (>70% Win-Rate in Tests)
- Performance: 100 Rollouts/Sekunde (5 Züge pro Rollout)
- Playout endet korrekt (Terminal-State oder Depth-Limit)

---

### 1.4 Integration & End-to-End Test (Woche 5)

**Ziel**: MCTS steuert tatsächlich Units in ASC

**Deliverables**:
- [ ] `TacticalMCTSController` Klasse
  - Erhält UnitGroup und Gegner-Info
  - Führt MCTS aus
  - Gibt beste Aktionen zurück
- [ ] Integration in ASC AI-Loop
  - KI ruft `TacticalMCTSController` für eigene Units
  - Führt zurückgegebene Aktionen auf echter GameMap aus
- [ ] Test-Szenario: 5 vs 5 Combat
  - MCTS-KI vs. simple Rule-Based AI (oder Dummy)
  - Überprüfe ob MCTS sinnvoll agiert (nicht suizidal)

**Success Criteria**:
- KI-Zug wird erfolgreich in ASC ausgeführt
- MCTS-KI gewinnt >60% gegen Dummy-AI in Combat-Szenarien
- Keine Crashes, keine illegalen Moves

---

### 1.5 Agent-System Erweiterbarkeit & Zukünftige Agents

**Architektur-Design für dynamische Erweiterung**

Das Agent-System ist explizit dafür designed, **nach dem MVP iterativ erweitert** zu werden. Neue taktische Verhaltensweisen können als zusätzliche Agents implementiert werden, ohne bestehenden Code zu ändern.

#### Erweiterte Agents (Post-MVP, Iteration 2+)

**Scoring-Layer (Action Utility)**:

- [ ] **`RetreatAgent`** (Score: -1.0 bis +1.0)
  - Erkennt wenn Unit in gefährlicher Lage ist (low HP, umzingelt)
  - Bevorzugt Rückzug zu sicheren Positionen (nahe eigene Units, Cover)
  - **Beispiel**: Unit mit 20% HP wird stark ermutigt, zurück zu fallen
  - Weight: HOCH wenn Unit angeschlagen (dynamisch adjustiert)

- [ ] **`FlankingAgent`** (Score: 0.0 bis +1.0)
  - Erkennt Flankierungs-Möglichkeiten (Angriff von Seite/Hinten)
  - Bevorzugt Moves, die Gegner umgehen und von ungünstiger Position angreifen
  - **Beispiel**: Statt Frontalangriff, bewege Unit um Gegner herum
  - Weight: MITTEL (z.B. 1.0)

- [ ] **`SupportRoleAgent`** (Score: 0.0 bis +1.0)
  - Speziell für Support-Units (Repair, Supply, Medic)
  - Bevorzugt Aktionen, die verbündete Units unterstützen
  - **Beispiel**: Repair-Unit fährt zu angeschlagener Panzer, repariert, fährt zurück
  - Weight: HOCH für Support-Units (z.B. 2.5), NIEDRIG für Combat-Units (z.B. 0.1)
  - **Interaktion mit RF**: RF-Agent dämpft, aber wenn Nutzen hoch genug (kritische Repair), kippt Entscheidung

- [ ] **`FormationAgent`** (Score: -0.5 bis +1.0)
  - Bevorzugt Moves, die Formation mit anderen Units erhält
  - Bestraft Moves, die Unit isolieren
  - **Beispiel**: Infantry bleibt bei Panzern (mutual support)
  - Weight: MITTEL (z.B. 1.0)

- [ ] **`TerrainAdvantageAgent`** (Score: 0.0 bis +1.0)
  - Bevorzugt Moves zu vorteilhaftem Terrain (Höhe, Cover, Defensiv-Bonus)
  - **Beispiel**: Unit bewegt sich auf Hügel für bessere Sicht/Reichweite
  - Weight: MITTEL (z.B. 0.8)

- [ ] **`AmmoConservationAgent`** (Score: -0.5 bis 0.0)
  - Bestraft Angriffe wenn Munition knapp
  - Ermutigt Nachschub-Suche
  - **Beispiel**: Unit mit 10% Ammo vermeidet unnötige Kämpfe
  - Weight: NIEDRIG-MITTEL (z.B. 0.5)

**Evaluation-Layer (State Evaluation)**:

- [ ] **`ThreatEvaluationAgent`** (State-Score: -1.0 bis +1.0)
  - Bewertet wie bedroht eigene Units sind (Gegner-Nähe, RF-Zonen)
  - Bevorzugt States mit niedriger Bedrohung
  - Weight: MITTEL (z.B. 1.2)

- [ ] **`PositionControlAgent`** (State-Score: -1.0 bis +1.0)
  - Bewertet Kontrolle über wichtige Map-Bereiche (Choke Points, Höhen)
  - Weight: MITTEL (z.B. 1.0)

- [ ] **`ProgressAgent`** (State-Score: 0.0 bis +1.0)
  - Bewertet Fortschritt zu Strategic Objective (z.B. "Erobere Sektor Nord")
  - Weight: variabel basierend auf Objective-Priorität

#### Dynamische Gewichtungs-Adjustierung

**"Global Goal" System**: Gewichtungen können zur Laufzeit basierend auf Strategic-Objective adjustiert werden:

**Beispiel-Szenarien**:

**Aggressive Assault** (Strategic Goal: "Vernichte alle Gegner schnell"):
- `AggressivenessAgent.weight = 2.5` (sehr hoch)
- `RetreatAgent.weight = 0.2` (sehr niedrig)
- `ReactionFireAgent.weight = 1.5` (reduziert, aber nicht ignoriert)

**Defensive Hold** (Strategic Goal: "Verteidige Position, minimiere Verluste"):
- `AggressivenessAgent.weight = 0.5` (niedrig)
- `RetreatAgent.weight = 2.0` (hoch)
- `FormationAgent.weight = 2.0` (hoch)
- `ReactionFireAgent.weight = 3.0` (extrem hoch)

**Support Mission** (Strategic Goal: "Repariere/versorge verbündete Gruppe"):
- `SupportRoleAgent.weight = 3.0` (extrem hoch)
- `AggressivenessAgent.weight = 0.1` (fast aus)
- `ReactionFireAgent.weight = 2.5` (hoch, aber Support-Nutzen kann überwiegen)

#### Parameter-Tuning Infrastruktur

**Deliverables (Post-MVP)**:
- [ ] **`AgentConfiguration`** Klasse
  - JSON/XML-basierte Konfiguration aller Agent-Weights
  - Verschiedene "Profiles" (Aggressive, Defensive, Balanced)
  - Zur Laufzeit ladbar (ohne Neucompilierung)

- [ ] **Tuning-Testbed**
  - Automatisierte Test-Szenarien (verschiedene Combat-Situationen)
  - Win-Rate Tracking für verschiedene Weight-Konfigurationen
  - Genetic Algorithm / Grid Search für optimale Weights (optional)

- [ ] **Debugging-Tools**
  - Logging: Welcher Agent hat welchen Score für welche Action gegeben?
  - Visualization: "Warum hat KI Action X gewählt?"
  - Replay-System: Kann vergangene Entscheidungen nachvollziehen

#### Interaktions-Beispiel: "Repair unter RF"

**Situation**: Repair-Unit soll angeschlagenen Panzer (15% HP) reparieren, muss aber durch Gegner-RF-Zone

**Agent-Bewertungen für "Move to Panzer + Repair"**:

| Agent | Score | Weight | Weighted |
|-------|-------|--------|----------|
| `LegalMoveAgent` | PASS (Veto) | N/A | - |
| `ReactionFireAgent` | -0.8 (hohes RF-Risk) | 3.0 | **-2.4** |
| `SupportRoleAgent` | +1.0 (kritische Repair) | 2.5 | **+2.5** |
| `AggressivenessAgent` | -0.2 (kein Attack) | 1.0 | **-0.2** |
| **GESAMT** | | | **-0.1** |

→ Action wird **knapp abgelehnt** (Score < 0)

**Aber**: Wenn Panzer noch wichtiger (z.B. einzige verbleibende Anti-Air-Unit):
- `SupportRoleAgent.weight = 4.0` (dynamisch erhöht)
- Weighted: +4.0 - 2.4 - 0.2 = **+1.4**
- → Action wird **gewählt**

Dies zeigt die **Flexibilität** des Systems: Komplexe Entscheidungen (RF-Risk vs. kritische Repair) entstehen durch Kombination einfacher Agents.

---

## Phase 2: Architektur-Dummies & Strategic Stubs
**Dauer**: 1-2 Wochen  
**Priorität**: MEDIUM (Architektonische Vorbereitung)

### 2.1 UnitGroup System (Dummy)

**Deliverables**:
- [ ] `UnitGroup` Klasse
  - Speichert Liste von Unit-IDs
  - Dummy-Objective: Immer "Attack nearest enemy"
  - Dummy-Gruppierung: Alle eigenen Units = 1 Gruppe
- [ ] `GroupManager` Klasse
  - Erstellt initial 1 Gruppe mit allen Units
  - Re-Grouping: NICHT implementiert (kommt später)

**Success Criteria**:
- Tactical MCTS kann auf `UnitGroup` zugreifen
- Architektur unterstützt später mehrere Gruppen (auch wenn aktuell nur 1)

---

### 2.2 Strategic Layer (Stub)

**Deliverables**:
- [ ] `IStrategicController` Interface
  - `planTurn() -> List<GroupObjective>`
- [ ] `DummyStrategicController` Implementierung
  - Gibt immer "Attack"-Objective für die eine Gruppe zurück
  - KEINE echte MCTS auf Strategic-Ebene
  - Placeholder für zukünftige Implementierung

**Success Criteria**:
- Tactical MCTS empfängt Objective vom Strategic Layer
- Interface ist bereit für echte Strategic-MCTS später

---

### 2.3 Memory System (Stub)

**Deliverables**:
- [ ] `StrategyMemory` Klasse (Minimal-Version)
  - Speichert letzte gesichtete Gegner-Positionen (einfache Liste)
  - KEINE Threat-Heatmap, KEINE Research-Queue (kommt später)
  - Serialisierung: Dummy (leere Implementierung)
- [ ] `IMemoryProvider` Interface
  - `getLastSeenEnemies() -> List<EnemyContact>`

**Success Criteria**:
- Tactical MCTS kann auf Memory-Daten zugreifen (auch wenn minimal)
- Architektur unterstützt später volle Memory-Features

---

## Phase 3: Testing, Tuning & Documentation
**Dauer**: 1-2 Wochen  
**Priorität**: HIGH (Qualitätssicherung)

### 3.1 Performance-Optimierung

**Deliverables**:
- [ ] Profiling: Identifiziere Bottlenecks
- [ ] Optimierungen:
  - Snapshot-Caching falls zu langsam
  - Action-Generation-Pruning verfeinern
  - Rollout-Depth anpassen
- [ ] Ziel: <5 Sekunden pro KI-Zug (10 Units)

---

### 3.2 Integration Tests

**Deliverables**:
- [ ] Test-Szenarien:
  - Simple Combat (5v5)
  - Asymmetrischer Combat (3v7)
  - Mixed Units (Panzer + Infanterie vs. reine Infanterie)
- [ ] Win-Rate Tracking: MCTS vs. Baseline AI

---

### 3.3 Documentation

**Deliverables**:
- [ ] Code-Dokumentation (Kommentare, README)
- [ ] Architektur-Diagramm (zeigt Dummy-Stubs vs. echte Implementation)
- [ ] Known Limitations & Future Work

---

## Gesamt-Timeline

| Phase | Wochen | Deliverable |
|-------|--------|-------------|
| **Phase 0** | 2-3 | Game State Cloning, Action Interface, Basic Evaluation |
| **Phase 1** | 4-5 | Tactical MCTS Core + Utility-Agent-Framework (Combat-Fokus) |
| **Phase 2** | 1-2 | Architektur-Dummies (Groups, Strategic Stub, Memory Stub) |
| **Phase 3** | 1-2 | Testing, Tuning, Documentation |
| **GESAMT** | **8-12 Wochen** | **Funktionierender Tactical Combat MVP mit erweiterbarem Agent-System** |

---

## Nach dem MVP: Ausbau-Plan

Nach erfolgreichem Durchstich können folgende Features iterativ hinzugefügt werden:

### Iteration 2: Strategic Layer (Wochen 12-15)
- Echte Strategic MCTS (Sektor-basiert)
- Multi-Gruppen Support
- Resource-Allocation

### Iteration 3: Memory Features (Wochen 16-18)
- Threat-Heatmap
- Enemy-Prediction
- Fog-of-War Intelligence

### Iteration 4: Non-Combat Actions (Wochen 19-21)
- Build-Queue
- Research-Queue
- Builder-Units, Supply-Units

### Iteration 5: Advanced Tactics (Wochen 22-24)
- Terrain-Analysis
- Koordinierte Multi-Gruppen-Angriffe
- Rollen-basierte Gruppen-Hierarchie

---

## Erfolgskriterien für MVP (Phase 0-3)

1. **Funktionalität**: 
   - KI kann eigenständig Combat-Entscheidungen treffen (Move + Attack)
   - Keine illegalen Moves, keine Crashes

2. **Performance**: 
   - <5 Sekunden pro KI-Zug (10 Units, 500 MCTS Iterationen)
   - Snapshot-Erstellung <5ms

3. **Qualität**:
   - MCTS-KI schlägt Dummy-AI in >60% der Combat-Szenarien
   - Taktisch plausible Entscheidungen (kein offensichtlich dummes Verhalten)

4. **Architektur**:
   - Modulare Interfaces erlauben einfaches Erweitern
   - Dummy-Stubs sind klar als solche dokumentiert
   - Code ist testbar und wartbar

---

## Risiken & Mitigations

| Risiko | Wahrscheinlichkeit | Impact | Mitigation |
|--------|-------------------|--------|------------|
| Game State Cloning zu langsam | HOCH | CRITICAL | Frühes Profiling, ggf. Snapshot-Scope reduzieren |
| MCTS findet keine guten Züge | MITTEL | HIGH | Tuning von Evaluation & Rollout-Policy |
| Integration mit ASC-Code schwierig | MITTEL | MEDIUM | Wrapper-Layer, gute Dokumentation von ASC-Internals |
| Performance-Budget überschritten | MITTEL | MEDIUM | Iteration-Budget dynamisch anpassen, Pruning verbessern |
| Reaction Fire falsch simuliert | NIEDRIG | HIGH | Gründliche Tests, Code-Review mit ASC-Experten |

---

## Nächste Schritte

1. **Sofort**: Phase 0.1 starten (Game State Cloning)
2. **Parallel**: ASC-Code-Analyse vertiefen (GameMap-Struktur, Command-System)
3. **Woche 2**: Erste Performance-Tests mit Snapshot
4. **Woche 3**: MCTS-Kern-Implementierung beginnen
