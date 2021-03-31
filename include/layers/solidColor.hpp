#pragma once
#include "../Femto"

namespace Graphics::layer {
    inline Graphics::LineFiller solidColor(u16 color){
        return {
            color,
            +[](u16 *line, u32 y, uptr color){
                 for(u32 x=0; x<screenWidth; ++x)
                     line[x] = color;
             }
        };
    };
}
