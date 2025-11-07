MAINTAINERCLEANFILES = Makefile.in

# Compiler flags
AM_CXXFLAGS = @SDL_CFLAGS@ @SIGC_CFLAGS@ -std=c++23

# Include paths
AM_CPPFLAGS = -I$(top_srcdir)/source \
              -I$(top_srcdir)/source/libs/paragui/include \
              @LokiInclude@

# Build convenience library (not installed, for linking into main ASC binary)
noinst_LTLIBRARIES = libmcts.la

# Source files (Phase 0.1: Domain layer only)
libmcts_la_SOURCES = \
	domain/unit_snapshot.cpp \
	domain/game_state_reader.cpp

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

# Test binary (enabled for snapshot testing)
noinst_PROGRAMS = snapshot_test
snapshot_test_SOURCES = domain/snapshot_test.cpp
snapshot_test_LDADD = libmcts.la
