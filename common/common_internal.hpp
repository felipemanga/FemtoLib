#pragma once

#include "../include/Femto"

extern "C" void SystemInit();

static u8 volume = 1;
static u32 maxFrameTime = 60;
static u32 frameRate = 0;
static u32 allocatedSize = 0;

namespace Graphics::_drawListInternal {
template <u32 bits, bool blend, bool solid>
void pixelCopy(u16 *dest, const u8 *src, u32 count, const u16 *palette, u32 alpha) {
    constexpr auto inv = 8 / bits;
    constexpr auto mask = (bits << 1) - 1;
    while (s32(count-=inv) >= 0) {
        u32 c = *src++;
        for_constexpr<0, inv, 1>([&](u32 i){
     // for (u32 i = 0; i < inv; ++i) { // to-do: profile
            u32 color = (c >> (inv - i - 1) * bits) & mask;
            if (solid || color) {
                if (blend) {
                    color = palette[color];
                    u32 bg = *dest;
                    bg = (bg * 0x00010001) & 0x07e0f81f;
                    color = (color * 0x00010001) & 0x07e0f81f;
                    bg += (color - bg) * alpha >> 5;
                    bg &= 0x07e0f81f;
                    *dest = (bg | bg >> 16);
                } else {
                    *dest = palette[color];
                }
            }
            dest++;
        });
    }
}

WEAK void pixelCopy8BPP(u16 *dest, const u8 *src, u32 count, const u16 *palette) {
    while(count--){
        if(u32 c = *src++)
            *dest = palette[c];
        dest++;
    }
}

WEAK void pixelCopy8BPP2X(u16 *dest, const u8 *src, u32 count, const u16 *palette) {
    while(count--){
        if(u32 c = *src++)
            dest[0] = dest[1] = palette[c];
        dest += 2;
    }
}

WEAK void pixelCopy8BPPA(u16 *dest, const u8 *src, u32 count, const u16 *palette, u32 alpha) {
    while(count--){
        if(u32 color = *src++){
            color = palette[color];
            u32 bg = *dest;
            bg = (bg * 0x00010001) & 0x07e0f81f;
            color = (color * 0x00010001) & 0x07e0f81f;
            bg += (color - bg) * alpha >> 5;
            bg &= 0x07e0f81f;
            *dest = (bg | bg >> 16);
        }
        dest++;
    }
}

WEAK void pixelCopy8BPPA2X(u16 *dest, const u8 *src, u32 count, const u16 *palette, u32 alpha) {
    while(count--){
        if(u32 color = *src++){
            color = palette[color];
            color = (color * 0x00010001) & 0x07e0f81f;
            u32 bg = dest[0];
            bg = (bg * 0x00010001) & 0x07e0f81f;
            bg += (color - bg) * alpha >> 5;
            bg &= 0x07e0f81f;
            dest[0] = (bg | bg >> 16);

            bg = dest[1];
            bg = (bg * 0x00010001) & 0x07e0f81f;
            bg += (color - bg) * alpha >> 5;
            bg &= 0x07e0f81f;
            dest[1] = (bg | bg >> 16);
        }
        dest += 2;
    }
}

WEAK void pixelCopy8BPPS(u16 *dest, const u8 *src, u32 count, const u16 *palette) {
    while(count--){
        *dest++ = palette[*src++];
    }
}

WEAK void pixelCopy8BPPS2X(u16 *dest, const u8 *src, u32 count, const u16 *palette) {
    while(count--){
        dest[0] = dest[1] = palette[*src++];
        dest++;
    }
}

WEAK void pixelCopy8BPPAS(u16 *dest, const u8 *src, u32 count, const u16 *palette, u32 alpha) {
    while(count--){
        u32 color = *src++;
        color = palette[color];
        u32 bg = *dest;
        bg = (bg * 0x00010001) & 0x07e0f81f;
        color = (color * 0x00010001) & 0x07e0f81f;
        bg += (color - bg) * alpha >> 5;
        bg &= 0x07e0f81f;
        *dest++ = (bg | bg >> 16);
    }
}

WEAK void pixelCopy4BPP(u16 *dest, const u8 *src, u32 count, const u16 *palette) {
    pixelCopy<4, false, false>(dest, src, count, palette, 0);
}
WEAK void pixelCopy2BPP(u16 *dest, const u8 *src, u32 count, const u16 *palette) {
    pixelCopy<2, false, false>(dest, src, count, palette, 0);
}

WEAK void pixelCopy4BPPA(u16 *dest, const u8 *src, u32 count, const u16 *palette, u32 alpha) {
    pixelCopy<4, true, false>(dest, src, count, palette, alpha);
}
WEAK void pixelCopy2BPPA(u16 *dest, const u8 *src, u32 count, const u16 *palette, u32 alpha) {
    pixelCopy<2, true, false>(dest, src, count, palette, alpha);
}

WEAK void pixelCopy4BPPS(u16 *dest, const u8 *src, u32 count, const u16 *palette) {
    pixelCopy<4, false, true>(dest, src, count, palette, 0);
}
WEAK void pixelCopy2BPPS(u16 *dest, const u8 *src, u32 count, const u16 *palette) {
    pixelCopy<2, true, false>(dest, src, count, palette, 0);
}

WEAK void pixelCopy4BPPAS(u16 *dest, const u8 *src, u32 count, const u16 *palette, u32 alpha){
    pixelCopy<4, true, true>(dest, src, count, palette, alpha);
}
WEAK void pixelCopy2BPPAS(u16 *dest, const u8 *src, u32 count, const u16 *palette, u32 alpha){
    pixelCopy<2, true, true>(dest, src, count, palette, alpha);
}

}

#ifndef TARGET_ESP8266
WEAK void init() {
    printf("Default Init\n"); fflush(stdout);
    Graphics::palette = Graphics::generalPalette;
    Graphics::init();
    Audio::init();
    setMaxFPS(0);
}

WEAK void update() {}
#else
void init();
void update();
#endif

void (*updateHandler)() = +[](){};
void (*updateDisplay)() = +[](){};

u32 getFPS(){
    return frameRate;
}

void setMaxFPS(u32 fps) {
    maxFrameTime = fps ? round(f32(1'000'000) / f32(fps)) : 0;
    frameRate = fps;
}

void streamI16(File &file, u32 frameWidth, u32 frameHeight, u32 fillColor) {
    u16 line[screenWidth];
    s32 y = screenHeight/2 - frameHeight/2, max = frameHeight;
    s32 x = screenWidth/2 - frameWidth/2;
    if (y < 0) {
        y = 0;
    }

    if (x < 0) {
        frameWidth = screenWidth;
        x = 0;
    }

    if (x || y) {
        for (u32 i = 0; i < screenWidth; ++i) {
            line[i] = fillColor;
        }
        for (s32 i = 0; i < y; ++i) {
            flushLine16(line);
        }
        max = std::min(y + frameHeight, screenHeight);
    }

    for (; y<max && file.read(line + x, frameWidth * 2); ++y) {
        flushLine16(line);
    }

    for (u32 i = 0; i < screenWidth; ++i) {
        line[i] = fillColor;
    }

    for (;u32(y) < screenHeight; ++y) {
        flushLine16(line);
    }
}

#include "logo.hpp"

static u32 prevFrame = 0;
static u32 frameCount = 0;
static u32 refFrameTime = 0;

static u32 updateLoop() {
    u32 now = getTimeMicro();
    u32 frameTime = now - prevFrame;
    bool expired = !maxFrameTime || frameTime >= maxFrameTime;
    Schedule::runUpdateHooks(expired, now);
    if (!expired) return maxFrameTime - frameTime;

    frameCount++;
    if(now - refFrameTime >= 1000000){
        frameRate = frameCount;
        frameCount = 0;
        refFrameTime = now;
    }

    frameTime -= maxFrameTime;
    if (frameTime < maxFrameTime) prevFrame += maxFrameTime;
    else prevFrame = now;

    updateHandler();
    updateDisplay();

    return 0;
}

static void blockingRun(){
    updateHandler = update;
    init();
    showLogo();
    while(true){
        updateLoop();
    }
}
