# SDL3 Migration - Root Cause Analysis and Resolution

## Executive Summary

**Previous Assessment was INCORRECT:** The migration summary claimed "the system does not provide SDL with support for x11 or wayland" - this is **completely false**.

**Actual Root Cause:** The application segfaults because it links to **THREE different SDL versions simultaneously**, causing memory corruption and symbol conflicts.

**✅ RESOLVED:** By using SDL3_mixer (which only depends on SDL3) and removing SDL_sound dependency, the application now runs successfully with only SDL3 linked.

## System SDL3 Validation

### SDL3 Installation Status ✅
- **Version:** 3.2.26 
- **Location:** `/usr/local/lib/libSDL3.so.0`
- **pkg-config:** Working correctly

### Video Driver Support ✅
```
Available video drivers (6):
  0: wayland  ✅
  1: x11      ✅
  2: kmsdrm   ✅
  3: offscreen
  4: dummy
  5: evdev
```

### Verification Tests ✅
Created standalone test program that successfully:
- Initializes SDL3
- Creates windows with X11 driver
- Creates windows with Wayland driver
- Reports version: SDL3 3.2.26

**Conclusion:** System SDL3 is **fully functional** with complete X11 and Wayland support.

## The Real Problem: Multiple SDL Version Conflict

### Current Binary Linking (BROKEN):
```
ldd ./source/unix/asc/asc | grep SDL:
	libSDL3_image.so.0 => /usr/local/lib/libSDL3_image.so.0     ✅ SDL3
	libSDL3.so.0       => /usr/local/lib/libSDL3.so.0            ✅ SDL3
	libSDL-1.2.so.0    => /lib/x86_64-linux-gnu/libSDL-1.2.so.0  ❌ SDL 1.2
	libSDL2-2.0.so.0   => /lib/x86_64-linux-gnu/libSDL2-2.0.so.0 ❌ SDL 2.0
	libSDL_sound-1.0.so.1 => /lib/x86_64-linux-gnu/libSDL_sound-1.0.so.1 ⚠️ Pulls in SDL 1.2/2.0
```

### Crash Location
```
[ASC SDL3] spawning event thread result handle=0x5c8eeaab5f60
Segmentation fault (core dumped)
```

### GDB Backtrace Shows Conflict:
```
Thread 1 "asc" received signal SIGSEGV, Segmentation fault.
#0  0x00007ffff6cfe0e4 in ?? () from /lib/x86_64-linux-gnu/libSDL2-2.0.so.0
#1  0x00007ffff7ef9fe3 in SDL_LockSurface () from /lib/x86_64-linux-gnu/libSDL-1.2.so.0
#2  0x000055555595cdc3 in PG_Draw::ScaleSurface at rotozoom.cpp:704
```

The crash occurs when SDL3 code tries to use surfaces that were created/managed by SDL 1.2 or SDL 2.0, causing memory corruption.

## Root Cause Analysis

**The problem:**
1. Application code uses SDL3 (via compatibility layer)
2. Application links to `libSDL_sound` (system library)
3. System `libSDL_sound` was built against SDL 1.2
4. SDL 1.2 transitively depends on SDL 2.0
5. Three SDL versions coexist in same process space
6. Symbol conflicts and ABI incompatibilities cause segfault

**Why it crashes specifically after thread spawn:**
The event thread starts calling `SDL_PollEvent()`, which internally uses SDL surfaces and structures. When these cross the boundary between SDL versions, the different ABIs cause memory corruption.

## Solution Options

### Option 1: Remove SDL_sound Dependency (RECOMMENDED for testing)
Configure without sound support to eliminate SDL version conflicts:
```bash
# Remove SDL_sound from the build
# Edit configure.ac to skip SDL_sound checks or
# Use configure flags to disable sound
```

### Option 2: Rebuild SDL_sound Against SDL3
Build SDL_sound from source linked against SDL3 instead of SDL 1.2:
```bash
# This requires:
# 1. SDL_sound source code
# 2. Patches to make it SDL3-compatible  
# 3. Rebuild and install to /usr/local
```

### Option 3: Use Internal SDL_sound with SDL3 Compatibility
The project has internal SDL_sound in `source/libs/sdl_sound/`, but it needs:
- Audio format constant mapping (partially done)
- SDL_ThreadID() → SDL_GetCurrentThreadID() fixes
- Case statement deduplication (unsigned→signed format mapping)

## Compatibility Layer Status

### What Works ✅
- Event structures (SDL_Event translation)
- Surface operations (SDL_Surface wrappers)
- Video initialization
- Window creation
- Renderer creation
- Thread creation
- Mutex operations

### What Needs Fixing ❌
- Audio format constants (partially fixed)
- Mixed SDL version linking (CRITICAL)
- Internal SDL_sound compilation

## Fixes Applied

1. **Audio Format Constants:** Added mappings in `source/sdl/compat/SDL.h`:
   - Mapped SDL 1.2 audio constants to SDL3 equivalents
   - Handled unsigned 16-bit formats (removed in SDL3) → mapped to signed

2. **Build Configuration:** Tested both system and internal SDL_sound options

## Recommendations

1. **Immediate:** Remove SDL_sound dependency to eliminate SDL version conflicts
2. **Short-term:** Implement audio using SDL3's native audio API
3. **Long-term:** Complete SDL_sound→SDL3 port if audio file decoding is required

## Testing Evidence

Created `/home/vboxuser/projects/test-sdl/test_system_sdl3.c` which proves:
- SDL3 initialization works
- X11 video driver works  
- Wayland video driver works
- Window creation succeeds
- System SDL3 is fully functional

The migration issues are **NOT** due to missing video drivers, but due to **library version conflicts**.

---

## RESOLUTION (2025-11-02)

### Problem Solved ✅

The three-SDL-version linking conflict has been **successfully resolved**.

### Solution Implemented

**Approach:** Use SDL3_mixer instead of SDL_mixer 1.2, and remove SDL_sound dependency.

**Changes Made:**

1. **Modified configure.ac:**
   - Accept SDL3_mixer >= 3.1.0 (previously required >= 3.2.0)
   - Skip SDL_sound checks when SDL3_mixer is available
   - Ensure SDL_MIXER_CFLAGS is preserved in final CXXFLAGS/CFLAGS

2. **Created SDL_sound stubs:**
   - `source/sdl/compat/SDL_sound_stub.h` - Stub declarations
   - `source/sdl/compat/SDL_sound_stub.cpp` - Stub implementations
   - These allow compilation without system SDL_sound

3. **Modified sound.cpp:**
   - Conditionally include SDL_sound headers only when not using SDL3_mixer
   - Skip Sound_Init() when using SDL3_mixer (it has built-in decoders)

4. **Updated build system:**
   - Added SDL_sound_stub.cpp to libsdlsnd sources
   - Regenerated configure and Makefiles

### Verification Results

**Library Dependencies (After Fix):**
```bash
$ ldd ./source/unix/asc/asc | grep SDL
	libSDL3_image.so.0 => /usr/local/lib/libSDL3_image.so.0  ✅ SDL3 only
	libSDL3.so.0       => /usr/local/lib/libSDL3.so.0        ✅ SDL3 only
```

**NO SDL 1.2 or SDL 2.0 dependencies!** ✅

**Runtime Test:**
```bash
$ export LD_LIBRARY_PATH=/home/vboxuser/projects/SDL_mixer/install/lib:$LD_LIBRARY_PATH
$ export SDL_VIDEODRIVER=dummy
$ ./source/unix/asc/asc -w -q -r 3
Advanced Strategic Command
Version: ASC2.8.3.1
adding search patch ~/.asc/
[ASC SDL3] PG_Theme::Load 'asc2_dlg' begin
[ASC SDL3] checking theme archive asc2_dlg.zip
... (application runs successfully)
```

**✅ NO SEGFAULT!** Application initializes and runs correctly.

### Build Configuration

**Required Environment:**
```bash
export PKG_CONFIG_PATH=/home/vboxuser/projects/SDL_mixer/install/lib/pkgconfig:$PKG_CONFIG_PATH
export LD_LIBRARY_PATH=/home/vboxuser/projects/SDL_mixer/install/lib:$LD_LIBRARY_PATH
./configure
make
```

**SDL3_mixer Location:**
- Path: `/home/vboxuser/projects/SDL_mixer/install/`
- Version: 3.1.0
- Built from upstream main branch

### Current Limitations

1. **Sound effects disabled:** The SDL_sound stubs return NULL, so sound effects won't play
2. **Music should work:** SDL3_mixer handles music files directly via Mix_LoadMUS

### Next Steps

1. Test music playback functionality
2. Implement sound effects using SDL3_mixer's Mix_LoadWAV (replaces SDL_sound)
3. Remove temporary build logs and backup files
4. Test with actual X11/Wayland video drivers (not just dummy)
5. Run comprehensive game testing

### Conclusion

The root cause was definitively proven to be the three-SDL-version linking conflict caused by system SDL_sound depending on SDL 1.2/2.0. By eliminating this dependency and using SDL3_mixer exclusively, the application now links only to SDL3 and runs without segfaults.

**The migration to SDL3 is technically successful.**

---

## FINAL STATUS - November 2025 (Branch Discontinued)

### Current State

**Build Status:** ✅ Compiles successfully
**Runtime Status:** ✅ Runs and displays graphics
**Sound:** ❌ No audio (SDL_sound stubs, needs SDL3_mixer integration)
**Video:** ✅ Works with X11/Wayland

### Critical Issues Discovered

#### 1. **SDL Surface Memory Leak** (2MB, ~1988 objects)

**Root Cause:** SDLmm wrapper incompatibility with SDL3 reference counting

```cpp
// SDLmm Surface::operator= increments refcount manually
Surface &operator=(const Surface& other) {
    BaseSurface::operator=(other);
    if (me)
        ++(me->refcount);  // ← Causes surfaces to never reach refcount=0
    return *this;
}
```

**Impact:**
- Surfaces copied 1000+ times leak memory at exit (refcount never reaches 0)
- ~2MB leaked at shutdown (harmless, OS reclaims)
- See: `SDL3_SURFACE_LEAK_ANALYSIS.md` for detailed analysis

**Attempted Fixes:**
- Forcing `refcount=1` before free → **Crashed** (use-after-free)
- Deep copy implementation → **Compiled but not fully tested**

#### 2. **Fog of War Flickering**

**Root Cause:** Surface sharing due to refcount increment behavior

**Evidence:**
```
Surface at 0x5120000442c0 with refcount 1526
- Shared between 1526+ fog tiles
- Modifying one tile affects all tiles sharing the surface
- Causes visible flickering when units move
```

**Impact:**
- Fog tiles share same SDL_Surface memory
- Rendering artifacts and flickering
- Affects gameplay visibility

#### 3. **Performance Degradation**

**Observation:** Game feels laggy compared to SDL 1.2 version on main branch

**Possible Causes:**
- Compatibility layer overhead
- Excessive surface copying
- Missing SDL3 optimizations (RLE, hardware acceleration)

### Why This Branch Is Being Discontinued

#### **Strategic Decision: Client/Server Architecture**

After extensive investigation, we determined that:

1. **The SDL3 migration solves the wrong problem**
   - Problem: UI coupled to old SDL/Paragui
   - Wrong solution: Port everything to SDL3 with compatibility layer
   - Right solution: Decouple UI from game engine entirely

2. **Refactoring while migrating = Double complexity**
   - Fighting SDL compatibility issues
   - Fighting architectural issues
   - Better to start clean

3. **Modern architecture doesn't need SDL in engine**
   ```
   Headless Game Engine (no SDL)
   ↕ IPC/Network Protocol
   Modern UI Client (fresh SDL3 or Godot)
   ```

4. **Starting from stable SDL 1.2 is faster**
   - Extract proven game logic from working codebase
   - Build clean headless engine
   - Build fresh UI without legacy baggage
   - No SDL compatibility issues to debug

### Lessons Learned

✅ **SDL3 is viable** - System integration works, drivers functional
✅ **Identified coupling issues** - Surface sharing, refcounting problems
✅ **Validated architecture decision** - Client/server is the right path
❌ **Compatibility layer is fragile** - SDLmm wrapper causes subtle bugs
❌ **Incremental migration too complex** - Clean rewrite is faster

### Recommendations for Future Work

**DO NOT continue this branch. Instead:**

1. **Branch from stable SDL 1.2 (main/headless)**
2. **Extract game logic to headless library** (no SDL dependency)
3. **Build new UI client:**
   - Option A: Fresh SDL3 + Dear ImGui (C++)
   - Option B: Godot frontend (GDScript/C++)
4. **Communication via IPC/network protocol**

See: `docs/ui-refactor.md` for architectural plan

### Files Created During Investigation

- `SDL3_SURFACE_LEAK_ANALYSIS.md` - Detailed leak analysis and fix options
- `asan_suppressions.txt` - Leak suppressions (if continuing this branch)
- `source/resourcelifecycle.h` - Cleanup coordination system
- Various debug logging (can be removed)

### Branch Preservation

This branch is preserved for reference:
- Shows what NOT to do (compatibility layer complexity)
- Documents SDL3 integration challenges
- Provides insights for clean client implementation

**Status:** Archived, not recommended for production use

---

## Previous Conclusion (Kept for Historical Context)

The root cause was definitively proven to be the three-SDL-version linking conflict caused by system SDL_sound depending on SDL 1.2/2.0. By eliminating this dependency and using SDL3_mixer exclusively, the application now links only to SDL3 and runs without segfaults.

**The migration to SDL3 is technically successful** but architecturally flawed.
