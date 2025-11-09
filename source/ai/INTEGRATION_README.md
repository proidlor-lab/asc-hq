# MCTS AI Integration Guide

**Created**: 2025-11-08  
**Purpose**: Guide for integrating MCTS AI into ASC

---

## Overview

This directory contains a modular integration skeleton that allows ASC to use different AI implementations through a factory pattern. It provides clean separation between modern MCTS code and legacy ASC code.

---

## Architecture

### Component Structure

```
source/ai/
├── ai_factory.h              ✅ NEW: Factory for creating AI instances
├── ai_factory.cpp            ✅ NEW: Factory implementation
├── mcts_ai.h                 ✅ NEW: MCTS AI wrapper (implements BaseAI)
├── mcts_ai.cpp               ✅ NEW: MCTS AI implementation
├── ai.h                      (existing: Classic rule-based AI)
├── ai.cpp                    (existing: Classic AI implementation)
│
└── mcts/                     ✅ MCTS implementation
    ├── core/                 (MCTS algorithm)
    ├── domain/               (Game state, actions, evaluation)
    ├── infrastructure/       ✅ NEW: Integration support
    │   ├── legacy_game_interface.h    ✅ NEW: Adapter for legacy code
    │   └── mcts_config_loader.h       ✅ NEW: Configuration system
    │
    ├── INTEGRATION_NOTES.md  ✅ NEW: Detailed integration documentation
    └── mcts_ai_profiles.ini  ✅ NEW: Configuration profiles
```

---

## Features

### 1. AI Factory Pattern

**File**: `ai_factory.h/cpp`

Create different AI types through a single interface:

```cpp
// Create classic AI
BaseAI* ai = AIFactory::createAI(AIFactory::AI_CLASSIC, gameMap, playerID);

// Create MCTS AI (balanced profile)
BaseAI* ai = AIFactory::createAI(AIFactory::AI_MCTS_BALANCED, gameMap, playerID);

// Create MCTS AI (aggressive profile)
BaseAI* ai = AIFactory::createAI(AIFactory::AI_MCTS_AGGRESSIVE, gameMap, playerID);
```

**Supported AI Types**:
- `AI_CLASSIC` - Original rule-based AI
- `AI_MCTS_BALANCED` - MCTS with balanced parameters
- `AI_MCTS_AGGRESSIVE` - MCTS tuned for offensive play
- `AI_MCTS_DEFENSIVE` - MCTS tuned for defensive play
- `AI_MCTS_FAST` - MCTS with reduced computation time
- `AI_MCTS_DEEP` - MCTS with deep search

### 2. Configuration Profiles

**File**: `mcts_ai_profiles.ini`

Customize MCTS AI behavior through INI files:

```ini
[custom_profile]
maxIterations=200
maxTimeMs=2000
rolloutDepthLimit=10
explorationConstant=1.414
materialWeight=2.0
positionWeight=1.0
healthWeight=1.5
threatWeight=1.2
```

Load at runtime:
```cpp
MCTSConfig config = MCTSConfigLoader::loadConfig("custom_profile");
```

### 3. Legacy Code Isolation

**File**: `mcts/infrastructure/legacy_game_interface.h`

All legacy code interactions are isolated behind modern interfaces:

```cpp
auto legacyInterface = createLegacyGameInterface(gameMap);

// Modern C++ interface
if (legacyInterface->canMove(unitID, destination)) {
    legacyInterface->executeMove(unitID, destination);
}
```

**Benefits**:
- MCTS core code never touches legacy GameMap directly
- Easy to mock for testing
- Clear migration path when legacy code is modernized

---

## Integration Steps

### Step 1: Update Player Class

**File**: `source/player.h`

Add AI type field:
```cpp
class Player {
    BaseAI* ai;
    int aiType;  // NEW: Store AI type for persistence
    
    // NEW: Getter/setter
    int getAIType() const { return aiType; }
    void setAIType(int type) { aiType = type; }
};
```

Initialize in constructor:
```cpp
Player::Player() 
    : ai(nullptr)
    , aiType(0)  // Default to AI_CLASSIC
    // ...
{
}
```

### Step 2: Update AI Instantiation

**File**: `source/turncontrol.cpp` (line ~105)

**Before**:
```cpp
if (!actmap->player[actmap->actplayer].ai)
    actmap->player[actmap->actplayer].ai = new AI(actmap, actmap->actplayer);
```

**After**:
```cpp
if (!actmap->player[actmap->actplayer].ai) {
    AIFactory::AIType aiType = static_cast<AIFactory::AIType>(
        actmap->player[actmap->actplayer].getAIType()
    );
    actmap->player[actmap->actplayer].ai = 
        AIFactory::createAI(aiType, actmap, actmap->actplayer);
}
```

### Step 3: Update Save/Load System

**File**: `source/loaders.cpp` (line ~420)

**Before**:
```cpp
if (a & (1 << i)) {
    AI* ai = new AI(spfld, i);
    ai->read(*stream);
    spfld->player[i].ai = ai;
}
```

**After**:
```cpp
if (a & (1 << i)) {
    // Read AI type with backward compatibility
    int aiTypeValue = 0;
    if (saveFileVersion >= VERSION_WITH_AI_TYPES) {
        aiTypeValue = stream->readInt();
    }
    
    // Validate and create
    AIFactory::AIType aiType = AIFactory::isValidAIType(aiTypeValue)
        ? static_cast<AIFactory::AIType>(aiTypeValue)
        : AIFactory::getDefaultAIType();
    
    spfld->player[i].setAIType(aiTypeValue);
    spfld->player[i].ai = AIFactory::createAI(aiType, spfld, i);
    spfld->player[i].ai->read(*stream);
}
```

### Step 4: Update Save Format

**File**: `source/player.cpp`

Add to `Player::write()`:
```cpp
void Player::write(tnstream& stream) const {
    // ... existing code ...
    
    if (ai) {
        stream.writeInt(aiType);  // NEW: Save AI type
    }
}
```

Add to `Player::read()`:
```cpp
void Player::read(tnstream& stream) {
    // ... existing code ...
    
    if (version >= VERSION_WITH_AI_TYPE) {
        aiType = stream.readInt();  // NEW: Load AI type
    } else {
        aiType = 0;  // Default to classic for old saves
    }
}
```

### Step 5: Update Build System

**File**: `source/ai/Makefile.am`

Add new files:
```makefile
# MCTS AI integration
AI_SOURCES = \
    ai.cpp \
    ai_factory.cpp \
    mcts_ai.cpp

AI_HEADERS = \
    ai.h \
    ai_factory.h \
    mcts_ai.h

# MCTS library (already exists)
SUBDIRS = mcts
```

---

## Usage Examples

### Example 1: Set AI Type for a Player

```cpp
// In game setup
GameMap* map = /* ... */;
int playerID = 0;

// Set player to use MCTS AI
map->player[playerID].setAIType(AIFactory::AI_MCTS_BALANCED);

// AI will be created automatically when turn starts
```

### Example 2: Custom Configuration

```cpp
// Create custom MCTS AI
AIFactory::AIConfig config;
config.enableLogging = true;
config.difficultyLevel = 8;

BaseAI* ai = AIFactory::createAI(
    AIFactory::AI_MCTS_BALANCED,
    gameMap,
    playerID,
    config
);
```

### Example 3: Runtime Profile Switch

```cpp
// Get MCTS AI instance
MCTS_AI* mctsAI = dynamic_cast<MCTS_AI*>(player.ai);
if (mctsAI) {
    // Load custom profile
    auto profile = MCTS_AI::getProfileByName("aggressive");
    mctsAI->setProfile(profile);
}
```

### Example 4: Console Command for Testing

```cpp
// Add console command to switch AI
if (cmd.startsWith("setai ")) {
    int playerID = parsePlayerID(cmd);
    std::string aiType = parseAIType(cmd);
    
    // Delete old AI
    delete map->player[playerID].ai;
    
    // Create new AI
    AIFactory::AIType type = AIFactory::parseAIType(aiType);
    map->player[playerID].ai = AIFactory::createAI(type, map, playerID);
    map->player[playerID].setAIType(type);
    
    std::cout << "Player " << playerID << " AI changed to " << aiType << std::endl;
}
```

---

## Configuration Reference

### MCTS Parameters

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `maxIterations` | int | 200 | Maximum MCTS iterations per decision |
| `maxTimeMs` | int | 2000 | Maximum time in ms per decision |
| `rolloutDepthLimit` | int | 10 | Maximum depth for simulation rollouts |
| `explorationConstant` | double | 1.414 | UCB1 exploration constant (√2) |
| `earlyTerminationThreshold` | double | 0.95 | Confidence threshold for early stop |

### Evaluation Weights

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `materialWeight` | double | 2.0 | Weight for material advantage |
| `positionWeight` | double | 1.0 | Weight for positional advantage |
| `healthWeight` | double | 1.5 | Weight for unit health |
| `threatWeight` | double | 1.2 | Weight for threat assessment |

### Debug Options

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `enableLogging` | bool | false | Enable AI logging |
| `enableDebugOutput` | bool | false | Enable debug output |

---

## Testing

### Unit Tests

```bash
# Test AI factory
cd source/ai
make test_ai_factory

# Test MCTS AI wrapper
make test_mcts_ai
```

### Integration Test

```cpp
// Create test game
GameMap* map = createTestMap();

// Set player 0 to MCTS AI
map->player[0].setAIType(AIFactory::AI_MCTS_BALANCED);
map->player[0].stat = Player::computer;

// Set player 1 to Classic AI
map->player[1].setAIType(AIFactory::AI_CLASSIC);
map->player[1].stat = Player::computer;

// Run game
while (!map->isGameOver()) {
    runAITurn(map);
}

// Check results
std::cout << "Winner: " << map->getWinner() << std::endl;
```

### Manual Testing

1. **Start Game**: Launch ASC
2. **Setup**: Create 2-player game, set player 1 to MCTS AI
3. **Play**: Let AI play 10 turns
4. **Save**: Save game to file
5. **Load**: Reload game, verify AI type preserved
6. **Switch**: Use console command to switch AI type

---

## Troubleshooting

### Problem: MCTS AI not available

**Solution**: Check if MCTS compiled in:
```cpp
if (!AIFactory::isAITypeAvailable(AIFactory::AI_MCTS_BALANCED)) {
    std::cout << "MCTS AI not compiled. Use --enable-mcts-ai" << std::endl;
}
```

### Problem: AI crashes on creation

**Check**:
1. GameMap pointer is valid (not nullptr)
2. Player ID is in range (0-7)
3. MCTS components initialized

**Debug**:
```cpp
AIFactory::AIConfig config;
config.enableLogging = true;
config.enableDebugOutput = true;
BaseAI* ai = AIFactory::createAI(type, map, playerID, config);
```

### Problem: Old save files don't load

**Solution**: Ensure backward compatibility code is correct:
```cpp
// In loaders.cpp
int aiTypeValue = 0;  // Default to classic
if (saveFileVersion >= VERSION_WITH_AI_TYPES) {
    aiTypeValue = stream->readInt();
}
```

### Problem: MCTS AI too slow/fast

**Solution**: Adjust profile in `mcts_ai_profiles.ini`:
```ini
[custom_faster]
maxIterations=100  # Reduce for faster
maxTimeMs=1000
rolloutDepthLimit=5
```

---

## Performance Tuning

### Quick Decisions
```ini
maxIterations=50-100
maxTimeMs=500-1000
rolloutDepthLimit=3-5
```

### Balanced Quality
```ini
maxIterations=200-300
maxTimeMs=2000-3000
rolloutDepthLimit=10-15
```

### High Quality
```ini
maxIterations=500-1000
maxTimeMs=5000-10000
rolloutDepthLimit=20-30
```

---

## Next Steps

1. **Implement Legacy Interface**: Complete `legacy_game_interface.cpp`
2. **Implement MCTS Logic**: Complete `processUnits()` in `mcts_ai.cpp`
3. **Add Unit Tests**: Test all components
4. **Add UI**: Player setup dialog for AI selection
5. **Profile Performance**: Measure turn times, optimize

---

## See Also

- [INTEGRATION_NOTES.md](mcts/INTEGRATION_NOTES.md) - Detailed legacy code analysis
- [STATUS.md](mcts/STATUS.md) - Current implementation status
- [docs/implementation_roadmap.md](mcts/docs/implementation_roadmap.md) - Full roadmap

---

## Contact

For questions or issues with the integration skeleton, refer to:
- Project documentation in `source/ai/mcts/`
- ASC repository: https://github.com/ValHaris/asc-hq
