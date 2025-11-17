# Building ASC-HQ

This document contains quick build instructions for ASC-HQ.

## Quick Start - CMake (Recommended)

CMake provides the fastest builds with out-of-tree artifact management (clean source tree).

**Currently supports:** MCTS AI module only (Phase 1 of migration)

### Build Commands

```bash
# Navigate to project root
cd /home/vboxuser/projects/asc-hq-codex

# Create build directory (out-of-tree build)
mkdir build
cd build

# Configure
cmake ..

# Build (use all CPU cores)
make -j$(nproc)

# Build artifacts will be in:
# - build/lib/libmcts.a          (MCTS library)
# - build/bin/snapshot_test       (test executable)
# - build/bin/action_executor_test (test executable)
# - build/bin/evaluator_test      (test executable)
```

### Different Build Types

```bash
# Debug build (with debug symbols, no optimization)
mkdir build-debug
cd build-debug
cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j$(nproc)

# Release build (optimized, no debug symbols)
mkdir build-release
cd build-release
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
```

### Incremental Builds

After making code changes:

```bash
cd build
make -j$(nproc)    # Only rebuilds changed files
```

### Clean Build

```bash
# Option 1: Clean and rebuild
cd build
make clean
make -j$(nproc)

# Option 2: Complete fresh build
cd /home/vboxuser/projects/asc-hq-codex
rm -rf build
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### Using Ninja (Faster Alternative to Make)

Ninja is 3-5x faster than Make for large projects:

```bash
# Install Ninja (if not already installed)
sudo apt-get install ninja-build

# Build with Ninja
mkdir build
cd build
cmake -G Ninja ..
ninja -j$(nproc)
```

## Legacy Build - Autotools (Current Default)

Autotools is the current build system for the full ASC project.

### Prerequisites

```bash
sudo apt-get update
sudo apt-get install build-essential autoconf automake libtool \
    libsdl1.2-dev libsdl-image1.2-dev libsdl-mixer1.2-dev \
    libsdl-sound1.2-dev libboost-dev libsigc++-2.0-dev \
    libpng-dev zlib1g-dev libbz2-dev libfreetype6-dev \
    libphysfs-dev libexpat1-dev liblua5.1-0-dev
```

### Build Commands (Out-of-Tree - Recommended)

**Out-of-tree builds keep your source directory clean:**

```bash
# Bootstrap (generate configure script)
./bootstrap

# Create build directory
mkdir build-autotools
cd build-autotools

# Configure (pointing to parent directory)
../configure

# Build
make -j$(nproc)

# Binaries will be in:
# - build-autotools/source/unix/asc/asc (main game)
# - build-autotools/source/unix/mapeditor/asc_mapedit
```

**Benefits:**
- ✅ Source tree stays clean (no .o files mixed with code)
- ✅ Multiple build configs possible (debug, release)
- ✅ Easy to clean: `rm -rf build-autotools/`
- ✅ Matches CI/CD behavior

### Build Commands (In-Tree - Not Recommended)

```bash
# Configure
./configure

# Build
make -j$(nproc)

# Install (optional)
sudo make install
```

**Warning:** In-tree builds scatter `.o`, `.lo`, and `.a` files throughout your source tree. Use out-of-tree builds instead.

## Build System Comparison

| Feature | CMake (Modern) | Autotools (Legacy) |
|---------|----------------|-------------------|
| **Build artifacts** | `build/` directory (clean) | `build-autotools/` (clean with out-of-tree) |
| **Reconfigure speed** | 1-2 seconds | 20-30 seconds |
| **Build speed** | Fast (especially with Ninja) | Moderate |
| **Source discovery** | Auto-glob (mostly automatic) | Manual file listing |
| **Multiple configs** | Easy (build-debug/, build-release/) | Easy with out-of-tree builds |
| **IDE support** | Excellent (CLion, VS Code) | Poor |
| **Out-of-tree builds** | Enforced (impossible to build in-tree) | Supported (must create build dir manually) |
| **Current coverage** | MCTS module only | Full project |

## Troubleshooting

### CMake: "Could not find SDL"

```bash
sudo apt-get install libsdl1.2-dev pkg-config
```

### CMake: Build directory in source tree

CMake prevents in-source builds. Always use a separate build directory:

```bash
# Wrong (in-source build)
cd source/ai/mcts
cmake .

# Correct (out-of-tree build)
mkdir build && cd build
cmake ..
```

### Autotools: "No rule to make target"

The Autotools build may have stale dependencies. Clean and reconfigure:

```bash
make distclean
./configure
make -j$(nproc)
```

### Build Artifacts in Source Tree

If you see `.o` or `.lo` files in your source directories, you're using Autotools in-tree builds:

```bash
# Clean up
make clean

# Switch to CMake or use out-of-tree Autotools builds (see above)
```

## Development Workflow

### Adding New Files to MCTS Module

With CMake, new `.cpp` files in these directories are auto-discovered:
- `source/ai/mcts/domain/*.cpp`
- `source/ai/mcts/domain/abilities/*.cpp`
- `source/ai/mcts/agents/*.cpp`
- `source/ai/mcts/core/*.cpp`
- `source/ai/mcts/infrastructure/*.cpp`

After adding a file:

```bash
cd build
rm CMakeCache.txt   # Force re-scan
cmake ..
make -j$(nproc)
```

With Autotools, you must manually edit `source/ai/mcts/Makefile.am` and add the file to `libmcts_la_SOURCES`.

## Migration Status

**Phase 1 (Current):** MCTS module builds with CMake
**Phase 2 (Planned):** Full project CMake migration

See `docs/modernization/BUILD_SYSTEM_MODERNIZATION.md` for the complete migration roadmap.

## Additional Documentation

- Full installation guide: `doc/install.html`
- CMake migration plan: `docs/modernization/BUILD_SYSTEM_MODERNIZATION.md`
- Project documentation index: `doc/index.html`
