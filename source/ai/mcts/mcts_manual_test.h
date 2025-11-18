/***************************************************************************
 * mcts_manual_test.h - Manual integration test interface for MCTS AI
 *
 * Purpose: Simple interface to test MCTS engine with real ASC game state
 *          Can be called from console/debug commands
 *
 * Usage:
 *   From game code or console:
 *   runMCTSManualTest(actmap);  // Test current game state
 ***************************************************************************/

#ifndef MCTS_MANUAL_TEST_H
#define MCTS_MANUAL_TEST_H

class GameMap;

namespace asc {
namespace mcts {

/**
 * Manual integration test for MCTS AI
 *
 * Runs a quick MCTS search on the current game state and
 * prints results to console/log.
 *
 * @param gameMap Current game map (uses actmap typically)
 * @param iterations Number of MCTS iterations (default: 100)
 * @param playerID Which player to search for (default: current player)
 *
 * @return true if test succeeded, false on error
 */
bool runMCTSManualTest(GameMap* gameMap, int iterations = 100, int playerID = -1);

/**
 * Quick sanity test - just verify components load
 *
 * @return true if all components initialized successfully
 */
bool mctsQuickSanityCheck();

}  // namespace mcts
}  // namespace asc

#endif  // MCTS_MANUAL_TEST_H
