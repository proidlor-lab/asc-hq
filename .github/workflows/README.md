# CI/CD Workflows

This directory contains GitHub Actions workflows for building and testing ASC-HQ.

## Workflows

### ci.yml - Main CI Pipeline

The main continuous integration pipeline runs on all pushes and pull requests.

#### Jobs

1. **build-gui** - Autotools build with GUI support
   - Platform: Ubuntu 24.04
   - Build system: Autotools
   - Features: Full GUI, all features enabled
   - Uses ccache for faster builds

2. **build-headless** - Autotools build headless mode
   - Platform: Ubuntu 24.04
   - Build system: Autotools
   - Features: Headless mode, no GUI dependencies
   - Uses ccache for faster builds

3. **cmake-build** - CMake build (MCTS module) ✨ NEW
   - Platform: Ubuntu 24.04
   - Build system: CMake + Ninja
   - Scope: MCTS AI module only (Phase 1)
   - Features:
     - Out-of-tree builds (verifies clean source tree)
     - Builds and runs all MCTS tests
     - Verifies build artifacts are correctly placed
     - Uses ccache for faster builds
   - **This job validates the CMake migration!**

4. **mcts-tests-autotools** - MCTS module tests (Autotools)
   - Platform: Ubuntu 24.04
   - Build system: Autotools
   - Scope: MCTS module test executables
   - Status: Tests run with `continue-on-error: true` (not blocking)

5. **static-analysis** - Static code analysis
   - Platform: Ubuntu 24.04
   - Tools: cppcheck, clang-tidy
   - Uploads analysis reports as artifacts

6. **build-status** - Summary of all builds
   - Waits for all jobs to complete
   - Reports status of each build
   - **Fails if CMake build fails** (enforces CMake migration quality)

## CMake Build Details

The `cmake-build` job performs these steps:

1. **Install dependencies** - CMake, Ninja, SDL, sigc++, ccache
2. **Generate config.h** - Runs Autotools configure to generate config.h (needed during transition)
3. **Configure CMake** - Out-of-tree build in `build/` directory
4. **Build with Ninja** - Fast parallel build
5. **Verify artifacts** - Checks that `libmcts.a` and test executables exist
6. **Run tests** - Executes all MCTS tests (snapshot, action_executor, evaluator)
7. **Verify clean source** - Ensures no build artifacts pollute source tree
8. **Upload logs on failure** - CMake logs available for debugging

### Expected Build Outputs

```
build/
├── lib/
│   └── libmcts.a          # MCTS library (650KB)
├── bin/
│   ├── snapshot_test      # Test executable (51KB)
│   ├── action_executor_test # Test executable (190KB)
│   └── evaluator_test     # Test executable (118KB)
```

## Build Matrix (Current)

| Job | Build System | Scope | Status Required |
|-----|-------------|-------|-----------------|
| build-gui | Autotools | Full project | ✓ Yes |
| build-headless | Autotools | Full project | ✓ Yes |
| **cmake-build** | **CMake** | **MCTS only** | **✓ Yes** |
| mcts-tests-autotools | Autotools | MCTS only | ⚠️ Optional |
| static-analysis | N/A | Source code | ⚠️ Optional |

## Future Migration Plan

### Phase 1 (Current) ✓
- CMake builds MCTS module
- Parallel builds (Autotools + CMake)
- Both systems validated in CI

### Phase 2 (Planned)
- Expand CMake to more modules
- Add CMake build for full project
- Deprecate Autotools jobs gradually

### Phase 3 (Future)
- CMake only
- Remove Autotools jobs
- Faster CI (Ninja builds)

## Troubleshooting CI Failures

### CMake build fails: "Could not find SDL"

The workflow installs SDL via `apt-get`. If this fails, check:
- Ubuntu version (workflow uses 24.04)
- Package availability in Ubuntu repositories

### CMake build fails: "config.h not found"

During transition, CMake needs `config.h` from Autotools. The workflow generates it via:

```bash
./bootstrap
./configure
make clean
```

If this step fails, the CMake build will fail.

### Tests fail in cmake-build job

Check the test output in the job logs. Tests run from `build/bin/`:

```bash
cd build/bin
./snapshot_test
./action_executor_test
./evaluator_test
```

### Build artifacts in source tree

The workflow verifies out-of-tree builds:

```bash
find source/ai/mcts -name "*.o" -o -name "*.a"
```

If this finds artifacts, the build fails. This indicates:
- In-source build occurred (should never happen with CMake)
- Autotools artifacts from previous run (CI should be clean)

## Local Testing

To reproduce CI builds locally:

### Test Autotools build:

```bash
./bootstrap
./configure --enable-debug
make clean
make -j$(nproc)
```

### Test CMake build (matches CI):

```bash
# Generate config.h (needed during transition)
./bootstrap
./configure
make clean

# CMake build
mkdir build
cd build
cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug ..
ninja -v

# Run tests
cd bin
./snapshot_test
./action_executor_test
./evaluator_test
```

## Performance

### Build Times (Approximate)

| Job | Duration | Notes |
|-----|----------|-------|
| build-gui | ~5-10 min | With ccache |
| build-headless | ~5-10 min | With ccache |
| **cmake-build** | **~2-3 min** | **Faster! (Ninja + smaller scope)** |
| mcts-tests-autotools | ~3-5 min | Depends on main build |
| static-analysis | ~2-4 min | Parallel to builds |

CMake builds are significantly faster due to:
- Ninja backend (parallel by default)
- Smaller scope (MCTS only)
- Better dependency tracking

## Caching Strategy

The workflow caches:
- **ccache** - Compiled objects (500MB limit)
- **CMake build directory** - Incremental builds

Cache keys are based on:
- `configure.ac` hash (Autotools)
- `CMakeLists.txt` hash (CMake)

This speeds up subsequent builds significantly.
