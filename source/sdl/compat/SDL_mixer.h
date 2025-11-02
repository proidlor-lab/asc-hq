#ifndef ASC_SDL_COMPAT_SDL_MIXER_H
#define ASC_SDL_COMPAT_SDL_MIXER_H

#if defined(ASC_HAVE_SDL3_MIXER) && ASC_HAVE_SDL3_MIXER
#include "SDL.h"
#include <SDL3/SDL_error.h>
#include <SDL3_mixer/SDL_mixer.h>
#else

#include "SDL.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Version helpers mimic the SDL_mixer 1.x macros so legacy code compiles. */
#define SDL_MIXER_MAJOR_VERSION 1
#define SDL_MIXER_MINOR_VERSION 2
#define SDL_MIXER_PATCHLEVEL    0

#define SDL_MIXER_VERSION(X)           \
   do {                                \
      (X)->major = SDL_MIXER_MAJOR_VERSION; \
      (X)->minor = SDL_MIXER_MINOR_VERSION; \
      (X)->patch = SDL_MIXER_PATCHLEVEL;    \
   } while (0)

#define MIX_MAJOR_VERSION   SDL_MIXER_MAJOR_VERSION
#define MIX_MINOR_VERSION   SDL_MIXER_MINOR_VERSION
#define MIX_PATCHLEVEL      SDL_MIXER_PATCHLEVEL
#define MIX_VERSION(X)      SDL_MIXER_VERSION(X)

#define MIX_CHANNELS        8
#define MIX_DEFAULT_FREQUENCY 22050
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
#define MIX_DEFAULT_FORMAT  SDL_AUDIO_S16LE
#else
#define MIX_DEFAULT_FORMAT  SDL_AUDIO_S16BE
#endif
#define MIX_MAX_VOLUME      128

typedef struct Mix_Chunk {
   int allocated;
   Uint8* abuf;
   Uint32 alen;
   Uint8 volume;
} Mix_Chunk;

typedef struct Mix_Music Mix_Music;

static inline const SDL_version* Mix_Linked_Version(void)
{
   static SDL_version version = { SDL_MIXER_MAJOR_VERSION,
                                  SDL_MIXER_MINOR_VERSION,
                                  SDL_MIXER_PATCHLEVEL };
   return &version;
}

static inline int Mix_OpenAudio(int, Uint16, int, int)
{
   SDL_SetError("SDL_mixer stub: audio mixing not available");
   return -1;
}

static inline int Mix_QuerySpec(int* frequency, Uint16* format, int* channels)
{
   if (frequency)
      *frequency = 0;
   if (format)
      *format = 0;
   if (channels)
      *channels = 0;
   return 0;
}

static inline void Mix_HookMusicFinished(void (*callback)(void))
{
   (void)callback;
}

static inline void Mix_ChannelFinished(void (*callback)(int))
{
   (void)callback;
}

static inline int Mix_Playing(int)
{
   return 0;
}

static inline int Mix_HaltChannel(int)
{
   return 0;
}

static inline void Mix_FreeMusic(Mix_Music* music)
{
   (void)music;
}

static inline Mix_Music* Mix_LoadMUS(const char*)
{
   SDL_SetError("SDL_mixer stub: Mix_LoadMUS unavailable");
   return NULL;
}

static inline int Mix_PlayMusic(Mix_Music*, int)
{
   SDL_SetError("SDL_mixer stub: Mix_PlayMusic unavailable");
   return -1;
}

static inline int Mix_VolumeMusic(int)
{
   return 0;
}

static inline void Mix_PauseMusic(void)
{
}

static inline void Mix_ResumeMusic(void)
{
}

static inline int Mix_HaltMusic(void)
{
   return 0;
}

static inline void Mix_CloseAudio(void)
{
}

static inline int Mix_FadeInChannel(int, Mix_Chunk*, int, int)
{
   SDL_SetError("SDL_mixer stub: Mix_FadeInChannel unavailable");
   return -1;
}

static inline int Mix_PlayChannel(int, Mix_Chunk*, int)
{
   SDL_SetError("SDL_mixer stub: Mix_PlayChannel unavailable");
   return -1;
}

static inline int Mix_Volume(int, int)
{
   return 0;
}

static inline int Mix_FadeOutChannel(int, int)
{
   return 0;
}

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* defined(ASC_HAVE_SDL3_MIXER) && ASC_HAVE_SDL3_MIXER */

#endif /* ASC_SDL_COMPAT_SDL_MIXER_H */
