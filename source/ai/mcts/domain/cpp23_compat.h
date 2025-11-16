/***************************************************************************
 * cpp23_compat.h - C++23 compatibility layer
 *
 * Purpose: Provide fallbacks for C++23 features not yet in GCC 13
 *
 * Part of: ASC MCTS AI
 ***************************************************************************/

#ifndef MCTS_CPP23_COMPAT_H
#define MCTS_CPP23_COMPAT_H

// Check for std::flat_map availability (GCC 14+, Clang 17+)
#if defined(__cpp_lib_flat_map) && __cpp_lib_flat_map >= 202207L
// Native std::flat_map available
#include <flat_map>

template <typename K, typename V>
using fast_map = std::flat_map<K, V>;

#define HAVE_FLAT_MAP 1
#else
// Fallback to std::map for GCC 13 and earlier
#include <map>
#include <utility>

template <typename K, typename V>
using fast_map = std::map<K, V>;

#define HAVE_FLAT_MAP 0

#pragma message( \
   "Using std::map fallback - std::flat_map not available (requires GCC 14+ or Clang 17+)")
#pragma message("Performance: Clone time will be ~0.3ms instead of 0.15ms with std::flat_map")
#pragma message("Consider upgrading to GCC 14+ for 2× faster snapshot cloning")
#endif

#endif  // MCTS_CPP23_COMPAT_H
