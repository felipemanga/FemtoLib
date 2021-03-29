#pragma once

#ifndef __EMSCRIPTEN__

WEAK void showLogo(){
    File logo;
    if (!logo.openRO("data/logo.i16")){
        LOG("Could not find logo\n");
        setBacklight(1);
        return;
    }

    setBacklight(0);

    u32 size = logo.size();
    constexpr u32 frameSize = screenWidth * screenHeight * 2;
    u32 frameCount = size / frameSize;
    logo.seek(random(0, frameCount) * frameSize);
    streamI16(logo, 220, 176, 0xFFFF);
    for (f32 lum = 0; lum < 1; lum += f32(0.02)) {
        setBacklight(lum);
        delay(10);
    }

    if (Audio::internal::sinkInstance) {
        logo.openRO("data/logo.raw");
        Audio::setVolume(volume << 8);
    }

    if (!logo){
        delay(2000);
    } else {
        auto& src = Audio::play<0>(logo);
        src.setLoop(false);
        while (!src.ended() && !isPressed(Button::C)) {
            delay(30);
        }
        Audio::stop<0>();
    }
}

#else

static void redraw();

WEAK void showLogo(){
    static File logo;
    static auto backup = updateDisplay;

    if (!logo.openRO("data/logo.i16")){
        LOG("Could not find logo\n");
        updateHandler = update;
        setBacklight(1);
        return;
    }

    setBacklight(0);
    updateDisplay = redraw;

    u32 size = logo.size();
    constexpr u32 frameSize = screenWidth * screenHeight * 2;
    u32 frameCount = size / frameSize;
    logo.seek(random(0, frameCount) * frameSize);
    u16 line[screenWidth];
    for (u32 y=0; y<screenHeight && logo.read(line); ++y) {
        flushLine16(line);
    }

    static f32 startTime = getTime();
    static Audio::RAWFileSource *src = nullptr;
    static bool startSound = false;
    if (logo.openRO("data/logo.raw") && Audio::internal::sinkInstance){
        startSound = true;
    }

    updateHandler = +[](){
        static f32 lum;
        if (lum < 1){
            setBacklight(lum);
            lum += f32(0.02);
            return;
        }

        if (startSound) {
            startSound = false;
            src = &Audio::play<0>(logo);
            src->setLoop(false);
            Audio::setVolume(1<<8);
        }

        bool ended = (src && src->ended())
            || (getTime() - startTime > 2000)
            || isPressed(Button::C);

        if (ended) {
            Audio::stop<0>();
            updateHandler = update;
            updateDisplay = backup;
        }
    };
}

#endif
