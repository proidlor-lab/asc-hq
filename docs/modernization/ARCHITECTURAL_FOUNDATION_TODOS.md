# Architectural Foundation TODO List

**Purpose**: Tasks that must be completed BEFORE extracting business logic for the client-server architecture.

**Context**: This GPL repository will become a game server with API endpoints. A separate proprietary client will handle UI/graphics. We need a solid foundation before starting API extraction.

---

## Execution Strategy

### Phase 1: Safety Net (Week 1-2)
Build infrastructure to catch regressions and enable confident refactoring.

### Phase 2: Critical Fixes (Week 3-6)
Fix memory leaks and security issues critical for 24/7 server operation.

### Phase 3: Organization (Week 7-8)
Document architecture and remove technical debt.

### Phase 4: Structural Refactoring (Week 9-14)
Break down God classes into maintainable modules.

**Total Estimated Time**: 2.5-3.5 months

---

## TIER 1: Development Infrastructure (Week 1-2)

### 1. CI/CD Pipeline ⭐ CRITICAL
**Status**: Pending
**Priority**: HIGHEST
**Effort**: 2-3 days
**Blocks**: Nothing (do first!)

**Why**: Catch regressions immediately, ensure code always builds

**Tasks**:
- [ ] Set up GitHub Actions workflow
- [ ] Automated builds on every commit
- [ ] Run all tests automatically
- [ ] Build both GUI and headless modes
- [ ] Test with multiple compiler versions (GCC, Clang)
- [ ] Cache dependencies for speed
- [ ] Add build status badge to README

**Benefit**:
- Prevents "works on my machine" issues
- Immediate feedback on breaking changes
- Safe refactoring with confidence

---

### 2. Code Formatting (clang-format) ⭐ HIGH
**Status**: Pending
**Priority**: HIGH
**Effort**: 1-2 days
**Blocks**: Nothing (cosmetic but valuable)

**Why**: Consistent style makes code readable and diffs meaningful

**Current Problem**: Mix of 2, 3, 4, 8-space indents across codebase

**Tasks**:
- [ ] Create `.clang-format` configuration file
- [ ] Match MCTS module style (already clean)
- [ ] Format entire codebase once
- [ ] Add format check to CI/CD
- [ ] Document formatting standards

**Benefit**:
- Easier code navigation
- Cleaner git diffs (only logical changes, no whitespace noise)
- No style debates
- Professional appearance

---

### 3. Static Analysis Tools ⭐ HIGH
**Status**: Pending
**Priority**: HIGH
**Effort**: 2-3 days
**Blocks**: Nothing

**Why**: Find bugs without running code

**Tools to Integrate**:
- [ ] clang-tidy (modern C++ checks, bug detection)
- [ ] cppcheck (additional static analysis)
- [ ] Include in CI pipeline (fail on errors)

**Finds**:
- Memory leaks
- Null pointer dereferences
- Uninitialized variables
- Dead code
- Modern C++ anti-patterns
- Security vulnerabilities

**Benefit**: Automated code review, catch bugs early

---

### 4. Modern Test Framework ⭐ HIGH
**Status**: Pending
**Priority**: HIGH
**Effort**: 3-4 days
**Blocks**: Nothing (can run old tests in parallel)

**Why**: Current custom framework is limited, industry-standard tools provide better features

**Tasks**:
- [ ] Choose framework: Google Test (industry standard) or Catch2 (header-only)
- [ ] Migrate existing unit tests
- [ ] Add test coverage reporting (gcov/lcov)
- [ ] Integrate coverage into CI
- [ ] Set coverage baseline and goals

**Benefit**:
- Better test organization (fixtures, suites)
- Parameterized tests
- Coverage metrics (know what's tested)
- Easier to write new tests

---

## TIER 2: Code Safety (Week 2-4)

### 5. Memory Leak Audit & Fixes ⭐ CRITICAL
**Status**: Pending
**Priority**: CRITICAL (for server!)
**Effort**: 3-4 weeks (ongoing)
**Blocks**: None, but CRITICAL for server stability

**Why**: Servers run 24/7, leaks are fatal

**Process**:
1. [ ] Run Valgrind on headless mode
2. [ ] Run AddressSanitizer builds (-fsanitize=address)
3. [ ] Run LeakSanitizer builds
4. [ ] Fix all definite leaks
5. [ ] Document suspected leaks
6. [ ] Create leak regression tests

**Focus Areas** (from codebase analysis):
- [ ] `source/basegfx.cpp` - graphics buffer management
- [ ] `source/basestrm.cpp` - stream handling
- [ ] `source/attack.cpp` - combat calculations
- [ ] `source/gameoptions.cpp` - configuration
- [ ] Legacy AI files (`ai/base.cpp`, `ai/tactics.cpp`)
- [ ] All `new`/`delete` without RAII (30+ files)
- [ ] All `malloc`/`free` usage (56 occurrences)

**Benefit**:
- Server can run for days/weeks without restart
- No memory exhaustion
- Predictable resource usage

---

### 6. Security Audit of Dependencies ⭐ HIGH
**Status**: Pending
**Priority**: HIGH
**Effort**: 1 week
**Blocks**: None, but do before release

**Why**: Bundled 20-year-old libraries may have known CVEs

**Dependencies to Audit**:
- [ ] Loki 0.1.6 (2005) - 20 years old
- [ ] ParaGUI (custom) - 18 years old
- [ ] SDL_mm (2005) - 20 years old
- [ ] sdl_sound (stripped version)
- [ ] revel (video codec)
- [ ] Check CVE databases for all

**Actions**:
- [ ] Search CVE databases for known vulnerabilities
- [ ] Update dependencies if possible
- [ ] Apply patches for unfixable libraries
- [ ] Document security posture
- [ ] Create security policy document

**Benefit**:
- Know your attack surface
- Mitigate known vulnerabilities
- Safe for public release

---

## TIER 3: Build Optimization (Week 3-5)

### 7. Build System Optimization ⭐ MEDIUM
**Status**: Pending
**Priority**: MEDIUM
**Effort**: 1-2 weeks
**Blocks**: Nothing

**Why**: Faster builds = faster iteration, happier developers

**Current**: Full rebuild takes ~5-10 minutes
**Target**: Incremental builds <30 seconds

**Optimizations**:
- [ ] Precompiled headers (PCH) for large includes (SDL, Boost, wxWidgets)
- [ ] Set up ccache (cache compilation results)
- [ ] Parallel builds (make -j auto-detect cores)
- [ ] Incremental build improvements
- [ ] Separate headless from GUI builds (server doesn't need GUI)
- [ ] Profile build to find bottlenecks

**Benefit**:
- Developer productivity (edit-compile-test loop)
- Faster CI/CD pipeline
- Less waiting, more coding

---

## TIER 4: Understanding & Organization (Week 4-6)

### 8. Architectural Documentation ⭐ HIGH
**Status**: Pending
**Priority**: HIGH
**Effort**: 1-2 weeks
**Blocks**: Nothing, but helps all future planning

**Why**: Can't refactor what you don't understand

**Documents to Create**:
- [ ] `docs/architecture/MODULE_STRUCTURE.md` - what each folder contains
- [ ] `docs/architecture/DEPENDENCY_GRAPH.md` - what depends on what
- [ ] `docs/architecture/DATA_FLOW.md` - how data moves through system
- [ ] `docs/architecture/GAME_LOOP.md` - initialization → game loop → cleanup
- [ ] `docs/architecture/HEADLESS_MODE.md` - how headless mode works
- [ ] `docs/architecture/CURRENT_UI_COUPLING.md` - where UI is coupled to logic
- [ ] `docs/architecture/GOD_CLASSES.md` - analysis of GameMap, Vehicle, etc.

**Content**:
- Module boundaries
- Core data structures
- Control flow
- Current UI coupling points
- Entry points and main loops
- Network/multiplayer architecture

**Benefit**:
- Onboarding new developers
- Planning refactoring work
- Understanding legacy decisions
- Identifying extraction targets

---

### 9. Remove Dead Code ⭐ MEDIUM
**Status**: Pending
**Priority**: MEDIUM
**Effort**: 1-2 weeks (ongoing)
**Blocks**: CI/CD (need safety net before deleting)

**Why**: Less code = easier to understand, faster builds, less confusion

**From Analysis**:
- [ ] 100+ lines of commented-out code
- [ ] Unused BGI graphics functions (from 1993!)
- [ ] Deprecated code paths
- [ ] Unreachable code (static analysis will help)
- [ ] Dead functions (never called)

**Process**:
1. [ ] Run static analysis to find unused code
2. [ ] Search for commented code blocks
3. [ ] Remove with confidence (have CI/CD + tests as safety)
4. [ ] Document removals in commit messages

**Benefit**:
- Cleaner codebase
- Faster builds (less to compile)
- Less confusion (no wondering if old code is still used)
- Smaller binary size

---

### 10. Refactor God Classes ⭐ MEDIUM-HIGH
**Status**: Pending
**Priority**: MEDIUM-HIGH
**Effort**: 4-6 weeks (large task)
**Blocks**: Test framework, CI/CD (need safety)

**Why**: Can't work with 18,550-line classes, impossible to extract APIs

**Current Problem**:
- `GameMap.cpp`: 18,550 lines - does EVERYTHING!
- `Vehicle.cpp`: 14,781 lines - too many responsibilities!

**Target Architecture** (use MCTS as model):
```
GameMap/
  ├─ MapGeometry (terrain, coordinates, pathfinding)
  ├─ MapState (unit positions, ownership, fog of war)
  ├─ MapRules (movement rules, visibility rules)
  └─ MapPersistence (save/load functionality)

Vehicle/
  ├─ VehicleState (position, health, ammo)
  ├─ VehicleAbilities (movement, attack, abilities)
  ├─ VehicleAI (AI parameters, behavior)
  └─ VehicleGraphics (rendering, sprites) - can stay for now
```

**Approach**:
1. [ ] Identify distinct responsibilities (SRP - Single Responsibility Principle)
2. [ ] Extract interfaces for each responsibility
3. [ ] Move implementations to new classes
4. [ ] Test at each step (incremental refactoring)
5. [ ] Use MCTS module as reference (clean architecture)

**Benefit**:
- Testable components
- Clear module boundaries
- Easier to extract business logic APIs
- Maintainable code

---

## Success Criteria

After completing these tasks, you should have:

✅ **Safe Development**:
- CI/CD catches all regressions
- Tests run automatically
- Static analysis finds bugs

✅ **Clean Code**:
- Consistent formatting
- No dead code
- Clear architecture

✅ **Stable Server**:
- No memory leaks
- Known security posture
- Can run 24/7

✅ **Fast Iteration**:
- Builds <30 seconds
- Easy to navigate code
- Modular structure

---

## Then You Can Start Business Logic Extraction

With this foundation, extracting business logic becomes:
- **Safer** (tests catch regressions)
- **Faster** (quick builds)
- **Easier** (clear module boundaries)
- **More reliable** (no memory leaks)

---

## References

- MCTS module (`source/ai/mcts/`) - Reference implementation for modern C++23 architecture
- Current analysis docs in this directory
- Technical debt audits (already in repo)

---

**Created**: 2025-11-16
**Status**: Planning Phase
**Next Step**: Start with CI/CD setup (TIER 1, Task 1)
