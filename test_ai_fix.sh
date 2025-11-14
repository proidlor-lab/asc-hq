#!/bin/bash
# Test script to verify AI type selection works correctly

echo "========================================"
echo "Testing AI Type Selection Fix"
echo "========================================"
echo ""

MAP_FILE="$HOME/.asc/SeaBattle.map"

if [ ! -f "$MAP_FILE" ]; then
    echo "Warning: $MAP_FILE not found"
    echo "Using built-in map instead"
    MAP_FILE="data/maps/tutorial.ascmap"
fi

echo "Test 1: MCTS Defensive vs Classic"
echo "-----------------------------------"
./source/unix/asc/asc -M "$MAP_FILE" -H \
    --player1 mcts_defensive \
    --player2 classic \
    -T 1 -r 1 2>&1 | grep -E "(Player.*AI type|AIFactory)"

echo ""
echo "Test 2: MCTS Balanced vs MCTS Aggressive"
echo "-----------------------------------"
./source/unix/asc/asc -M "$MAP_FILE" -H \
    --player1 mcts_balanced \
    --player2 mcts_aggressive \
    -T 1 -r 1 2>&1 | grep -E "(Player.*AI type|AIFactory)"

echo ""
echo "Test 3: All MCTS Variants"
echo "-----------------------------------"
for ai in mcts_balanced mcts_aggressive mcts_defensive mcts_fast mcts_deep; do
    echo "Testing $ai:"
    ./source/unix/asc/asc -M "$MAP_FILE" -H \
        --player1 "$ai" \
        --player2 classic \
        -T 1 -r 1 2>&1 | grep "AIFactory" | head -1
done

echo ""
echo "========================================"
echo "Expected: Each test should show correct AI types"
echo "If you see 'MCTS' in AIFactory messages, the fix works!"
echo "========================================"
