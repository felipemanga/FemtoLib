#pragma once
#include "../Femto"

namespace Graphics::layer {
    template <u16 color>
    class SolidColor {
    public:
        void bind(){}

        void operator () (u16 *line, u32){
            for(u32 x=0; x<screenWidth; ++x)
                line[x] = color;
        }
    };

    inline Graphics::LineFiller solidColor(u16 color){
        return {
            color,
            +[](uptr color, u16 *line, u32 y){
                 for(u32 x=0; x<screenWidth; ++x)
                     line[x] = color;
             }
        };
    };
}
