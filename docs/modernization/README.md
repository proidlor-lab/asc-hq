# Modernization Documentation

**Purpose**: Documentation for the ASC modernization effort to transform the codebase into a production-ready game server.

**Created**: 2025-11-16
**Status**: Planning & Initial Implementation

---

## Overview

This directory contains documentation for modernizing the Advanced Strategic Command (ASC) codebase before extracting business logic for a client-server architecture.

### Strategic Goal

Transform this GPL repository into a **game server** that exposes game logic via APIs. A separate proprietary client will handle UI and graphics, enabling Steam distribution without GPL contamination.

---

## Documentation Index

### Planning Documents

1. **[ARCHITECTURAL_FOUNDATION_TODOS.md](./ARCHITECTURAL_FOUNDATION_TODOS.md)**
   - Comprehensive TODO list for pre-API-extraction modernization
   - Organized into 4 tiers by priority
   - Estimated timelines and effort
   - Success criteria

### Implementation Guides

2. **[CI_CD_SETUP.md](./CI_CD_SETUP.md)**
   - GitHub Actions CI/CD pipeline documentation
   - Dependency management
   - Build process
   - Troubleshooting guide

---

## Quick Start

### Current Phase: Tier 1 - Development Infrastructure

**Focus**: Building safety nets before major refactoring

#### Completed
- ✅ Architectural TODO list created
- ✅ CI/CD pipeline configured (`.github/workflows/ci.yml`)
- ✅ Documentation structure established

#### In Progress
- 🔄 CI/CD pipeline testing and refinement

#### Next Steps
1. Test CI/CD pipeline on GitHub
2. Configure clang-format
3. Set up static analysis
4. Modernize test framework

---

## Project Context

### From the Codebase Analysis

**Strengths**:
- ✅ Recently migrated to C++23 successfully
- ✅ MCTS AI module is exemplary (modern C++23, clean architecture)
- ✅ Modern build system (Autotools, GCC 13.3.0)
- ✅ Good test pass rate (97% on MCTS)
- ✅ Extensive documentation (57 markdown files)

**Challenges**:
- ❌ SDL 1.2 end-of-life (security issue)
- ❌ Tight UI/core coupling (GameMap has rendering)
- ❌ Ancient dependencies (Loki 20y, ParaGUI 18y)
- ❌ Manual memory management (30+ files with raw new/delete)
- ❌ God classes (GameMap: 18,550 lines, Vehicle: 14,781 lines)

### Architecture Vision

```
┌─────────────────────────────────────┐
│  This Repo (GPL - Game Server)      │
│  ├─ Core game logic                 │
│  ├─ AI (including MCTS)             │
│  ├─ Multiplayer/networking          │
│  ├─ Game state management           │
│  └─ API layer (REST/WebSocket)      │
└─────────────────────────────────────┘
            ↕ Network Protocol
┌─────────────────────────────────────┐
│  New Client (Proprietary)           │
│  ├─ Modern renderer (Unreal/Unity)  │
│  ├─ UI/UX                           │
│  ├─ Audio/graphics                  │
│  └─ Steam integration               │
└─────────────────────────────────────┘
```

---

## Modernization Roadmap

### Phase 1: Foundation (Weeks 1-2) - **CURRENT**
- [x] Create TODO list
- [x] Set up CI/CD
- [ ] Configure code formatting
- [ ] Set up static analysis
- [ ] Modernize test framework

### Phase 2: Critical Fixes (Weeks 3-6)
- [ ] Memory leak audit and fixes
- [ ] Security audit of dependencies
- [ ] Build system optimization

### Phase 3: Organization (Weeks 7-8)
- [ ] Architectural documentation
- [ ] Dead code removal

### Phase 4: Structural (Weeks 9-14)
- [ ] Refactor God classes

### Phase 5: API Extraction (Months 4-6)
- [ ] Design game state API
- [ ] Extract core game mechanics
- [ ] Implement authentication
- [ ] Create network protocol

**Total Timeline**: ~6 months to production-ready game server

---

## Key Principles

### 1. Safety First
- CI/CD catches all regressions
- Tests before refactoring
- Incremental changes

### 2. MCTS as Template
The `source/ai/mcts/` module demonstrates best practices:
- Clean separation (domain/core/infrastructure)
- Modern C++23 features
- Smart pointers, no manual memory management
- Dependency injection
- Comprehensive tests

**Apply these patterns project-wide.**

### 3. Server Stability
For 24/7 operation:
- No memory leaks
- Known security posture
- Performance under load
- Graceful error handling

### 4. Incremental Progress
Not a rewrite - modernize piece by piece:
- Each change tested
- Each commit buildable
- No big-bang migrations

---

## Success Criteria

After Tier 1-4 completion, you should have:

✅ **Safe Development**
- CI/CD catches regressions automatically
- Tests run on every commit
- Static analysis finds bugs early

✅ **Clean Code**
- Consistent formatting (clang-format)
- No dead code
- Clear module boundaries
- Documented architecture

✅ **Stable Server**
- Zero known memory leaks
- Security audit complete
- Can run 24/7 without crashes

✅ **Fast Iteration**
- Incremental builds <30 seconds
- Easy code navigation
- Modular, testable structure

---

## Contributing to Modernization

### Before Starting Work

1. Read relevant TODO document
2. Check CI/CD is passing
3. Update TODO status (mark in_progress)
4. Create feature branch

### During Work

1. Keep CI/CD passing (fix breaks immediately)
2. Add tests for changes
3. Follow MCTS patterns for new code
4. Document significant decisions

### Before Completing

1. All tests pass
2. CI/CD green
3. Update documentation
4. Mark TODO complete
5. Create pull request

---

## Reference Implementation

**Always refer to `source/ai/mcts/` as the gold standard**:
- Modern C++23 patterns
- Clean architecture
- Proper testing
- Good documentation

When in doubt: "How would this be done in MCTS?"

---

## Tools & Resources

### Build Tools
- GCC 13.3.0 with C++23 support
- Autotools (Automake 1.16.5, Autoconf 2.71+)
- ccache for fast builds

### Analysis Tools
- cppcheck (static analysis)
- clang-tidy (modern C++ linting)
- Valgrind (memory leaks)
- AddressSanitizer (memory errors)

### Testing
- Custom framework (to be replaced)
- Target: Google Test or Catch2
- MCTS: 97% pass rate (64/66 tests)

---

## Questions?

- Check existing documentation first
- Look at MCTS module for examples
- Review codebase analysis (from initial exploration)
- Ask for clarification when needed

---

## Status Updates

**2025-11-16**: Initial planning complete
- Architectural TODO list created
- CI/CD pipeline configured
- Documentation structure established
- Ready to begin Tier 1 implementation

---

**Next Update**: After CI/CD testing complete
