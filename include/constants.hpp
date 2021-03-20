#pragma once

#include "Femto"

#ifndef MAX_FILLERS
#define MAX_FILLERS 4
#endif

inline constexpr u32 maxFillers = MAX_FILLERS;

#if defined(POKITTO_CLASSIC) || defined(POK_SIM)
#define LCD_ST7775
#endif

#if defined(TARGET_LPC11U6X) && defined(LCD_ST7775)
#define USE_SCANLINE_RENDERER
#endif

#if defined(POK_SIM) || defined(__EMSCRIPTEN__)
#define USE_SDL_RENDERER
#endif

#if defined(LCD_ST7775)
inline constexpr u32 screenWidth = 220;
inline constexpr u32 screenHeight = 176;

inline constexpr u32 colorFromRGB(u32 rgb){
    u32 R = (rgb >> (16+3)) & 0x1F;
    u32 G = (rgb >> (8+2)) & 0x3F;
    u32 B = (rgb >> 3) & 0x1F;
    return (R << 11) + (G << 5) + B;
}

inline constexpr u32 colorFromRGB(u8 R, u8 G, u8 B){
    return ((R>>3) << 11) + ((G>>2) << 5) + (B>>3);
}
#endif
