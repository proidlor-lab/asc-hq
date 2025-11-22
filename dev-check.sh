#!/usr/bin/env bash
# Incremental local gate: configure (out-of-tree), build, and test.
# Noise is minimized—focus is on build/test failures.
# Environment overrides:
#   BUILD_DIR=build-dev      # out-of-tree build directory
#   BUILD_TYPE=Debug         # CMAKE_BUILD_TYPE
#   RECONFIGURE=1            # force reconfigure even if cache exists
#   GENERATOR="Ninja"        # override generator; default prefers Ninja if available

set -euo pipefail

BUILD_DIR="${BUILD_DIR:-build-dev}"
BUILD_TYPE="${BUILD_TYPE:-Debug}"
RECONFIGURE="${RECONFIGURE:-0}"

pick_generator() {
  if [[ -n "${GENERATOR:-}" ]]; then
    echo "${GENERATOR}"
  elif command -v ninja >/dev/null 2>&1; then
    echo "Ninja"
  else
    echo ""
  fi
}

configure() {
  local gen
  gen="$(pick_generator)"
  local args=(
    -S .
    -B "${BUILD_DIR}"
    -DCMAKE_BUILD_TYPE="${BUILD_TYPE}"
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
  )
  if [[ -n "${gen}" ]]; then
    args=(-G "${gen}" "${args[@]}")
  fi
  cmake "${args[@]}"
}

if [[ "${RECONFIGURE}" == "1" || ! -f "${BUILD_DIR}/CMakeCache.txt" ]]; then
  configure
fi

cmake --build "${BUILD_DIR}" -j"$(nproc)"
ctest --output-on-failure --test-dir "${BUILD_DIR}"
