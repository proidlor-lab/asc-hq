# tests/Makefile.am - Google Test-based modern test suite
# Created: 2025-11-17

if HAVE_GTEST

# Google Test source files (build it from submodule)
GTEST_DIR = $(top_srcdir)/third_party/googletest/googletest
GMOCK_DIR = $(top_srcdir)/third_party/googletest/googlemock

# Build Google Test as a convenience library
noinst_LTLIBRARIES = libgtest.la libgtest_main.la

libgtest_la_SOURCES = \
	$(GTEST_DIR)/src/gtest-all.cc

libgtest_la_CPPFLAGS = \
	-I$(GTEST_DIR)/include \
	-I$(GTEST_DIR) \
	-pthread

libgtest_la_LDFLAGS = -pthread

libgtest_main_la_SOURCES = \
	$(GTEST_DIR)/src/gtest_main.cc

libgtest_main_la_CPPFLAGS = \
	-I$(GTEST_DIR)/include \
	-I$(GTEST_DIR) \
	-pthread

libgtest_main_la_LIBADD = libgtest.la
libgtest_main_la_LDFLAGS = -pthread

# Test programs
TESTS = example_test \
	mcts_snapshot_test \
	mcts_action_executor_test \
	mcts_evaluator_test

check_PROGRAMS = $(TESTS)

# Common flags for all tests
AM_CPPFLAGS = \
	-I$(top_srcdir)/source \
	-I$(top_srcdir)/source/ai \
	-I$(top_srcdir)/source/ai/mcts \
	-I$(top_srcdir)/source/ai/mcts/domain \
	-I$(top_srcdir)/source/ai/mcts/core \
	-I$(top_srcdir)/source/ai/mcts/agents \
	-I$(top_srcdir)/source/ai/mcts/infrastructure \
	-I$(top_srcdir)/source/libs/paragui/include \
	-I$(GTEST_DIR)/include \
	@LokiInclude@ \
	@SDL_CFLAGS@ \
	@SIGC_CFLAGS@ \
	-pthread

AM_CXXFLAGS = -std=c++23 -Wall -Wextra

# Example test executable
example_test_SOURCES = unit/example_test.cpp
example_test_LDADD = \
	libgtest.la \
	libgtest_main.la
example_test_LDFLAGS = -pthread

# MCTS Snapshot test
mcts_snapshot_test_SOURCES = unit/mcts_snapshot_test.cpp
mcts_snapshot_test_LDADD = \
	libgtest.la \
	libgtest_main.la \
	$(top_builddir)/source/ai/mcts/libmcts.la \
	-lpng -lbz2
mcts_snapshot_test_LDFLAGS = -pthread

# MCTS Action Executor test
mcts_action_executor_test_SOURCES = unit/mcts_action_executor_test.cpp
mcts_action_executor_test_LDADD = \
	libgtest.la \
	libgtest_main.la \
	$(top_builddir)/source/ai/mcts/libmcts.la \
	-lpng -lbz2
mcts_action_executor_test_LDFLAGS = -pthread

# MCTS Evaluator test
mcts_evaluator_test_SOURCES = unit/mcts_evaluator_test.cpp
mcts_evaluator_test_LDADD = \
	libgtest.la \
	libgtest_main.la \
	$(top_builddir)/source/ai/mcts/libmcts.la \
	-lpng -lbz2
mcts_evaluator_test_LDFLAGS = -pthread

endif # HAVE_GTEST

# Always distribute test sources, even if Google Test is not available
EXTRA_DIST = \
	unit \
	integration \
	helpers

.PHONY: run-tests
run-tests: check
	@echo "All Google Test tests passed!"
