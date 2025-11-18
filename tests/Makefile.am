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
# 	version_identifier_test \
# 	stream_encoding_test \
# 	game_events_test

check_PROGRAMS = $(TESTS)

# Common flags for all tests
AM_CPPFLAGS = \
	-I$(top_srcdir)/source \
	-I$(top_srcdir)/source/unittests \
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

# Phase 1: Legacy test migrations

# Common libraries needed for Phase 1 tests (matching unittester dependencies)
TEST_COMMON_LIBS = \
	$(top_builddir)/source/libs/triangul/libtriangul.la \
	$(top_builddir)/source/ai/libai.la \
	$(top_builddir)/source/libs/sdlmm/src/libSDLmm.la \
	$(top_builddir)/source/sdl/libsdl.la \
	$(LOKI_LIBS) $(LUA_GAME_LIB) \
	$(top_builddir)/source/util/libutil.la \
	$(top_builddir)/source/tasks/libtaskcore.la \
	$(top_builddir)/source/tasks/libtasks.la \
	$(top_builddir)/source/graphics/libgraphics.la \
	$(top_builddir)/source/dialogs/libdialogs.la \
	$(top_builddir)/source/dialogs/libgamedialogs.la \
	$(top_builddir)/source/actions/libactions.la \
	$(top_builddir)/source/libs/paragui/src/libparagui.la \
	$(top_builddir)/source/widgets/libwidgets.la \
	$(top_builddir)/source/sdl/libsdlsnd.la \
	$(top_builddir)/source/network/libnetwork.la \
	$(top_builddir)/source/video/libvideo.la \
	$(top_builddir)/source/actions/libdirectunitactions.la \
	$(top_builddir)/source/actions/libmapactions.la \
	$(top_builddir)/source/lua/libluainterfacegame.la \
	$(BZIP2_LIBS) $(SIGC_LIBS) $(SDL_LIBS) $(FREETYPE_LIBS) $(BOOST_LIBS) $(SDL_SOUND_LIB) $(WX_LIBS)

# Version Identifier test
version_identifier_test_SOURCES = \
	unit/version_identifier_test.cpp \
	$(top_srcdir)/source/versionidentifier.cpp \
	$(top_srcdir)/source/ascstring.cpp \
	$(top_srcdir)/source/basestrm.cpp \
	$(top_srcdir)/source/simplestream.cpp
version_identifier_test_LDADD = \
	libgtest.la \
	libgtest_main.la \
	$(TEST_COMMON_LIBS)
version_identifier_test_LDFLAGS = -pthread

# Stream Encoding test
stream_encoding_test_SOURCES = \
	unit/stream_encoding_test.cpp \
	$(top_srcdir)/source/basestrm.cpp \
	$(top_srcdir)/source/simplestream.cpp \
	$(top_srcdir)/source/sgstream.cpp
stream_encoding_test_LDADD = \
	libgtest.la \
	libgtest_main.la \
	$(TEST_COMMON_LIBS)
stream_encoding_test_LDFLAGS = -pthread

# Game Events test
game_events_test_SOURCES = unit/game_events_test.cpp
game_events_test_LDADD = \
	libgtest.la \
	libgtest_main.la \
	$(TEST_COMMON_LIBS)
game_events_test_LDFLAGS = -pthread

endif # HAVE_GTEST

# Always distribute test sources, even if Google Test is not available
EXTRA_DIST = \
	unit \
	integration \
	helpers

.PHONY: run-tests
run-tests: check
	@echo "All Google Test tests passed!"
