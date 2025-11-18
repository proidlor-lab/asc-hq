#!/bin/bash
# run-static-analysis.sh - Run static analysis on ASC codebase
#
# Usage:
#   ./run-static-analysis.sh           # Run both cppcheck and clang-tidy
#   ./run-static-analysis.sh cppcheck  # Run only cppcheck
#   ./run-static-analysis.sh tidy      # Run only clang-tidy

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Check if tools are installed
CPPCHECK_AVAILABLE=false
TIDY_AVAILABLE=false

if command -v cppcheck &> /dev/null; then
    CPPCHECK_AVAILABLE=true
    CPPCHECK_VERSION=$(cppcheck --version)
fi

if command -v clang-tidy &> /dev/null; then
    TIDY_AVAILABLE=true
    TIDY_VERSION=$(clang-tidy --version | head -1)
fi

# Determine what to run
RUN_CPPCHECK=false
RUN_TIDY=false

if [[ "$1" == "cppcheck" ]]; then
    RUN_CPPCHECK=true
elif [[ "$1" == "tidy" ]]; then
    RUN_TIDY=true
else
    # Run both by default
    RUN_CPPCHECK=true
    RUN_TIDY=true
fi

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}ASC Static Analysis${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""

# ====================
# CPPCHECK
# ====================

if $RUN_CPPCHECK; then
    if ! $CPPCHECK_AVAILABLE; then
        echo -e "${RED}ERROR: cppcheck not found!${NC}"
        echo "Install it with:"
        echo "  sudo apt-get install cppcheck"
        exit 1
    fi

    echo -e "${GREEN}Running cppcheck...${NC}"
    echo -e "${BLUE}Version: $CPPCHECK_VERSION${NC}"
    echo ""

    # Create output directory
    mkdir -p analysis-reports

    # Run cppcheck
    echo "Analyzing source code (this may take a few minutes)..."
    cppcheck \
        --enable=warning,style,performance,portability \
        --suppress=missingIncludeSystem \
        --suppress=unusedFunction \
        --suppress=unmatchedSuppression \
        --inline-suppr \
        --std=c++20 \
        --language=c++ \
        --quiet \
        --template='{file}:{line}:{severity}:{id}:{message}' \
        --error-exitcode=0 \
        -I source/ \
        --file-list=<(find source/ \( -name "*.cpp" -o -name "*.h" -o -name "*.hpp" \) \
                      -type f ! -path "source/libs/*" ! -path "source/dos/*") \
        2>&1 | tee analysis-reports/cppcheck-report.txt

    ISSUE_COUNT=$(wc -l < analysis-reports/cppcheck-report.txt)
    echo ""
    echo -e "${BLUE}cppcheck found $ISSUE_COUNT potential issues${NC}"
    echo "Report saved to: analysis-reports/cppcheck-report.txt"
    echo ""
fi

# ====================
# CLANG-TIDY
# ====================

if $RUN_TIDY; then
    if ! $TIDY_AVAILABLE; then
        echo -e "${RED}ERROR: clang-tidy not found!${NC}"
        echo "Install it with:"
        echo "  sudo apt-get install clang-tidy"
        exit 1
    fi

    echo -e "${GREEN}Running clang-tidy...${NC}"
    echo -e "${BLUE}Version: $TIDY_VERSION${NC}"
    echo ""

    # Create output directory
    mkdir -p analysis-reports

    # Check if compile_commands.json exists
    if [[ ! -f compile_commands.json ]]; then
        echo -e "${YELLOW}WARNING: compile_commands.json not found${NC}"
        echo "For best results, generate it with:"
        echo "  pip install scan-build"
        echo "  intercept-build make"
        echo ""
        echo "Running without compile database (may have limited results)..."
    fi

    # Run clang-tidy on a sample of files (full analysis would take too long)
    echo "Analyzing MCTS module (modern C++23 code)..."

    find source/ai/mcts/ -name "*.cpp" ! -path "*/test/*" | head -10 | while read -r file; do
        echo "  Analyzing: $file"
        clang-tidy "$file" \
            --config-file=.clang-tidy \
            -- \
            -std=c++23 \
            -I source/ \
            -I source/ai/mcts/ \
            -I source/libs/loki-0.1.6/include/ \
            2>&1 | grep -v "^[0-9]* warnings generated" || true
    done | tee analysis-reports/clang-tidy-mcts-report.txt

    echo ""
    echo -e "${BLUE}clang-tidy analysis complete${NC}"
    echo "Report saved to: analysis-reports/clang-tidy-mcts-report.txt"
    echo ""
    echo -e "${YELLOW}Note: Only MCTS module analyzed for now${NC}"
    echo "To analyze more files, edit this script or run clang-tidy manually"
    echo ""
fi

# ====================
# Summary
# ====================

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}Analysis Complete${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""

if $RUN_CPPCHECK && $CPPCHECK_AVAILABLE; then
    echo "  cppcheck: analysis-reports/cppcheck-report.txt"
fi

if $RUN_TIDY && $TIDY_AVAILABLE; then
    echo "  clang-tidy: analysis-reports/clang-tidy-mcts-report.txt"
fi

echo ""
echo -e "${GREEN}Next steps:${NC}"
echo "  1. Review the analysis reports"
echo "  2. Fix critical bugs and security issues"
echo "  3. Consider modernization suggestions"
echo ""
