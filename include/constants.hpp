#pragma once

#include "Femto"

#if defined(POKITTO_CLASSIC) || defined(POK_SIM)
#define LCD_ST7775
#endif

#if defined(TARGET_LPC11U6X) && defined(LCD_ST7775)
#define USE_SCANLINE_RENDERER
#endif

#if defined(POK_SIM)
#define USE_SDL_RENDERER
#endif

#if defined(LCD_ST7775)
#define SCREEN_WIDTH 220
#define SCREEN_HEIGHT 176
#define SCREEN_565
#endif

#ifdef TARGET_SDL
#include <SDL2/SDL.h>
#endif

inline constexpr u32 screenWidth = SCREEN_WIDTH;
inline constexpr u32 screenHeight = SCREEN_HEIGHT;

#ifdef SCREEN_565
inline constexpr u32 screenBPP = 16;

inline constexpr u32 colorFromRGB(u32 rgb){
    u32 R = (rgb >> (16+3)) & 0x1F;
    u32 G = (rgb >> (8+2)) & 0x3F;
    u32 B = (rgb >> 3) & 0x1F;
    return (R << 11) + (G << 5) + B;
}

inline constexpr u32 colorFromRGB(u8 R, u8 G, u8 B){
    return ((R>>3) << 11) + ((G>>2) << 5) + (B>>3);
}

inline constexpr u32 blendColors(u32 fg, u32 bg, f32 falpha) {
    // Alpha converted from [0..255] to [0..31]
    u32 alpha = falpha.getInternal() + 4;
    alpha >>= 3;
    fg = (fg | fg << 16) & 0x07e0f81f;
    bg = (bg | bg << 16) & 0x07e0f81f;
    bg += (fg - bg) * alpha >> 5;
    bg &= 0x07e0f81f;
    return (bg | bg >> 16);
}
#endif
