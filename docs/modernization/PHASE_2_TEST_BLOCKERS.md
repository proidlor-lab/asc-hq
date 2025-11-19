# Phase 2 Test Migration - Blockers and Analysis

**Date**: 2025-11-19
**Status**: BLOCKED - Architectural Coupling Issues

## Summary

Phase 2 test migration (actiontest.cpp → action_container_test.cpp) has revealed fundamental architectural coupling issues that prevent clean unit testing of business logic.

## What Was Attempted

1. ✅ Created Google Test version: `tests/unit/action_container_test.cpp`
2. ✅ Updated `tests/Makefile.am` to add test to build system
3. ✅ Created `libcommon.la` convenience library with ~70 common source files
4. ❌ Linking failed due to missing GUI/SDL dependencies

## Root Cause Analysis

### The GameMap Coupling Problem

Phase 2 tests require `GameMap` to function. GameMap is an 18,500-line "God class" that is tightly coupled to:

- **GUI Framework**: ParaGUI widgets (PG_Widget, PG_Window, PG_MapDisplay)
- **Graphics**: SDL surfaces, blitters, rendering pipeline
- **Sound System**: Sound::play(), positional audio
- **Animation**: showAttackAnimation(), battle displays
- **Timing/Threading**: ticker global, releasetimeslice()
- **File I/O**: Complex stream hierarchies
- **Game State**: Everything else in the game

### Build Architecture Issue

The legacy build system uses a "monolithic binary" approach:

```makefile
# From source/unix/asc/Makefile.am
commonSources = soundList.cpp typen.cpp applicationstarter.cpp \
                ... (70+ source files) ...

nodist_unittester_SOURCES = $(commonSources)
unittester_LDADD = $(commonLibs)
```

The unittester compiles the ENTIRE game engine (70+ files) directly into the test binary. This works for the legacy tests but is:

- **Not modular**: Can't test individual components in isolation
- **Slow to build**: ~3+ minutes to compile libcommon.la
- **Tightly coupled**: Business logic mixed with UI/graphics
- **Hard to mock**: Can't inject test doubles for dependencies

## Current Linking Errors

After creating libcommon.la, we still get undefined references to:

### GUI Framework (ParaGUI)
```
undefined reference to `PG_Widget::eventShow()'
undefined reference to `PG_Widget::eventHide()'
undefined reference to `PG_Widget::createMySurface(int, int)'
undefined reference to `PG_Window::handleButtonClick(PG_Button*)'
undefined reference to `PG_Widget::Update(bool)'
```

### Graphics/Animation
```
undefined reference to `showAttackAnimation(tfight&, GameMap*, int, int)'
```

### Sound
```
undefined reference to `Sound::play()'
```

### Timing/Threading
```
undefined reference to `ticker'
undefined reference to `releasetimeslice()'
```

### Error Handling
```
undefined reference to `fatalError(char const*, ...)'
```

## Why This Matters for Modernization

**The core modernization goal** is to decouple business logic from UI/graphics to enable:

1. Headless game engine (for AI, servers, tools)
2. Clean unit tests of game rules
3. Modern architecture (separation of concerns)
4. Easier refactoring and maintenance

**Phase 2 tests reveal** that this decoupling hasn't happened yet. GameMap cannot be instantiated without pulling in GUI/SDL dependencies.

## Options Going Forward

### Option A: Continue with Monolithic Approach (Short-term)
- Link all GUI libraries into test (libparagui, libSDL, etc.)
- Tests become integration tests, not unit tests
- Maintains current architecture
- **Pros**: Can migrate all legacy tests quickly
- **Cons**: Doesn't advance modernization goals, slow test execution

### Option B: Refactor GameMap First (Long-term, Recommended)
- Extract business logic from GameMap into pure C++ classes
- Create minimal "GameState" or "GameEngine" class without GUI deps
- Update tests to use decoupled classes
- **Pros**: Aligns with modernization goals, enables true unit testing
- **Cons**: Requires significant refactoring effort

### Option C: Hybrid Approach
- Continue Phase 1 tests (simple, decoupled components) ✅
- Document Phase 2 blockers (this file)
- Start Tier 2 modernization (extract GameMap business logic)
- Return to Phase 2 tests after decoupling is done
- **Pros**: Makes progress on both fronts
- **Cons**: Test migration remains incomplete

## Recommendation

**Proceed with Option C - Hybrid Approach**

### Immediate Actions:
1. ✅ Complete Phase 1 tests (version_identifier, stream_encoding, game_events) - DONE
2. Document Phase 2 blockers (this file) - IN PROGRESS
3. Switch focus to Tier 2: Code Safety & Modernization
   - Extract business logic from GameMap
   - Create headless game engine classes
   - Remove GUI dependencies from core logic

### Future Actions (after Tier 2 progress):
4. Return to Phase 2 tests with refactored architecture
5. Write unit tests for extracted business logic
6. Gradually migrate remaining legacy tests

## Test Migration Status

### Phase 1: Simple Unit Tests (✅ COMPLETE)
| Legacy Test | Google Test | Status | Notes |
|-------------|-------------|--------|-------|
| testversionidentifier.cpp | version_identifier_test.cpp | ✅ PASS (17/17) | No GameMap dependency |
| streamencoding.cpp | stream_encoding_test.cpp | ✅ PASS (4/4) | Minimal dependencies |
| eventtest.cpp | game_events_test.cpp | ✅ PASS (1/1) | Placeholder only |

### Phase 2: GameMap-Dependent Tests (❌ BLOCKED)
| Legacy Test | Google Test | Status | Blocker |
|-------------|-------------|--------|---------|
| actiontest.cpp | action_container_test.cpp | ❌ BLOCKED | GameMap GUI coupling |
| maptest.cpp | - | ⏸️ NOT STARTED | GameMap GUI coupling |
| movementtest.cpp | - | ⏸️ NOT STARTED | GameMap GUI coupling |
| attacktest.cpp | - | ⏸️ NOT STARTED | GameMap GUI coupling |
| recyclingtest.cpp | - | ⏸️ NOT STARTED | GameMap GUI coupling |
| repairtest.cpp | - | ⏸️ NOT STARTED | GameMap GUI coupling |
| researchtest.cpp | - | ⏸️ NOT STARTED | GameMap GUI coupling |
| transfercontroltest.cpp | - | ⏸️ NOT STARTED | GameMap GUI coupling |
| objectconstructiontest.cpp | - | ⏸️ NOT STARTED | GameMap GUI coupling |
| viewtest.cpp | - | ⏸️ NOT STARTED | GameMap GUI coupling |
| jumptest.cpp | - | ⏸️ NOT STARTED | GameMap GUI coupling |
| diplomacytest.cpp | - | ⏸️ NOT STARTED | GameMap GUI coupling |

### Phase 3: AI Tests (Already Modern)
| Legacy Test | Google Test | Status | Notes |
|-------------|-------------|--------|-------|
| ai-move1.cpp | - | ⏸️ NOT STARTED | May be decoupled enough |
| ai-service1.cpp | - | ⏸️ NOT STARTED | May be decoupled enough |

## Files Created/Modified

### Created:
- `tests/unit/action_container_test.cpp` - Google Test version of actiontest.cpp
- `tests/libcommon.la` - Convenience library with 70+ common sources
- This documentation file

### Modified:
- `tests/Makefile.am` - Added libcommon.la and action_container_test
- `.gitignore` - Added action_container_test executable

## Next Steps

1. **Immediate**: Switch to Tier 2 modernization work
   - Focus on extracting GameMap business logic
   - Create headless game engine architecture
   - See: `docs/modernization/ARCHITECTURAL_FOUNDATION_TODOS.md`

2. **Track Progress**: Update this file as decoupling progresses

3. **Revisit Tests**: Return to Phase 2 test migration after business logic is extracted

## Technical Debt Identified

- GameMap is a 18,500-line God class mixing concerns
- No separation between business logic and presentation
- Tests can't run without full GUI/SDL stack
- Build system uses monolithic binary approach
- Missing dependency injection for testability

## Success Criteria for Unblocking Phase 2

To enable Phase 2 test migration, we need:

1. ✅ GameMap business logic extracted to standalone classes
2. ✅ Core game rules testable without GUI dependencies
3. ✅ Action system decoupled from presentation layer
4. ✅ Map data structures separated from rendering
5. ✅ Test fixtures can create minimal game state

These are all Tier 2 modernization goals. Once achieved, Phase 2 tests become straightforward to implement.
