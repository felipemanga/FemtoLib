#pragma once
#include "../Femto"

namespace Graphics::layer {
    inline LineFiller NOP(){
        return {0, +[](u16*, u32, uptr){}};
    }
}
