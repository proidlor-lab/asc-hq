
MAINTAINERCLEANFILES	= Makefile.in

# Build MCTS AI subdirectory
SUBDIRS = mcts

AM_CXXFLAGS		= @SDL_CFLAGS@ @SIGC_CFLAGS@ -Dsgmain @LokiInclude@

AM_CPPFLAGS = -I$(top_srcdir)/source/libs/paragui/include 

noinst_LTLIBRARIES 	= libai.la

libai_la_SOURCES	= base.cpp misc.cpp strategy.cpp valuation.cpp buildingcapture.cpp service.cpp tactics.cpp \
                          ai_factory.cpp mcts_ai.cpp
libai_la_LIBADD		= @SDL_LIBS@ mcts/libmcts.la

EXTRA_DIST=$(srcdir)/*.h
