# dev-check.sh (Local Gate)

Quick, low-noise local gate that mirrors CI’s “does it build and run tests?” steps using an out-of-tree CMake/Ninja build.

## What it does
- Configures (if needed) an out-of-tree build directory.
- Builds incrementally (`cmake --build`).
- Runs `ctest` with failures surfaced; skips static analysis/clang-tidy by default to stay quiet.

## Usage
```bash
./dev-check.sh
```

## Environment knobs
- `BUILD_DIR` (default `build-dev`): out-of-tree build directory; reused across runs.
- `BUILD_TYPE` (default `Debug`): passed to `-DCMAKE_BUILD_TYPE`.
- `RECONFIGURE=1`: force a fresh CMake configure.
- `GENERATOR="Ninja"`: override generator; falls back to default if unset and Ninja is unavailable.

## Notes
- Artifacts stay in `BUILD_DIR`; source tree remains clean.
- Script is incremental: if `CMakeCache.txt` exists and `RECONFIGURE` is not set, it skips reconfigure.
- Output is minimal; focus is on build/test breakages. Add `RUN_TIDY` back into the script if you need clang-tidy locally.
