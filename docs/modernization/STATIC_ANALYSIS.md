# Static Analysis Setup

**Purpose**: Automated code quality and bug detection for ASC codebase.

**Created**: 2025-11-16
**Status**: Configured and ready to use

---

## Overview

Static analysis tools help catch bugs, security vulnerabilities, and code quality issues without running the code. This document describes the static analysis setup for ASC.

## Tools Configured

### 1. **cppcheck** - Lightweight C++ static analyzer
- Focuses on bugs and undefined behavior
- Fast, minimal false positives
- Runs on CI/CD automatically

### 2. **clang-tidy** - LLVM-based C++ linter
- Modern C++ best practices
- Code modernization suggestions
- Requires compile database for best results

---

## Installation

Install the tools locally:

```bash
sudo apt-get update
sudo apt-get install -y cppcheck clang-tidy
```

Verify installation:

```bash
cppcheck --version
clang-tidy --version
```

---

## Configuration Files

### `.clang-tidy`

Configures clang-tidy checks and options:

**Enabled check categories:**
- `bugprone-*` - Bug-prone code patterns
- `performance-*` - Performance issues
- `modernize-*` - C++11/14/17/20/23 modernization
- `readability-*` - Code readability
- `cppcoreguidelines-*` - C++ Core Guidelines
- `clang-analyzer-*` - Deep static analysis

**Disabled checks:**
- Legacy-incompatible checks (e.g., `avoid-c-arrays`)
- Noise generators (e.g., `magic-numbers`, `identifier-length`)
- Checks requiring massive refactoring

**Philosophy:**
- Focus on bugs and security issues (errors)
- Modernization suggestions (warnings)
- Minimal noise for legacy code
- MCTS module held to stricter standards

### Excluded paths

Third-party libraries automatically excluded:
- `source/libs/*` - Bundled libraries (Loki, ParaGUI, SDL_mm)
- `source/dos/*` - DOS-specific legacy code

---

## Running Static Analysis

### Quick Start

Run all analysis:

```bash
./run-static-analysis.sh
```

Run only cppcheck:

```bash
./run-static-analysis.sh cppcheck
```

Run only clang-tidy:

```bash
./run-static-analysis.sh tidy
```

### Output

Reports are saved to `analysis-reports/`:
- `cppcheck-report.txt` - cppcheck findings
- `clang-tidy-mcts-report.txt` - clang-tidy findings (MCTS module)

### CI/CD Integration

Static analysis runs automatically on every push via GitHub Actions (`.github/workflows/ci.yml`):

```yaml
static-analysis:
  name: Static Analysis
  runs-on: ubuntu-24.04

  steps:
  - name: Run cppcheck
    run: |
      cppcheck \
        --enable=warning,style,performance,portability \
        --suppress=missingIncludeSystem \
        --suppress=unusedFunction \
        --inline-suppr \
        --quiet \
        --error-exitcode=0 \
        source/
```

---

## Understanding the Results

### cppcheck output format

```
file.cpp:42:warning:uninitvar:Uninitialized variable: foo
```

- **file.cpp:42** - Location (file:line)
- **warning** - Severity (error, warning, style, performance, portability)
- **uninitvar** - Check ID
- **Message** - Description

### clang-tidy output format

```
file.cpp:42:10: warning: use nullptr [modernize-use-nullptr]
```

- **file.cpp:42:10** - Location (file:line:column)
- **warning** - Severity
- **Message** - Issue description
- **[check-name]** - Check ID

---

## Severity Levels

### Critical (Fix immediately)
- Buffer overflows
- Use-after-free
- Null pointer dereferences
- Memory leaks
- Uninitialized variables

### High (Fix soon)
- Resource leaks
- Thread safety issues
- Logic errors
- Security vulnerabilities

### Medium (Fix when refactoring)
- Modernization suggestions
- Performance optimizations
- Code style issues

### Low (Optional)
- Readability improvements
- Minor style inconsistencies

---

## Advanced Usage

### Generate compile database

For better clang-tidy results:

```bash
# Install build interceptor
pip install scan-build

# Generate compile_commands.json
intercept-build make

# Now clang-tidy will have complete type information
clang-tidy source/ai/mcts/core/mcts_search.cpp
```

### Analyze specific files

```bash
# cppcheck on single file
cppcheck --enable=all source/gamemap.cpp

# clang-tidy on single file
clang-tidy source/gamemap.cpp -- -std=c++23 -I source/
```

### Suppress false positives

**In code (cppcheck):**
```cpp
// cppcheck-suppress uninitvar
int x;  // Intentionally uninitialized
```

**In code (clang-tidy):**
```cpp
// NOLINTNEXTLINE(modernize-use-nullptr)
void* ptr = NULL;  // NULL required for legacy API
```

**In .clang-tidy:**
```yaml
Checks: '-bugprone-easily-swappable-parameters'
```

---

## Integration with Development Workflow

### 1. Before Committing

Run analysis on modified files:

```bash
# Get changed files
git diff --name-only | grep '\.cpp$' > changed_files.txt

# Run cppcheck on changed files
cppcheck --file-list=changed_files.txt
```

### 2. During Code Review

CI/CD automatically runs analysis - check the reports in GitHub Actions artifacts.

### 3. Refactoring Sessions

Run full analysis to identify improvement opportunities:

```bash
./run-static-analysis.sh
grep "error:" analysis-reports/cppcheck-report.txt
```

---

## Baseline and Progress Tracking

### Establish Baseline

```bash
# Run analysis
./run-static-analysis.sh

# Count issues
wc -l analysis-reports/cppcheck-report.txt
# Example: 1247 issues

# Save baseline
cp analysis-reports/cppcheck-report.txt baseline-2025-11-16.txt
```

### Track Improvements

```bash
# After fixes
./run-static-analysis.sh

# Compare
wc -l analysis-reports/cppcheck-report.txt
# Example: 982 issues (265 fixed!)

# See what was fixed
diff baseline-2025-11-16.txt analysis-reports/cppcheck-report.txt
```

---

## Common Issues in Legacy Code

### Issue: Uninitialized variables

**Before:**
```cpp
int count;
if (condition) {
    count = getValue();
}
return count;  // ERROR: uninitialized if condition is false
```

**After:**
```cpp
int count = 0;  // Always initialize
if (condition) {
    count = getValue();
}
return count;
```

### Issue: Use of NULL instead of nullptr

**Before:**
```cpp
Widget* ptr = NULL;  // C-style
```

**After:**
```cpp
Widget* ptr = nullptr;  // Modern C++
```

### Issue: Manual memory management

**Before:**
```cpp
Widget* ptr = new Widget();
// ... risk of leak
delete ptr;
```

**After:**
```cpp
auto ptr = std::make_unique<Widget>();
// Automatic cleanup
```

---

## Performance Considerations

### cppcheck
- **Speed**: Fast (few minutes for full codebase)
- **Memory**: Low (<500 MB)
- **When to run**: Every commit

### clang-tidy
- **Speed**: Slow (can take hours for full codebase)
- **Memory**: High (1-2 GB)
- **When to run**:
  - MCTS module: Every commit
  - Full codebase: Weekly or before releases
  - Modified files only: On demand

---

## Troubleshooting

### "cppcheck: command not found"

```bash
sudo apt-get install cppcheck
```

### "clang-tidy: command not found"

```bash
sudo apt-get install clang-tidy
```

### clang-tidy gives many "unknown type" errors

You need a compile database:

```bash
pip install scan-build
intercept-build make
```

### Too many false positives

Tune `.clang-tidy` to disable noisy checks:

```yaml
Checks: '-readability-magic-numbers'
```

---

## Next Steps

1. **Install tools locally** (if not already):
   ```bash
   sudo apt-get install cppcheck clang-tidy
   ```

2. **Run initial analysis**:
   ```bash
   ./run-static-analysis.sh
   ```

3. **Review critical issues**:
   ```bash
   grep "error:" analysis-reports/cppcheck-report.txt
   ```

4. **Fix high-priority issues**:
   - Buffer overflows
   - Memory leaks
   - Null pointer dereferences

5. **Establish baseline**:
   ```bash
   cp analysis-reports/cppcheck-report.txt baseline-$(date +%Y-%m-%d).txt
   ```

---

## References

- [cppcheck Manual](http://cppcheck.net/manual.pdf)
- [clang-tidy Checks](https://clang.llvm.org/extra/clang-tidy/checks/list.html)
- [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines)

---

**Status**: Configuration complete. Ready for use. Install tools and run initial analysis.
