# C++23 Migration Troubleshooting Guide

**Date**: 2025-11-07  
**Status**: Living Document

---

## Common Compilation Errors & Solutions

### ❌ Error: "call of overloaded 'unique_ptr(int)' is ambiguous"

**Full Error Message**:
```
../../source/libs/loki-0.1.6/include/loki/Functor.h:1244:21: error: call of overloaded 'unique_ptr(int)' is ambiguous
 1244 |         Functor() : spImpl_(0)
      |                     ^~~~~~~~~~
```

**Cause**: 
When converting from `std::auto_ptr` to `std::unique_ptr`, the initialization with `0` (integer zero) becomes ambiguous because `std::unique_ptr` doesn't have the same implicit conversions as `auto_ptr`.

**Solution**: 
Replace all `0` with `nullptr` when used with `std::unique_ptr`.

**File**: `source/libs/loki-0.1.6/include/loki/Functor.h`

**Changes Required**:
```cpp
// Line 1244 - Constructor initialization
- Functor() : spImpl_(0)
+ Functor() : spImpl_(nullptr)

// Line 1267 - Ternary operator return
- return spImpl_.get() ? &std::unique_ptr<Impl>::get : 0;
+ return spImpl_.get() ? &std::unique_ptr<Impl>::get : nullptr;

// Line 1284 - Comparison
- return spImpl_.get() == 0;
+ return spImpl_.get() == nullptr;

// Line 1289 - Reset
- spImpl_.reset(0);
+ spImpl_.reset(nullptr);

// Lines 1297, 1299 - Comparisons in operator==
- if(spImpl_.get()==0 && rhs.spImpl_.get()==0)
+ if(spImpl_.get()==nullptr && rhs.spImpl_.get()==nullptr)

- if(spImpl_.get()!=0 && rhs.spImpl_.get()!=0)
+ if(spImpl_.get()!=nullptr && rhs.spImpl_.get()!=nullptr)
```

**Quick Fix Command**:
```bash
sed -i 's/spImpl_(0)/spImpl_(nullptr)/g' source/libs/loki-0.1.6/include/loki/Functor.h
sed -i 's/\.get() : 0/.get() : nullptr/g' source/libs/loki-0.1.6/include/loki/Functor.h
sed -i 's/\.get() == 0/.get() == nullptr/g' source/libs/loki-0.1.6/include/loki/Functor.h
sed -i 's/\.get()!=0/.get()!=nullptr/g' source/libs/loki-0.1.6/include/loki/Functor.h
sed -i 's/\.get()==0/.get()==nullptr/g' source/libs/loki-0.1.6/include/loki/Functor.h
sed -i 's/\.reset(0)/.reset(nullptr)/g' source/libs/loki-0.1.6/include/loki/Functor.h
```

**Status**: ✅ Fixed

---

### ⚠️ Warning: "'++' expression of 'volatile'-qualified type is deprecated"

**Full Warning**:
```
source/libs/loki-0.1.6/include/loki/Threads.h:230:20: warning: '++' expression of 'volatile'-qualified type is deprecated [-Wvolatile]
  230 |         { return ++lval; }
```

**Cause**: 
C++20 deprecated compound assignments and increment/decrement operations on volatile types.

**Impact**: 
⚠️ **Non-blocking** - These are warnings, not errors. The code still compiles and runs correctly.

**Solution (Future)**: 
Replace volatile operations with `std::atomic` when refactoring threading code.

**Example Modernization**:
```cpp
// Old (Loki library)
static IntType AtomicIncrement(volatile IntType& lval)
{ return ++lval; }

// Modern C++23
static IntType AtomicIncrement(std::atomic<IntType>& lval)
{ return lval.fetch_add(1) + 1; }
```

**Status**: ⚠️ Acceptable (warnings only)

---

### ❌ Error: "'template<class _Arg, class _Result> struct std::unary_function' is deprecated"

**Full Error**:
```
../../../source/memsize_interface.h:27:28: warning: 'template<class _Arg, class _Result> struct std::unary_function' is deprecated [-Wdeprecated-declarations]
   27 |  struct MemorySum : public unary_function<T, void>
```

**Cause**: 
`std::unary_function` and `std::binary_function` were deprecated in C++11 and removed in C++17.

**Solution**: 
Replace base class inheritance with manual typedef declarations.

**File**: `source/memsize_interface.h`

**Before**:
```cpp
template<class T> 
struct MemorySum : public unary_function<T, void>
{
    MemorySum() : size(0) {}
    void operator() (const T& x) { size += getMemoryFootprint(x); }
    void operator() (const T* x) { size += getMemoryFootprint(*x); }
    int size;
};
```

**After**:
```cpp
template<class T> 
struct MemorySum
{
    using argument_type = T;
    using result_type = void;
    
    MemorySum() : size(0) {}
    void operator() (const T& x) { size += getMemoryFootprint(x); }
    void operator() (const T* x) { size += getMemoryFootprint(*x); }
    int size;
};
```

**Status**: ✅ Fixed

---

## General C++23 Migration Checklist

When migrating code to C++23, check for:

- [ ] `std::auto_ptr` → `std::unique_ptr` or `std::shared_ptr`
- [ ] `0` → `nullptr` with smart pointers
- [ ] `register` keyword → remove entirely
- [ ] `throw()` → `noexcept`
- [ ] `std::unary_function`/`binary_function` → manual typedefs
- [ ] Volatile compound operations → `std::atomic` (if possible)

---

## Build Verification

After making changes, verify with:

```bash
# Test MCTS module
cd source/ai/mcts
make clean && make

# Check for errors (should be none)
echo $?  # Should output: 0

# Full project build
cd ../../..
./configure
make
```

---

## References

- **Migration Plan**: `source/ai/mcts/C++23_MIGRATION_PLAN.md`
- **Completion Report**: `C++23_MIGRATION_COMPLETE.md`
- **C++17 Notes**: `source/ai/mcts/C++17_MIGRATION_NOTES.md`

---

**Last Updated**: 2025-11-07  
**Maintainer**: ASC Development Team
