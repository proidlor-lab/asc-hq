#include "config.h"

#if defined(ASC_HAVE_SDL3_MIXER) && ASC_HAVE_SDL3_MIXER

// When using SDL3_mixer, we don't need SDL_sound for audio decoding
// SDL3_mixer has built-in decoders. These are stubs to allow linking.

#include <cstddef>
#include "SDL_sound_stub.h"

extern "C" {

int Sound_Init(void) { 
    return 1; /* Success */ 
}

void Sound_Quit(void) {}

const char* Sound_GetError(void) { 
    return "SDL_sound stub - not available with SDL3_mixer"; 
}

Sound_Sample* Sound_NewSample(void*, const char*, Sound_AudioInfo*, unsigned int) { 
    return NULL; 
}

unsigned int Sound_DecodeAll(Sound_Sample*) { 
    return 0; 
}

void Sound_FreeSample(Sound_Sample*) {}

}

#endif /* defined(ASC_HAVE_SDL3_MIXER) && ASC_HAVE_SDL3_MIXER */
