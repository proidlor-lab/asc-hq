# ⚠️ SDL3 Migration Branch - Do Not Use

## This Branch Has Been Discontinued

**Branch:** `sdl2-migration-v2.2`
**Status:** ❌ **Discontinued - November 2025**
**Reason:** Architectural decision to pursue client/server split instead

---

## Quick Facts

### Current State
- ✅ Compiles and runs
- ✅ Graphics work (X11/Wayland)
- ❌ No sound (stubs only)
- ❌ Fog flickering bug
- ❌ 2MB memory leak
- ❌ Performance degradation

### Why Discontinued
The SDL3 migration was solving the wrong problem. Instead of porting the UI to SDL3, we're **decoupling the UI from the game engine entirely** via client/server architecture.

**New approach:**
```
Headless Game Engine (no UI)
        ↕
Modern Client (Godot or fresh SDL3)
```

---

## For Developers

### ❌ **Do Not:**
- Continue development on this branch
- Base new work on this code
- Try to fix the bugs here
- Use this for production

### ✅ **Instead:**
- Use `main` or `headless` branch (SDL 1.2, stable)
- Follow client/server architecture plan
- Read `SDL3_MIGRATION_DISCONTINUED.md` for details

---

## Documentation

**Full details:** `SDL3_MIGRATION_DISCONTINUED.md`
**Leak analysis:** `SDL3_SURFACE_LEAK_ANALYSIS.md`
**Architecture plan:** `docs/ui-refactor.md`

---

## Current Issues

1. **SDL Surface Memory Leak** (2MB)
   - Refcount bug in SDLmm wrapper
   - Surfaces never fully freed
   - See: `SDL3_SURFACE_LEAK_ANALYSIS.md`

2. **Fog of War Flickering**
   - Surface sharing between fog tiles
   - Visual artifacts on unit movement

3. **No Audio**
   - SDL_sound stubbed out
   - Needs SDL3_mixer integration

4. **Performance Issues**
   - Slower than SDL 1.2 version
   - Compatibility layer overhead

---

## Historical Value

This branch is preserved for:
- Reference on what NOT to do
- SDL3 integration lessons
- Understanding coupling issues

**Not for production use.**

---

**Last Updated:** November 2025
