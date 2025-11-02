
AUTOMAKE_OPTIONS = subdir-objects

MAINTAINERCLEANFILES	= Makefile.in

AM_CXXFLAGS = @SDL_CFLAGS@ @SDL_IMAGE_CFLAGS@ @SDL_MIXER_CFLAGS@ @SIGC_CFLAGS@ @LokiInclude@
AM_CFLAGS   = @SDL_CFLAGS@ @SDL_IMAGE_CFLAGS@ @SDL_MIXER_CFLAGS@

AM_CPPFLAGS      = -I$(srcdir) -I$(top_builddir) -I$(top_srcdir)/source/libs/sdl_sound

noinst_LTLIBRARIES	= libsdl.la libsdlsnd.la

libsdl_la_SOURCES 	= graphics.cpp events.cpp compat/compat.cpp
libsdl_la_LIBADD	= @SDL_LIBS@ @SDL_IMAGE_LIBS@ 

libsdlsnd_la_SOURCES 	= sound.cpp  
libsdlsnd_la_LIBADD	= libsdl.la @SDL_LIBS@ @SDL_MIXER_LIBS@ @SDL_SOUND_LIB@ 

EXTRA_DIST=$(srcdir)/*.h $(srcdir)/compat/*.h $(srcdir)/sdlstretch.cpp $(srcdir)/sdl.vcproj  
