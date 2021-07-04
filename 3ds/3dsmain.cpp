#include "../common/common_internal.hpp"
#include <chrono>
#include <3ds.h>

std::unordered_map<int, SDL_Joystick*> joysticks;
template<Button button> bool buttonState;
SDL_AudioDeviceID audioDevice;
static s32 scanlineY;
static SDL_Surface *screen;
static u16 *frameBuffer;
static void *nextHook;

u32 getTimeMicro() {
    return svcGetSystemTick() * 1000000 / SYSCLOCK_ARM11;
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
    for(int i=0; i < 10; ++i)
        frameBuffer[20 * (screen->pitch >> 1) + 10 + i] = audioDevice;
    SDL_Flip(screen);
}

void setBacklight(f32 value) {
    value = (value < 0) ? 0 : value;
    value = (value > 1) ? 1 : value;
    backlight = value;
}

u32 getFreeRAM() {
    // u64 ram = SDL_GetSystemRAM();
    // ram *= 1024;
    // ram *= 1024;
    // if (ram >> 32) return ~u32{};
    // return ram;
    return 32*1024;
}

u32 getTime() {
    return SDL_GetTicks();
}

static void* updateEvents(bool isFrame) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
	if( e.type == SDL_QUIT ){
            exit(0);
        }
	if( e.type == SDL_JOYBUTTONUP || e.type == SDL_JOYBUTTONDOWN ){
            bool isDown = e.type == SDL_JOYBUTTONDOWN;
	    switch( e.jbutton.button ){
            case 0: exit(0); break; // start
            case 1: buttonState<Button::A> = isDown; break;
            case 2: buttonState<Button::B> = isDown; break;
            case 3: buttonState<Button::C> = isDown; break;
            case 4: buttonState<Button::D> = isDown; break;
            }
        } else if(e.type == SDL_JOYAXISMOTION) {
            const int deadzone = 10000;

            if( e.jaxis.axis == 0)
            {
                if( e.jaxis.value < -deadzone ){
                    buttonState<Button::Left> = true;
                    buttonState<Button::Right> = false;
                }else if( e.jaxis.value > deadzone ) {
                    buttonState<Button::Left> = false;
                    buttonState<Button::Right> = true;
                }else{
                    buttonState<Button::Left> = false;
                    buttonState<Button::Right> = false;
                }
            }

            if( e.jaxis.axis == 1)
            {
                if( e.jaxis.value < -deadzone ){
                    buttonState<Button::Up> = true;
                    buttonState<Button::Down> = false;
                }else if( e.jaxis.value > deadzone ) {
                    buttonState<Button::Up> = false;
                    buttonState<Button::Down> = true;
                }else{
                    buttonState<Button::Up> = false;
                    buttonState<Button::Down> = false;
                }
            }

        }
    }
    return nextHook;
}

extern "C" void flushLine16(u16 *line) {
    if (frameBuffer) {
        auto frameLine = frameBuffer + scanlineY * (screen->pitch >> 1);
        for(u32 x=0; x<screenWidth; ++x){
            frameLine[x] = line[x];
        }
    }

    if (u32(++scanlineY) >= screenHeight) {
        scanlineY = 0;
        redraw();
    }
}

void delay(u32 milli) {
    u32 micro = milli * 1000;
    u32 start = getTimeMicro();
    while((getTimeMicro() - start) < micro) {
        Schedule::runUpdateHooks(false, getTime());
        SDL_Delay(15);
    }
}

int main(){
    if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) != 0) {
        LOG("Error initializing SDL: ", SDL_GetError(), "\n");
        return -1;
    }

    osSetSpeedupEnable(true);
    SDL_JoystickEventState(SDL_ENABLE);
    u32 numJoysticks = SDL_NumJoysticks();
    for( u32 i=0; i<numJoysticks; ++i ){
        joysticks[i] = SDL_JoystickOpen(i);
    }

    screen = SDL_SetVideoMode(screenWidth, screenHeight, 16, SDL_SWSURFACE|/* */SDL_TOPSCR/*/SDL_DUALSCR/**/|SDL_FITHEIGHT); // SDL_HWSURFACE);

    if (!screen) {
        LOG("No window surface\n");
        return -3;
    }

    // SDL_SetSurfaceBlendMode(screen, SDL_BLENDMODE_BLEND);

    frameBuffer = decl_cast(frameBuffer, screen->pixels);
    if (!frameBuffer) {
        LOG("No framebuffer\n");
        return -5;
    }

    nextHook = Schedule::installUpdateHook(updateEvents);

    updateHandler = update;
    init();
    showLogo();
    while(true){
        updateLoop();
    }
    return 0;
}
