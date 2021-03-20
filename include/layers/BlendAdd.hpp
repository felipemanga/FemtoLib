#pragma once
#include "../Femto.hpp"

namespace Graphics::layer {
    class BlendAdd {
    public:
        LineFiller child;
        BlendAdd(const LineFiller &child) : child(child) {}
        void update(u16 *line, u32 y){
            u16 tmp[screenWidth + 16];
//            MemOps::set(tmp, 0, screenWidth*2);
            auto z = reinterpret_cast<u32*>(tmp) + (screenWidth+16)/2;
            for(s32 x = -((screenWidth + 16)/2); x; ++x)
                z[x] = 0;

            child.update(tmp + 8, y, child.data);
            for(u32 i=0; i<screenWidth; ++i){
                u32 in = tmp[i + 8];
                if (!in) continue;

                u32 out = line[i];
                const u32 RgB = 0b11111'000000'11111;
                out =  (out & RgB)
                    | ((out & ~RgB) << 16);

                in  =  (in & RgB)
                    | ((in & ~RgB) << 16);

                out += in;
                in = out & 0b11111'000000'11111'00000'111111'00000;
                if (in) {
                    out |= (in >> 1)
                        | (in >> 2)
                        | (in >> 3)
                        | (in >> 4)
                        | (in >> 5);
                    out &= 0b111111000001111100000011111;
                }
                out |= out >> 16;
                line[i] = out;
            }
        }
    };
}
