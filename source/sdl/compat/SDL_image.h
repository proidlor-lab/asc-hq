#ifndef ASC_SDL_COMPAT_SDL_IMAGE_H
#define ASC_SDL_COMPAT_SDL_IMAGE_H

#ifndef SDL_IMAGE_DISABLE_OLD_NAMES
#define SDL_IMAGE_DISABLE_OLD_NAMES
#endif

#include "SDL.h"
#include <SDL3_image/SDL_image.h>

#ifdef __cplusplus
extern "C" {
#endif

SDL_Surface* SDLCompat_IMG_Load_RW(SDL_RWops* src, int freesrc);
SDL_Surface* SDLCompat_IMG_LoadPNG_RW(SDL_RWops* src);
SDL_Surface* SDLCompat_IMG_LoadPCX_RW(SDL_RWops* src);

#ifdef __cplusplus
}
#endif

#ifdef IMG_Load_RW
#undef IMG_Load_RW
#endif
#ifdef IMG_LoadPNG_RW
#undef IMG_LoadPNG_RW
#endif
#ifdef IMG_LoadPCX_RW
#undef IMG_LoadPCX_RW
#endif

#define IMG_Load_RW SDLCompat_IMG_Load_RW
#define IMG_LoadPNG_RW SDLCompat_IMG_LoadPNG_RW
#define IMG_LoadPCX_RW SDLCompat_IMG_LoadPCX_RW

#endif /* ASC_SDL_COMPAT_SDL_IMAGE_H */
