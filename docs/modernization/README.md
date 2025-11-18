# Modernization Documentation

**Purpose**: Documentation for the ASC modernization effort to transform the codebase into a production-ready game server.

**Created**: 2025-11-16
**Last Updated**: 2025-11-17
**Status**: Tier 1 Complete - In Progress

---

## Overview

This directory contains documentation for modernizing the Advanced Strategic Command (ASC) codebase before extracting business logic for a client-server architecture.

### Strategic Goal

Transform this GPL repository into a **game server** that exposes game logic via APIs. A separate proprietary client will handle UI and graphics, enabling Steam distribution without GPL contamination.

---

## Documentation Index

### Status & Planning

1. **[STATUS.md](./STATUS.md)** - Current progress, metrics, and next actions
2. **[ARCHITECTURAL_FOUNDATION_TODOS.md](./ARCHITECTURAL_FOUNDATION_TODOS.md)** - Detailed task list organized by tier

### Implementation Guides

3. **[CI_CD_SETUP.md](./CI_CD_SETUP.md)** - GitHub Actions CI/CD pipeline
4. **[STATIC_ANALYSIS.md](./STATIC_ANALYSIS.md)** - Static analysis tools (cppcheck, clang-tidy)
5. **[TEST_FRAMEWORK_MIGRATION.md](./TEST_FRAMEWORK_MIGRATION.md)** - Google Test migration plan
6. **[GOOGLE_TEST_QUICK_REFERENCE.md](./GOOGLE_TEST_QUICK_REFERENCE.md)** - Google Test quick reference
7. **[BUILD_SYSTEM_MODERNIZATION.md](./BUILD_SYSTEM_MODERNIZATION.md)** - Build optimization strategies

**Note**: Code formatting already applied (`.clang-format` config, enforced by CI)

---

## Current Status

**Phase**: Tier 1 Complete (Development Infrastructure)
**Progress**: 40% overall (4/10 tasks complete)

**Recent completions**:
- ✅ CI/CD pipeline (GitHub Actions)
- ✅ Code formatting applied (1,051+ files)
- ✅ Static analysis baseline (1,650 issues identified)
- ✅ Test framework Phase 1 complete (Google Test integrated)
- ✅ Automated testing on every commit

**See [STATUS.md](./STATUS.md) for detailed progress and next actions.**

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

**See [ARCHITECTURAL_FOUNDATION_TODOS.md](./ARCHITECTURAL_FOUNDATION_TODOS.md) for detailed roadmap.**

High-level phases:
1. **Tier 1: Development Infrastructure** (Weeks 1-2) - ✅ Complete
2. **Tier 2: Code Safety** (Weeks 2-4) - Memory leaks, security audit
3. **Tier 3: Build Optimization** (Weeks 3-5) - Build speed improvements
4. **Tier 4: Organization** (Weeks 4-6) - Architecture docs, God class refactoring

**Total Timeline**: ~3 months for foundation, then 3+ months for API extraction

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

**See [ARCHITECTURAL_FOUNDATION_TODOS.md](./ARCHITECTURAL_FOUNDATION_TODOS.md) for complete success criteria.**

After Tier 1-4 completion:
- ✅ Safe development (CI/CD, tests, static analysis)
- ✅ Clean code (consistent formatting, clear architecture)
- ✅ Stable server (no memory leaks, security audited)
- ✅ Fast iteration (<30s builds, modular structure)

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

**See individual implementation guides for detailed tool documentation.**

- **Build**: GCC 13.3.0 (C++23), Autotools, ccache, CMake (MCTS module)
- **Analysis**: cppcheck 2.13.0, clang-tidy (LLVM 18.1.3), Valgrind, AddressSanitizer
- **Testing**: Google Test (active), custom framework (legacy, parallel)
- **CI/CD**: GitHub Actions (6 jobs including automated testing)

---

## Testing Infrastructure

### Google Test Framework ✅ Active

**Location**: `tests/` directory

**Current Status**:
- ✅ Google Test integrated (Phase 1 complete)
- ✅ 13 tests running in CI/CD
- ✅ Automated execution on every commit
- ✅ Test results visible in GitHub Actions

**Quick Start**:
```bash
# Build and run tests
./configure
cd tests/
make check

# Run specific tests
./example_test --gtest_filter=StringTest.*

# List all tests
./example_test --gtest_list_tests
```

**CI/CD Integration**:
- Job: `google-test-suite`
- Triggers: Every push, every PR
- Environment: Clean Ubuntu 24.04
- Results: Visible in Actions tab

**Documentation**:
- [Google Test Quick Reference](./GOOGLE_TEST_QUICK_REFERENCE.md) - **Start here!**
- [Test Framework Migration Plan](./TEST_FRAMEWORK_MIGRATION.md) - Complete migration guide
- [Test Getting Started Guide](../../tests/README.md) - Developer guide
- See MCTS module (`source/ai/mcts/`) for test examples

**Next Phase**: Migrate MCTS tests to Google Test framework

---

## Questions?

- Check [STATUS.md](./STATUS.md) for current progress
- Look at `source/ai/mcts/` module for modern C++23 examples
- Refer to specific implementation guides for detailed instructions
