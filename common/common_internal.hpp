#pragma once

#include "../include/Femto"

#define WEAK          __attribute__ ((weak))

extern "C" void SystemInit();

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

WEAK void showLogo(){
    File logo;
    if (!logo.openRO("data/logo.i16")){
        LOG("Could not find logo\n");
        return;
    }

    setBacklight(0);

    u32 size = logo.size();
    constexpr u32 frameSize = screenWidth * screenHeight * 2;
    u32 frameCount = size / frameSize;
    logo.seek(random(0, frameCount) * frameSize);
    u16 line[screenWidth];
    for (u32 y=0; y<screenHeight && logo.read(line); ++y) {
        flushLine16(line);
    }

    for (f32 lum = 0; lum < 1; lum += f32(0.02)) {
        setBacklight(lum);
        delay(10);
    }

    logo.openRO("data/logo.raw");
    if (!logo || !Audio::internal::sinkInstance)
        delay(2000);
    else {
        auto& src = Audio::play<0>(logo);
        src.setLoop(false);
        Audio::setVolume(1<<8);
        while (!src.ended() && !isPressed(Button::C)) {
            delay(30);
        }
        Audio::stop<0>();
    }
}

u32 getTimeMicro();

void delay(u32 milli){
    u32 micro = milli * 1000;
    u32 start = getTimeMicro();
    while((getTimeMicro() - start) < micro) {
        Schedule::runUpdateHooks(false, getTime());
    }
}

static void run(){
    updateHandler = update;
    init();
    showLogo();

    auto prevFrame = 0;
    u32 frameCount = 0;
    u32 refFrameTime = 0;

    while(true){
        u32 now = getTimeMicro();
        u32 frameTime = now - prevFrame;
        bool expired = !maxFrameTime || frameTime >= maxFrameTime;
        Schedule::runUpdateHooks(expired, now);
        if (!expired) continue;

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
}

u32 getFPS(){
    return frameRate;
}

void setMaxFPS(u32 fps) {
    maxFrameTime = fps ? round(f32(1'000'000) / f32(fps)) : 0;
    frameRate = fps;
}

