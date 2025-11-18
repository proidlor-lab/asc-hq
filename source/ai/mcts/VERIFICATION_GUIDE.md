# How to Verify MCTS AI is Being Invoked

**Status**: ✅ Enhanced logging added  
**Date**: 2025-11-08

---

## ⚠️ Important: Phase 1.1b MVP Behavior

### Why All MCTS Variants Behave Identically

**This is expected in Phase 1.1b!**

All MCTS AI variants (Balanced, Aggressive, Defensive, Fast, Deep) currently use **the same simple heuristic algorithm** and will behave identically. This is by design for the MVP integration phase.

**Current Algorithm (All MCTS Variants)**:
```
For each unit:
  1. Find enemies within 10-hex radius
  2. If enemy in attack range → attack first enemy
  3. Otherwise → wait (end turn)
```

**Phase 1.2+**: Each variant will use actual MCTS search with different parameters:
- **Balanced**: 200 iterations, moderate exploration
- **Aggressive**: High exploration, offense-focused evaluation
- **Defensive**: Low exploration, defense-focused evaluation  
- **Fast**: 100 iterations (quick decisions)
- **Deep**: 500 iterations (stronger play)

---

## How to Verify MCTS AI is Actually Running

### Method 1: Check Log Output (Best)

I've added enhanced logging to confirm which AI is being used.

**Run with verbose logging**:

```bash
# GUI mode
./source/unix/asc/asc --verbose 1

# Headless mode
./source/unix/asc/asc --headless \
    --mapfile test.ascmap \
    --player1 mcts_balanced \
    --player2 classic \
    --verbose 1
```

**Expected Log Output**:

```
AIFactory: Creating MCTS Balanced AI for player 0
Initializing MCTS_AI for player 0 with profile: Balanced
=== Starting MCTS AI turn for player 0 ===
>>> AI Profile: Balanced (explorationConstant=1.414000, iterations=200)
>>> NOTE: Phase 1.1b MVP - Using simple heuristics, not actual MCTS search yet
Processing units for player 0
Processed 5 units, 3 actions executed
=== MCTS AI turn complete for profile: Balanced ===
```

**vs Classic AI Output**:

```
AIFactory: Creating Classic AI for player 1
=== Classic AI turn starting for player 1 ===
[classic AI detailed strategic output]
```

---

### Method 2: Check Log File

ASC writes logs to `~/.asc/asc.log`:

```bash
# Watch log in real-time
tail -f ~/.asc/asc.log | grep -E "(AIFactory|MCTS|Classic)"

# Check after running
cat ~/.asc/asc.log | grep "AIFactory"
```

**What to look for**:
```
AIFactory: Creating MCTS Balanced AI for player 0
AIFactory: Creating Classic AI for player 1
```

---

### Method 3: Headless Mode with Statistics

```bash
./source/unix/asc/asc --headless \
    --mapfile test.ascmap \
    --player1 mcts_balanced \
    --player2 mcts_aggressive \
    --turnlimit 10 \
    --verbose 1 2>&1 | tee test_output.txt

# Check output
grep "AIFactory" test_output.txt
grep "MCTS AI" test_output.txt
```

**Expected Output**:
```
AIFactory: Creating MCTS Balanced AI for player 0
  Player 0 AI type set to: mcts_balanced (type=1)
AIFactory: Creating MCTS Aggressive AI for player 1
  Player 1 AI type set to: mcts_aggressive (type=2)
```

---

### Method 4: Use Debugger

```bash
gdb ./source/unix/asc/asc
(gdb) break MCTS_AI::run
(gdb) break AI::run
(gdb) run --headless --mapfile test.ascmap --player1 mcts_balanced

# When it breaks:
(gdb) bt
(gdb) print profile.name
(gdb) continue
```

---

## Verbosity Levels

| Level | Flag | What You See |
|-------|------|--------------|
| **0** | (default) | Minimal output |
| **1** | `--verbose 1` | AI factory messages, major events |
| **2** | `--verbose 2` | + MCTS turn start/end, profile info |
| **3** | `--verbose 3` | + Unit processing details |
| **4+** | `--verbose 4` | + Debug messages (very detailed) |

**Recommended**: Use `--verbose 1` to see AI creation without spam.

---

## Complete Test Examples

### Test 1: Verify Factory Creates Correct AI

```bash
./source/unix/asc/asc --headless \
    --mapfile data/maps/tutorial.ascmap \
    --player1 classic \
    --player2 mcts_balanced \
    --turnlimit 2 \
    --verbose 1 2>&1 | grep "AIFactory"
```

**Expected Output**:
```
AIFactory: Creating Classic AI for player 0
AIFactory: Creating MCTS Balanced AI for player 1
```

---

### Test 2: Verify MCTS AI Runs

```bash
./source/unix/asc/asc --headless \
    --mapfile data/maps/tutorial.ascmap \
    --player1 mcts_balanced \
    --player2 mcts_aggressive \
    --turnlimit 1 \
    --verbose 2 2>&1 | grep -E "(MCTS|Profile)"
```

**Expected Output**:
```
AIFactory: Creating MCTS Balanced AI for player 0
Initializing MCTS_AI for player 0 with profile: Balanced
=== Starting MCTS AI turn for player 0 ===
>>> AI Profile: Balanced (explorationConstant=1.414000, iterations=200)
>>> NOTE: Phase 1.1b MVP - Using simple heuristics, not actual MCTS search yet
=== MCTS AI turn complete for profile: Balanced ===

AIFactory: Creating MCTS Aggressive AI for player 1  
Initializing MCTS_AI for player 1 with profile: Aggressive
=== Starting MCTS AI turn for player 1 ===
>>> AI Profile: Aggressive (explorationConstant=2.000000, iterations=200)
>>> NOTE: Phase 1.1b MVP - Using simple heuristics, not actual MCTS search yet
=== MCTS AI turn complete for profile: Aggressive ===
```

**Key Indicators**:
- ✅ Different profile names (Balanced vs Aggressive)
- ✅ Different parameters (explorationConstant=1.414 vs 2.0)
- ✅ MCTS AI is definitely running (not Classic AI)
- ⚠️ But behavior is identical (MVP limitation)

---

### Test 3: Compare All Variants

```bash
#!/bin/bash
for ai in classic mcts_balanced mcts_aggressive mcts_defensive mcts_fast mcts_deep; do
    echo "=== Testing $ai ==="
    ./source/unix/asc/asc --headless \
        --mapfile data/maps/tutorial.ascmap \
        --player1 "$ai" \
        --player2 classic \
        --turnlimit 1 \
        --verbose 1 2>&1 | grep "AIFactory"
    echo ""
done
```

**Expected Output**:
```
=== Testing classic ===
AIFactory: Creating Classic AI for player 0
AIFactory: Creating Classic AI for player 1

=== Testing mcts_balanced ===
AIFactory: Creating MCTS Balanced AI for player 0
AIFactory: Creating Classic AI for player 1

=== Testing mcts_aggressive ===
AIFactory: Creating MCTS Aggressive AI for player 0
AIFactory: Creating Classic AI for player 1

=== Testing mcts_defensive ===
AIFactory: Creating MCTS Defensive AI for player 0
AIFactory: Creating Classic AI for player 1

=== Testing mcts_fast ===
AIFactory: Creating MCTS Fast AI for player 0
AIFactory: Creating Classic AI for player 1

=== Testing mcts_deep ===
AIFactory: Creating MCTS Deep AI for player 0
AIFactory: Creating Classic AI for player 1
```

---

## Log Messages Summary

### When AI is Created
```
AIFactory: Creating [AI Type] AI for player [N]
Initializing MCTS_AI for player [N] with profile: [Profile Name]
```

### When MCTS AI Runs
```
=== Starting MCTS AI turn for player [N] ===
>>> AI Profile: [Name] (explorationConstant=[X], iterations=[Y])
>>> NOTE: Phase 1.1b MVP - Using simple heuristics, not actual MCTS search yet
Processing units for player [N]
Processed [N] units, [N] actions executed
=== MCTS AI turn complete for profile: [Name] ===
```

### When Classic AI Runs
```
=== Classic AI turn starting for player [N] ===
[detailed classic AI output]
```

---

## Troubleshooting

### Problem: No Log Output

**Solution 1**: Increase verbosity
```bash
./asc --verbose 2
```

**Solution 2**: Check log file
```bash
cat ~/.asc/asc.log
```

**Solution 3**: Redirect stderr
```bash
./asc --headless --mapfile test.ascmap --verbose 1 2>&1 | less
```

---

### Problem: Can't Tell Which AI is Running

**Check**:
```bash
# Look for factory messages
grep "AIFactory" ~/.asc/asc.log

# Look for profile info
grep "Profile:" ~/.asc/asc.log

# Look for MCTS-specific messages
grep "MCTS" ~/.asc/asc.log
```

---

### Problem: All AI Behave the Same

**This is EXPECTED in Phase 1.1b MVP!**

✅ **Confirmed Working If**:
- Log shows "Creating MCTS [Variant] AI"
- Log shows different profile names
- Log shows different parameters (explorationConstant, iterations)

⚠️ **Will Be Fixed in Phase 1.2**:
- Actual MCTS search implementation
- Variants will use different strategies
- Behavior will be distinct

---

## What Each Log Line Means

### `AIFactory: Creating MCTS Balanced AI for player 0`
- ✅ Factory pattern working
- ✅ Correct AI type selected
- ✅ Player aiType field being used

### `Initializing MCTS_AI for player 0 with profile: Balanced`
- ✅ MCTS_AI constructor executed
- ✅ Profile loaded correctly
- ✅ Integration successful

### `>>> AI Profile: Balanced (explorationConstant=1.414, iterations=200)`
- ✅ Profile parameters loaded
- ✅ Configuration system working
- ⚠️ **Not yet used** (Phase 1.2 will use these)

### `>>> NOTE: Phase 1.1b MVP - Using simple heuristics`
- ℹ️ Reminder that actual MCTS not implemented yet
- ℹ️ This is temporary for integration testing
- ℹ️ Will be removed in Phase 1.2

### `=== MCTS AI turn complete for profile: Balanced ===`
- ✅ MCTS_AI::run() executed successfully
- ✅ No crashes
- ✅ Integration working

---

## Quick Verification Checklist

Run this command:
```bash
./source/unix/asc/asc --headless \
    --mapfile data/maps/tutorial.ascmap \
    --player1 mcts_balanced \
    --player2 mcts_aggressive \
    --turnlimit 1 \
    --verbose 2 2>&1 | tee verification.log
```

Then check:

- [ ] Log contains "AIFactory: Creating MCTS Balanced AI"
- [ ] Log contains "AIFactory: Creating MCTS Aggressive AI"
- [ ] Log contains "AI Profile: Balanced"
- [ ] Log contains "AI Profile: Aggressive"
- [ ] Different explorationConstant values shown
- [ ] Log contains "Phase 1.1b MVP" message
- [ ] No crashes or errors
- [ ] Both players' turns complete

**If all checked**: ✅ MCTS AI is definitely being invoked!

**If behavior is identical**: ⚠️ This is expected in Phase 1.1b MVP

---

## Understanding the Current State

### What Works (Phase 1.1b ✅)
- ✅ AI Factory creates correct AI type
- ✅ MCTS_AI class instantiated with correct profile
- ✅ Profile parameters loaded (explorationConstant, iterations, etc.)
- ✅ MCTS_AI::run() executes
- ✅ Units are processed
- ✅ Actions are taken
- ✅ Logging shows everything working

### What's Not Implemented Yet (Phase 1.2 ⏳)
- ⏳ Actual MCTS tree search
- ⏳ Using profile parameters for decision-making
- ⏳ Different behavior per variant
- ⏳ Evaluation functions
- ⏳ Simulation/rollout

### Why This Approach?

**Phase 1.1b Goal**: Validate integration architecture
- Test factory pattern
- Test UI selection
- Test command-line selection
- Test save/load
- Test AI lifecycle (create, run, destroy)

**Phase 1.2 Goal**: Implement actual intelligence
- Replace heuristics with MCTS
- Use profile parameters
- Differentiate variants
- Add evaluation functions

---

## Next Steps

### For You (Now)
1. Run verification tests above
2. Confirm log messages appear
3. Verify different profiles are loaded
4. Accept that behavior is identical (for now)

### For Phase 1.2 (Future)
1. Implement MCTS search engine integration
2. Wire up evaluation functions
3. Use profile parameters in search
4. Remove MVP heuristic
5. Test behavior differences

---

## Summary

**To verify MCTS is working**:

```bash
# Run this
./source/unix/asc/asc --headless \
    --mapfile test.ascmap \
    --player1 mcts_balanced \
    --player2 classic \
    --verbose 1 2>&1 | grep -E "(AIFactory|Profile)"

# You should see
AIFactory: Creating MCTS Balanced AI for player 0
>>> AI Profile: Balanced (explorationConstant=1.414000, iterations=200)
AIFactory: Creating Classic AI for player 1
```

**If you see those messages**: ✅ **MCTS AI is definitely being invoked!**

**Identical behavior is expected** - This is Phase 1.1b MVP for integration testing.

**Behavior will differ in Phase 1.2** when actual MCTS search is implemented.
