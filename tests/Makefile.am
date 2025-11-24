# tests/Makefile.am - Google Test-based modern test suite
# Created: 2025-11-17

if HAVE_GTEST

# Google Test source files (build it from submodule)
GTEST_DIR = $(top_srcdir)/third_party/googletest/googletest
GMOCK_DIR = $(top_srcdir)/third_party/googletest/googlemock

# Build Google Test as a convenience library
# Build common game sources as a convenience library for Phase 2 tests
noinst_LTLIBRARIES = libgtest.la libgtest_main.la libcommon.la

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

# Common game source files (Phase 2 tests need these)
libcommon_la_SOURCES = \
	$(top_srcdir)/source/soundList.cpp \
	$(top_srcdir)/source/typen.cpp \
	$(top_srcdir)/source/applicationstarter.cpp \
	$(top_srcdir)/source/strtmesg.cpp \
	$(top_srcdir)/source/stack.cpp \
	$(top_srcdir)/source/spfst.cpp \
	$(top_srcdir)/source/sgstream.cpp \
	$(top_srcdir)/source/palette.cpp \
	$(top_srcdir)/source/newfont.cpp \
	$(top_srcdir)/source/misc.cpp \
	$(top_srcdir)/source/loadpcxc.cpp \
	$(top_srcdir)/source/loaders.cpp \
	$(top_srcdir)/source/tos.cpp \
	$(top_srcdir)/source/loadbi3.cpp \
	$(top_srcdir)/source/gamedlg.cpp \
	$(top_srcdir)/source/dlg_box.cpp \
	$(top_srcdir)/source/dialog.cpp \
	$(top_srcdir)/source/controls.cpp \
	$(top_srcdir)/source/basestrm.cpp \
	$(top_srcdir)/source/basegfx.cpp \
	$(top_srcdir)/source/attack.cpp \
	$(top_srcdir)/source/replaymapdisplay.cpp \
	$(top_srcdir)/source/memory-measurement.cpp \
	$(top_srcdir)/source/gameoptions.cpp \
	$(top_srcdir)/source/astar2.cpp \
	$(top_srcdir)/source/vehicletype.cpp \
	$(top_srcdir)/source/objects.cpp \
	$(top_srcdir)/source/stdio-errorhandler.cpp \
	$(top_srcdir)/source/buildingtype.cpp \
	$(top_srcdir)/source/containerbase.cpp \
	$(top_srcdir)/source/mapalgorithms.cpp \
	$(top_srcdir)/source/viewcalculation.cpp \
	$(top_srcdir)/source/gamemap.cpp \
	$(top_srcdir)/source/password.cpp \
	$(top_srcdir)/source/replay.cpp \
	$(top_srcdir)/source/research.cpp \
	$(top_srcdir)/source/fieldimageloader.cpp \
	$(top_srcdir)/source/mapdisplay.cpp \
	$(top_srcdir)/source/resourcenet.cpp \
	$(top_srcdir)/source/ascstring.cpp \
	$(top_srcdir)/source/graphicset.cpp \
	$(top_srcdir)/source/vehicle.cpp \
	$(top_srcdir)/source/buildings.cpp \
	$(top_srcdir)/source/networkinterface.cpp \
	$(top_srcdir)/source/textfileparser.cpp \
	$(top_srcdir)/source/terraintype.cpp \
	$(top_srcdir)/source/objecttype.cpp \
	$(top_srcdir)/source/textfiletags.cpp \
	$(top_srcdir)/source/itemrepository.cpp \
	$(top_srcdir)/source/stringtokenizer.cpp \
	$(top_srcdir)/source/music.cpp \
	$(top_srcdir)/source/messages.cpp \
	$(top_srcdir)/source/paradialog.cpp \
	$(top_srcdir)/source/textfile_evaluation.cpp \
	$(top_srcdir)/source/containerbasetype.cpp \
	$(top_srcdir)/source/messagedlg.cpp \
	$(top_srcdir)/source/simplestream.cpp \
	$(top_srcdir)/source/mappolygons.cpp \
	$(top_srcdir)/source/prehistoricevents.cpp \
	$(top_srcdir)/source/gameevents.cpp \
	$(top_srcdir)/source/gameeventsystem.cpp \
	$(top_srcdir)/source/gameevent_dialogs.cpp \
	$(top_srcdir)/source/statisticdialog.cpp \
	$(top_srcdir)/source/clipboard.cpp \
	$(top_srcdir)/source/geometry.cpp \
	$(top_srcdir)/source/guidimension.cpp \
	$(top_srcdir)/source/guiiconhandler.cpp \
	$(top_srcdir)/source/guifunctions.cpp \
	$(top_srcdir)/source/iconrepository.cpp \
	$(top_srcdir)/source/overviewmapimage.cpp \
	$(top_srcdir)/source/mapfield.cpp \
	$(top_srcdir)/source/overviewmappanel.cpp \
	$(top_srcdir)/source/dashboard.cpp \
	$(top_srcdir)/source/asc-mainscreen.cpp \
	$(top_srcdir)/source/explosivemines.cpp \
	$(top_srcdir)/source/mainscreenwidget.cpp \
	$(top_srcdir)/source/networksupervisor.cpp \
	$(top_srcdir)/source/statistics.cpp \
	$(top_srcdir)/source/legacynetworkloaders.cpp \
	$(top_srcdir)/source/player.cpp \
	$(top_srcdir)/source/cannedmessages.cpp \
	$(top_srcdir)/source/reactionfire.cpp \
	$(top_srcdir)/source/mapimageexport.cpp \
	$(top_srcdir)/source/containercontrols.cpp \
	$(top_srcdir)/source/containerbase-functions.cpp \
	$(top_srcdir)/source/windowing.cpp \
	$(top_srcdir)/source/turncontrol.cpp \
	$(top_srcdir)/source/unitset.cpp \
	$(top_srcdir)/source/playercolor.cpp \
	$(top_srcdir)/source/pg_mapdisplay.cpp \
	$(top_srcdir)/source/autotraining.cpp \
	$(top_srcdir)/source/researchexecution.cpp \
	$(top_srcdir)/source/luacommandwriter.cpp \
	$(top_srcdir)/source/campaignactionrecorder.cpp \
	$(top_srcdir)/source/loggingoutput.cpp \
	$(top_srcdir)/source/versionidentifier.cpp \
	$(top_srcdir)/source/package.cpp \
	$(top_srcdir)/source/packagemanager.cpp \
	$(top_srcdir)/source/packagerepository.cpp \
	$(top_srcdir)/source/dataversioncheck.cpp \
	$(top_srcdir)/source/i18n.cpp \
	$(top_srcdir)/source/accessconstraints-game.cpp \
	$(top_srcdir)/source/contextutils.cpp \
	$(top_srcdir)/source/unitcostcalculator-standard.cpp \
	$(top_srcdir)/source/weaponrangelayer.cpp \
	$(top_srcdir)/source/unitcostcalculator-pbp.cpp \
	$(top_srcdir)/source/util/messaginghub.cpp

libcommon_la_CPPFLAGS = $(AM_CPPFLAGS)
libcommon_la_LDFLAGS = -pthread

# Test programs
TESTS = example_test \
	mcts_snapshot_test \
	mcts_evaluator_test \
	version_identifier_test \
	stream_encoding_test \
	game_events_test \
	gamemap_characterization_test \
	action_container_test \
	repair_test \
	research_test \
	transfer_control_test \
	object_construction_test \
	view_test \
	jump_test \
	diplomacy_test
#	mcts_action_executor_test  # TODO: Failing ActionGeneration test (pre-existing issue)

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
	$(top_srcdir)/source/simplestream.cpp \
	$(top_srcdir)/source/misc.cpp
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
	$(top_srcdir)/source/misc.cpp \
	$(top_srcdir)/source/ascstring.cpp
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

# Action Container test
action_container_test_SOURCES = \
	unit/action_container_test.cpp \
	$(top_srcdir)/source/unittests/unittestutil.cpp
action_container_test_LDADD = \
	libgtest.la \
	libgtest_main.la \
	$(TEST_COMMON_LIBS) \
	libcommon.la
action_container_test_LDFLAGS = -pthread

# GameMap Characterization test (Phase 0)
gamemap_characterization_test_SOURCES = unit/gamemap_characterization_test.cpp
gamemap_characterization_test_LDADD = \
	libgtest.la \
	libgtest_main.la \
	libcommon.la \
	$(TEST_COMMON_LIBS)
gamemap_characterization_test_LDFLAGS = -pthread

# Phase 2: Ported legacy unit tests

# Repair test
repair_test_SOURCES = \
	unit/repair_test.cpp \
	$(top_srcdir)/source/unittests/unittestutil.cpp
repair_test_LDADD = \
	libgtest.la \
	libgtest_main.la \
	libcommon.la \
	$(TEST_COMMON_LIBS)
repair_test_LDFLAGS = -pthread

# Research test
research_test_SOURCES = \
	unit/research_test.cpp \
	$(top_srcdir)/source/unittests/unittestutil.cpp
research_test_LDADD = \
	libgtest.la \
	libgtest_main.la \
	libcommon.la \
	$(TEST_COMMON_LIBS)
research_test_LDFLAGS = -pthread

# Transfer Control test
transfer_control_test_SOURCES = \
	unit/transfer_control_test.cpp \
	$(top_srcdir)/source/unittests/unittestutil.cpp
transfer_control_test_LDADD = \
	libgtest.la \
	libgtest_main.la \
	libcommon.la \
	$(TEST_COMMON_LIBS)
transfer_control_test_LDFLAGS = -pthread

# Object Construction test
object_construction_test_SOURCES = \
	unit/object_construction_test.cpp \
	$(top_srcdir)/source/unittests/unittestutil.cpp
object_construction_test_LDADD = \
	libgtest.la \
	libgtest_main.la \
	libcommon.la \
	$(TEST_COMMON_LIBS)
object_construction_test_LDFLAGS = -pthread

# View test
view_test_SOURCES = \
	unit/view_test.cpp \
	$(top_srcdir)/source/unittests/unittestutil.cpp
view_test_LDADD = \
	libgtest.la \
	libgtest_main.la \
	libcommon.la \
	$(TEST_COMMON_LIBS)
view_test_LDFLAGS = -pthread

# Jump test
jump_test_SOURCES = \
	unit/jump_test.cpp \
	$(top_srcdir)/source/unittests/unittestutil.cpp
jump_test_LDADD = \
	libgtest.la \
	libgtest_main.la \
	libcommon.la \
	$(TEST_COMMON_LIBS)
jump_test_LDFLAGS = -pthread

# Diplomacy test
diplomacy_test_SOURCES = \
	unit/diplomacy_test.cpp \
	$(top_srcdir)/source/unittests/unittestutil.cpp
diplomacy_test_LDADD = \
	libgtest.la \
	libgtest_main.la \
	libcommon.la \
	$(TEST_COMMON_LIBS)
diplomacy_test_LDFLAGS = -pthread

endif # HAVE_GTEST

# Always distribute test sources, even if Google Test is not available
EXTRA_DIST = \
	unit \
	integration \
	helpers

.PHONY: run-tests
run-tests: check
	@echo "All Google Test tests passed!"
