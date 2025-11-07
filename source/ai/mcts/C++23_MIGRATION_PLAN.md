# C++23 Migration Plan for ASC

**Date**: 2025-11-07  
**Status**: 🚧 **In Progress**  
**Objective**: Migrate ASC codebase from C++11/14 to C++23

---

## Executive Summary

Migration to C++23 requires addressing compatibility issues introduced in C++17, C++20, and C++23:

### Critical Blockers (Must Fix)
1. **`std::auto_ptr`** - Removed in C++17 (226 instances)
2. **`register` keyword** - Removed in C++17 (75 instances)  
3. **`throw()` exception specs** - Removed in C++17 (25+ instances in Loki)

### Additional Considerations
- Legacy third-party libraries (Loki, ParaGUI) may need updates
- Const-correctness issues (already identified in C++17 notes)
- Potential header changes in standard library

---

## Compatibility Issues Inventory

### 1. std::auto_ptr → std::unique_ptr/std::shared_ptr
**Count**: 226 instances across codebase  
**Locations**: 
- `guifunctions.cpp` - ~14 instances
- `lua/commands.cpp` - ~100+ instances  
- `turncontrol.cpp` - Several instances
- `gameoptions.cpp` - Several instances
- Many other files

**Migration Strategy**:
- Most `auto_ptr` can be replaced with `std::unique_ptr`
- Release semantics (`auto_ptr.release()`) must be reviewed
- Some may need `std::shared_ptr` if ownership is shared

**Example Fix**:
```cpp
// Before
auto_ptr<AttackCommand> ac(new AttackCommand(unit));

// After  
std::unique_ptr<AttackCommand> ac(new AttackCommand(unit));
// or better
auto ac = std::make_unique<AttackCommand>(unit);
```

### 2. register Keyword Removal
**Count**: 75 instances  
**Locations**:
- `source/libs/paragui/src/draw/stretch.cpp` - 29 instances
- `source/tester/scanner.cpp` - 20 instances
- `source/libs/paragui/src/core/missing.cpp` - 5 instances
- Various other files

**Migration Strategy**:
Simply remove the `register` keyword (it's been ignored since C++11)

**Example Fix**:
```cpp
// Before
register int x = 0;

// After
int x = 0;
```

### 3. Dynamic Exception Specifications throw()
**Count**: 25+ instances in Loki library  
**Locations**:
- `source/libs/loki-0.1.6/include/loki/ScopeGuard.h` - 14 instances
- `source/libs/loki-0.1.6/include/loki/yasli/yasli_memory.h` - 12 instances
- Other Loki headers

**Migration Strategy**:
Replace `throw()` with `noexcept`

**Example Fix**:
```cpp
// Before
void foo() throw();

// After
void foo() noexcept;
```

---

## Migration Steps

### Phase 1: Build Configuration (Quick)
✅ **Status**: Ready to implement

1. Update `configure.ac` line 101:
   ```bash
   CXXFLAGS="$CXXFLAGS -std=c++23 -Wno-sign-compare -D_UNIX_ -D_SDL_"
   ```

2. Update `source/ai/mcts/Makefile.am` line 4:
   ```makefile
   AM_CXXFLAGS = @SDL_CFLAGS@ @SIGC_CFLAGS@ -std=c++23
   ```

3. Regenerate build files:
   ```bash
   ./bootstrap
   ./configure
   ```

### Phase 2: Fix auto_ptr (Medium - Large Effort)
⏳ **Estimated Time**: 4-8 hours  
📋 **Strategy**: Systematic file-by-file replacement

**Priority Order**:
1. `lua/commands.cpp` (highest density)
2. `guifunctions.cpp`
3. Other command-related files
4. Remaining files

**Automated Approach**:
```bash
# Find and replace pattern (requires manual review)
find source/ -name "*.cpp" -o -name "*.h" | xargs sed -i 's/auto_ptr</std::unique_ptr</g'
# Fix double std:: prefix
find source/ -name "*.cpp" -o -name "*.h" | xargs sed -i 's/std::std::unique_ptr/std::unique_ptr/g'
# Then manually review all .release() calls
```

**Important**: After replacement, also fix `std::unique_ptr` initialization with `0` → `nullptr` in Loki library:
```cpp
// Before (causes ambiguous constructor error)
Functor() : spImpl_(0) {}

// After
Functor() : spImpl_(nullptr) {}
```

### Phase 3: Remove register Keyword (Quick)
⏳ **Estimated Time**: 30 minutes  
📋 **Strategy**: Automated removal with verification

```bash
# Remove register keyword (spaces matter for context)
find source/ -name "*.cpp" -o -name "*.h" | xargs sed -i 's/\bregister //g'
```

### Phase 4: Fix throw() Specifications (Medium)
⏳ **Estimated Time**: 2-3 hours  
📋 **Strategy**: Focus on Loki library headers

Since Loki is a bundled library, we can patch it directly:
```bash
find source/libs/loki-0.1.6/ -name "*.h" | xargs sed -i 's/throw()/noexcept/g'
```

### Phase 5: Build and Test (Iterative)
⏳ **Estimated Time**: Variable  
📋 **Strategy**: Incremental build, fix, repeat

1. Attempt full build
2. Fix compilation errors one by one
3. Address any new const-correctness issues
4. Test runtime behavior

---

## C++23 Features Benefits

Once migration is complete, ASC can leverage:

### Language Features
- **`std::expected`** - Better error handling than exceptions
- **`std::print`** - Formatted output (replaces printf/iostream mess)
- **Deducing this** - Simplified method chaining
- **`if consteval`** - Compile-time vs runtime branches
- **Multidimensional subscript operator** - Better array syntax

### Library Improvements  
- **`std::spanstream`** - Better buffer I/O
- **`std::flat_map`** - Faster container for many use cases
- **Ranges improvements** - More expressive algorithms
- **`std::format` extensions** - Enhanced formatting

### For MCTS AI Module
- **`std::mdspan`** - Excellent for game state representation
- **Better constexpr** - More compile-time AI optimization
- **Improved coroutines** - Async AI evaluation

---

## Risk Assessment

### High Risk
- **Breaking stable code** - ASC is mature software
- **Runtime behavior changes** - Smart pointer semantics differ slightly
- **Testing coverage** - Unknown test suite completeness

### Medium Risk  
- **Third-party libraries** - Loki/ParaGUI may have other issues
- **Platform compatibility** - C++23 compiler availability
- **Build system quirks** - Autotools with C++23

### Low Risk
- **MCTS module** - Already modern C++ (C++17 compatible)
- **register keyword** - No-op, safe to remove
- **Exception specs** - Simple text replacement

---

## Rollback Plan

If migration fails or introduces bugs:

1. **Revert configuration**:
   ```bash
   git checkout configure.ac source/ai/mcts/Makefile.am
   ./bootstrap && ./configure
   ```

2. **Revert code changes**:
   ```bash
   git checkout source/
   ```

3. **Document issues** in this file for future attempt

---

## Compiler Requirements

### Minimum Versions for C++23
- **GCC**: 14.0+ (full support)
- **Clang**: 17.0+ (most features)
- **MSVC**: VS 2022 17.10+ (most features)

### Current Configuration
Check what's available:
```bash
g++ --version
g++ -std=c++23 -E -dM - < /dev/null | grep __cplusplus
```

---

## Progress Tracking

- [ ] Phase 1: Build configuration updated
- [ ] Phase 2: auto_ptr fixes (0/226)
- [ ] Phase 3: register keyword removed (0/75)  
- [ ] Phase 4: throw() specs fixed (0/25+)
- [ ] Phase 5: Successful clean build
- [ ] Phase 6: Runtime testing passed

---

## Notes & Discoveries

*(To be filled during migration)*

### Build Errors Encountered


### Code Pattern Changes


### Performance Observations


---

**Next Steps**: Begin Phase 1 - Update build configuration files
