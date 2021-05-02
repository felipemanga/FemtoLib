#pragma once
#include "../Femto"

namespace Graphics::layer {
    inline LineFiller NOP(){
        return {0, +[](uptr, u32, u16*){}};
    }
}
