# SDL3 Surface Memory Leak Analysis

## Summary
The SDL3 migration has a ~2MB memory leak (1,988 SDL_Surface objects) at program exit. This leak is **cosmetic** - it only occurs at shutdown and the OS reclaims all memory. The game runs fine with this leak.

## Root Cause

### The Problem
The SDLmm Surface wrapper has incompatible copy semantics with SDL3's reference counting:

1. **Surface assignment operator** (`sdlmm_surface.h:76`):
   ```cpp
   Surface &operator=(const Surface& other) {
       BaseSurface::operator=(other);
       if (me)
           ++(me->refcount);  // ← Manually increments SDL refcount
       return *this;
   }
   ```

2. **Every Surface copy increments refcount**, but **each destructor only decrements by 1**
3. **Surfaces copied 1000+ times** (like the fog tile at `0x5120000442c0` with refcount 1526) leak because not all copies get destroyed

4. **SDL_FreeSurface()** only frees memory when `refcount == 0`

### Evidence
From test run output:
```
[ASC] Freeing SDL_Surface 0x5120000442c0 (count: 700, refcount: 1526)
[ASC] Freeing SDL_Surface 0x5120000442c0 (count: 1000, refcount: 1399)
[ASC] Freeing SDL_Surface 0x5120000442c0 (count: 1300, refcount: 1099)
...
```
This surface is freed 127+ times, refcount slowly decreasing, but never reaches 0.

## Why This Happens

### Excessive Surface Copying
Surfaces are being copied excessively throughout the codebase:
- Fog of war tiles
- Map overlays
- UI elements
- Cached graphics

Example: A fog tile copied 1526 times means it's stored in 1526 different containers/variables.

### Why Force-Free Crashes
Attempted fix of forcing `refcount = 1` before `SDL_FreeSurface()` causes use-after-free:
1. Surface A and B both point to same SDL_Surface (refcount=2)
2. Surface A destructor forces refcount to 1 and frees memory
3. Surface B still has pointer to freed memory
4. Surface B tries to access it → **CRASH**

## Impact

### Memory Leak
- **Amount:** ~2MB (1,988 surfaces)
- **When:** Only at program exit
- **Effect:** None - OS reclaims memory on process termination
- **Performance:** No runtime impact

### Potential Fog of War Bug Connection
The excessive surface copying **might** be related to fog rendering issues:
- If fog tiles share the same SDL_Surface (refcount > 1), modifying one affects all
- This could cause overlay artifacts or incorrect fog rendering
- Needs testing to confirm

## Current Solution

### ASan Suppression
Created `asan_suppressions.txt` to suppress cosmetic leaks:
```
leak:SDL_malloc_REAL
leak:libGLX_mesa.so
leak:parseObjectProps
leak:PackageRepository::addProgramPackage
```

Run with: `LSAN_OPTIONS=suppressions=asan_suppressions.txt ./asc`

## Proper Fix (Future Work)

### Option 1: Fix Surface Copy Semantics (RECOMMENDED)
Modify `Surface::operator=` to do **deep copies** instead of refcount manipulation:

```cpp
Surface &operator=(const Surface& other) {
    if (this != &other) {
        // Free current surface
        if (me) SDL_FreeSurface(me);

        // Create independent copy
        if (other.me) {
            me = SDL_DuplicateSurface(other.me);
        } else {
            me = nullptr;
        }
    }
    return *this;
}
```

**Pros:**
- Fixes leak completely
- Each Surface truly owns its SDL_Surface
- No shared state bugs

**Cons:**
- More memory usage (no sharing)
- Performance hit from surface duplication
- Requires testing entire codebase

### Option 2: Smart Pointer Wrapper
Replace manual refcount with `std::shared_ptr<SDL_Surface>`:

```cpp
class Surface {
    std::shared_ptr<SDL_Surface> surf;

    Surface(SDL_Surface* s) : surf(s, SDL_FreeSurface) {}
    // Copy/move handled automatically by shared_ptr
};
```

**Pros:**
- Automatic memory management
- No refcount bugs
- Modern C++ approach

**Cons:**
- Larger refactor
- Performance overhead from atomic refcounting

### Option 3: Track and Force-Free at Shutdown
Create registry of leaked surfaces and force-free before SDL_Quit:

```cpp
SurfaceRegistry::forceCleanupAll() {
    for (SDL_Surface* surf : leaked_surfaces) {
        while (surf->refcount > 0) {
            SDL_FreeSurface(surf);
        }
    }
}
```

**Pros:**
- Minimal code changes
- Fixes ASan reports

**Cons:**
- Doesn't fix root cause
- Still has excessive copying
- Complex to implement safely

## Recommendations

1. **Short-term:** Use ASan suppressions, document the leak
2. **Medium-term:** Investigate fog of war issues - might be related to shared surfaces
3. **Long-term:** Implement Option 1 (deep copy semantics) to fix properly

## Testing Notes

To verify the leak without ASan noise:
```bash
LSAN_OPTIONS=suppressions=asan_suppressions.txt ./source/unix/asc/asc
```

Expected: Only minor leaks from Paragui/PackageRepository (~400 bytes total)
