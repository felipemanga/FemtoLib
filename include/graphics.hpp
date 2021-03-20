#pragma once

namespace Graphics {
    inline const u16 *palette;

    inline u32 primaryColor;
    inline u32 secondaryColor;

    inline u8 linePadding = 1;
    inline u8 charPadding = 1;
    inline bool doubleFontSize;
}

#include "general.hpp"
#include "print.hpp"

#ifdef USE_SCANLINE_RENDERER
#include "ScanlineRenderer.hpp"
#endif

#ifdef USE_SDL_RENDERER
#include "SDLRenderer.hpp"
#endif
