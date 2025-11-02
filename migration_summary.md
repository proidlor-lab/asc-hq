# SDL3 Migration Progress

## Completed Work
- Added comprehensive SDL3 compatibility layer in `source/sdl/compat/compat.cpp`, including surface handling, event translation, timers, RWops, and image-loading bridge functions.
- Introduced shim headers under `source/sdl/compat` (audio, rwops, thread, mixer, version, etc.) and legacy include directory to redirect third-party SDL1.2 headers (Paragui, SDL_sound) toward the SDL3 API.
- Created a fallback `SDL_mixer` stub (`compat/mixer_stub.cpp`) so legacy code can compile when SDL3_mixer is unavailable, and updated `source/sdl/Makefile.am` accordingly.
- Updated graphics pipeline code (`source/graphics/surface.cpp`, `source/graphics/blitter.h`, and various UI modules) to rely on new compat helpers instead of removed SDL1.2 structures like `SDL_PixelFormat` and surface clip members.
- Added numerous macro/typedef bridges in `source/sdl/compat/SDL.h` and `SDL_image.h` to map renamed SDL3 identifiers back to the SDL1.2-era names used throughout the codebase.

## Current Build Status
- Incremental `make -j2` builds continue to succeed.
- Runtime launch on headless boxes now exits cleanly: the compat layer enumerates the SDL3 drivers, retries each real backend, and aborts with: `SDL video driver 'offscreen' is headless; set SDL_VIDEODRIVER to a windowing backend`.
- On machines where SDL3 actually loads a desktop driver (e.g. `x11`/`wayland`), `SDL_CreateWindow` succeeds and we enter `initializeEventHandling`, but the process still segfaults shortly afterwards (no stack trace yet because ptrace is denied in the harness).

## Known Issue
- **Window available, event thread crash**: Even with `SDL_VIDEODRIVER=x11`/`wayland` set, SDL reports `SDL_GetDisplays failed` and we soon hit a segmentation fault immediately after `initializeEventHandling` starts. Need to capture a backtrace outside the sandbox or add structured logging to narrow down whether the crash happens inside the new SDL event pump, Paragui’s startup screen, or another legacy path that still expects SDL1.2 structs.

## Recommended Next Steps
1. Reproduce the crash on a workstation where `gdb`/`lldb` is allowed (or run within a container with ptrace enabled) to obtain a stack trace after the x11/wayland window is created.
2. Inspect the event/thread startup code (`source/sdl/events.cpp`, `source/paradialog.cpp`, `startupScreen`) for assumptions that still rely on SDL1.2 semantics; add guard rails or migrate remaining direct surface-pointer math if needed.
3. Once the x11/wayland path is stable, remove the temporary tracing and re-check input/event handling (`SDL_EnableUNICODE`, key-repeat stubs, Paragui supplier).
