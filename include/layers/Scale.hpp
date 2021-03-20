#pragma once
#include "../Femto"

namespace Graphics::layer {
    class Scale {
        f32 factorX, factorY, factorInverseX, factorInverseY;
    public:
        LineFiller child;

        Scale(const LineFiller &child, f32 factor = 1) : child(child) {
            setScale(factor);
        }

        Scale(f32 factorX = 1, f32 factorY = 1) :
            factorX(factorX),
            factorY(factorY),
            factorInverseX(f32(1.0f) / factorX),
            factorInverseY(f32(1.0f) / factorY) {}

        void setScale(f32 factor){
            factorX = factorY = factor;
            factorInverseX = factorInverseY = f32(1.0f) / factor;
        }

        void setScale(f32 x, f32 y){
            factorX = x;
            factorY = y;
            factorInverseX = f32(1.0f) / x;
            factorInverseY = f32(1.0f) / y;
        }

        void update(u16 *line, u32 y){
            y = round(y * factorInverseY);
            if (factorX > 1) {
                child.update(line, y, child.data);
                u32 fx = factorInverseX.getInternal();
                for (s32 x = screenWidth - 1; x >= 0; --x) {
                    line[x] = line[x * fx >> 8];
                }
            } else if (factorX < 1) {
                u16 tmp[screenWidth + 16];
                {
                    u32 fx = factorX.getInternal();
                    for (s32 x = screenWidth - 1; x >= 0; --x) {
                        tmp[x] = line[x * fx >> 8];
                    }
                }

                child.update(tmp, y, child.data);
                u32 m = floor(screenWidth * factorX);
                u32 fx = factorInverseX.getInternal();
                for (u32 x = 0; x < m; ++x) {
                    line[x] = tmp[x * fx >> 8];
                }
            } else {
                child.update(line, y, child.data);
            }
        }
    };
}
