#ifndef ASC_SDL_COMPAT_SDL_SOUND_STUB_H
#define ASC_SDL_COMPAT_SDL_SOUND_STUB_H

#if defined(ASC_HAVE_SDL3_MIXER) && ASC_HAVE_SDL3_MIXER

// When using SDL3_mixer, we don't need SDL_sound for audio decoding
// SDL3_mixer has built-in decoders. These are stubs to allow compilation.

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Sound_Sample {
    void *opaque;
    void *buffer;
    unsigned int buffer_size;
} Sound_Sample;

typedef struct Sound_AudioInfo {
    unsigned short format;
    unsigned char channels;
    unsigned int rate;
} Sound_AudioInfo;

int Sound_Init(void);
void Sound_Quit(void);
const char* Sound_GetError(void);
Sound_Sample* Sound_NewSample(void*, const char*, Sound_AudioInfo*, unsigned int);
unsigned int Sound_DecodeAll(Sound_Sample*);
void Sound_FreeSample(Sound_Sample*);

#ifdef __cplusplus
}
#endif

#endif /* defined(ASC_HAVE_SDL3_MIXER) && ASC_HAVE_SDL3_MIXER */

#endif /* ASC_SDL_COMPAT_SDL_SOUND_STUB_H */
