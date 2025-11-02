/*
 * SDmm - a C++ wrapper for SDL and related libraries
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


#ifndef SDLMM_PIXELFORMAT_H
#define SDLMM_PIXELFORMAT_H

#include "sdlmm_color.h"

namespace SDLmm {
  //! 
  /*!

    \author Adam Gates
  */

  class  PixelFormat {
  protected:
    SDL_CompatPixelFormat info;
    SDL_PixelFormat formatEnum;
    SDL_Surface *surfaceRef;

    void init(SDL_PixelFormat fmt, SDL_Surface *surface);

  public:
    PixelFormat();
    explicit PixelFormat(SDL_PixelFormat fmt);
    explicit PixelFormat(SDL_Surface *surface);
    explicit PixelFormat(const SDL_Surface *surface);
    explicit PixelFormat(const SDL_CompatPixelFormat &formatInfo);
    
    //! \name Informational methods
    //@{
    bool valid() const { return info.BitsPerPixel != 0 || info.BytesPerPixel != 0; }

    //@}
    Uint8 BitsPerPixel() const { return info.BitsPerPixel; }
    Uint8 BytesPerPixel() const { return info.BytesPerPixel; }
    Uint8 Rshift() const { return info.Rshift; }
    Uint8 Gshift() const { return info.Gshift; }
    Uint8 Bshift() const { return info.Bshift; }
    Uint8 Ashift() const { return info.Ashift; }

    Uint8 Rloss() const { return info.Rloss; }
    Uint8 Gloss() const { return info.Gloss; }
    Uint8 Bloss() const { return info.Bloss; }
    Uint8 Aloss() const { return info.Aloss; }

    Uint32 Rmask() const { return info.Rmask; }
    Uint32 Gmask() const { return info.Gmask; }
    Uint32 Bmask() const { return info.Bmask; }
    Uint32 Amask() const { return info.Amask; }

    Color colorkey() const { return info.colorkey; }
    Uint8 alpha() const { return info.alpha; }
    SDL_Palette *palette() const { return info.palette; }

    //! Map a RGB color value to a pixel format.
    /*!
      Maps the RGB color value to this PixelFormat's pixel format and
      returns the pixel value as a 32-bit int.

      If the format has a palette (8-bit) the index of the closest
      matching color in the palette will be returned.

      If the specified pixel format has an alpha component it will be
      returned as all 1 bits (fully opaque).

      \return A pixel value best approximating the given RGB color
      value for a given pixel format. If the pixel format bpp (color
      depth) is less than 32-bpp then the unused upper bits of the
      return value can safely be ignored (e.g., with a 16-bpp format
      the return value can be assigned to a Uint16, and similarly a
      Uint8 for an 8-bpp format).

      \param r, g, b the RGB values.
      \sa GetRGB, GetRGBA, MapRGBA
    */
    Color MapRGB(Uint8 r, Uint8 g, Uint8 b) const;
    
    Color MapRGB(const ColorRGB& colorrgb) const { return MapRGB(colorrgb.r, colorrgb.g, colorrgb.b); }

    Color MapRGB(const SDL_Color& colorrgb) const { return MapRGB(colorrgb.r, colorrgb.g, colorrgb.b); }
    
    //! Map a RGBA color value to a pixel format.
    /*!
      Maps the RGBA color value to this PixelFormat's pixel format and
      returns the pixel value as a 32-bit int.

      If the format has a palette (8-bit) the index of the closest
      matching color in the palette will be returned.

      If the specified pixel format has no alpha component the alpha
      value will be ignored (as it will be in formats with a palette).

      \return A pixel value best approximating the given RGBA color
      value for a given pixel format. If the pixel format bpp (color
      depth) is less than 32-bpp then the unused upper bits of the
      return value can safely be ignored (e.g., with a 16-bpp format
      the return value can be assigned to a Uint16, and similarly a
      Uint8 for an 8-bpp format).

      \param r, g, b, a the RGBA values.
      \sa GetRGB, GetRGBA, MapRGB
    */
    Color MapRGBA(Uint8 r, Uint8 g, Uint8 b, Uint8 a) const;

    Color MapRGBA(const ColorRGBA& colorrgba) const { return MapRGBA(colorrgba.r, colorrgba.g, colorrgba.b, colorrgba.a); }
    
    //! Get RGB values from a pixel in the pixel format of this PixelFormat.
    /*!
      This function uses the entire 8-bit [0..255] range when
      converting color components from pixel formats with less than
      8-bits per RGB component (e.g., a completely white pixel in
      16-bit RGB565 format would return [0xff, 0xff, 0xff] not [0xf8,
      0xfc, 0xf8]).
      
      \param pixel the pixel value
      \param r, g, b references to integers where the RGB result will be stored.
      
      \sa GetRGBA, MapRGB, MapRGBA
    */
    void GetRGB(Color pixel, Uint8 &r, Uint8 &g, Uint8 &b) const;
    
    const ColorRGB GetRGB(Color pixel) const
    {
        ColorRGB        colorrgb;
        GetRGB(pixel, colorrgb.r, colorrgb.g, colorrgb.b);
        return colorrgb;
    }
      
    //! Get RGBA values from a pixel in the pixel format of this PixelFormat.
    /*!
      This function uses the entire 8-bit [0..255] range when
      converting color components from pixel formats with less than
      8-bits per RGB component (e.g., a completely white pixel in
      16-bit RGB565 format would return [0xff, 0xff, 0xff] not [0xf8,
      0xfc, 0xf8]).

      \note If the PixelFormat has no alpha component, the alpha will be
      returned as 0xff (100% opaque).

      \param pixel the pixel value
      \param r, g, b, a references to integers where the RGBA result will be stored.
      \sa GetRGB, MapRGB, MapRGBA */
    void GetRGBA(Color pixel, Uint8 &r, Uint8 &g, Uint8 &b, Uint8 &a) const;
    
    const ColorRGBA GetRGBA(Color pixel) const
    {
        ColorRGBA        colorrgba;
        GetRGBA(pixel, colorrgba.r, colorrgba.g, colorrgba.b, colorrgba.a);
        return colorrgba;
    }
  };
}

#endif // SDLMM_PIXELFORMAT_H
