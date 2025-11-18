#!/bin/bash
# format-code.sh - Format entire ASC codebase with clang-format
#
# Usage:
#   ./format-code.sh          # Dry run (show what would be formatted)
#   ./format-code.sh --apply  # Actually format the files

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Check if clang-format is installed
if ! command -v clang-format &> /dev/null; then
    echo -e "${RED}ERROR: clang-format not found!${NC}"
    echo "Please install it with:"
    echo "  sudo apt-get install clang-format"
    exit 1
fi

CLANG_FORMAT_VERSION=$(clang-format --version)
echo -e "${BLUE}Using: $CLANG_FORMAT_VERSION${NC}"
echo ""

# Determine mode
DRY_RUN=true
if [[ "$1" == "--apply" ]]; then
    DRY_RUN=false
    echo -e "${YELLOW}⚠️  APPLYING FORMATTING (files will be modified)${NC}"
else
    echo -e "${GREEN}DRY RUN MODE (no files will be modified)${NC}"
    echo "Run with --apply to actually format files"
fi
echo ""

# Find all C++ source files (excluding third-party libs)
echo "Finding C++ source files..."
echo "(Excluding third-party libraries in source/libs/)"
FILES=$(find source/ \( -name "*.cpp" -o -name "*.h" -o -name "*.hpp" \) -type f \
    ! -path "source/libs/*")
FILE_COUNT=$(echo "$FILES" | wc -l)

echo -e "${GREEN}Found $FILE_COUNT files to format${NC}"
echo ""

# Count files that would be changed
if $DRY_RUN; then
    echo "Checking which files need formatting..."
    CHANGED_COUNT=0
    TOTAL=0

    while IFS= read -r file; do
        TOTAL=$((TOTAL + 1))

        # Show progress every 100 files
        if (( TOTAL % 100 == 0 )); then
            echo "  Checked $TOTAL/$FILE_COUNT files..."
        fi

        # Check if file would be changed
        if ! clang-format "$file" | diff -q "$file" - &> /dev/null; then
            CHANGED_COUNT=$((CHANGED_COUNT + 1))
            if (( CHANGED_COUNT <= 20 )); then
                echo -e "  ${YELLOW}Would format:${NC} $file"
            fi
        fi
    done <<< "$FILES"

    echo ""
    if (( CHANGED_COUNT > 20 )); then
        echo -e "${YELLOW}... and $((CHANGED_COUNT - 20)) more files${NC}"
    fi
    echo ""
    echo -e "${BLUE}Summary:${NC}"
    echo "  Total files: $FILE_COUNT"
    echo "  Need formatting: $CHANGED_COUNT"
    echo "  Already formatted: $((FILE_COUNT - CHANGED_COUNT))"
    echo ""

    if (( CHANGED_COUNT > 0 )); then
        echo -e "${GREEN}To apply formatting, run:${NC}"
        echo "  ./format-code.sh --apply"
    else
        echo -e "${GREEN}All files are already formatted!${NC}"
    fi
else
    # Apply formatting
    echo "Formatting files..."
    FORMATTED=0
    TOTAL=0

    while IFS= read -r file; do
        TOTAL=$((TOTAL + 1))

        # Show progress every 50 files
        if (( TOTAL % 50 == 0 )); then
            echo "  Formatted $TOTAL/$FILE_COUNT files..."
        fi

        # Format file in-place
        clang-format -i "$file"
        FORMATTED=$((FORMATTED + 1))
    done <<< "$FILES"

    echo ""
    echo -e "${GREEN}✓ Formatted $FORMATTED files${NC}"
    echo ""
    echo -e "${YELLOW}Next steps:${NC}"
    echo "  1. Review the changes: git diff"
    echo "  2. Build the project: make clean && make"
    echo "  3. Run tests: make check"
    echo "  4. If all looks good: git add -A && git commit -m 'Apply clang-format to entire codebase'"
fi

echo ""
echo -e "${BLUE}Done!${NC}"
