# MCTS AI - Manual Integration Test Guide

This guide explains how to manually test the MCTS AI engine with real game state in ASC.

## Quick Start

### Option 1: From C++ Code

Add this to any place in the ASC codebase where you have access to `actmap`:

```cpp
#include "ai/mcts/mcts_manual_test.h"

// Quick sanity check (no game state needed)
asc::mcts::mctsQuickSanityCheck();

// Full test with current game state
asc::mcts::runMCTSManualTest(actmap);

// Custom parameters
asc::mcts::runMCTSManualTest(actmap, 200, 0);  // 200 iterations, player 0
```

### Option 2: From Lua Console (if ASC has Lua bindings)

```lua
-- Bind the function first (in ASC Lua initialization)
-- Then call from console:
mctsTest()
```

### Option 3: Via Debug Command

Add a debug command in ASC's command system:

```cpp
// In commands.cpp or similar
#include "ai/mcts/mcts_manual_test.h"

class MCTSTestCommand : public Command {
public:
    void execute() override {
        asc::mcts::runMCTSManualTest(actmap, 100);
    }
};
```

## Example Integration Points

### 1. In Player Turn Start

Test MCTS at the beginning of the player's turn:

```cpp
// In turn processing code
void GameMap::nextTurn() {
    // ... existing code ...
    
    #ifdef DEBUG_MCTS
    if (actplayer == 0) {  // Only for player 0
        asc::mcts::runMCTSManualTest(this, 50);
    }
    #endif
}
```

### 2. In AI Decision Point

Replace or test alongside existing AI:

```cpp
// In AI module
void AI::makeDecision() {
    // Test MCTS recommendation
    asc::mcts::runMCTSManualTest(gameMap, 200, playerID);
    
    // Then run normal AI or compare results
    // ... existing AI code ...
}
```

### 3. Via Console Command

Add to console command processor:

```cpp
// In console.cpp
void Console::processCommand(const std::string& cmd) {
    if (cmd == "mcts-test") {
        asc::mcts::runMCTSManualTest(actmap);
        return;
    }
    
    if (cmd == "mcts-quick") {
        asc::mcts::mctsQuickSanityCheck();
        return;
    }
    
    // ... other commands ...
}
```

## Output Example

When you run the test, you'll see output like:

```
╔═══════════════════════════════════════════════════════════╗
║         MCTS AI - Manual Integration Test                ║
╚═══════════════════════════════════════════════════════════╝

Testing for Player: 0
Map Size: 100x100
Player 0 units: 12
Total units on map: 24

1. Creating game state snapshot...
   ✓ Snapshot created: 24 units

2. Creating tactical evaluator...
   ✓ SimpleCombatEvaluator created

3. Configuring MCTS search...
   - Max iterations: 100
   - Time limit: 5000 ms
   - Rollout depth: 10
   - Exploration: 1.414

4. Running MCTS search...
   ✓ Search completed in 235 ms

═══════════════════════════════════════════════════════════
                    SEARCH RESULTS                         
═══════════════════════════════════════════════════════════

Search Statistics:
  Iterations run:    100 / 100
  Nodes expanded:    156
  Total rollouts:    100
  Search time:       235.000 ms
  Tree depth:        8
  Tree nodes:        157

✓ BEST ACTION FOUND:
  Visits:            23
  Average value:     0.245
  Win rate:          62.3%

  Action Type:       MOVE
  Unit ID:           42 (Tank)
  Target position:   (45, 32)

═══════════════════════════════════════════════════════════
✓ TEST COMPLETED SUCCESSFULLY
═══════════════════════════════════════════════════════════
```

## Parameters

### runMCTSManualTest Parameters

```cpp
bool runMCTSManualTest(
    GameMap* gameMap,      // Current game state
    int iterations = 100,  // Number of MCTS iterations (more = better but slower)
    int playerID = -1      // Player to search for (-1 = current player)
);
```

**Recommended iteration counts:**
- **Quick test**: 50-100 iterations (~100-200ms)
- **Normal quality**: 200-500 iterations (~400ms-1s)
- **High quality**: 1000+ iterations (1-3s)

## Troubleshooting

### Build Issues

If you get linker errors, make sure the MCTS library is linked:

```makefile
# In your Makefile
your_binary_LDADD = ... $(top_builddir)/source/ai/mcts/libmcts.la
```

### Runtime Issues

**"ERROR: gameMap is null"**
- Make sure you're passing a valid GameMap pointer

**"WARNING: Player has no units to move"**
- The current player has no units (game over or testing wrong player)

**"NO ACTION FOUND"**
- No legal moves available
- Or search failed to explore any promising branches
- Try increasing iterations

### Performance Notes

- First call may be slower due to initialization
- Subsequent calls reuse some cached data
- Large maps (>150x150) may take longer to snapshot
- Many units (>100) will slow down action generation

## Next Steps

1. **Start simple**: Run `mctsQuickSanityCheck()` first
2. **Test in-game**: Add `runMCTSManualTest()` to a safe location
3. **Verify output**: Check console for detailed statistics
4. **Compare with AI**: See how MCTS decisions compare to current AI
5. **Tune parameters**: Adjust iterations/depth for your use case

## Advanced: Custom Test Scenarios

You can create specific test scenarios:

```cpp
// Test defensive scenario
MCTSConfig config;
config.maxIterations = 200;
config.rolloutDepthLimit = 15;  // Look further ahead

// Create search with custom config
auto evaluator = asc::mcts::EvaluatorFactory::createSimpleCombatEvaluator();
asc::mcts::MCTSSearch search(std::move(evaluator), config);

// Create snapshot and run
auto reader = asc::mcts::GameStateReader(gameMap);
auto snapshot = reader.createFullSnapshot(0);
auto result = search.search(*snapshot, 0);

// Analyze results programmatically
if (result.bestAction.has_value()) {
    // Use result.bestAction
}
```

## Safety

- ✅ **Safe to call anytime** - Read-only on game state
- ✅ **No side effects** - Doesn't modify the game
- ✅ **Catch exceptions** - All errors are caught and reported
- ⚠️ **Can be slow** - Don't call in tight loops or time-critical code
- ⚠️ **Debug builds** - Better for development/testing than release

## Questions?

See the source code:
- `source/ai/mcts/mcts_manual_test.h` - Interface
- `source/ai/mcts/mcts_manual_test.cpp` - Implementation
- `source/ai/mcts/core/mcts_search.h` - MCTS engine
