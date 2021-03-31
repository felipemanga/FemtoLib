#pragma once

#include "../include/Femto"

#define WEAK          __attribute__ ((weak))

extern "C" void SystemInit();

static u8 volume = 1;
static u32 maxFrameTime = 60;
static u32 frameRate = 0;
static u32 allocatedSize = 0;

namespace Graphics::_drawListInternal {
WEAK void pixelCopy8BPP(u16* dest, const u8* src, u32 count, const u16* palette) {
    while(count--){
        if(u32 c = *src++)
            *dest = palette[c];
        dest++;
    }
}

WEAK void pixelCopy8BPPA(u16* dest, const u8* src, u32 count, const u16* palette, u32 alpha) {
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

WEAK void pixelCopy8BPPS(u16* dest, const u8* src, u32 count, const u16* palette) {
    while(count--){
        *dest++ = palette[*src++];
    }
}

WEAK void pixelCopy8BPPAS(u16* dest, const u8* src, u32 count, const u16* palette, u32 alpha) {
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
}

WEAK void init() {
    Graphics::init(colorFromRGB(0x1155AA));
    Audio::init();
    setMaxFPS(0);
}

WEAK void update() {}

void (*updateHandler)() = +[](){};
void (*updateDisplay)() = +[](){};

u32 getFPS(){
    return frameRate;
}

void setMaxFPS(u32 fps) {
    maxFrameTime = fps ? round(f32(1'000'000) / f32(fps)) : 0;
    frameRate = fps;
}

void streamI16(File &file, u32 frameWidth = screenWidth, u32 frameHeight = screenHeight, u32 fillColor = 0) {
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

static void updateLoop() {
    u32 now = getTimeMicro();
    u32 frameTime = now - prevFrame;
    bool expired = !maxFrameTime || frameTime >= maxFrameTime;
    Schedule::runUpdateHooks(expired, now);
    if (!expired) return;

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
}

static void blockingRun(){
    updateHandler = update;
    init();
    showLogo();
    while(true){
        updateLoop();
    }
}
