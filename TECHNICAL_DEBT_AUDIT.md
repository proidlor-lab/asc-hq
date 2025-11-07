# Technical Debt Audit - ASC Project

**Date**: 2025-11-07  
**Version**: ASC 2.8.3.0  
**Purpose**: Pre-MCTS development audit

---

## Executive Summary

Overall technical health: **⚠️ MODERATE** with some critical areas needing attention

**Key Findings**:
- ✅ **Build system works** - Autotools functional, C++23 compatible
- ⚠️ **Ancient dependencies** - Most libraries are 15-20 years old
- ✅ **Unit tests exist** - 2,431 lines of test code
- ❌ **No CI/CD** - No automated testing infrastructure
- ⚠️ **Bundled libraries** - Many outdated, heavily patched versions

**Recommendation**: Address critical items (#1-3) before major MCTS development.

---

## 1. Build System Analysis

### Autotools Stack ✅ Modern
| Tool | Version | Status | Notes |
|------|---------|--------|-------|
| **Autoconf** | 2.71+ | ✅ Good | Modern version |
| **Automake** | 1.16.5 | ✅ Good | Recent version |
| **Libtool** | 2.4.7+ | ✅ Good | Modern version |
| **GNU Make** | 4.3+ | ✅ Good | Modern version |
| **GCC** | 13.3.0 | ✅ Excellent | C++23 support |

**Assessment**: ✅ **Build system is modern and well-maintained**

### Configuration
- `configure.ac`: Well-structured, C++23 enabled
- Version detection from source code (strtmesg.cpp)
- Proper dependency checking with PKG_CHECK_MODULES

**Issues**: None critical

---

## 2. External Dependencies

### System Libraries (PKG-CONFIG)

| Dependency | Required Version | Your System | Status | Age |
|------------|------------------|-------------|--------|-----|
| **SDL 1.2** | 1.2.x | 1.2.68 | ⚠️ OLD | ~15 years |
| **Lua** | >= 5.1 | Not detected | ❌ Missing | - |
| **SigC++** | 2.0 | Detected | ✅ OK | Modern |
| **FreeType** | 2.x | Detected | ✅ OK | Modern |
| **libpng** | - | Detected | ✅ OK | Modern |
| **Ogg/Vorbis** | - | Detected | ✅ OK | Modern |
| **wxWidgets** | 3.x | 3.2 | ✅ OK | Modern |

**Critical Issues**:

1. **❌ SDL 1.2 (CRITICAL)**
   - **Age**: Released 2012, deprecated since 2013
   - **Status**: End-of-life, no security updates
   - **Risk**: Security vulnerabilities, no modern OS support
   - **Recommendation**: **Migrate to SDL2** (blocker for production)
   - **Effort**: High (3-6 weeks)
   - **Impact**: Entire graphics/input system

2. **⚠️ Lua detection issue**
   - pkg-config can't find lua
   - Likely naming issue (lua5.1 vs lua)
   - **Fix**: Check PKG_CONFIG_PATH or install lua-dev package

---

## 3. Bundled Libraries (source/libs/)

### Overview
ASC bundles 10 third-party libraries, most heavily patched.

| Library | Version | Age | Status | Recommendation |
|---------|---------|-----|--------|----------------|
| **Loki** | 0.1.6 | 20 yrs | ✅ Fixed C++23 | Keep (already patched) |
| **ParaGUI** | Custom | ~18 yrs | ⚠️ Unmaintained | Keep (heavily patched) |
| **SDL_mm** | 0.1.8+ | ~20 yrs | ⚠️ Old | Consider removal |
| **sdl_sound** | Stripped | ~15 yrs | ⚠️ Old | Keep (OGG only) |
| **revel** | ? | ~15 yrs | ⚠️ Old | Keep (video codec) |
| **bzlib** | ? | ? | ✅ OK | Keep (compression) |
| **triangul** | ? | ? | ⚠️ Unknown | Keep (geometry) |
| **getopt** | ? | ? | ✅ OK | Keep (Windows) |
| **rand** | glibc | ? | ✅ OK | Keep (RNG) |

### Detailed Analysis

#### 1. **Loki 0.1.6** ✅ Acceptable
- **Status**: ✅ C++23 compatible after our fixes
- **Age**: 20 years (2005)
- **Usage**: Functors, Singletons, SmartPtr
- **Risk**: Low (isolated, working)
- **Action**: ✅ **Keep** - Already fixed, upgrade not worth it

#### 2. **ParaGUI** ⚠️ Concerning
- **Status**: Heavily patched custom version
- **Note**: "neither compatible with last release nor CVS head"
- **Risk**: Medium-High (unmaintained, custom patches)
- **Issue**: No upstream, can't get security updates
- **Mitigation**: Well-tested in ASC, working
- **Action**: ⏸️ **Keep** for now, plan long-term replacement

**Long-term**: Consider migration to:
- **ImGui** (modern, immediate mode GUI)
- **Dear ImGui** (C++, game-focused)
- **GTK4** or **Qt** (if going native UI)

#### 3. **SDL_mm** ⚠️ Questionable
- **Status**: C++ wrapper for SDL 1.2
- **Age**: ~20 years
- **Issue**: "Using sdlmm 0.1.8 is not an option!"
- **Risk**: High (depends on deprecated SDL 1.2)
- **Action**: ⏸️ **Keep** until SDL2 migration, then remove

#### 4. **sdl_sound** ⚠️ OK for now
- **Status**: Stripped version (OGG Vorbis only)
- **Reason**: Linux distros don't provide full version (MP3 licensing)
- **Risk**: Low-Medium (audio only)
- **Action**: ✅ **Keep** - Serves specific purpose

#### 5. **revel** ⚠️ Unknown
- **Status**: Video codec library
- **Website**: http://revel.sourceforge.net/ (possibly dead)
- **Risk**: Medium (optional feature)
- **Action**: ✅ **Keep** - Optional dependency for video

---

## 4. Testing Infrastructure

### Unit Tests ⚠️ Basic
- **Location**: `source/unittests/`
- **Size**: 2,431 lines of test code
- **Tests**:
  - Attack tests
  - Movement tests
  - Diplomacy tests
  - Repair tests
  - Object construction tests
  - Event tests
  - Map tests
  - AI tests (move, service)

**Assessment**: ⚠️ **Minimal but functional**

**Issues**:
1. ❌ **No CI/CD** - No automated test execution
2. ❌ **No test coverage metrics**
3. ⚠️ **No MCTS-specific tests yet**
4. ⚠️ **Manual test execution only**

**Recommendations for MCTS**:
1. ✅ **Add MCTS unit tests** from the start
2. ✅ **Set up simple CI** (GitHub Actions)
3. ✅ **Add coverage reporting** (gcov/lcov)

---

## 5. Code Quality Issues

### Known Issues from TODO

**Low Priority** (from source/TODO):
- 640x480 building dialog issues
- Redraw problems with Win9x (ancient OS)
- Dynamic_array → std::vector migration (partially done)
- Reaction fire edge cases
- Statistics screen improvements

**Assessment**: ⚠️ **Legacy issues**, not blockers for MCTS

---

## 6. MCTS-Specific Readiness

### Current State ✅ Good Foundation
- ✅ C++23 enabled and working
- ✅ Build system functional
- ✅ MCTS module structure in place
- ✅ Basic domain layer implemented
- ✅ Documentation exists

### Blockers for MCTS Development ✅ None Critical
- No immediate blockers
- Can proceed with MCTS implementation

### Nice-to-Have Before MCTS
1. Set up basic CI/CD
2. Add MCTS test framework
3. Fix Lua detection issue (for scripting integration)

---

## 7. Critical Issues Summary

### 🔴 CRITICAL (Security/Stability)

**#1: SDL 1.2 End-of-Life** ❌ BLOCKER FOR PRODUCTION
- **Impact**: Security vulnerabilities, no modern OS support
- **Effort**: High (3-6 weeks)
- **Timeline**: Before production release
- **MCTS Impact**: None (can develop on SDL 1.2)
- **Action**: Plan SDL2 migration after MCTS phase 1

### 🟡 HIGH PRIORITY (Technical Debt)

**#2: No CI/CD Infrastructure** ⚠️ HINDERS DEVELOPMENT
- **Impact**: Manual testing, slow feedback
- **Effort**: Low (1-2 days)
- **Timeline**: Before MCTS phase 2
- **Action**: Set up GitHub Actions for automated builds/tests

**#3: Lua Detection Issue** ⚠️ MINOR BLOCKER
- **Impact**: Scripting integration blocked
- **Effort**: Low (30 minutes)
- **Timeline**: When needed for MCTS scripting
- **Action**: Fix pkg-config or use lua5.1 explicitly

### 🟢 MEDIUM PRIORITY (Quality of Life)

**#4: Ancient Bundled Libraries** ⚠️ MAINTENANCE BURDEN
- **Impact**: Harder to maintain, security concerns
- **Effort**: Variable (weeks to months)
- **Timeline**: Long-term (post-MCTS)
- **Action**: Document, plan gradual replacement

**#5: Minimal Test Coverage** ⚠️ RISK
- **Impact**: Regressions harder to catch
- **Effort**: Medium (ongoing)
- **Timeline**: Continuous
- **Action**: Add tests as you develop MCTS

---

## 8. Recommendations for MCTS Development

### ✅ SAFE TO PROCEED

You can **start MCTS development immediately**. No critical blockers.

### Recommended Actions BEFORE Major MCTS Work

**Priority 1** (1-2 days):
1. ✅ Set up basic CI/CD (GitHub Actions)
   ```yaml
   # .github/workflows/build.yml
   - Build on push
   - Run unit tests
   - Check C++23 compatibility
   ```

2. ✅ Create MCTS test framework
   ```cpp
   // source/ai/mcts/test/
   - mcts_test_main.cpp
   - game_state_test.cpp
   - tree_policy_test.cpp
   ```

**Priority 2** (30 min - 1 hour):
3. ✅ Fix Lua detection
   ```bash
   # Try: pkg-config --cflags lua5.1
   # Or: export PKG_CONFIG_PATH=/usr/lib/pkgconfig
   ```

4. ✅ Document current MCTS dependencies
   - What ASC APIs are you using?
   - What's the integration boundary?

**Priority 3** (Ongoing):
5. ✅ Add tests as you develop
6. ✅ Document design decisions
7. ✅ Keep C++23 compatibility

### DON'T WORRY ABOUT (Can defer):
- ❌ SDL2 migration (do after MCTS works)
- ❌ ParaGUI replacement (GUI not needed for MCTS AI)
- ❌ Legacy TODO items (not MCTS-related)

---

## 9. Build System Health Check

### Quick Verification ✅
```bash
# Test current build
./configure
make clean
make -j$(nproc)
make check  # Run unit tests

# Expected: All pass with C++23
```

### Build Performance
- **Parallel builds**: ✅ Supported (`make -j`)
- **Incremental builds**: ✅ Working
- **Clean builds**: ✅ Fast enough

---

## 10. Risk Matrix

| Risk | Likelihood | Impact | Priority | Mitigation |
|------|-----------|--------|----------|------------|
| SDL 1.2 security vuln | High | High | 🔴 Critical | Plan SDL2 migration |
| No CI breaks code | Medium | Medium | 🟡 High | Add CI/CD |
| Loki bugs | Low | Low | 🟢 Low | Already fixed C++23 |
| ParaGUI issues | Low | Medium | 🟢 Low | Well-tested |
| MCTS integration bugs | Medium | Medium | 🟡 High | Add tests |

---

## 11. Decision Matrix

| Should I... | Answer | Reason |
|------------|--------|--------|
| Start MCTS development now? | ✅ **YES** | No blockers |
| Upgrade Loki to 0.1.7? | ❌ **NO** | Already fixed 0.1.6 |
| Set up CI/CD first? | ✅ **YES** | 1-2 days, big benefit |
| Migrate to SDL2 first? | ❌ **NO** | Can do after MCTS |
| Replace ParaGUI? | ❌ **NO** | Not needed for AI |
| Fix Lua detection? | ⏸️ **MAYBE** | Only if using Lua scripting |
| Add MCTS tests? | ✅ **YES** | Essential for quality |

---

## 12. Action Plan

### Phase 0: Pre-MCTS Setup (1-2 days)
- [ ] Set up GitHub Actions CI/CD
- [ ] Create MCTS test framework structure
- [ ] Document MCTS dependencies on ASC core
- [ ] (Optional) Fix Lua pkg-config issue

### Phase 1: MCTS Development (Your current phase)
- [ ] Implement MCTS core with tests
- [ ] Integrate with ASC game state
- [ ] Performance benchmarking
- [ ] All work in C++23

### Phase 2: Post-MCTS (Future)
- [ ] Plan SDL2 migration
- [ ] Consider ParaGUI alternatives
- [ ] Expand test coverage
- [ ] Update other bundled libraries

---

## 13. Conclusion

### Overall Assessment: ⚠️ MODERATE TECHNICAL DEBT

**Good News** ✅:
- Build system is modern
- C++23 migration complete
- Basic tests exist
- MCTS can proceed

**Concerns** ⚠️:
- Ancient dependencies (SDL 1.2, ParaGUI)
- No automated testing
- Heavily patched bundled libraries

**Bottom Line**: 
🎯 **START MCTS DEVELOPMENT NOW**

Set up CI/CD in parallel, but don't block on it. The technical debt is manageable and doesn't prevent MCTS work. Address SDL2 migration after proving MCTS value.

---

**Prepared by**: AI Assistant  
**Date**: 2025-11-07  
**Next Review**: After MCTS Phase 1 completion
