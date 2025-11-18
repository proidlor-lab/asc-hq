cmake_minimum_required(VERSION 3.20)
project(asc-hq VERSION 2.0 LANGUAGES CXX)

# C++ standard
# Note: Using C++20 for compatibility during migration
# MCTS module uses C++23 features, but core ASC code builds with C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler flags
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

# Output directories (keep build organized)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Find dependencies (optional for now - MCTS module has minimal external deps)
# find_package(SDL)
# find_package(Boost COMPONENTS regex)
# find_package(PNG)

# Note: MCTS module is self-contained and only needs standard C++ headers
# External dependencies will be added as we migrate more modules

# Include subdirectories
add_subdirectory(source/ai/mcts)

# Main executable (to be added in Phase 2)
# add_subdirectory(source)

message(STATUS "")
message(STATUS "==============================================")
message(STATUS "ASC-HQ CMake Build Configuration")
message(STATUS "==============================================")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: C++${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Build directory: ${CMAKE_BINARY_DIR}")
message(STATUS "")
message(STATUS "Phase 1: Building MCTS module only")
message(STATUS "Phase 2: Full project migration (TBD)")
message(STATUS "==============================================")
message(STATUS "")
