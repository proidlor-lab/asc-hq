MAINTAINERCLEANFILES = Makefile.in

# Compiler flags
AM_CXXFLAGS = @SDL_CFLAGS@ @SIGC_CFLAGS@ -std=c++23

# Include paths
AM_CPPFLAGS = -I$(top_srcdir)/source \
              -I$(top_srcdir)/source/libs/paragui/include \
              @LokiInclude@

# Build convenience library (not installed, for linking into main ASC binary)
noinst_LTLIBRARIES = libmcts.la

# Source files
# Phase 0.1: Game State Cloning
# Phase 0.2: Action Execution Interface
# Phase 0.3: Basic Evaluation Function
# Phase 1: Core MCTS Engine
libmcts_la_SOURCES = \
	domain/unit_snapshot.cpp \
	domain/game_state_reader.cpp \
	domain/action_types.cpp \
	domain/simulation_action_executor.cpp \
	domain/real_game_action_executor.cpp \
	domain/action_executor_factory.cpp \
	domain/simple_combat_evaluator.cpp \
	domain/evaluator_factory.cpp \
	core/mcts_search.cpp \
	infrastructure/legacy_game_interface.cpp \
	mcts_manual_test.cpp

# Header files (for distribution)
EXTRA_DIST = \
	$(srcdir)/domain/*.h \
	$(srcdir)/domain/*.cpp \
	$(srcdir)/core/*.h \
	$(srcdir)/agents/*.h \
	$(srcdir)/coordination/*.h \
	$(srcdir)/infrastructure/*.h \
	$(srcdir)/*.md \
	$(srcdir)/docs/*.md

# Test binaries
noinst_PROGRAMS = snapshot_test action_executor_test evaluator_test
# mcts_test - TODO: Requires full ASC library linkage (Phase 1.4 integration)

# Phase 0.1 test
snapshot_test_SOURCES = domain/snapshot_test.cpp
snapshot_test_LDADD = libmcts.la

# Phase 0.2 test
action_executor_test_SOURCES = domain/action_executor_test.cpp
action_executor_test_LDADD = libmcts.la

# Phase 0.3 test
evaluator_test_SOURCES = domain/evaluator_test.cpp
evaluator_test_LDADD = libmcts.la

# Phase 1 test (DISABLED - needs full ASC library)
# mcts_test_SOURCES = core/mcts_test.cpp
# mcts_test_LDADD = libmcts.la $(top_builddir)/source/libasc.la
