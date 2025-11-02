# SDL3 Migration - Root Cause Analysis

## Executive Summary

**Previous Assessment was INCORRECT:** The migration summary claimed "the system does not provide SDL with support for x11 or wayland" - this is **completely false**.

**Actual Root Cause:** The application segfaults because it links to **THREE different SDL versions simultaneously**, causing memory corruption and symbol conflicts.

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
