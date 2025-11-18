# MCTS AI Integration Options - Analysis & Recommendations

## Current State Analysis

### Existing AI Architecture

**✅ Good Design - Clean Polymorphic Architecture:**
```cpp
// Base interface (baseaiinterface.h)
class BaseAI {
public:
    virtual void run(MapDisplayInterface* mapDisplay) = 0;
    virtual bool isRunning() = 0;
    virtual VisibilityStates getVision() = 0;
    virtual void read(tnstream& stream) = 0;
    virtual void write(tnstream& stream) const = 0;
    virtual ~BaseAI() {};
};

// Player class (player.h)
class Player {
    BaseAI* ai;  // Polymorphic AI pointer
    enum PlayerStatus { human, computer, off, supervisor, suspended } stat;
    // ...
};
```

**Current Implementation:**
- Single AI type: `class AI : public BaseAI` (in `source/ai/ai.h`)
- Hardcoded instantiation in two places:
  1. `turncontrol.cpp:105` - Runtime creation
  2. `loaders.cpp:420` - Loading from saved games
- No AI type selection mechanism exists

### Assessment

**Strengths:**
✅ Clean polymorphic design via `BaseAI` interface  
✅ Proper separation of concerns  
✅ Easy to extend with new AI types  
✅ No major refactoring needed for core architecture

**Weaknesses:**
❌ No AI type selection/configuration system  
❌ Hardcoded `new AI()` in two locations  
❌ No persistence of AI type choice in save files  
❌ No user interface for AI selection

**Verdict: Architecture is SOLID - Only needs AI factory pattern**

---

## Integration Options

### Option 1: AI Factory with Configuration (RECOMMENDED ⭐)

**Approach:** Add an AI factory and player preference for AI type.

**Changes Required:**

1. **Create AI Factory** (`source/ai/ai_factory.h/cpp`):
```cpp
class AIFactory {
public:
    enum AIType {
        AI_CLASSIC = 0,  // Original AI
        AI_MCTS = 1,     // New MCTS AI
        // Future: AI_NEURAL, AI_MINIMAX, etc.
    };
    
    static BaseAI* createAI(AIType type, GameMap* map, int player);
    static AIType parseAIType(const std::string& name);
    static std::string getAITypeName(AIType type);
    static std::vector<std::string> getAvailableAINames();
};
```

2. **Add AI Type to Player** (`source/player.h`):
```cpp
class Player {
    BaseAI* ai;
    int aiType;  // AIFactory::AIType (persisted to save files)
    // ...
};
```

3. **Update Instantiation Points**:
```cpp
// turncontrol.cpp
if (!actmap->player[actmap->actplayer].ai) {
    auto type = static_cast<AIFactory::AIType>(
        actmap->player[actmap->actplayer].aiType
    );
    actmap->player[actmap->actplayer].ai = 
        AIFactory::createAI(type, actmap, actmap->actplayer);
}

// loaders.cpp
if (a & (1 << i)) {
    int aiType = stream->readInt();  // NEW: Read AI type
    spfld->player[i].ai = AIFactory::createAI(
        static_cast<AIFactory::AIType>(aiType), spfld, i
    );
    spfld->player[i].ai->read(*stream);
}
```

4. **Update Save/Load** (`gamemap.cpp`, `loaders.cpp`):
- Save AI type alongside AI state
- Load AI type before creating AI instance

5. **Add UI Selection** (optional, can default to classic):
- Player setup dialog: Dropdown for AI type
- Map editor: AI type per player
- In-game: "Change AI Type" option

**Pros:**
✅ Clean, extensible design  
✅ Backward compatible (default to classic AI)  
✅ User can choose AI per player  
✅ Proper persistence in save files  
✅ Easy to add more AI types later  
✅ Minimal code changes to existing system

**Cons:**
⚠️ Requires save file format update (handle with version check)  
⚠️ Need to update 4-5 files  
⚠️ Optional UI work for selection

**Effort:** Medium (4-6 hours)

---

### Option 2: Simple Global Configuration

**Approach:** Global preference for which AI to use (all computer players use same AI).

**Changes Required:**

1. **Add Global Setting** (`source/global.h`):
```cpp
extern int globalAIType;  // 0 = Classic, 1 = MCTS
```

2. **Update Configuration** (`source/sg.cpp` or preferences):
- Load from config file or command-line arg
- Default to classic (0)

3. **Update Instantiation**:
```cpp
// turncontrol.cpp & loaders.cpp
if (globalAIType == 1) {
    ai = new MCTS_AI(actmap, actmap->actplayer);
} else {
    ai = new AI(actmap, actmap->actplayer);
}
```

**Pros:**
✅ Extremely simple  
✅ No save file changes needed  
✅ Quick to implement (1-2 hours)  
✅ Easy to test different AIs

**Cons:**
❌ All computer players must use same AI  
❌ Not persistent per-player  
❌ Less flexible for future  
❌ Not extensible

**Effort:** Low (1-2 hours)

---

### Option 3: Runtime AI Switching

**Approach:** Allow changing AI type mid-game via console command.

**Changes Required:**

1. **Add Console Command**:
```cpp
// In console command handler
if (cmd.startsWith("setai ")) {
    int player = parsePlayer(cmd);
    std::string aiType = parseAIType(cmd);
    
    // Delete old AI
    delete actmap->player[player].ai;
    
    // Create new AI
    actmap->player[player].ai = AIFactory::createAI(aiType, actmap, player);
}
```

2. **No Persistence**: Doesn't save to file, just for testing

**Pros:**
✅ Perfect for development/testing  
✅ No save file changes  
✅ Easy AI comparison  
✅ Can combine with Option 1 or 2

**Cons:**
❌ Not persistent  
❌ Not user-friendly  
❌ Requires console access

**Effort:** Low (1 hour) - Good complement to other options

---

### Option 4: Map-Specific AI (Advanced)

**Approach:** Map designer specifies AI type in map file.

**Changes Required:**

1. **Add to Map Format** (`source/gamemap.h`):
```cpp
class GameMap {
    int playerAIType[8];  // Per-player AI type in map
    // ...
};
```

2. **Map Editor Support**: UI to set AI type per player

3. **Use When Loading Map**: Respect map's AI configuration

**Pros:**
✅ Map designers can choose AI  
✅ Different scenarios can test different AIs  
✅ Per-player granularity

**Cons:**
❌ Requires map format changes  
❌ Map editor work needed  
❌ More complex

**Effort:** High (8-10 hours)

---

## Recommended Implementation Plan

### Phase 1: Infrastructure (Option 1 - Factory Pattern) ⭐

**Step 1: Create AI Factory** (1-2 hours)
```
Files to create:
- source/ai/ai_factory.h
- source/ai/ai_factory.cpp
```

**Step 2: Create MCTS AI Wrapper** (2-3 hours)
```
Files to create:
- source/ai/mcts_ai.h      (BaseAI implementation)
- source/ai/mcts_ai.cpp    (Bridges to source/ai/mcts/*)
```

**Step 3: Update Instantiation Points** (1 hour)
```
Files to modify:
- source/turncontrol.cpp   (runai function)
- source/loaders.cpp       (readAI/writeAI functions)
```

**Step 4: Add Player AI Type Field** (1 hour)
```
Files to modify:
- source/player.h          (Add aiType field)
- source/player.cpp        (Initialize, save/load)
- source/gamemap.cpp       (Handle in serialization)
```

**Total Effort: ~5-7 hours**

### Phase 2: User Interface (Optional, Later)

**Step 1: Player Setup Dialog** (2-3 hours)
- Add AI type dropdown
- Default to Classic for backward compatibility

**Step 2: In-Game Commands** (1 hour)
- Console command to switch AI
- Debug/testing support

**Total Effort: ~3-4 hours**

### Phase 3: Testing & Polish (1-2 hours)

- Test save/load with both AI types
- Test AI switching
- Backward compatibility testing
- Documentation

---

## MCTS AI Wrapper Design

### Interface Implementation

```cpp
// source/ai/mcts_ai.h
#ifndef MCTS_AI_H
#define MCTS_AI_H

#include "../baseaiinterface.h"
#include "mcts/core/mcts_search.h"
#include "mcts/domain/i_tactical_evaluator.h"
#include <memory>

class MCTS_AI : public BaseAI {
private:
    GameMap* gameMap;
    int playerID;
    
    // MCTS components (lazy-initialized)
    std::unique_ptr<asc::mcts::ITacticalEvaluator> evaluator;
    std::unique_ptr<asc::mcts::MCTSSearch> search;
    
    // Configuration
    asc::mcts::MCTSConfig config;
    
    // State
    bool running;
    VisibilityStates vision;
    
    // Internal methods
    void initialize();
    void processUnits();
    void executeBestAction(const asc::mcts::MCTSResult& result);
    
public:
    MCTS_AI(GameMap* map, int player);
    
    // BaseAI interface
    void run(MapDisplayInterface* mapDisplay) override;
    bool isRunning() override { return running; }
    VisibilityStates getVision() override { return vision; }
    void read(tnstream& stream) override;
    void write(tnstream& stream) const override;
    
    ~MCTS_AI() override = default;
};

#endif // MCTS_AI_H
```

### Key Design Decisions

1. **Lazy Initialization**: Create MCTS components in `run()` first call
2. **Incremental Integration**: Start simple, enhance over time
3. **Configuration**: Use reasonable defaults, tune later
4. **State Management**: MCTS is stateless between turns (fresh tree each turn)

---

## Backward Compatibility Strategy

### Save File Versioning

```cpp
// When writing AI
void tgameloaders::writeAI() {
    int aiMask = 0;
    for (int i = 0; i < 8; i++)
        if (spfld->player[i].ai)
            aiMask |= (1 << i);
    
    stream->writeInt(aiMask);
    stream->writeInt(SAVE_VERSION);  // NEW: Write version
    
    for (int i = 0; i < 8; i++) {
        if (spfld->player[i].ai) {
            stream->writeInt(spfld->player[i].aiType);  // NEW: Write AI type
            spfld->player[i].ai->write(*stream);
        }
    }
}

// When reading AI
void tgameloaders::readAI() {
    int aiMask = stream->readInt();
    
    // Check for version marker
    int version = 0;
    if (stream->has_more_data()) {
        version = stream->readInt();
    }
    
    for (int i = 0; i < 8; i++) {
        if (aiMask & (1 << i)) {
            int aiType = 0;  // Default to classic
            if (version >= NEW_VERSION) {
                aiType = stream->readInt();  // NEW: Read AI type
            }
            
            spfld->player[i].ai = AIFactory::createAI(
                static_cast<AIFactory::AIType>(aiType), spfld, i
            );
            spfld->player[i].ai->read(*stream);
        }
    }
}
```

### Fallback Strategy

- Old saves: No AI type field → Default to Classic AI (0)
- New saves with Classic AI: Works identically to old saves
- New saves with MCTS AI: Loads MCTS AI, if unavailable → fallback to Classic
- Version check prevents reading garbage data

---

## Configuration & Tuning

### MCTS AI Configuration

```cpp
// source/ai/mcts_ai.cpp - Default configuration
void MCTS_AI::initialize() {
    // Create evaluator
    evaluator = asc::mcts::EvaluatorFactory::createSimpleCombatEvaluator();
    
    // Configure MCTS
    config.maxIterations = 200;     // Tune: 100-1000
    config.maxTimeMs = 2000;        // 2 seconds per move
    config.rolloutDepthLimit = 10;  // Look ahead 10 moves
    config.explorationConstant = 1.414;  // sqrt(2) - standard UCB1
    config.earlyTerminationThreshold = 0.95;  // 95% confidence
    
    // Create search engine
    search = std::make_unique<asc::mcts::MCTSSearch>(
        evaluator->clone(), config
    );
}
```

### Tunables (via config file or preferences):

```ini
[MCTS_AI]
iterations=200
time_limit_ms=2000
rollout_depth=10
exploration=1.414
early_termination=0.95
enable_logging=false
```

---

## Testing Strategy

### Unit Tests
- AI factory creation
- MCTS AI instantiation
- Save/load with both AI types
- AI type switching

### Integration Tests
1. **Single Player**: MCTS vs Classic
2. **Multi-Player**: Mixed AI types
3. **Save/Load**: Round-trip test
4. **Performance**: Time per turn
5. **Correctness**: Legal moves only

### Manual Testing
1. Start game with MCTS AI
2. Let it play 10 turns
3. Save game
4. Load game, verify AI type preserved
5. Switch AI type mid-game (console)
6. Compare quality vs Classic AI

---

## Migration Path

### Step 1: Add Factory (No Breaking Changes)
- Create factory, default to Classic AI
- Existing saves work identically
- No user-visible changes

### Step 2: Add MCTS AI (Opt-In)
- MCTS AI available via factory
- Default still Classic
- Users can test via console command

### Step 3: Add Persistence (Optional Breaking)
- Save AI type to files
- Backward compatible loading
- Forward compatible (old ASC loads, uses Classic)

### Step 4: Add UI (Polish)
- Player setup dialog
- Map editor support
- User-friendly selection

### Step 5: Tune & Optimize
- Performance profiling
- Parameter tuning
- Quality assessment

---

## Recommendation Summary

**Implement Option 1 (AI Factory) with this priority:**

1. **Phase 1 (Essential)**: AI Factory + MCTS AI Wrapper
   - ~5-7 hours work
   - Fully functional, testable
   - Backward compatible
   - Extensible for future

2. **Phase 2 (Nice-to-Have)**: UI Selection
   - ~3-4 hours work
   - User-friendly
   - Can be deferred

3. **Phase 3 (Optional)**: Advanced Features
   - Per-map AI types
   - AI comparison tools
   - Performance dashboards

**Total MVP: 5-7 hours for fully functional MCTS AI as selectable computer player**

The existing architecture is **excellent** and requires **no refactoring** - just extension via the factory pattern. This is a testament to good original design!
