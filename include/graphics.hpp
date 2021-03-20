#pragma once
#ifdef _FEMTO_INTERNAL_

namespace Graphics {
    inline const u16 *palette;
    inline u32 color;
    inline u8 linePadding;
    inline u8 charPadding;
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
#endif
