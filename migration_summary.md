# SDL3 Migration Progress

## Completed Work
- Added comprehensive SDL3 compatibility layer in `source/sdl/compat/compat.cpp`, including surface handling, event translation, timers, RWops, and image-loading bridge functions.
- Introduced shim headers under `source/sdl/compat` (audio, rwops, thread, mixer, version, etc.) and legacy include directory to redirect third-party SDL1.2 headers (Paragui, SDL_sound) toward the SDL3 API.
- Created a fallback `SDL_mixer` stub (`compat/mixer_stub.cpp`) so legacy code can compile when SDL3_mixer is unavailable, and updated `source/sdl/Makefile.am` accordingly.
- Updated graphics pipeline code (`source/graphics/surface.cpp`, `source/graphics/blitter.h`, and various UI modules) to rely on new compat helpers instead of removed SDL1.2 structures like `SDL_PixelFormat` and surface clip members.
- Added numerous macro/typedef bridges in `source/sdl/compat/SDL.h` and `SDL_image.h` to map renamed SDL3 identifiers back to the SDL1.2-era names used throughout the codebase.

## Current Build & Runtime Status
- Incremental `make -j2` builds (with SDL3_mixer and ASan) continue to succeed.
- Running the binaries with `SDL_VIDEODRIVER=x11` on a workstation that has a real display driver now brings up the UI, loads maps, and executes commands without crashes—as long as the shared game data under `/usr/local/share/games/asc` comes from a pristine branch.
- If `make install` is invoked from the experimental SDL3 branch, it overwrites `/usr/local/share/games/asc` with corrupt assets (most notably `main.ascdat`/palette data). Subsequent runs on *any* branch then show black unit silhouettes and can loop indefinitely while logging palette conversions until the clean assets are restored.
- Re-running `make install DESTDIR=...` (or just `make install`) from an untouched branch replaces the corrupted assets and immediately fixes the rendering for every build, confirming the executable itself is fine.

## Recent Findings
- The palette corruption reproduces only after installing data from the SDL3 branch. Installing from a clean branch fixes *all* builds, so the fault lies in the generated assets, not the new renderer.
- Temporary logging guarded by the `ASC_DEBUG_PALETTE=1` environment variable shows that 8-bit unit sprites retain full palettes, the `ColorTransform_PlayerTrueCol` math outputs correct colors, and `ColorMerger_AlphaOverwrite` writes those values into the destination surface. This proves the runtime color pipeline is sound; the corrupted on-disk assets are the real culprit.
- The runaway logging (`ColorConverter<1,4> index=0 ...`) happens because the bad assets contain large monochrome regions that trigger the per-pixel trace—further evidence that the assets, not the code, are wrong.

## Known Risks
- `make install` writes binaries *and* data into `/usr/local/share/games/asc`. Installing from the SDL3 branch currently produces malformed palette/graphics bundles and can taint other branches until the data is overwritten again.
- The SDL logging hooks are verbose and will bloat logs if `ASC_DEBUG_PALETTE` is left set. Remove or guard them once we finish investigating asset issues.

## Debug Logging Touchpoints
The following files contain temporary SDL3-specific tracing (enabled when `ASC_DEBUG_PALETTE=1`):
- `source/graphicset.cpp` – dumps the first loaded unit surface, palette, and initial pixels.
- `source/graphics/surface.cpp` – reports when the default palette is assigned to palettized surfaces.
- `source/graphics/ColorTransform_PlayerColor.h` – logs player-color translation results.
- `source/graphics/blitter.h` – traces `ColorConverter<1,4>`, `ColorMerger_AlphaOverwrite<4>`, and `ColorMerger_AlphaMixer<4>` operations.

## Recommended Next Steps
1. Back up `/usr/local/share/games/asc` before testing new installer builds, or install into an alternate prefix (e.g., `make install DESTDIR=/tmp/asc-staging`).
2. Diff the asset outputs from the SDL3 branch against the known-good branch to pinpoint what in the packaging process corrupts the data (likely the palette builder step inside `main.ascdat` generation).
3. Once the asset generator is understood, remove the temporary palette/logging hooks and re-verify rendering on a clean system.

## Cache Investigation Status
- Instrumented `PackageRepository` and `FileCache` to trace cache contents; verified the synthetic `ASC` package no longer carries stale dependencies, and the `FileInfo` list matches the known-good cache.
- Despite the package fix, the SDL3 branch still produces an `asc2.cache` (96 313 741 B) that diverges from the good cache at byte ~2475, so the corruption now lies in the next serialized data set (likely one of the item repositories).
- Next step is to keep logging enabled, delete `~/.asc`, rebuild the cache, and extend tracing to the subsequent data loaders so we can pinpoint which block first diverges and patch that writer.
