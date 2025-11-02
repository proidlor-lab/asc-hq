/*
 * SDLmm - a C++ wrapper for SDL and related libraries
 * Copyright (C) 2001 David Hedbor <david@hedbor.org>
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "sdlmm_config.h"

#include <SDL.h>
#include "sdlmm_pixelformat.h"

// The SDLmm::PixelFormat class.

namespace SDLmm {

  void PixelFormat::init(SDL_PixelFormat fmt, SDL_Surface *surface)
  {
    surfaceRef = surface;
    if (surfaceRef) {
      formatEnum = surfaceRef->format;
      info = SDLCompat_BuildSurfacePixelFormat(surfaceRef);
    } else {
      formatEnum = fmt;
      info = SDLCompat_BuildPixelFormat(formatEnum);
    }
  }

  PixelFormat::PixelFormat()
  {
    init(SDL_PIXELFORMAT_UNKNOWN, nullptr);
  }

  PixelFormat::PixelFormat(SDL_PixelFormat fmt)
  {
    init(fmt, nullptr);
  }

  PixelFormat::PixelFormat(SDL_Surface *surface)
  {
    init(surface ? surface->format : SDL_PIXELFORMAT_UNKNOWN, surface);
  }

  PixelFormat::PixelFormat(const SDL_Surface *surface)
  {
    init(surface ? surface->format : SDL_PIXELFORMAT_UNKNOWN,
         const_cast<SDL_Surface *>(surface));
  }

  PixelFormat::PixelFormat(const SDL_CompatPixelFormat &formatInfo)
  {
    formatEnum = SDL_PIXELFORMAT_UNKNOWN;
    surfaceRef = nullptr;
    info = formatInfo;
  }

  Color PixelFormat::MapRGB(Uint8 r, Uint8 g, Uint8 b) const {
    const SDL_PixelFormatDetails* details = SDL_GetPixelFormatDetails(formatEnum);
    if (!details)
      return 0;
    return SDL_MapRGB(details, info.palette, r, g, b);
  }

  Color PixelFormat::MapRGBA(Uint8 r, Uint8 g, Uint8 b, Uint8 a) const {
    const SDL_PixelFormatDetails* details = SDL_GetPixelFormatDetails(formatEnum);
    if (!details)
      return 0;
    return SDL_MapRGBA(details, info.palette, r, g, b, a);
  }
  
  void PixelFormat::GetRGB(Color pixel, Uint8 &r, Uint8 &g, Uint8 &b) const {
    const SDL_PixelFormatDetails* details = SDL_GetPixelFormatDetails(formatEnum);
    if (!details) {
      r = g = b = 0;
      return;
    }
    SDL_GetRGB(pixel, details, info.palette, &r, &g, &b);
  }

  void PixelFormat::GetRGBA(Color pixel, Uint8 &r, Uint8 &g, Uint8 &b, Uint8 &a) const {
    const SDL_PixelFormatDetails* details = SDL_GetPixelFormatDetails(formatEnum);
    if (!details) {
      r = g = b = 0;
      a = 255;
      return;
    }
    SDL_GetRGBA(pixel, details, info.palette, &r, &g, &b, &a);
  }
}
