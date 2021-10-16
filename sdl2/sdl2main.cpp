#include "../common/common_internal.hpp"
#include <chrono>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#else
#include <thread>
#endif

#ifndef PIXEL_SIZE
#define PIXEL_SIZE 1
#endif

template<Button button> bool buttonState;
SDL_AudioDeviceID audioDevice;
static s32 scanlineY;
static SDL_Window *window;
static SDL_Renderer *renderer;
static SDL_Surface *screen, *vscreen;
static u16 *frameBuffer;
static void *nextHook;

u32 getTimeMicro() {
    auto timePoint = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(timePoint.time_since_epoch()).count();
}

inline bool menuButton(){
    return buttonState<Button::D>;
}

bool isPressed(Button button){
    switch(button){
    case Button::A: return buttonState<Button::A>;
    case Button::B: return buttonState<Button::B>;
    case Button::C: return buttonState<Button::C>;
    case Button::Up: return buttonState<Button::Up>;
    case Button::Down: return buttonState<Button::Down>;
    case Button::Left: return buttonState<Button::Left>;
    case Button::Right: return buttonState<Button::Right>;
    default: break;
    }
    return false;
}

static f32 backlight;

static void redraw() {
    SDL_UnlockSurface( vscreen );
    SDL_FillRect(screen, nullptr, 0);
    SDL_SetSurfaceAlphaMod(vscreen, f32ToS24q8(backlight));
    SDL_BlitScaled( vscreen, nullptr, screen, nullptr );
    SDL_LockSurface(vscreen);
    SDL_UpdateWindowSurface(window);
}

void setBacklight(f32 value) {
    value = (value < 0) ? f32(0) : value;
    value = (value >= 1) ? s24q8ToF32(255) : value;
    backlight = value;
}

u32 getFreeRAM() {
#ifdef __EMSCRIPTEN__
    u32 ram = 32 * 1024 - allocatedSize;
#else
    u64 ram = SDL_GetSystemRAM();
    ram *= 1024;
    ram *= 1024;
    if (ram >> 32) return ~u32{};
#endif
    return ram;
}

u32 getTime() {
    auto timePoint = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(timePoint.time_since_epoch()).count();
    // return ::time(NULL);
}

static void* updateEvents(bool isFrame) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if( e.type == SDL_WINDOWEVENT ){
            if(e.window.event == SDL_WINDOWEVENT_RESIZED){
                screen = SDL_GetWindowSurface( window );
            }
            continue;
        }
#ifndef __EMSCRIPTEN__
	if( e.type == SDL_QUIT )
            exit(0);
#endif

	if( e.type == SDL_KEYUP || e.type == SDL_KEYDOWN ){
            bool isDown = e.type == SDL_KEYDOWN;
	    switch( e.key.keysym.sym ){
            case SDLK_LCTRL:
            case SDLK_a:
            case SDLK_z:
                buttonState<Button::A> = isDown;
                break;

            case SDLK_LALT:
            case SDLK_b:
            case SDLK_s:
            case SDLK_x:
                buttonState<Button::B> = isDown;
                break;

            case SDLK_c:
            case SDLK_d:
                buttonState<Button::C> = isDown;
                break;
#ifdef VERTICAL_SCREEN
            case SDLK_i:
            case SDLK_UP:
                buttonState<Button::Left> = isDown;
                break;

            case SDLK_k:
            case SDLK_DOWN:
                buttonState<Button::Right> = isDown;
                break;

            case SDLK_j:
            case SDLK_LEFT:
                buttonState<Button::Down> = isDown;
                break;

            case SDLK_l:
            case SDLK_RIGHT:
                buttonState<Button::Up> = isDown;
                break;
#else
            case SDLK_i:
            case SDLK_UP:
                buttonState<Button::Up> = isDown;
                break;

            case SDLK_k:
            case SDLK_DOWN:
                buttonState<Button::Down> = isDown;
                break;

            case SDLK_j:
            case SDLK_LEFT:
                buttonState<Button::Left> = isDown;
                break;

            case SDLK_l:
            case SDLK_RIGHT:
                buttonState<Button::Right> = isDown;
                break;
#endif

#ifndef __EMSCRIPTEN__
            case SDLK_ESCAPE:
                exit(0);
                break;
#endif
            }
        }
    }
    return nextHook;
}

extern "C" void flushLine16(u16 *line) {
#ifdef VERTICAL_SCREEN
    auto frameLine = frameBuffer + (screenHeight - scanlineY - 1);
    for(u32 x=0, x2 = 0; x<screenWidth; ++x, x2 += (vscreen->pitch >> 1)){
        frameLine[x2] = line[x];
    }
#else
    auto frameLine = frameBuffer + scanlineY * (vscreen->pitch >> 1);
    for(u32 x=0; x<screenWidth; ++x){
        frameLine[x] = line[x];
    }
#endif

    if (u32(++scanlineY) >= screenHeight) {
        scanlineY = 0;
        redraw();
    }
}

void delay(u32 milli) {
    #ifndef __EMSCRIPTEN__
    u32 micro = milli * 1000;
    u32 start = getTimeMicro();
    while((getTimeMicro() - start) < micro) {
        Schedule::runUpdateHooks(false, getTime());
        SDL_Delay(15);
    }
    #endif
    redraw();
}

int main(){
    if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
        LOG("Error initializing SDL: ", SDL_GetError(), "\n");
        return -1;
    }


#ifdef VERTICAL_SCREEN
    u32 windowHeight = screenWidth;
    u32 windowWidth = screenHeight;
#else
    u32 windowWidth = screenWidth;
    u32 windowHeight = screenHeight;
#endif


    window = SDL_CreateWindow(
        PROJECT_NAME,
        0, 0, windowWidth * PIXEL_SIZE, windowHeight * PIXEL_SIZE,
        SDL_WINDOW_SHOWN|SDL_WINDOW_ALLOW_HIGHDPI);

    if (!window) {
        LOG("Error creating window: ", SDL_GetError(), "\n");
        return -2;
    }

    renderer = SDL_CreateRenderer(window, -1, /**/ SDL_RENDERER_PRESENTVSYNC | /**/ SDL_RENDERER_SOFTWARE);

    screen = SDL_GetWindowSurface( window );
    vscreen = SDL_CreateRGBSurface(
	0, // flags
	windowWidth, // w
	windowHeight, // h
	screenBPP, // depth
	0,
	0,
	0,
	0
	);

    if (!screen) {
        LOG("No window surface\n");
        return -3;
    }

    if (!vscreen) {
        LOG("No virtual surface\n");
        return -4;
    }

    SDL_SetSurfaceBlendMode(vscreen, SDL_BLENDMODE_BLEND);

    SDL_LockSurface(vscreen);
    frameBuffer = decl_cast(frameBuffer, vscreen->pixels);
    if (!frameBuffer) {
        LOG("No framebuffer\n");
        return -5;
    }

    nextHook = Schedule::installUpdateHook(updateEvents);

    updateHandler = update;
    init();
    showLogo();
    #ifdef __EMSCRIPTEN__
    emscripten_set_main_loop_arg(+[](void*){updateLoop();}, nullptr, -1, 1 );
    #else
    using namespace std::chrono_literals;
    while(true){
        if (auto timeLeft = std::chrono::microseconds(updateLoop()) - 1ms; timeLeft > std::chrono::microseconds(maxFrameTime / 4)) {
            std::this_thread::sleep_for(timeLeft);
        }
    }
    #endif

    return 0;
}
