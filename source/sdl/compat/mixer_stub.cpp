#include "config.h"

#if !defined(ASC_HAVE_SDL3_MIXER) || !ASC_HAVE_SDL3_MIXER

#include "SDL.h"
#include "SDL_mixer.h"

#include <SDL3/SDL_error.h>

struct Mix_Music {
   int unused;
};

namespace {
SDL_version gMixerStubVersion = { SDL_MIXER_MAJOR_VERSION,
                                  SDL_MIXER_MINOR_VERSION,
                                  SDL_MIXER_PATCHLEVEL };

void (*gMusicFinishedCallback)(void) = nullptr;
void (*gChannelFinishedCallback)(int) = nullptr;
} // namespace

extern "C" {

const SDL_version* Mix_Linked_Version(void)
{
   return &gMixerStubVersion;
}

int Mix_OpenAudio(int, Uint16, int, int)
{
   SDL_SetError("SDL_mixer stub: audio mixing not available");
   return -1;
}

int Mix_QuerySpec(int* frequency, Uint16* format, int* channels)
{
   if (frequency)
      *frequency = 0;
    if (format)
      *format = 0;
   if (channels)
      *channels = 0;
   return 0;
}

void Mix_HookMusicFinished(void (*callback)(void))
{
   gMusicFinishedCallback = callback;
   (void)gMusicFinishedCallback;
}

void Mix_ChannelFinished(void (*callback)(int))
{
   gChannelFinishedCallback = callback;
   (void)gChannelFinishedCallback;
}

int Mix_Playing(int)
{
   return 0;
}

int Mix_HaltChannel(int)
{
   return 0;
}

void Mix_FreeMusic(Mix_Music* music)
{
   if (music)
      delete music;
}

Mix_Music* Mix_LoadMUS(const char*)
{
   SDL_SetError("SDL_mixer stub: Mix_LoadMUS unavailable");
   return nullptr;
}

int Mix_PlayMusic(Mix_Music*, int)
{
   SDL_SetError("SDL_mixer stub: Mix_PlayMusic unavailable");
   return -1;
}

int Mix_VolumeMusic(int)
{
   return 0;
}

void Mix_PauseMusic(void)
{
}

void Mix_ResumeMusic(void)
{
}

int Mix_HaltMusic(void)
{
   return 0;
}

void Mix_CloseAudio(void)
{
}

int Mix_FadeInChannel(int, Mix_Chunk*, int, int)
{
   SDL_SetError("SDL_mixer stub: Mix_FadeInChannel unavailable");
   return -1;
}

int Mix_PlayChannel(int, Mix_Chunk*, int)
{
   SDL_SetError("SDL_mixer stub: Mix_PlayChannel unavailable");
   return -1;
}

int Mix_Volume(int, int)
{
   return 0;
}

int Mix_FadeOutChannel(int, int)
{
   return 0;
}

} // extern "C"

#endif /* !ASC_HAVE_SDL3_MIXER */
