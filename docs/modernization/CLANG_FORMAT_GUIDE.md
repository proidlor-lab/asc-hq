# clang-format Setup and Usage Guide

**Created**: 2025-11-16
**Status**: Configuration ready, awaiting execution

---

## Overview

Automated code formatting for the entire ASC codebase using clang-format. This ensures consistent code style across all 1,000+ source files without manual work.

---

## What Was Created

### 1. `.clang-format` Configuration

Located in project root, this file defines the formatting rules:

**Key Settings**:
- **Indentation**: 3 spaces (matches MCTS module)
- **Line length**: 100 characters
- **Brace style**: Attach (K&R style)
- **Pointer alignment**: Left (`int* ptr` not `int *ptr`)
- **Standard**: C++20
- **Include sorting**: Automatic

**Based on**: Google style guide with customizations to match MCTS module

### 2. `format-code.sh` Script

Automated script to format the entire codebase:

```bash
./format-code.sh           # Dry run (preview changes)
./format-code.sh --apply   # Actually format files
```

**Features**:
- ✅ Dry run mode (safe preview)
- ✅ Progress indicators
- ✅ Summary statistics
- ✅ Colored output for readability

---

## Installation

### Install clang-format

```bash
sudo apt-get update
sudo apt-get install clang-format
```

**Verify installation**:
```bash
clang-format --version
# Should show: clang-format version 14.0 or higher
```

---

## Usage

### Step 1: Dry Run (Safe Preview)

**ALWAYS do this first** to see what will change:

```bash
./format-code.sh
```

**Output shows**:
- How many files found
- Which files need formatting
- Summary statistics

**Example**:
```
Found 1051 files to format
Checking which files need formatting...
  Would format: source/attack.cpp
  Would format: source/basegfx.cpp
  ...

Summary:
  Total files: 1051
  Need formatting: 847
  Already formatted: 204

To apply formatting, run:
  ./format-code.sh --apply
```

### Step 2: Apply Formatting

Once you're comfortable with the preview:

```bash
./format-code.sh --apply
```

**This will**:
- Format all 1,000+ C++ files in-place
- Take 2-5 minutes
- Show progress as it works

### Step 3: Verify Changes

**Check what changed**:
```bash
git diff | head -200    # Preview first 200 lines
git diff --stat         # Summary of changed files
```

**Build to ensure nothing broke**:
```bash
make clean
make -j2
```

**Run tests**:
```bash
make check
```

### Step 4: Commit

If everything builds and tests pass:

```bash
git add -A
git commit -m "style: Apply clang-format to entire codebase

- Configured clang-format based on MCTS module style
- 3-space indentation, 100-char line limit
- Formatted 1,051 source files automatically
- No functional changes, only whitespace/style"
```

---

## What Gets Formatted

### Files Included
- All `.cpp` files in `source/`
- All `.h` files in `source/`
- All `.hpp` files in `source/`

### Files Excluded
- Third-party libraries (`source/libs/`) - **INCLUDED** (we maintain these)
- Build artifacts (*.o, executables)
- Documentation files

**Note**: Third-party libraries ARE formatted to match our style since we've bundled and modified them.

---

## Before and After Examples

### Example 1: Inconsistent Indentation

**Before**:
```cpp
class Vehicle{
    int x;
  int y;      // 2 spaces
      int z;  // 6 spaces!
public:
   void move( ){
     if(canMove){
       doMove( );
     }
   }
};
```

**After**:
```cpp
class Vehicle {
   int x;
   int y;
   int z;
public:
   void move() {
      if (canMove) {
         doMove();
      }
   }
};
```

### Example 2: Pointer Alignment

**Before** (mixed styles):
```cpp
int *ptr1;
int* ptr2;
int  *  ptr3;
```

**After** (consistent):
```cpp
int* ptr1;
int* ptr2;
int* ptr3;
```

### Example 3: Spacing

**Before**:
```cpp
if(x>5&&y<10){
    foo(1,2,3);
}
```

**After**:
```cpp
if (x > 5 && y < 10) {
   foo(1, 2, 3);
}
```

---

## Configuration Details

### Indentation
- **3 spaces** (matches MCTS module)
- No tabs (spaces only)
- Consistent across entire codebase

### Braces
- **Attach style** (K&R):
  ```cpp
  if (condition) {  // Brace on same line
     code();
  }
  ```

### Line Length
- **100 characters maximum**
- Long lines wrapped intelligently
- Comments reflowed

### Pointers and References
- **Left-aligned**: `int* ptr`, `const Type& ref`
- Consistent across all files

### Include Sorting
Automatic sorting into groups:
1. Own headers (`"mcts_search.h"`)
2. ASC headers (`"../../gamemap.h"`)
3. C++ standard library (`<vector>`, `<string>`)
4. System headers (`<SDL.h>`)

---

## Integration with CI/CD

After initial formatting, add to `.github/workflows/ci.yml`:

```yaml
- name: Check code formatting
  run: |
    # Check if any files would be reformatted
    ./format-code.sh | grep "Need formatting: 0" || {
      echo "ERROR: Code is not formatted!"
      echo "Run: ./format-code.sh --apply"
      exit 1
    }
```

This prevents future commits from introducing inconsistent formatting.

---

## Editor Integration

### VS Code
Install "clang-format" extension, add to `.vscode/settings.json`:
```json
{
  "editor.formatOnSave": true,
  "C_Cpp.clang_format_style": "file"
}
```

### Vim
Add to `.vimrc`:
```vim
autocmd FileType cpp setlocal formatprg=clang-format
```

### Emacs
```elisp
(require 'clang-format)
(global-set-key [C-M-tab] 'clang-format-region)
```

---

## Troubleshooting

### Issue: clang-format not found

**Solution**:
```bash
sudo apt-get update
sudo apt-get install clang-format
```

### Issue: Script shows no files

**Solution**: Run from project root directory:
```bash
cd /path/to/asc-hq-codex
./format-code.sh
```

### Issue: "Too many changes, I'm scared!"

**Solution**:
1. Create a backup branch first:
   ```bash
   git checkout -b backup-before-format
   git checkout -
   ```
2. Format on your working branch
3. If anything goes wrong, switch back to backup

### Issue: Build fails after formatting

**Solution**: This is VERY unlikely (formatting doesn't change logic), but:
1. Check `git diff` for unexpected changes
2. Look for accidental changes in macros or string literals
3. Report issue - may be a clang-format bug

---

## Manual Formatting

### Format a single file
```bash
clang-format -i source/attack.cpp
```

### Format specific directory
```bash
find source/ai/mcts/ -name "*.cpp" -o -name "*.h" | xargs clang-format -i
```

### Preview changes without applying
```bash
clang-format source/attack.cpp | diff source/attack.cpp -
```

---

## Statistics

Based on codebase analysis:

- **Total source files**: ~1,051
- **Estimated time**: 2-5 minutes to format all
- **Expected changes**: ~60-80% of files (indentation fixes)
- **Build time after**: Same (no code changes)

---

## Benefits

### Immediate
- ✅ Consistent code style across all files
- ✅ Cleaner git diffs (only logical changes)
- ✅ Professional appearance
- ✅ No more style debates

### Long-term
- ✅ Easier code reviews (focus on logic, not style)
- ✅ Faster onboarding (no style guide to learn)
- ✅ Automatic enforcement via CI
- ✅ Editor integration (format on save)

---

## Next Steps

1. **Install clang-format**:
   ```bash
   sudo apt-get install clang-format
   ```

2. **Preview changes**:
   ```bash
   ./format-code.sh
   ```

3. **If happy, apply**:
   ```bash
   ./format-code.sh --apply
   ```

4. **Verify**:
   ```bash
   make clean && make -j2
   make check
   ```

5. **Commit**:
   ```bash
   git add -A
   git commit -m "style: Apply clang-format to entire codebase"
   ```

6. **Update CI** (next task) to enforce formatting

---

## References

- clang-format documentation: https://clang.llvm.org/docs/ClangFormat.html
- Configuration options: https://clang.llvm.org/docs/ClangFormatStyleOptions.html
- LLVM style guide: https://llvm.org/docs/CodingStandards.html

---

**Status**: ✅ Configuration ready
**Action required**: Install clang-format and run `./format-code.sh`
