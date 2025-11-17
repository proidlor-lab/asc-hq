# Build System Modernization

**Purpose**: Improve build reliability, reproducibility, and developer experience.

**Created**: 2025-11-16
**Status**: Analysis & Planning

---

## Current Problems

### 1. Build Artifacts in Source Tree

**Problem**: Build artifacts mixed with source code
- Object files (.o, .lo) in source directories
- Generated files (Makefile.in, configure) committed to Git
- Dependency files (.deps/) scattered throughout

**Java/Maven equivalent**: Like committing `target/` directory

**Impact**:
- Local builds succeed due to stale artifacts
- CI builds fail with fresh checkout
- Hard to know what's source vs generated
- Merge conflicts in generated files

### 2. In-Tree vs Out-of-Tree Builds

**Current**: In-tree builds (build artifacts in source directory)

**Problem**:
```bash
# Build artifacts mixed with source
source/
  gamemap.cpp         # Source
  gamemap.o           # Build artifact (gitignored)
  gamemap.lo          # Libtool artifact (gitignored)
  .deps/gamemap.Po    # Dependency file (gitignored)
```

**Java/Maven equivalent**: Building in `src/` instead of `target/`

### 3. Missing Include Detection

**Problem**: Missing `#include` directives not caught locally
- Transitive includes "accidentally work"
- Example: File A includes File B, File B includes <vector>, File A uses std::vector without including it
- Works until someone removes the include from File B

**Result**: "It builds on my machine" syndrome

---

## State-of-the-Art Solutions

### 1. Include-What-You-Use (IWYU)

**Best tool for catching missing includes**

Install:
```bash
sudo apt-get install iwyu
```

Usage:
```bash
# Analyze single file
iwyu source/ai/mcts/infrastructure/mcts_config_loader.cpp

# Fix automatically
iwyu_tool.py -p . source/ai/mcts/ | fix_includes.py
```

**Example output**:
```
mcts_config_loader.h should add these lines:
#include <vector>  // for vector

mcts_config_loader.h should remove these lines:
- #include <memory>  // Not used

The full include-list for mcts_config_loader.h:
#include <string>
#include <map>
#include <vector>
#include "core/mcts_config.h"
```

**Benefits**:
- Finds missing includes BEFORE CI
- Removes unnecessary includes (faster compilation)
- Enforces include hygiene

### 2. Stricter Compiler Flags

**Add to configure.ac**:
```bash
# Catch more errors at compile time
CXXFLAGS="-Werror=missing-include-dirs \
          -Werror=undef \
          -Wno-deprecated \
          -pedantic"
```

**For MCTS module** (modern C++23):
```makefile
AM_CXXFLAGS = -std=c++23 \
              -Wall -Wextra -Wpedantic \
              -Werror=return-type \
              -Werror=switch \
              -fno-omit-frame-pointer
```

### 3. Out-of-Tree Builds

**Standard practice in modern C++ projects**

**Setup**:
```bash
# Source stays clean
mkdir build
cd build
../configure
make
```

**Directory structure**:
```
asc-hq-codex/
├── source/           # Source code (read-only during build)
├── build/            # Build artifacts (gitignored)
│   ├── Makefile
│   ├── source/
│   │   └── gamemap.o
│   └── asc           # Binary
└── build-debug/      # Separate debug build (gitignored)
```

**Java/Maven equivalent**: Exactly like `target/` directory

**Benefits**:
- Source tree stays pristine
- Multiple build configs (debug, release, asan)
- `git clean -fdx` safe
- Faster builds (ccache works better)

### 4. Modern Build System (CMake)

**Long-term goal**: Migrate from Autotools to CMake

**Why**:
- Industry standard (95% of C++ projects)
- Better IDE integration (CLion, VS Code)
- Easier cross-platform builds
- Better dependency management
- Out-of-tree builds by default

**Comparison**:

| Feature | Autotools (current) | CMake (modern) |
|---------|-------------------|---------------|
| Build speed | Slow | Fast (Ninja backend) |
| Configuration | ./configure (shell) | cmake (C++) |
| IDE support | Poor | Excellent |
| Dependency mgmt | Manual | FetchContent, Conan, vcpkg |
| Cross-platform | Linux focus | Windows, Mac, Linux |
| Learning curve | Steep | Moderate |
| Out-of-tree | Manual | Default |

**Example CMakeLists.txt**:
```cmake
cmake_minimum_required(VERSION 3.20)
project(asc-hq CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# External dependencies
find_package(SDL REQUIRED)
find_package(Boost REQUIRED COMPONENTS regex)

# MCTS library
add_library(mcts
  source/ai/mcts/core/mcts_search.cpp
  source/ai/mcts/core/mcts_config.cpp
  source/ai/mcts/infrastructure/mcts_config_loader.cpp
)
target_link_libraries(mcts PRIVATE Boost::regex)
target_include_directories(mcts PUBLIC source)

# Tests
enable_testing()
add_subdirectory(tests)
```

**Migration strategy**: Incremental (keep Autotools, add CMake alongside)

---

## Immediate Fixes (Week 1-2)

### Fix 1: Install Include-What-You-Use

**Action**:
```bash
# Install
sudo apt-get install iwyu

# Run on MCTS module
iwyu_tool.py -p . source/ai/mcts/ > iwyu-report.txt

# Review and fix
cat iwyu-report.txt
```

**CI Integration** (.github/workflows/ci.yml):
```yaml
- name: Check includes (IWYU)
  run: |
    sudo apt-get install -y iwyu
    iwyu_tool.py -p . source/ai/mcts/ 2>&1 | tee iwyu-report.txt
  continue-on-error: true  # Warning only at first
```

### Fix 2: Enforce Clean Builds in CI

**Action**: Already done! (`make clean` before build)

**Enhance** (verify clean state):
```yaml
- name: Verify clean state
  run: |
    # Fail if any build artifacts exist
    if [ -n "$(find source -name '*.o' -o -name '*.lo')" ]; then
      echo "ERROR: Build artifacts in source tree!"
      exit 1
    fi

- name: Clean build
  run: make clean || true

- name: Build
  run: make -j2
```

### Fix 3: Add Build Artifact Checks

**Pre-commit hook** (.git/hooks/pre-commit):
```bash
#!/bin/bash
# Prevent accidental commit of build artifacts

if git diff --cached --name-only | grep -E '\.(o|lo|la|a)$'; then
  echo "ERROR: Attempting to commit build artifacts!"
  echo "Run: git reset HEAD <file>"
  exit 1
fi
```

### Fix 4: Stricter Compiler Flags (MCTS Only)

**Update source/ai/mcts/Makefile.am**:
```makefile
# MCTS module: Modern C++23, strict warnings
AM_CXXFLAGS = -std=c++23 \
              -Wall -Wextra -Wpedantic \
              -Werror=return-type \
              -Werror=switch \
              -Werror=missing-include-dirs \
              -Wno-deprecated
```

---

## Medium-Term Improvements (Month 1-3)

### Improvement 1: Out-of-Tree Builds

**Update documentation** (README.md):
```markdown
## Building (Recommended)

# Out-of-tree build (recommended)
mkdir build && cd build
../configure --enable-debug
make -j$(nproc)

# Run tests
make check

# Binary at: build/source/unix/asc/asc
```

**Update CI**:
```yaml
- name: Configure (out-of-tree)
  run: |
    mkdir build && cd build
    ../configure --enable-debug

- name: Build
  run: cd build && make -j2
```

**Benefits**:
- Clean source tree
- Multiple build configs
- Matches CI exactly

### Improvement 2: Dependency Graph Visualization

**Generate include dependency graph**:
```bash
# Install
sudo apt-get install graphviz

# Generate
cinclude2dot --src source/ai/mcts | dot -Tpng > mcts-dependencies.png
```

**Use**: Identify circular dependencies, unnecessary includes

### Improvement 3: Precompiled Headers (PCH)

**Speed up compilation by 30-50%**

Create `source/pch.h`:
```cpp
// Precompiled header - rarely changing headers
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <algorithm>
#include <iostream>
#include <fstream>
```

**Autotools integration**: Complex, easier with CMake

---

## Long-Term Modernization (Month 3-6)

### Option 1: CMake Migration

**Phase 1**: MCTS module only
```bash
source/ai/mcts/CMakeLists.txt
```

**Phase 2**: Parallel build systems (Autotools + CMake)

**Phase 3**: Deprecate Autotools

**Benefits**:
- Modern tooling
- Better IDE support
- Easier onboarding
- Industry standard

**Risks**:
- Learning curve
- Migration effort
- Breaking changes

### Option 2: Meson Build System

**Alternative to CMake**

**Pros**:
- Faster than CMake
- Simpler syntax
- Python-based
- Great Ninja backend

**Cons**:
- Less widespread than CMake
- Smaller ecosystem

**Example meson.build**:
```python
project('asc-hq', 'cpp', version: '2.0', default_options: ['cpp_std=c++23'])

mcts_lib = library('mcts',
  sources: files('source/ai/mcts/core/mcts_search.cpp'),
  include_directories: include_directories('source'),
  dependencies: [boost_dep, sdl_dep]
)
```

---

## Comparison: C++ vs Java Ecosystem

| Aspect | Java/Maven/Gradle | C++ (Current) | C++ (Modern) |
|--------|------------------|---------------|--------------|
| **Build artifacts** | `target/` (gitignored) | Source tree (messy) | `build/` (gitignored) |
| **Dependency mgmt** | Maven Central | Manual | Conan, vcpkg |
| **Build tool** | Maven/Gradle | Autotools | CMake, Meson |
| **Build speed** | Incremental (good) | Incremental (ok) | Ninja (excellent) |
| **Include hygiene** | Auto (packages) | Manual (#include) | IWYU, modules (C++20) |
| **IDE support** | Excellent | Poor | Good (with CMake) |
| **Clean builds** | `mvn clean` | `make clean` | `rm -rf build` |
| **Reproducibility** | Good (pom.xml) | Poor | Good (CMake + Conan) |

**Key insight**: Java solved these problems 20 years ago. Modern C++ is catching up with CMake + Conan/vcpkg.

---

## Recommended Action Plan

### Phase 1: Quick Wins (This Week) ✓ PRIORITY

1. ✅ Add IWYU to CI/CD
2. ✅ Enforce clean builds in CI
3. ✅ Add pre-commit hook for build artifacts
4. ✅ Stricter compiler flags for MCTS module
5. Document out-of-tree build process

### Phase 2: Build Hygiene (Next 2 Weeks)

1. Migrate to out-of-tree builds (CI first, then docs)
2. Run IWYU on entire codebase, fix critical issues
3. Add dependency graph visualization
4. Set up build artifact checker in CI

### Phase 3: Modernization Planning (Month 2)

1. Prototype CMake build for MCTS module
2. Evaluate Conan for dependency management
3. Benchmark build times (Autotools vs CMake+Ninja)
4. Create CMake migration roadmap

### Phase 4: Gradual Migration (Month 3-6)

1. Parallel build systems (Autotools + CMake)
2. Migrate modules incrementally
3. Update CI to use CMake
4. Deprecate Autotools when complete

---

## Tools Reference

### Include Analysis
- **include-what-you-use (IWYU)**: Detects missing/extra includes
- **cinclude2dot**: Visualize include dependencies
- **clang -H**: Show include hierarchy

### Build Systems
- **CMake**: Industry standard, cross-platform
- **Meson**: Fast, Python-based alternative
- **Bazel**: Google's build system (overkill for this project)
- **xmake**: Modern Lua-based (emerging)

### Dependency Management
- **Conan**: C/C++ package manager (like npm for C++)
- **vcpkg**: Microsoft's C++ package manager
- **CPM.cmake**: CMake-based dependency manager

### Build Performance
- **ccache**: Compiler cache (already using)
- **distcc**: Distributed compilation
- **Ninja**: Fast build backend for CMake
- **sccache**: Shared cache (Rust-based)

### Static Analysis (Already Set Up)
- **clang-tidy**: Modern C++ linter ✓
- **cppcheck**: Bug detector ✓
- **IWYU**: Include hygiene

---

## Installation Guide

### IWYU (Include-What-You-Use)

```bash
# Ubuntu/Debian
sudo apt-get install iwyu

# Verify
iwyu --version

# Generate compile database (needed for IWYU)
pip3 install scan-build
intercept-build make

# Run IWYU
iwyu_tool.py -p . source/ai/mcts/
```

### CMake (for future migration)

```bash
# Ubuntu/Debian
sudo apt-get install cmake ninja-build

# Verify
cmake --version  # Should be 3.20+
ninja --version
```

### Conan (for future dependency management)

```bash
# Install
pip3 install conan

# Initialize
conan profile detect --force

# Example: Install Boost via Conan
conan install boost/1.82.0@ --build=missing
```

---

## Success Metrics

**Phase 1 Complete When**:
- ✅ IWYU running in CI
- ✅ No build artifacts in source tree
- ✅ CI always does clean builds
- ✅ MCTS module has strict compiler flags

**Phase 2 Complete When**:
- ✅ Out-of-tree builds documented and used in CI
- ✅ IWYU violations < 50 (down from ~200 estimated)
- ✅ Build dependency graph generated

**Fully Modernized When**:
- ✅ CMake build system
- ✅ Conan dependency management
- ✅ Build time < 5 min (full), < 30s (incremental)
- ✅ Zero IWYU violations
- ✅ Out-of-tree builds only

---

## Resources

- [Include-What-You-Use](https://include-what-you-use.org/)
- [CMake Tutorial](https://cmake.org/cmake/help/latest/guide/tutorial/index.html)
- [Professional CMake](https://crascit.com/professional-cmake/)
- [Conan Documentation](https://docs.conan.io/)
- [C++23 Modules](https://en.cppreference.com/w/cpp/language/modules) (future)
- [Modern CMake](https://cliutils.gitlab.io/modern-cmake/)

---

## CMake Migration: Step-by-Step Guide

### Prerequisites

```bash
# Install CMake (Ubuntu/Debian)
sudo apt-get update
sudo apt-get install cmake ninja-build

# Verify installation
cmake --version    # Should be 3.20+
ninja --version
```

### Step 1: Create Top-Level CMakeLists.txt (15 minutes)

Create `CMakeLists.txt` in project root:

```cmake
cmake_minimum_required(VERSION 3.20)
project(asc-hq VERSION 2.0 LANGUAGES CXX)

# C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler flags
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

# Find dependencies
find_package(SDL REQUIRED)
find_package(Boost REQUIRED COMPONENTS regex)
find_package(PNG REQUIRED)

# Include subdirectories
add_subdirectory(source/ai/mcts)

# Main executable (to be added later)
# add_subdirectory(source)
```

**Test:**
```bash
mkdir build && cd build
cmake ..
# Should configure successfully (no build targets yet)
```

### Step 2: Create MCTS Module CMakeLists.txt (30 minutes)

Create `source/ai/mcts/CMakeLists.txt`:

```cmake
# MCTS AI Module
# Modern C++23, strict warnings

# Auto-discover source files by category
file(GLOB DOMAIN_SOURCES "domain/*.cpp")
file(GLOB DOMAIN_ABILITY_SOURCES "domain/abilities/*.cpp")
file(GLOB AGENT_SOURCES "agents/*.cpp")
file(GLOB CORE_SOURCES "core/*.cpp")
file(GLOB INFRA_SOURCES "infrastructure/*.cpp")

# Exclude test files from library
list(FILTER DOMAIN_SOURCES EXCLUDE REGEX ".*_test\\.cpp$")

# Build MCTS library
add_library(mcts
    ${DOMAIN_SOURCES}
    ${DOMAIN_ABILITY_SOURCES}
    ${AGENT_SOURCES}
    ${CORE_SOURCES}
    ${INFRA_SOURCES}
    mcts_manual_test.cpp
)

# Include directories
target_include_directories(mcts PUBLIC
    ${CMAKE_SOURCE_DIR}/source
    ${CMAKE_SOURCE_DIR}/source/libs/paragui/include
)

# Compiler flags for MCTS module (strict)
target_compile_options(mcts PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Werror=return-type
    -Werror=switch
    -Wno-deprecated
)

# Test executables
add_executable(snapshot_test domain/snapshot_test.cpp)
target_link_libraries(snapshot_test PRIVATE mcts)

add_executable(action_executor_test domain/action_executor_test.cpp)
target_link_libraries(action_executor_test PRIVATE mcts)

add_executable(evaluator_test domain/evaluator_test.cpp)
target_link_libraries(evaluator_test PRIVATE mcts)

# Enable testing
enable_testing()
add_test(NAME snapshot_test COMMAND snapshot_test)
add_test(NAME action_executor_test COMMAND action_executor_test)
add_test(NAME evaluator_test COMMAND evaluator_test)
```

**Test:**
```bash
cd build
cmake ..
cmake --build .
# Should build MCTS library and tests
```

### Step 3: Test the Build (10 minutes)

```bash
# Clean build
cd /home/vboxuser/projects/asc-hq-codex
rm -rf build
mkdir build && cd build

# Configure
cmake -G Ninja ..
# Or: cmake .. (uses Make by default)

# Build
ninja -j$(nproc)
# Or: make -j$(nproc)

# Run tests
ctest --output-on-failure
# Or: ./source/ai/mcts/snapshot_test
```

**Expected output:**
```
[1/25] Building CXX object source/ai/mcts/CMakeFiles/mcts.dir/domain/action_types.cpp.o
[2/25] Building CXX object source/ai/mcts/CMakeFiles/mcts.dir/domain/combat_calculator.cpp.o
...
[25/25] Linking CXX executable source/ai/mcts/evaluator_test
Test project /home/vboxuser/projects/asc-hq-codex/build
    Start 1: snapshot_test
1/3 Test #1: snapshot_test ....................   Passed    0.01 sec
    Start 2: action_executor_test
2/3 Test #2: action_executor_test .............   Passed    0.02 sec
    Start 3: evaluator_test
3/3 Test #3: evaluator_test ...................   Passed    0.01 sec

100% tests passed, 0 tests failed out of 3
```

### Step 4: Update .gitignore (2 minutes)

Add to `.gitignore`:

```gitignore
# CMake build directories
/build/
/build-*/
/cmake-build-*/

# CMake files (if accidentally generated in source)
CMakeCache.txt
CMakeFiles/
cmake_install.cmake
```

### Step 5: Document New Build Process (5 minutes)

Update `README.md` with CMake instructions:

```markdown
## Building with CMake (Recommended)

### Quick Start
```bash
# Out-of-tree build
mkdir build && cd build
cmake -G Ninja ..
ninja

# Run tests
ctest --output-on-failure
```

### Build Configurations

```bash
# Debug build
mkdir build-debug && cd build-debug
cmake -DCMAKE_BUILD_TYPE=Debug ..
ninja

# Release build
mkdir build-release && cd build-release
cmake -DCMAKE_BUILD_TYPE=Release ..
ninja
```

### Using Make instead of Ninja

```bash
mkdir build && cd build
cmake ..                # Don't specify -G Ninja
make -j$(nproc)
```

### Incremental Builds

```bash
# After making code changes
cd build
ninja                   # Only rebuilds changed files

# Force clean rebuild
ninja clean
ninja
```

### Legacy Build (Autotools - Deprecated)

See [Autotools Build Instructions](docs/AUTOTOOLS_BUILD.md) if needed.
```

### Step 6: Parallel Build Systems (Transition Period)

**Keep both systems working:**

- Autotools remains the "official" build (for now)
- CMake is "experimental" but fully functional
- CI can test both builds in parallel

**Timeline:**
- Week 1-2: CMake builds MCTS module only
- Week 3-4: Test and fix any issues
- Month 2: Migrate main application to CMake
- Month 3: Make CMake the default, deprecate Autotools

### Step 7: Common Issues and Solutions

#### Issue: "Could not find SDL"

**Solution:**
```bash
# Ubuntu/Debian
sudo apt-get install libsdl1.2-dev libsdl-image1.2-dev

# Or tell CMake where to find it
cmake -DSDL_INCLUDE_DIR=/usr/include/SDL ..
```

#### Issue: File globbing doesn't detect new files

**Solution:**
```bash
# Force CMake to re-scan
rm CMakeCache.txt
cmake ..

# Or touch CMakeLists.txt
touch source/ai/mcts/CMakeLists.txt
ninja
```

#### Issue: Build artifacts in source tree

**Solution:** CMake prevents this by design. If you see artifacts in `source/`, you're using Autotools, not CMake.

```bash
# Verify you're using CMake
cd build
cmake --build .   # This is CMake
make              # This might be Autotools if build/ has Makefile.am
```

### Migration Checklist

**Phase 1: MCTS Module (Week 1)**
- [ ] Install CMake and Ninja
- [ ] Create top-level CMakeLists.txt
- [ ] Create source/ai/mcts/CMakeLists.txt
- [ ] Test build: `mkdir build && cd build && cmake .. && ninja`
- [ ] Verify tests pass: `ctest`
- [ ] Update .gitignore
- [ ] Document new build process

**Phase 2: Validation (Week 2-3)**
- [ ] Compare Autotools vs CMake build outputs
- [ ] Run full test suite with both builds
- [ ] Measure build times (CMake should be faster)
- [ ] Test on clean system (CI environment)

**Phase 3: Expansion (Month 2)**
- [ ] Migrate main application build
- [ ] Migrate all subdirectories
- [ ] Update CI to use CMake
- [ ] Update developer documentation

**Phase 4: Completion (Month 3)**
- [ ] Make CMake the default build
- [ ] Mark Autotools as deprecated
- [ ] Remove Autotools files (or archive)
- [ ] Celebrate! 🎉

### Benefits Achieved

After migration:
- ✅ **Clean source tree** - No more .o/.lo files in source/
- ✅ **Multiple build configs** - Debug, Release, ASAN builds simultaneously
- ✅ **Faster builds** - Ninja is 3-5x faster than Make
- ✅ **Better IDE support** - CLion, VS Code work seamlessly
- ✅ **Modern tooling** - Industry-standard build system
- ✅ **Easier onboarding** - New developers familiar with CMake

---

**Next Steps**: Start with Step 1 - create top-level CMakeLists.txt and test basic configuration.
