# SDL3 Migration Branch - Discontinued

## ⚠️ **This Branch Is No Longer Active**

**Date Discontinued:** November 2025
**Branch Name:** `sdl2-migration-v2.2`
**Reason:** Architectural decision to pursue client/server split instead

---

## Quick Summary

### What Works ✅
- Compiles successfully
- Runs and displays graphics
- X11/Wayland video support
- No crashes (after SDL_sound removal)

### What Doesn't Work ❌
- Sound/audio (stubs only, needs SDL3_mixer integration)
- Memory leak: 2MB in SDL surfaces at exit (cosmetic)
- Fog of war flickering (surface sharing bug)
- Performance slightly degraded vs SDL 1.2

### Why We Stopped

**The SDL3 migration was solving the wrong problem.**

**Problem:** Legacy UI (Paragui) tightly coupled to SDL
**Wrong approach:** Port everything to SDL3 with compatibility layer
**Right approach:** Decouple UI from game engine entirely

**New plan:** Client/Server Architecture
```
Headless Game Engine (C++, no SDL)
        ↕ Network/IPC
Modern UI Client (Godot or SDL3+ImGui)
```

---

## Technical Issues Discovered

### 1. SDL Surface Refcount Leak

**File:** `source/libs/sdlmm/src/sdlmm_surface.h:76`

```cpp
Surface &operator=(const Surface& other) {
    BaseSurface::operator=(other);
    if (me)
        ++(me->refcount);  // ← Manually increments SDL3 refcount
    return *this;
}
```

**Problem:**
- Each Surface copy increments refcount
- `SDL_FreeSurface()` only decrements by 1
- Surfaces copied 1000+ times never free memory
- ~1988 surfaces leak ~2MB at program exit

**Impact:** Cosmetic (OS reclaims on exit), but indicates architectural issue

**See:** `SDL3_SURFACE_LEAK_ANALYSIS.md` for detailed analysis

### 2. Fog of War Flickering

**Cause:** Multiple fog tiles share same SDL_Surface due to refcount behavior

**Evidence:**
```
[ASC] Freeing SDL_Surface 0x5120000442c0 (count: 700, refcount: 1526)
```
One surface shared by 1526 objects!

**Impact:** Modifying one fog tile affects all tiles → visual flickering

### 3. Performance Regression

**Observation:** Game feels slightly laggy vs SDL 1.2 main branch

**Possible causes:**
- Compatibility layer overhead
- Excessive surface duplication
- Missing SDL3 optimizations

---

## What We Learned

### Positive Findings ✅

1. **SDL3 system integration works**
   - Libraries compile and link correctly
   - X11/Wayland drivers functional
   - No driver or platform issues

2. **Surface compatibility is possible**
   - SDL_Surface structure similar enough
   - Basic rendering works
   - Event system translates cleanly

3. **Architecture issues are real**
   - Coupling between UI and game logic is severe
   - Paragui is deeply embedded
   - Clean separation is needed

### Negative Findings ❌

1. **Compatibility layers are fragile**
   - SDLmm wrapper has subtle bugs
   - Refcounting semantics don't match SDL3
   - Small differences cause big problems

2. **Incremental migration is expensive**
   - Fighting compatibility issues
   - Debugging subtle surface bugs
   - Time better spent on clean architecture

3. **Paragui is a dead end**
   - Ancient codebase (SDL 1.2 era)
   - No SDL3 port exists
   - Replacing it anyway in new architecture

---

## Decision: Client/Server Architecture

### Why This Is Better

**Instead of:**
```
SDL 1.2 Code → SDL3 Code (with bugs)
```

**Do:**
```
SDL 1.2 Code → Extract Game Logic → Headless Engine
                                    ↓
                            New UI (Godot/SDL3+ImGui)
```

### Advantages

1. **Start from stable base** (SDL 1.2 main branch works correctly)
2. **No SDL compatibility issues** (engine has no SDL dependency)
3. **Fresh UI without legacy** (new client, modern approach)
4. **Multiplayer-ready** (client/server from start)
5. **Faster to working product** (no debugging SDL3 quirks)

### Disadvantages

- Larger initial refactor
- More upfront architectural work

**But:** Same end goal, cleaner path, faster overall

---

## Files and Changes Made

### New Files Created
- `SDL3_SURFACE_LEAK_ANALYSIS.md` - Leak investigation
- `SDL3_MIGRATION_DISCONTINUED.md` - This file
- `asan_suppressions.txt` - Leak suppressions
- `source/resourcelifecycle.h` - Cleanup system
- `source/sdl/compat/SDL_sound_stub.*` - Sound stubs

### Modified Files
- `source/libs/sdlmm/src/sdlmm_*.h` - SDL3 compatibility
- `source/sdl/compat/*.{h,cpp}` - Compatibility layer
- `source/applicationstarter.cpp` - Cleanup registration
- Many others (see git log)

### Build System
- `configure.ac` - SDL3_mixer detection
- Various `Makefile.am` - SDL3 flags

---

## How to Use This Branch (If Needed)

### Build Instructions

```bash
# Set up SDL3_mixer path
export PKG_CONFIG_PATH=/home/vboxuser/projects/SDL_mixer/install/lib/pkgconfig:$PKG_CONFIG_PATH
export LD_LIBRARY_PATH=/home/vboxuser/projects/SDL_mixer/install/lib:$LD_LIBRARY_PATH

# Configure and build
./configure
make -j2

# Run (no sound)
./source/unix/asc/asc
```

### Known Issues

1. **No sound** - SDL_sound stubs need SDL3_mixer implementation
2. **Memory leak warnings** - Use `LSAN_OPTIONS=suppressions=asan_suppressions.txt`
3. **Fog flickering** - Avoid heavy fog usage
4. **Performance** - Slightly slower than SDL 1.2

### If You Must Continue

**Not recommended**, but if you must:

1. Fix surface sharing: Implement deep copy in `sdlmm_surface.h`
2. Integrate SDL3_mixer properly (remove stubs)
3. Profile and optimize rendering
4. Test extensively for stability

**Better:** Follow client/server plan instead

---

## Next Steps (Recommended)

### 1. Archive This Branch

```bash
git checkout main
git tag archive/sdl3-migration-v2.2 sdl2-migration-v2.2
git branch -D sdl2-migration-v2.2
```

### 2. Start Client/Server Refactor

```bash
git checkout main  # or headless branch
git checkout -b client-server-refactor
```

### 3. Follow New Architecture

See: `docs/ui-refactor.md` for plan

---

## References

- **Leak Analysis:** `SDL3_SURFACE_LEAK_ANALYSIS.md`
- **Original Report:** `SDL3_VALIDATION_REPORT.md`
- **Migration Summary:** `migration_summary.md`
- **Architecture Plan:** `docs/ui-refactor.md`

---

## Contact / Questions

If you have questions about this branch or the decision to discontinue:

1. Check documentation files listed above
2. Review git history: `git log sdl2-migration-v2.2`
3. Compare with stable: `git diff main sdl2-migration-v2.2`

**Conclusion:** This branch served its purpose - it identified problems and validated the need for architectural change. The lessons learned here will inform the client/server implementation.
