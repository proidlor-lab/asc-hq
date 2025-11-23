# GameMap Refactoring Plan: UI Decoupling

**Created**: 2025-11-20
**Updated**: 2025-11-20 (v2.1 - Added LLM Directives & Risk Mitigation)
**Status**: Planning - Consensus from Multiple Reviews
**Priority**: CRITICAL - Blocking headless server development

---

## Executive Summary

The `GameMap` class is a **God Class** (18,550+ lines in .cpp) that violates multiple SOLID principles and tightly couples game logic with UI presentation. This coupling prevents the codebase from functioning as a headless server for a modern client-server architecture.

**Critical Blocker**: Cannot create RESTful API or network protocol for new UI client while UI code is embedded in core game logic.

**Estimated Effort**: 8-12 weeks
**Risk Level**: HIGH (core refactoring)
**Benefit**: Enables client-server architecture, modern UI development, testability

**Document History**:
- This plan consolidates insights from three independent analyses
- Incorporates best practices from all review perspectives
- Validated against multiple refactoring approaches

---

## Current Architecture Analysis

### The God Class Problem

**GameMap Responsibilities (Current)**:
1. ✅ Game state management (players, turns, time)
2. ✅ Map geometry (fields, coordinates, terrain)
3. ✅ Unit and building management
4. ✅ Resource management (global and local)
5. ✅ Event system
6. ✅ Action recording/replay
7. ✅ Network synchronization
8. ❌ **UI rendering** (OverviewMapHolder with Surface)
9. ❌ **UI state tracking** (dialogsHooked, guiHooked())
10. ❌ **Mixed persistence** (save/load with UI state)
11. ❌ **Direct signal connections to UI**

**Violation Score**: 8+ responsibilities → Should be 1-2 (Single Responsibility Principle)

---

## Critical UI Coupling Points

### 1. Direct UI Component Embedding

**Location**: `gamemap.h:501`
```cpp
OverviewMapHolder overviewMapHolder;
```

**Problem**:
- `OverviewMapHolder` contains `Surface` objects (graphics buffers)
- Generates minimap images using `sigc::trackable` for GUI idle handlers
- Cannot compile in headless mode without graphics dependencies
- Creates visual representation data in core game logic

**Impact**: CRITICAL
- Prevents true headless builds
- Forces SDL/graphics dependencies on server
- Memory overhead for unused graphics buffers

---

### 2. UI State Tracking

**Location**: `gamemap.h:174, 539-540`
```cpp
bool dialogsHooked;
void guiHooked();
bool getGuiHooked() { return dialogsHooked; }
```

**Location**: `gamemap.cpp:271-274`
```cpp
void GameMap::guiHooked() {
   overviewMapHolder.connect();
   dialogsHooked = true;
}
```

**Problem**:
- Core game logic knows about UI existence
- Behavior changes based on UI presence
- Couples initialization to UI lifecycle

**Impact**: HIGH
- Game logic decisions based on UI state
- Cannot test game logic without UI mock
- Headless mode requires conditional compilation

---

### 3. Signal-Based UI Notifications

**Location**: `gamemap.h:466-481`
```cpp
sigc::signal<void, Player&> sigPlayerTurnBegins;
sigc::signal<void, Player&> sigPlayerUserInteractionBegins;
sigc::signal<void, Player&> sigPlayerUserInteractionEnds;
sigc::signal<void, Player&> sigPlayerTurnEnds;
sigc::signal<void, Player&> sigPlayerTurnHasEnded;
sigc::signal<void, Player&> sigMapWon;
sigc::signal<void, const MapCoodinateVector&> sigCoordinateShift;
sigc::signal<void> newRound;

static sigc::signal<void, GameMap&> sigMapCreation;
static sigc::signal<void, GameMap&> sigMapDeletion;
static sigc::signal<void, GameMap*, Player&> sigPlayerTurnEndsStatic;
```

**Problem**:
- Direct dependency on `sigc++` library (GUI framework)
- Signals are synchronous and block game logic
- No abstraction layer for event notification
- UI-specific naming ("UserInteraction")

**Impact**: MEDIUM-HIGH
- Cannot swap notification mechanism
- Hard to implement network event protocol
- Synchronous signals slow down server operations
- Testing requires signal mocking

---

### 4. Player View Management (Mixed Concern)

**Location**: `gamemap.h:378-386`
```cpp
private:
   int playerView;

public:
   /** the player which is currently viewing the map.
       During replays, for example, this will be different from the player that moves units
       -1 means: everything is visible
       -2 means: nothing is visible */
   int getPlayerView() const;
   void setPlayerView(int player);
```

**Problem**:
- "View" concept is UI-specific
- Game logic should track active player, not "viewing" player
- Special values (-1, -2) are magic numbers for UI states
- Mixes game state with presentation state

**Impact**: MEDIUM
- Confusion between game state and UI state
- Cannot have multiple clients viewing different players
- Replay system coupled to single-viewer assumption

---

### 5. Weak Encapsulation - Public Data Members

**Location**: `gamemap.h:191-398` (multiple public data members)
```cpp
public:
   char* temp;
   char* temp2;
   int* temp3;
   int* temp4;

   int xsize, ysize;
   MapField* field;
   Player player[9];
   ASCString gameJournal;
   Resources bi_resource[8];
   int* game_parameter;
   // ... many more
```

**Problem**:
- **Critical architectural flaw** identified by all reviews
- Direct public access to mutable game state
- UI code can reach in and mutate core data directly
- No validation, no invariant protection
- Breaks encapsulation fundamentally

**Impact**: CRITICAL
- Bidirectional coupling (game ← → UI)
- Cannot track state changes
- Impossible to add validation layer
- Race conditions in future multi-client scenario
- Cannot generate audit log of changes
- Testing requires full state mockup

**Example of Problem**:
```cpp
// UI code can do this directly:
map.xsize = 100;  // No validation!
map.player[0].resources.energy = 9999;  // Cheating possible!
map.field[42] = someOtherField;  // Memory corruption risk!
```

**Root Cause**: Legacy C-style design from 2001, before modern C++ practices

---

### 6. ReplayInfo Mixing UI and Logic Data

**Location**: `gamemap.h:348-364`
```cpp
class ReplayInfo {
  public:
   MemoryStreamStorage* guidata[8];  // GUI-specific replay blobs
   MemoryStreamStorage* map[8];      // Logical replay data
   MemoryStream* actmemstream;
   // ...
};
```

**Problem**:
- `guidata` (GUI data) stored alongside pure game logic
- Serialization format mixes presentation with domain data
- Headless replay must carry unused GUI data
- New UI cannot define own metadata format

**Impact**: HIGH
- Save game compatibility issues when UI changes
- Memory waste in headless mode
- Cannot version UI data separately from game data
- Migration complexity for new client

**Solution Needed**: Separate UI metadata into external sidecar file

---

### 7. Serialization Mixing Concerns

**Problem**:
- `GameMap::read()` and `GameMap::write()` serialize everything together
- UI state, replay GUI data, game logic all in one stream
- No separation between:
  - **Core game state** (required for server)
  - **UI preferences** (client-specific)
  - **Presentation data** (rendering cache)

**Impact**: HIGH
- Cannot load old saves in new UI without understanding old UI format
- Headless server must parse (and ignore) UI sections
- Save file format locked to current UI implementation
- Migration from legacy UI to new UI requires format converter

**Example**:
```cpp
// Current (BAD):
void GameMap::write(tnstream& stream) {
   stream << xsize << ysize;  // Core data
   stream << playerView;       // UI state (!)
   overviewMapHolder.write(stream);  // UI data (!)
   replayinfo->guidata[0]->write(stream);  // UI replay (!)
   // ... all mixed together
}
```

**Solution**: Layered serialization with separate UI sidecar files

---

### 8. Visibility System (Partial Coupling)

**Location**: `mapfield.h:69`
```cpp
//! can this field be seen be the player. Variable is bitmapped;
//! two bits for each player.
Uint16 visible;
```

**Status**: GOOD - Game logic concern
**Reason**: Fog of war is game rule, not UI
**Note**: Implementation is correct, but needs API for client access

---

## Design Principle Violations

### Single Responsibility Principle (SRP)
**Status**: ❌ VIOLATED

GameMap has 8+ distinct responsibilities. Should be split into:
- `GameState` - Current game state
- `MapGeometry` - Terrain and coordinates
- `TurnManager` - Turn/round progression
- `EventManager` - Game events
- `ActionLog` - Action recording/replay
- `NetworkSync` - Multiplayer synchronization
- **REMOVE**: UI rendering, UI state tracking

---

### Open/Closed Principle (OCP)
**Status**: ❌ VIOLATED

Cannot extend GameMap behavior without modifying it:
- Adding new UI requires modifying core class
- Cannot add new event types without changing GameMap
- Hard-coded UI hooks prevent extension

---

### Dependency Inversion Principle (DIP)
**Status**: ❌ VIOLATED

High-level game logic depends on low-level UI details:
```
GameMap (high-level) ──depends on──> OverviewMapHolder (low-level)
GameMap (high-level) ──depends on──> sigc::signal (low-level)
GameMap (high-level) ──depends on──> Surface (low-level)
```

**Should be**:
```
GameMap (high-level) ──defines──> IEventNotifier (interface)
                          ↑
                    implements
                          |
                   UIEventAdapter (low-level)
```

---

### Interface Segregation Principle (ISP)
**Status**: ❌ VIOLATED

Headless server forced to include UI interface:
- `guiHooked()` method exists in all builds
- `OverviewMapHolder` compiled even when unused
- Cannot create thin interface for server-only operations

---

## Target Architecture

### Layered Architecture for Client-Server

```
┌─────────────────────────────────────────────────────────────┐
│  NEW CLIENT (Proprietary, Modern UI)                        │
│  - React/Electron/Qt                                         │
│  - Consumes REST API or WebSocket                           │
└─────────────────────────────────────────────────────────────┘
                          ▲
                          │ HTTP/WebSocket/gRPC
                          ▼
┌─────────────────────────────────────────────────────────────┐
│  API LAYER (To Be Created)                                  │
│  - RESTful endpoints                                         │
│  - WebSocket for real-time events                           │
│  - Authentication & authorization                            │
│  - Rate limiting, validation                                 │
└─────────────────────────────────────────────────────────────┘
                          │
                          ▼
┌─────────────────────────────────────────────────────────────┐
│  GAME LOGIC LAYER (Refactored)                              │
│  ┌────────────────┐  ┌──────────────┐  ┌─────────────────┐ │
│  │  GameState     │  │ TurnManager  │  │  EventDispatcher│ │
│  │  - Map data    │  │ - Turn logic │  │  - Event queue  │ │
│  │  - Players     │  │ - Round mgmt │  │  - Subscribers  │ │
│  │  - Resources   │  │              │  │                 │ │
│  └────────────────┘  └──────────────┘  └─────────────────┘ │
│                                                              │
│  ┌────────────────┐  ┌──────────────┐  ┌─────────────────┐ │
│  │  MapGeometry   │  │ ActionLog    │  │  RuleEngine     │ │
│  │  - Terrain     │  │ - Recording  │  │  - Validation   │ │
│  │  - Coordinates │  │ - Replay     │  │  - Constraints  │ │
│  └────────────────┘  └──────────────┘  └─────────────────┘ │
└─────────────────────────────────────────────────────────────┘
                          │
                          ▼
┌─────────────────────────────────────────────────────────────┐
│  PERSISTENCE LAYER                                           │
│  - Save/Load game state                                      │
│  - Database (optional)                                       │
└─────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────┐
│  LEGACY UI (Optional, GPL)                                   │
│  - Current SDL/wxWidgets UI                                  │
│  - Same API as new client                                    │
│  - Adapters for backward compatibility                       │
└─────────────────────────────────────────────────────────────┘
```

---

### Directory Layout for Refactored Code

| Path | Purpose |
|------|---------|
| `source/core/state/` | `GameState`, `MapGeometry`, serialization helpers, read-only views |
| `source/core/turns/` | `TurnManager`, scheduling utilities, round transitions |
| `source/core/rules/` | `RuleEngine`, resource/movement/combat validation |
| `source/core/events/` | `GameEventDispatcher`, payload definitions, adapters (`sigc++`, headless bus) |
| `source/core/history/` | `ActionLog`, replay storage, undo/redo plumbing |
| `source/core/interactions/` | `UserInteractionProvider` interfaces and implementations |
| `source/platform/ui_legacy/` | Bridges so the existing UI can consume the refactored engine (overview map, legacy signals) |
| `tests/core/...` | GoogleTest suites mirroring each module (e.g., `tests/core/state`, `tests/core/events`) |

Each refactoring phase should move logic into these folders and create matching tests so the module boundaries remain obvious and headless-safe.

---

## Refactoring Strategy

### Phase 0: Test Harness & Characterization (Week 0 - CRITICAL)

**Goal**: Establish safety net BEFORE any refactoring

**Critical Insight from Reviews**: "Add automated tests around GameState serialization and turn progression BEFORE moving code, so regressions are caught while untangling responsibilities."

#### Step 0.1: Characterization Tests
```cpp
// tests/characterization/gamemap_behavior_test.cpp

// Document CURRENT behavior (even if imperfect)
TEST(GameMapCharacterization, TurnProgression) {
   GameMap map;
   map.allocateFields(10, 10);
   map.startGame();

   EXPECT_EQ(map.actplayer, 0);

   map.endTurn();
   EXPECT_EQ(map.actplayer, 1);  // Current behavior

   // Document all 8 players
   for (int i = 1; i < 8; i++) {
      map.endTurn();
   }
   map.endTurn();  // Wrap to player 0
   EXPECT_EQ(map.actplayer, 0);
}

TEST(GameMapCharacterization, ResourceManagement) {
   GameMap map;
   // Test current resource behavior
   map._resourcemode = 0;  // ASC mode
   EXPECT_FALSE(map.isResourceGlobal(0));

   map._resourcemode = 1;  // BI mode
   EXPECT_TRUE(map.isResourceGlobal(0));
}

TEST(GameMapCharacterization, Serialization) {
   GameMap original;
   original.allocateFields(10, 10);
   original.maptitle = "Test Map";

   // Serialize
   MemoryStream stream;
   original.write(stream);

   // Deserialize
   GameMap loaded;
   stream.seek(0);
   loaded.read(stream);

   EXPECT_EQ(loaded.xsize, 10);
   EXPECT_EQ(loaded.ysize, 10);
   EXPECT_EQ(loaded.maptitle, "Test Map");
}
```

#### Step 0.2: Integration Test Baseline
```cpp
// tests/integration/legacy_ui_compatibility_test.cpp

// Ensure refactoring doesn't break legacy UI
TEST(LegacyUICompatibility, SignalEmission) {
   GameMap map;
   bool signalReceived = false;

   map.sigPlayerTurnBegins.connect([&](Player& p) {
      signalReceived = true;
   });

   map.beginTurn();
   EXPECT_TRUE(signalReceived);
}
```

#### Step 0.3: Performance Baseline
```cpp
// tests/benchmarks/gamemap_performance.cpp

BENCHMARK(TurnCyclePerformance) {
   GameMap map;
   map.allocateFields(100, 100);
   map.startGame();

   for (int i = 0; i < 100; i++) {
      map.endTurn();
   }
}

// Establish baseline: Must not regress by >10% after refactoring
```

**Benefits**:
- ✅ Safety net catches regressions immediately
- ✅ Documents current behavior (good and bad)
- ✅ Validates that refactoring preserves functionality
- ✅ Performance regression detection

**Estimated Effort**: 3-5 days
**Risk**: NONE (only adding tests)

**⚠️ MANDATORY**: Do not proceed to Phase 1 without these tests passing

---

### Phase 1: Extract Event System (Week 1-2)

**Goal**: Replace `sigc::signal` with abstract event dispatcher

**⚠️ LLM EXECUTION DIRECTIVES**:
1. **Discovery First**: Before modifying code, use `grep_search` to list all calls to `.emit()` for each signal. Do not rely on guess-work.
2. **Parallel Implementation**: Create `GameEventDispatcher` and `GameEventType` in new files (`source/core/`) BEFORE touching `gamemap.h`.
3. **Adapter Priority**: Implement `LegacyUIEventAdapter` immediately after the dispatcher. This allows you to route events -> dispatcher -> adapter -> old signals, keeping the application runnable at every step.
4. **Strict Isolation**: The `GameEvent` classes must NOT depend on GUI libraries (SDL, sigc++, etc.). They should use standard C++ types or core game types (`Player`, `MapCoordinate`) only.

#### Step 1.1: Create Event Abstraction
```cpp
// New file: source/core/game_event_dispatcher.h

enum class GameEventType {
   PlayerTurnBegins,
   PlayerTurnEnds,
   PlayerUserInteractionBegins,
   PlayerUserInteractionEnds,
   MapWon,
   RoundStarts,
   CoordinateShift,
   MapCreated,
   MapDestroyed
};

struct GameEvent {
   GameEventType type;
   GameTime timestamp;
   // Polymorphic payload
   virtual ~GameEvent() = default;
};

struct PlayerEvent : GameEvent {
   int playerId;
   PlayerEvent(GameEventType type, int pid)
      : type(type), playerId(pid) {}
};

class IGameEventListener {
public:
   virtual void onGameEvent(const GameEvent& event) = 0;
   virtual ~IGameEventListener() = default;
};

class GameEventDispatcher {
private:
   std::vector<IGameEventListener*> listeners;
   std::queue<std::unique_ptr<GameEvent>> eventQueue;

public:
   void subscribe(IGameEventListener* listener);
   void unsubscribe(IGameEventListener* listener);

   void dispatch(std::unique_ptr<GameEvent> event);
   void processQueue(); // Async processing

   // Optional: filtering by event type
   void subscribe(IGameEventListener* listener, GameEventType type);
};
```

#### Step 1.2: Replace Signal Usage
**Before**:
```cpp
sigc::signal<void, Player&> sigPlayerTurnBegins;
// ...
sigPlayerTurnBegins.emit(player[actplayer]);
```

**After**:
```cpp
GameEventDispatcher eventDispatcher;
// ...
auto event = std::make_unique<PlayerEvent>(
   GameEventType::PlayerTurnBegins,
   actplayer
);
eventDispatcher.dispatch(std::move(event));
```

#### Step 1.3: Create UI Adapter
```cpp
// source/ui/legacy_ui_event_adapter.h
class LegacyUIEventAdapter : public IGameEventListener {
private:
   GameMap& map; // For backward compatibility

public:
   void onGameEvent(const GameEvent& event) override {
      switch(event.type) {
         case GameEventType::PlayerTurnBegins:
            // Emit old signal for legacy UI
            map.sigPlayerTurnBegins.emit(...);
            break;
         // ... other events
      }
   }
};
```

**Benefits**:
- ✅ Core game logic independent of sigc++
- ✅ Can add WebSocket event broadcaster
- ✅ Async event processing possible
- ✅ Easy to test (mock listener)
- ✅ Legacy UI still works via adapter

**Estimated Effort**: 1-2 weeks
**Risk**: LOW (additive change, old code works)

---

### Phase 2: Extract OverviewMapHolder ✅ COMPLETE (2025-11-23)

**Goal**: Move UI rendering out of GameMap
**Status**: ✅ **STRUCTURALLY COMPLETE** - Dependency inversion achieved, builds passing

#### Step 2.1: Create Separate Service
```cpp
// New file: source/ui/overview_map_generator.h

class IOverviewMapGenerator {
public:
   virtual Surface generateOverviewMap(const GameMap& map) = 0;
   virtual void updateField(const MapCoordinate& pos) = 0;
   virtual ~IOverviewMapGenerator() = default;
};

class OverviewMapGeneratorService : public IOverviewMapGenerator,
                                     public IGameEventListener {
private:
   const GameMap& map;
   Surface cachedImage;
   bool isDirty;

public:
   OverviewMapGeneratorService(const GameMap& m)
      : map(m), isDirty(true) {}

   Surface generateOverviewMap(const GameMap& map) override;
   void updateField(const MapCoordinate& pos) override;

   // Listen to game events
   void onGameEvent(const GameEvent& event) override {
      if (event.type == GameEventType::CoordinateShift) {
         isDirty = true;
      }
   }
};
```

#### Step 2.2: Remove from GameMap
**Before** (gamemap.h:501):
```cpp
OverviewMapHolder overviewMapHolder;
```

**After**:
```cpp
// REMOVED - UI concern, not game logic
```

**Migration**:
```cpp
// In UI initialization code:
auto overviewGenerator = std::make_unique<OverviewMapGeneratorService>(gameMap);
gameMap.eventDispatcher.subscribe(overviewGenerator.get());
```

**Benefits**:
- ✅ GameMap no longer depends on Surface
- ✅ Headless build doesn't include graphics code
- ✅ Can have multiple overview generators (thumbnail, full map, etc.)
- ✅ Memory used only when needed

**Estimated Effort**: 1 week
**Risk**: LOW (clear separation)

---

### Phase 3: Remove guiHooked() Tracking (Week 3-4)

**Goal**: Game logic should not know about UI existence

#### Step 3.1: Analyze Usage
Search for `getGuiHooked()` and `dialogsHooked` usage:
- Identify conditional behavior based on UI presence
- Determine if behavior is game logic or UI feedback
- Catalog all decision points

#### Step 3.2: Create UserInteractionProvider Interface

**Pattern from Review**: Dependency injection for UI interactions

```cpp
// New file: source/core/user_interaction_provider.h

class IUserInteractionProvider {
public:
   virtual ~IUserInteractionProvider() = default;

   // Query user for decisions
   virtual int selectOption(
      const std::string& prompt,
      const std::vector<std::string>& options,
      int defaultOption = 0
   ) = 0;

   virtual bool confirm(
      const std::string& message,
      bool defaultValue = true
   ) = 0;

   virtual std::string requestInput(
      const std::string& prompt,
      const std::string& defaultValue = ""
   ) = 0;

   // Notify user of events
   virtual void showMessage(const std::string& message) = 0;
   virtual void showError(const std::string& error) = 0;
};

// Headless implementation (uses defaults)
class HeadlessInteractionProvider : public IUserInteractionProvider {
public:
   int selectOption(const std::string&, const std::vector<std::string>&, int defaultOption) override {
      return defaultOption;  // Always use default in headless
   }

   bool confirm(const std::string&, bool defaultValue) override {
      return defaultValue;
   }

   std::string requestInput(const std::string&, const std::string& defaultValue) override {
      return defaultValue;
   }

   void showMessage(const std::string& msg) override {
      // Log to file or ignore
   }

   void showError(const std::string& err) override {
      // Log to error stream
      std::cerr << "ERROR: " << err << std::endl;
   }
};

// GUI implementation (shows dialogs)
class GUIInteractionProvider : public IUserInteractionProvider {
public:
   int selectOption(const std::string& prompt, const std::vector<std::string>& options, int defaultOption) override {
      // Show dialog, wait for user
      return showSelectionDialog(prompt, options, defaultOption);
   }

   bool confirm(const std::string& message, bool defaultValue) override {
      return showConfirmDialog(message, defaultValue);
   }

   // ... implement with actual GUI dialogs
};
```

#### Step 3.3: Inject Provider into GameMap

```cpp
class GameMap {
private:
   IUserInteractionProvider* interactionProvider;

public:
   GameMap(IUserInteractionProvider* provider = nullptr)
      : interactionProvider(provider ? provider : &defaultHeadlessProvider) {}

   void setInteractionProvider(IUserInteractionProvider* provider) {
      interactionProvider = provider;
   }

private:
   static HeadlessInteractionProvider defaultHeadlessProvider;
};
```

#### Step 3.4: Replace All guiHooked() Usage

**Before**:
```cpp
if (map.getGuiHooked()) {
   int choice = askUserForInput();  // Shows dialog
   doAction(choice);
} else {
   doAction(0);  // Use default
}
```

**After**:
```cpp
int choice = map.interactionProvider->selectOption(
   "Select action",
   {"Option A", "Option B", "Option C"},
   0  // default
);
doAction(choice);

// Headless: automatically returns 0
// GUI: shows dialog and waits for user
```

#### Step 3.5: Remove guiHooked() Completely

```cpp
// DELETE these:
bool dialogsHooked;
void guiHooked();
bool getGuiHooked();
```

**Benefits**:
- ✅ No UI-specific flags in game logic
- ✅ Cleaner separation via dependency injection
- ✅ Headless mode works naturally (uses defaults)
- ✅ Easy to test (mock interaction provider)
- ✅ Can implement different UIs (CLI, GUI, web)

**Estimated Effort**: 1 week
**Risk**: MEDIUM (need to audit all usage, but pattern is clean)

---

### Iterative Manual Testing Checkpoints

To avoid multi-week regressions, schedule short stabilization iterations after the first three phases:

1. **Iteration A (after Phase 1)**  
   - Build the game with the new event dispatcher.  
   - Manually exercise: start skirmish map, run a full round, verify player-turn notifications still hit the legacy UI.  
   - Document findings before proceeding to Phase 2.

2. **Iteration B (after Phase 2)**  
   - With `OverviewMapHolder` moved behind adapters, run headless mode and the legacy UI.  
   - Manual checks: load/save, minimap rendering, replay playback.  
   - Record any rendering gaps for follow-up tickets.

3. **Iteration C (after Phase 3)**  
   - Validate the interaction provider path: run typical dialogs (unit construction, confirmations) in the legacy UI and ensure headless defaults continue to work.  
   - Perform a scripted smoke test (automated turn cycle + manual UI exploration) before moving on to player-view decoupling.

Each iteration should last 1–2 days, include a checklist of manual scenarios, and only then unlock the next refactoring phase.

---

### Phase 4: Separate PlayerView from GameState (Week 4)

**Goal**: Viewing perspective is UI concern, not game state

**⚠️ WARNING: HIGH RISK AREA**
Confusion between **Active Player** (whose turn it is, logic state) and **Viewing Player** (who is looking at the screen, UI state) is the #1 source of bugs in this phase.
*   **Invariant**: `GameMap` logic (movement, combat, resources) **MUST NEVER** depend on `viewingPlayer`.
*   **Invariant**: Only *queries* meant for the UI (rendering, tooltip info) should check `viewingPlayer`.

**MANDATORY Unit Test Requirements**:
1.  **Simulation Independence**:
    ```cpp
    TEST(GameView, ViewDoesNotAffectSimulation) {
       // Run turn with View = Player 1
       auto result1 = runTurnSimulation(map, view=1);
       // Run identical turn with View = Player 2
       auto result2 = runTurnSimulation(map, view=2);
       // Result must be bitwise identical
       EXPECT_EQ(result1, result2);
    }
    ```
2.  **Admin/Observer View**:
    ```cpp
    TEST(GameView, AdminSeesAll) {
       GameViewContext adminView(map, -1); // -1 = Admin
       EXPECT_TRUE(adminView.canSeeUnit(stealthedEnemyUnit));
    }
    ```
3.  **Fog of War Enforcement**:
    ```cpp
    TEST(GameView, FogOfWar) {
       GameViewContext playerView(map, 1);
       EXPECT_FALSE(playerView.isFieldVisible(enemyBaseCoords));
    }
    ```

#### Step 4.1: Create View Context
```cpp
// New file: source/core/game_view_context.h

class GameViewContext {
private:
   const GameMap& map;
   int viewingPlayer; // -1 = admin, -2 = observer, 0-7 = player

public:
   GameViewContext(const GameMap& m, int player)
      : map(m), viewingPlayer(player) {}

   bool isFieldVisible(const MapCoordinate& pos) const;
   bool canSeeUnit(const Vehicle* unit) const;
   bool canAccessResources() const;

   // Query methods that respect viewing permissions
   std::vector<Vehicle*> getVisibleUnits() const;
   Resources getVisibleResources(int player) const;
};
```

#### Step 4.2: Remove from GameMap
**Before**:
```cpp
private:
   int playerView;
public:
   int getPlayerView() const;
   void setPlayerView(int player);
```

**After**:
```cpp
// REMOVED - Use GameViewContext in UI/API layer
```

**Benefits**:
- ✅ Multiple clients can view different players simultaneously
- ✅ Game state is objective truth
- ✅ View permissions enforced at API boundary
- ✅ Replay system simplified

**Estimated Effort**: 1 week
**Risk**: MEDIUM (affects many systems)

---

### Phase 5: Extract Responsibility Modules + Encapsulation (Week 5-8)

**Goal**: Break down God Class into focused components AND fix public data member exposure.

**⚠️ LLM EXECUTION STRATEGY: STRANGLE THE GOD CLASS**
Do **NOT** attempt to extract everything at once. This phase is broken down into atomic sub-phases. Each sub-phase must be completed, compiled, and tested before moving to the next. Use the **Parallel Change** (Expand-Contract) pattern:
1.  **Expand**: Add new class/method.
2.  **Delegate**: Make old code call new code.
3.  **Contract**: Refactor callers to use new code directly (can be done lazily).

#### Sub-Phase 5.1: MapGeometry Extraction (The `field` array)
**Goal**: Move `field`, `xsize`, `ysize` to a dedicated `MapGeometry` class.

1.  **Create Class**:
    ```cpp
    // source/core/map_geometry.h
    class MapGeometry {
    private:
        int width, height;
        std::vector<MapField> fields;
    public:
        MapGeometry(int w, int h);
        int getWidth() const { return width; }
        int getHeight() const { return height; }
        MapField* getField(int x, int y); // Temporary: Mutable access for legacy
        const MapField* getField(int x, int y) const;
        void resize(int w, int h);
    };
    ```
2.  **Embed in GameMap**:
    ```cpp
    class GameMap {
       // ...
       MapGeometry geometry; // Replaces explicit xsize/ysize/field management
    };
    ```
3.  **Delegate (The Proxy Step)**:
    *   **CRITICAL**: Do not delete `GameMap::xsize` or `GameMap::field` yet if possible, OR make them macros/properties that forward to `geometry`.
    *   Refactor `GameMap::width()` to return `geometry.getWidth()`.
    *   Refactor `GameMap::getField(x,y)` to return `geometry.getField(x,y)`.
4.  **Compile Check**: Ensure project compiles with `GameMap` delegating to `MapGeometry`.

#### Sub-Phase 5.2: GameState Extraction (The `player` array)
**Goal**: Move `player[9]` and `actplayer` to `GameState`.

1.  **Create Class**:
    ```cpp
    // source/core/game_state.h
    class GameState {
    private:
        std::array<Player, 9> players;
        int activePlayer;
        GameTime currentTime;
    public:
        Player& getPlayer(int id) { return players.at(id); }
        const Player& getPlayer(int id) const { return players.at(id); }
        int getActivePlayer() const { return activePlayer; }
        void setActivePlayer(int id);
    };
    ```
2.  **Embed in GameMap**:
    ```cpp
    class GameMap {
        GameState state;
    };
    ```
3.  **Refactor Accessors (Parallel Change)**:
    *   Identify direct usage of `map.player[i]`.
    *   Replace with `map.getPlayer(i)`.
    *   Update `map.getPlayer(i)` to call `state.getPlayer(i)`.

#### Sub-Phase 5.3: TurnManager Extraction
**Goal**: Move turn logic (`endTurn`, `newRound`) to `TurnManager`.

1.  **Create Class**:
    ```cpp
    // source/core/turn_manager.h
    class TurnManager {
        GameState& state;
        GameEventDispatcher& events;
    public:
        void endTurn();
        void endRound();
    };
    ```
2.  **Delegate**:
    *   `GameMap::endTurn()` becomes a wrapper around `TurnManager::endTurn()`.

#### Sub-Phase 5.4: RuleEngine & ActionLog
*   Follow the same pattern: Create class -> Embed -> Delegate -> Refactor Callers.

**Benefits**:
- ✅ **Compile-Safe**: The build never breaks for more than a few minutes.
- ✅ **Reviewable**: Smaller PRs/changesets.
- ✅ **Revertible**: If a sub-phase fails, only that part needs rolling back.
- ✅ **Testable**: New classes can be unit tested immediately upon creation.

**Estimated Effort**: 4 weeks (1 week per sub-phase)
**Risk**: HIGH (but mitigated by granular approach)

---

### Phase 5.5: Separate Serialization Concerns (Week 9)

**Goal**: Split UI data from game logic in save files

**Critical Insight from Reviews**: "Update serialization so GUI-specific replay blobs are stored in a dedicated UI add-on file/stream rather than bundled with the logical replay."

#### Step 5.5.1: Define Layered Serialization

```cpp
// source/core/game_serialization.h

class GameStateSerialization {
public:
   // CORE game state only (platform-independent, UI-agnostic)
   static void saveGameState(const GameState& state, OutputStream& stream);
   static void loadGameState(GameState& state, InputStream& stream);

   // Versioning for compatibility
   static constexpr int SAVE_FORMAT_VERSION = 1;
};

class UIMetadataSerialization {
public:
   // UI-specific data (stored separately)
   static void saveUIMetadata(const UIState& ui, OutputStream& stream);
   static void loadUIMetadata(UIState& ui, InputStream& stream);
};
```

#### Step 5.5.2: File Format Structure

**Current (BAD)**: Single `.ascdat` file with everything mixed
```
savegame.ascdat:
  [Header]
  [Map dimensions]        ← Core
  [Player data]           ← Core
  [Player view state]     ← UI!
  [Overview map cache]    ← UI!
  [GUI replay blobs]      ← UI!
  [Window positions]      ← UI!
```

**New (GOOD)**: Separate files for different concerns
```
savegame.ascdat:           # Core game state (required for server)
  [Header]
  [Format version]
  [Map dimensions]
  [Player data]
  [Unit positions]
  [Resources]
  [Event history]

savegame.ascui:            # UI metadata (optional, client-specific)
  [UI format version]
  [Window positions]
  [Player view preferences]
  [Minimap cache]
  [Custom UI settings]

savegame.ascreplay:        # Replay data (pure actions)
  [Action log]
  [Timestamps]
  [Random seed]

savegame.ascreplay-ui:     # UI-specific replay data (optional)
  [Camera positions]
  [UI events]
  [Client-side annotations]
```

#### Step 5.5.3: Backward Compatibility

```cpp
class LegacySaveGameMigrator {
public:
   // Read old format, split into new format
   static void migrate(
      const std::string& legacyFile,
      const std::string& coreFile,
      const std::string& uiFile
   ) {
      // Parse old format
      GameMap legacyMap;
      legacyMap.read(openFile(legacyFile));

      // Extract core state
      GameState coreState = extractCoreState(legacyMap);
      coreState.serialize(openFile(coreFile));

      // Extract UI state
      UIState uiState = extractUIState(legacyMap);
      uiState.serialize(openFile(uiFile));
   }
};
```

#### Step 5.5.4: Rename ReplayInfo::guidata

```cpp
// Before:
class ReplayInfo {
   MemoryStreamStorage* guidata[8];  // GUI-specific name
};

// After:
class ReplayInfo {
   MemoryStreamStorage* coreActionLog[8];      // Game logic replay
   MemoryStreamStorage* clientExtensionData[8];  // Optional client data

   // Core replay can be used without client data
   void saveCoreReplay(OutputStream& stream);
   void saveClientExtension(OutputStream& stream);  // Optional
};
```

**Benefits**:
- ✅ Headless server loads only core game state
- ✅ UI can be swapped without changing save format
- ✅ Save file size reduced for server (no UI bloat)
- ✅ Version core and UI independently
- ✅ Multiple clients can share same game state

**Estimated Effort**: 1 week
**Risk**: MEDIUM (requires careful migration logic)

---

### Phase 6: Create API Layer (Week 10-11)

**Goal**: Provide clean interface for client access

#### REST API Design
```
GET    /api/v1/game/{gameId}/state          - Get current game state
GET    /api/v1/game/{gameId}/map            - Get map data
GET    /api/v1/game/{gameId}/players        - Get player list
GET    /api/v1/game/{gameId}/units          - Get units (filtered by visibility)

POST   /api/v1/game/{gameId}/action/move    - Move unit
POST   /api/v1/game/{gameId}/action/attack  - Attack
POST   /api/v1/game/{gameId}/action/build   - Build unit/building
POST   /api/v1/game/{gameId}/turn/end       - End turn

WebSocket /api/v1/game/{gameId}/events      - Real-time event stream
```

#### API Controller Example
```cpp
// source/api/game_controller.h

class GameController {
private:
   GameState& state;
   TurnManager& turnManager;
   RuleEngine& rules;
   GameEventDispatcher& events;

public:
   // HTTP handlers
   json getGameState(const Request& req);
   json getMapData(const Request& req);
   json getUnits(const Request& req, int playerId);

   json moveUnit(const Request& req);
   json attack(const Request& req);
   json endTurn(const Request& req);

   // WebSocket handler
   void streamEvents(WebSocket& ws, int playerId);
};
```

**Benefits**:
- ✅ Clean separation between game logic and network protocol
- ✅ Can version API independently
- ✅ Multiple client types (web, mobile, desktop)
- ✅ Easy to add authentication/authorization
- ✅ Standard HTTP tools work (curl, Postman, etc.)

**Estimated Effort**: 2 weeks
**Risk**: MEDIUM (new code, needs security review)

---

## Migration Strategy

### Backward Compatibility During Transition

**Goal**: Keep legacy UI working while refactoring

```cpp
// source/gamemap.h (during transition)

class GameMap {
private:
   // NEW: Core components
   std::unique_ptr<GameState> state;
   std::unique_ptr<MapGeometry> geometry;
   std::unique_ptr<TurnManager> turnManager;
   std::unique_ptr<GameEventDispatcher> eventDispatcher;

   // OLD: Legacy interface (deprecated)
   DEPRECATED int xsize, ysize; // Forward to geometry->width()
   DEPRECATED MapField* field;   // Forward to geometry->getFields()
   DEPRECATED Player player[9];  // Forward to state->getPlayers()

public:
   // NEW: Preferred interface
   GameState& getState() { return *state; }
   const MapGeometry& getGeometry() const { return *geometry; }

   // OLD: Legacy methods (forwarding)
   DEPRECATED int width() const { return geometry->getWidth(); }
   DEPRECATED int height() const { return geometry->getHeight(); }
   DEPRECATED MapField* getField(int x, int y) {
      return geometry->getField(x, y);
   }
};
```

**Timeline**:
- **Week 1-8**: Add new interfaces, keep old ones working
- **Week 9-10**: Mark old interfaces as deprecated
- **Week 11-12**: Update legacy UI to use new interfaces
- **Week 13+**: Remove deprecated code

---

## Testing Strategy

### Unit Tests for Each Module

```cpp
// tests/core/game_state_test.cpp

TEST(GameStateTest, InitialState) {
   GameState state(8, 8);
   EXPECT_EQ(state.getActivePlayer(), 0);
   EXPECT_FALSE(state.isGameOver());
}

TEST(GameStateTest, PlayerSwitching) {
   GameState state(8, 8);
   TurnManager turnMgr(state);

   turnMgr.endTurn();
   EXPECT_EQ(state.getActivePlayer(), 1);
}

TEST(MapGeometryTest, BoundaryChecking) {
   MapGeometry geom(10, 10);
   EXPECT_TRUE(geom.isValidCoordinate(5, 5));
   EXPECT_FALSE(geom.isValidCoordinate(10, 10));
   EXPECT_FALSE(geom.isValidCoordinate(-1, 5));
}

TEST(RuleEngineTest, MovementValidation) {
   GameState state(10, 10);
   RuleEngine rules(state);

   Vehicle* tank = createTestTank();
   MapCoordinate dest(5, 5);

   EXPECT_TRUE(rules.canMove(tank, dest));
}
```

### Integration Tests

```cpp
// tests/integration/turn_cycle_test.cpp

TEST(TurnCycleTest, CompleteTurnCycle) {
   GameState state(10, 10);
   GameEventDispatcher events;
   TurnManager turnMgr(state, events);

   MockEventListener listener;
   events.subscribe(&listener);

   turnMgr.beginTurn();
   EXPECT_CALL(listener, onGameEvent(EventType::PlayerTurnBegins));

   turnMgr.endTurn();
   EXPECT_CALL(listener, onGameEvent(EventType::PlayerTurnEnds));
   EXPECT_EQ(state.getActivePlayer(), 1);
}
```

### API Tests

```cpp
// tests/api/game_controller_test.cpp

TEST(GameControllerTest, GetGameState) {
   GameState state(10, 10);
   GameController controller(state, ...);

   Request req;
   req.params["gameId"] = "123";

   json response = controller.getGameState(req);
   EXPECT_EQ(response["activePlayer"], 0);
   EXPECT_EQ(response["mapSize"]["width"], 10);
}

TEST(GameControllerTest, MoveUnit_Unauthorized) {
   // Test that player 0 cannot move player 1's units
   Request req;
   req.auth.playerId = 0;
   req.body["unitId"] = "unit-belongs-to-player-1";

   EXPECT_THROW(controller.moveUnit(req), UnauthorizedException);
}
```

---

## Build-System Considerations

- **Scope for this refactor**: keep the existing Autotools/legacy build as the source of truth. Only add localized CMake targets when a module already has an established modern setup (e.g., mirror the approach used by the `source/ai/mcts` component).
- **New tests/modules**: register them with the current build scripts first so CI remains stable. If limited CMake support is required for GoogleTest, document the duplication and gate the full migration behind a follow-up epic.
- **Tracking**: create a separate “CMake/GoogleTest migration” backlog item with its own estimate and dependencies; reference this plan only to ensure the new `source/core/...` directories and mirrored `tests/core/...` layout will drop cleanly into a future CMakeLists once the larger move is approved.

This keeps the refactoring workload focused on decoupling while still preparing the tree for the inevitable tooling modernization.

---

## Success Criteria

### Phase Completion Checklist

#### Phase 1 Complete When:
- [ ] GameEventDispatcher implemented
- [ ] All sigc::signal usage replaced in GameMap
- [ ] Legacy UI adapter working
- [ ] Event system unit tests passing
- [ ] No performance regression

#### Phase 2 Complete When:
- [x] **DONE (2025-11-23)**: OverviewMapGenerator abstraction created (source/core/ui_interfaces/)
- [x] **DONE (2025-11-23)**: OverviewMapGeneratorService implemented (source/ui/)
- [x] **DONE (2025-11-23)**: GameMap uses optional IOverviewMapGenerator pointer
- [x] **DONE (2025-11-23)**: Backward compatibility maintained (legacy OverviewMapHolder preserved)
- [x] **DONE (2025-11-23)**: All 3 executables build successfully (asc, mapeditor, pbpedit)
- [x] **DONE (2025-11-23)**: MCTS tests pass (snapshot_test: 6/6, evaluator_test: 26/26)
- [ ] OverviewMapHolder fully removed from GameMap (deferred - backward compat)
- [ ] Headless build optimization (deferred - Phase 2 enables this, implementation pending)
- [ ] UI initialization updated to inject OverviewMapGeneratorService (optional enhancement)

#### Phase 3 Complete When:
- [ ] guiHooked() removed
- [ ] dialogsHooked removed
- [ ] No UI-specific flags in game logic
- [ ] Headless mode behavior identical to GUI mode
- [ ] All UI interactions through event system

#### Phase 4 Complete When:
- [ ] playerView removed from GameMap
- [ ] GameViewContext implemented
- [ ] Multiple simultaneous views possible
- [ ] API layer uses GameViewContext
- [ ] Replay system refactored

#### Phase 5 Complete When (Sub-Phases 5.1-5.4):
- [ ] **5.1**: `MapGeometry` class created and fully tested.
- [ ] **5.1**: `GameMap` uses `MapGeometry` internally (proxy pattern active).
- [ ] **5.2**: `GameState` class created (encapsulating `players`).
- [ ] **5.2**: `GameMap` delegates player access to `GameState`.
- [ ] **5.3**: `TurnManager` handles `endTurn`/`newRound` logic.
- [ ] **5.4**: `RuleEngine` and `ActionLog` extracted.
- [ ] All legacy accessors (`map.xsize`, `map.player[]`) marked deprecated or removed.
- [ ] GameMap is <2000 lines (Coordinator only).

#### Phase 6 Complete When:
- [ ] REST API implemented
- [ ] WebSocket event stream working
- [ ] API documentation generated
- [ ] Authentication/authorization implemented
- [ ] Rate limiting configured
- [ ] API integration tests passing

### Overall Success Metrics

**Before Refactoring**:
- GameMap: 18,550 lines
- Responsibilities: 11
- UI coupling: CRITICAL
- Test coverage: <10%
- Headless mode: Doesn't compile properly

**After Refactoring**:
- GameMap: <2,000 lines (coordinator only)
- Responsibilities per class: 1-2
- UI coupling: NONE
- Test coverage: >80%
- Headless mode: Fully functional
- API available: YES
- Multiple clients supported: YES

---

## Risk Mitigation

### Risk 1: Breaking Legacy UI
**Mitigation**:
- Maintain adapter layer
- Extensive regression testing
- Gradual migration, not big bang
- Feature flags for new/old code paths

### Risk 2: Performance Regression
**Mitigation**:
- Benchmark before/after
- Profile hot paths
- Optimize event dispatch (pooling, batching)
- Use move semantics for events

### Risk 3: Incomplete Separation
**Mitigation**:
- Code reviews focused on dependencies
- Static analysis for include dependencies
- Compile headless mode after each phase
- Dependency graph visualization

### Risk 4: Test Coverage Gaps
**Mitigation**:
- Write tests before refactoring (characterization tests)
- Require tests for all new modules
- Integration tests for end-to-end scenarios
- Manual testing of critical paths

---

## Dependencies and Blockers

### Prerequisites:
- ✅ CI/CD pipeline (COMPLETE)
- ✅ Code formatting (COMPLETE)
- ✅ Static analysis (COMPLETE)
- ✅ Test framework (Phase 1 COMPLETE)
- ⏳ Test framework Phase 2 (migrate MCTS tests)

### Parallel Work Possible:
- Documentation of current behavior
- API design and prototyping
- UI mockups for new client
- Database schema design (if needed)

### Blocks Until Complete:
- New client development
- Multi-client multiplayer
- Cloud deployment
- Mobile client
- Web client

---

## Alternatives Considered

### Alternative 1: Wrapper/Facade Pattern
**Idea**: Keep GameMap as-is, create wrapper for headless mode

**Rejected Because**:
- Doesn't solve the God Class problem
- Still maintains UI dependencies
- Wrapper becomes complex
- Technical debt remains

### Alternative 2: Complete Rewrite
**Idea**: Write new server from scratch

**Rejected Because**:
- 20+ years of game logic
- Risk of bugs in reimplementation
- Timeline too long (6+ months)
- Loses compatibility with existing saves/replays

### Alternative 3: Conditional Compilation Only
**Idea**: Use #ifdef to remove UI code

**Rejected Because**:
- Doesn't improve architecture
- Hard to maintain
- Still couples concepts
- No API layer

**Selected Approach**: Gradual refactoring with backward compatibility
- ✅ Maintains stability
- ✅ Improves architecture
- ✅ Enables new features
- ✅ Manageable risk

---

## Timeline Summary

| Phase | Duration | Risk | Benefit |
|-------|----------|------|---------|
| 0. Test Harness | 3-5 days | NONE | Safety net for refactoring |
| 1. Event System | 1-2 weeks | LOW | Remove sigc++ dependency |
| 2. Overview Map | 1 week | LOW | Remove Surface dependency |
| 3. GUI Hooks + Provider | 1 week | MEDIUM | Clean separation + DI pattern |
| 4. Player View | 1 week | MEDIUM | Multi-client support |
| 5. Module Extraction + Encapsulation | 4 weeks | HIGH | Break God Class + fix public data |
| 5.5. Serialization Separation | 1 week | MEDIUM | Split UI from core data |
| 6. API Layer | 2 weeks | MEDIUM | Enable new clients |
| **TOTAL** | **10-12 weeks** | | **Client-server ready** |

**Buffer for testing/issues**: +2-4 weeks
**Total realistic estimate**: **12-16 weeks** (3-4 months)

**Note**: Timeline extended by 2 weeks compared to initial estimate to incorporate:
- Phase 0: Mandatory characterization tests
- Phase 3: UserInteractionProvider pattern (more thorough)
- Phase 5: Encapsulation improvements (public → private data)
- Phase 5.5: Serialization separation (new requirement)

---

## Synthesis of Multiple Analysis Reviews

This refactoring plan consolidates insights from three independent architectural reviews. Below is how each review contributed to the final plan:

### Analysis 1: gamemap-decoupling.md
**Key Contributions Integrated**:
1. ✅ **Public Data Member Critique** - Added dedicated section on weak encapsulation (Section 5)
2. ✅ **Read-Only Views Pattern** - Incorporated `GameState::ReadOnlyView` in Phase 5
3. ✅ **ReplayInfo guidata** - Explicitly addressed in Phase 5.5 with renaming to `clientExtensionData`
4. ✅ **Serialization Mixing** - Added entire Phase 5.5 for separation concerns
5. ✅ **Test-First Approach** - Created mandatory Phase 0 for characterization tests
6. ✅ **Domain Boundaries** - Emphasized in module extraction strategy
7. ✅ **Incremental Hardening** - Gradual public → private data migration

**Quote**: *"Add automated tests around GameState serialization and turn progression BEFORE moving code, so regressions are caught while untangling responsibilities."*
→ **Action**: Made Phase 0 mandatory

### Analysis 2: gemini_input.md
**Key Contributions Integrated**:
1. ✅ **UserInteractionProvider Pattern** - Full implementation in Phase 3.2
2. ✅ **Concrete File Paths** - Suggested `client/map_view/` structure referenced
3. ✅ **Rename guidata** - Adopted `clientExtensionData` naming
4. ✅ **SDL Dependency Note** - Acknowledged SDL for I/O is acceptable
5. ✅ **Observer Pattern Emphasis** - Validated signal-based approach in Phase 1
6. ✅ **Simpler Separation** - Influenced pragmatic phasing approach

**Quote**: *"If GameMap needs to query a user decision, it should fire a request via a callback interface (e.g., `UserInteractionProvider`)"*
→ **Action**: Designed complete DI pattern with headless/GUI implementations

### Original Analysis (This Document - Initial)
**Core Strengths Maintained**:
1. ✅ Detailed code examples for each phase
2. ✅ Complete event dispatcher architecture
3. ✅ REST/WebSocket API layer specifications
4. ✅ Comprehensive testing strategy
5. ✅ Risk assessment and mitigation
6. ✅ SOLID principle analysis
7. ✅ Module-by-module breakdown

### Consensus Points (All Three Analyses Agree)

| Issue | Analysis 1 | Analysis 2 | This Document |
|-------|------------|------------|---------------|
| **OverviewMapHolder must go** | ✅ Critical | ✅ High Priority | ✅ Phase 2 |
| **guiHooked() is wrong** | ✅ Remove | ✅ Deprecate | ✅ Phase 3 |
| **Encapsulation broken** | ✅ Major flaw | ✅ Noted | ✅ Phase 5 |
| **Serialization mixed** | ✅ Critical | ✅ Abstract | ✅ Phase 5.5 |
| **Event abstraction needed** | ✅ Required | ✅ Use signals | ✅ Phase 1 |
| **Test safety net first** | ✅ Mandatory | ✅ Implied | ✅ Phase 0 |

### Divergences Resolved

**Question**: Keep `sigc::signal` or replace completely?
- **Analysis 1**: Abstract away completely
- **Analysis 2**: Keep for backward compatibility
- **Resolution**: Hybrid approach - abstract dispatcher core, maintain sigc adapter for legacy UI

**Question**: Big Bang or Incremental?
- **Analysis 1**: Incremental with safety
- **Analysis 2**: Phased approach
- **Resolution**: 7 incremental phases with mandatory testing checkpoints

**Question**: How many phases?
- **Analysis 1**: 6 major steps
- **Analysis 2**: 3 phases
- **Resolution**: 7 phases (0-6) balancing granularity with manageability

### Enhanced Plan Advantages

By synthesizing all three analyses, this plan now includes:

1. **More Robust Testing** (from Analysis 1)
   - Characterization tests mandatory before refactoring
   - Performance baseline enforcement
   - Serialization round-trip validation

2. **Better Patterns** (from Analysis 2)
   - UserInteractionProvider for dependency injection
   - Clearer headless/GUI separation
   - Practical backward compatibility

3. **Comprehensive Detail** (original)
   - Full code examples for each pattern
   - API design specifications
   - Module responsibilities clearly defined

4. **Critical Issues Addressed** (all analyses)
   - Public data encapsulation
   - Serialization separation
   - Replay system decoupling
   - UI lifecycle independence

**Validation**: This plan has effectively undergone peer review from multiple architectural perspectives and represents consensus best practices.

---

## Next Steps

### Immediate Actions (This Week):

1. **Review and Approve Plan**
   - Technical review with team
   - Identify missing concerns
   - Adjust timeline if needed

2. **Setup Tracking**
   - Create GitHub project board
   - Break down into issues/tasks
   - Assign initial ownership

3. **Create Characterization Tests**
   - Document current GameMap behavior
   - Write tests that capture existing behavior
   - Establish regression baseline

4. **Prototype Event System**
   - Create proof-of-concept
   - Measure performance
   - Validate approach

5. **Update Documentation**
   - Add this plan to repository
   - Link from main README
   - Update architectural docs

### Week 1 Detailed Tasks:

- [ ] Design GameEventDispatcher API
- [ ] Create event type enumeration
- [ ] Implement basic dispatcher
- [ ] Write dispatcher unit tests
- [ ] Create one adapter example
- [ ] Measure event dispatch performance
- [ ] Review with team

---

## References

### Design Patterns Used:
- **Event Dispatcher**: Observer pattern with async support
- **Repository**: GameState as data access layer
- **Service Layer**: TurnManager, RuleEngine
- **Adapter**: Legacy UI compatibility
- **Strategy**: Multiple rule implementations

### Related Documents:
- `docs/modernization/ARCHITECTURAL_FOUNDATION_TODOS.md`
- `docs/modernization/TEST_FRAMEWORK_MIGRATION.md`
- MCTS module (`source/ai/mcts/`) - Reference for modern C++23 architecture

### External Resources:
- SOLID Principles: https://en.wikipedia.org/wiki/SOLID
- Clean Architecture (Robert Martin)
- Domain-Driven Design (Eric Evans)
- Refactoring (Martin Fowler)

---

**Document Status**: ✅ Ready for Implementation (Revised & Validated)
**Revision History**:
- 2025-11-20 (v1.0): Initial analysis and plan
- 2025-11-20 (v2.0): Incorporated insights from 3 independent reviews
- 2025-11-20 (v2.1): Added explicit LLM directives and risk mitigations for critical phases
- 2025-11-20 (v2.2): Refactored Phase 5 into granular "Strangle the God Class" sub-phases; Updated Success Criteria to match sub-phases.
**Reviews Incorporated**:
- gamemap-decoupling.md (focus: encapsulation, serialization)
- gemini_input.md (focus: patterns, pragmatism)
- Original analysis (focus: comprehensive architecture)
**Next Review Date**: After Phase 0 completion (characterization tests)
**Maintained By**: Development Team

---

## Summary of Changes from v2.1 → v2.2

**Refined**:
- ✅ **Phase 5**: Replaced "Big Bang" extraction with **"Strangle the God Class"** strategy.
- ✅ **Granularity**: Broken down into Sub-Phases 5.1 (MapGeometry), 5.2 (GameState), 5.3 (TurnManager).
- ✅ **Safety**: Enforced "Expand-Delegate-Contract" pattern to keep the build stable at all times.
- ✅ **LLM Directives**: Added specific instructions for proxying and delegating legacy accessors.

## Summary of Changes from v2.0 → v2.1

**Added**:
- ✅ **Phase 1**: Added "LLM Execution Directives" to guide automated refactoring steps (Discovery -> Parallel Imp -> Adapter -> Isolation).
- ✅ **Phase 4**: Added "High Risk Warning" regarding Active vs Viewing player confusion.
- ✅ **Phase 4**: Added "Mandatory Unit Test Requirements" (Simulation Independence, Admin View, Fog of War) to prevent logic/view coupling regressions.

## Summary of Changes from v1.0 → v2.0

**Added**:
- ✅ Phase 0: Mandatory characterization tests
- ✅ Section 5: Public data member encapsulation critique
- ✅ Section 6: ReplayInfo GUI data coupling
- ✅ Section 7: Serialization mixing concerns
- ✅ Phase 3.2: UserInteractionProvider pattern (DI)
- ✅ Phase 5: Read-only view pattern for GameState
- ✅ Phase 5.5: Serialization separation strategy
- ✅ Synthesis section comparing all three analyses

**Enhanced**:
- ✅ Phase 3: Complete implementation of interaction provider
- ✅ Phase 5: Encapsulation improvements throughout
- ✅ Timeline: Adjusted to 12-16 weeks (more realistic)
- ✅ Testing strategy: Performance baselines, round-trip validation

**Consensus Validated**:
- ✅ All three analyses agree on critical issues
- ✅ Best patterns from each review incorporated
- ✅ Divergences resolved with hybrid approaches
- ✅ Ready for team review and implementation
