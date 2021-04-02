#pragma once
#include "../Femto"

namespace Graphics {

    namespace layer {
        class RotoZoom;
    }

    namespace {
        inline layer::RotoZoom *instance;
    }

    namespace layer {
        class RotoZoom {
        public:
            f32 lx, ly;
            f32 cx, cy;
            f32 ax, ay;

            s32 minX, maxX;
            s32 minY, maxY;

            const u8 *bitmap;
            const u16 *palette;

            u32 alpha;

            RotoZoom() {
                update = &RotoZoom::nullUpdate;
                bind();
            }

            void operator () (u16 *line, s32 y) {
                (this->*update)(line, y);
            }

            void (RotoZoom::*update)(u16 *, s32 y);
            void nullUpdate(u16*, s32) {}

            void rotozoom(const u8 *data, s32 x, s32 y, u8 alpha, f32 rotate, f32 scale, f32 anchorX, f32 anchorY){
                if (scale == 0) data = nullptr;
                bitmap = data;
                if (!bitmap){
                    update = &RotoZoom::nullUpdate;
                    return;
                }

                this->alpha = (u32(alpha) + 4) >> 3;

                update = (alpha < ((0xFF + 4) >> 3))
                    ? &RotoZoom::updateFunc<false>
                    : &RotoZoom::updateFunc<true>;

                f32 iscale = 1 / scale;

                f32 sr = sin(rotate);
                f32 cr = cos(rotate);
                cx = cr * iscale;
                cy = sr * iscale;
                lx = -sr * iscale;
                ly = cr * iscale;

                f32 W = bitmap[0] * scale;
                f32 H = bitmap[1] * scale;

                ax = ay = 0;

                std::array corner = {
                    Point2D{- W * anchorX, - H * anchorY},
                    Point2D{W * (1 - anchorX), - H * anchorY},
                    Point2D{- W * anchorX, H * (1 - anchorY)},
                    Point2D{W * (1 - anchorX), H * (1 - anchorY)}
                };

                // auto copy = corner;

                Point2D::rotateXY(corner, -rotate);
                // for (auto& point : corner)
                //     point.y = -point.y;

                minX = floor(corner[0].x);
                maxX = floor(corner[0].x);
                minY = floor(corner[0].y);
                maxY = floor(corner[0].y);

                for (auto& point : corner) {
                    minX = std::min<s32>(minX, floor(point.x));
                    minY = std::min<s32>(minY, floor(point.y));
                    maxX = std::max<s32>(maxX, floor(point.x));
                    maxY = std::max<s32>(maxY, floor(point.y));
                }

                // s32 clip = minY + y;
                // if (clip < 0){
                //     ax += lx * clip;
                //     ay += ly * clip;
                // }

                s32 dy = floor(minY - corner[0].y);
                if (dy < 0){
                    ay += ly * dy;
                    ax += lx * dy;
                }

                s32 dx = floor(minX - corner[0].x);
                if (dx < 0){
                    ay += cy * dx;
                    ax += cx * dx;
                }

                minX += x;
                minY += y;
                maxX += x;
                maxY += y;

                if (minY < 0){
                    ay -= ly * minY;
                    ax -= lx * minY;
                }
                if (maxX > s32(screenWidth)) {
                    maxX = screenWidth;
                }
                if (maxY <= 0 || minY >= s32(screenHeight)) {
                    update = &RotoZoom::nullUpdate;
                }
            }

            template<bool hasAlpha>
            void updateFunc(u16 *line, s32 y){
                if (y < minY) return;
                if (y + 1 >= maxY) update = &RotoZoom::nullUpdate;

                f32 px = ax, py = ay;
                ax += lx; ay += ly;

                u32 width = bitmap[0];
                u32 height = bitmap[1];
                u32 size = width * height;

                for (auto x = minX; x < maxX; ++x, px += cx, py += cy){
                    u32 tx = s32(floor(px));

                    if (tx >= width)
                        continue;

                    u32 index = tx + s32(floor(py)) * width;
                    if (index < size){
                        if (u32 color = bitmap[2 + index]){
                            color = palette[color];
                            if (hasAlpha) {
                                u32 bg = line[x];
                                bg = (bg * 0x00010001) & 0x07e0f81f;
                                color = (color * 0x00010001) & 0x07e0f81f;
                                bg += (color - bg) * alpha >> 5;
                                bg &= 0x07e0f81f;
                                color = (bg | bg >> 16);
                            }
                            line[x] = color;
                        }
                    }
                }
            }

            void bind(){
                instance = this;
            }
        };
    }

    template<u32 bits>
    inline void draw(const u8 *data, s32 x, s32 y, f32 alpha, f32 rotate, f32 scale = f32(1), f32 anchorX = f32(0.5), f32 anchorY = f32(0.5)){
        u8 a = std::clamp<s32>(f32ToS24q8(alpha), 0, 255);
        if constexpr (bits == 8) {
#ifdef DEBUG
            if (instance) {
#endif
                instance->palette = palette;
                instance->rotozoom(data, x, y, a, rotate, scale, anchorX, anchorY);
#ifdef DEBUG
            } else {
                LOG1("Error: Missing RotoZoom layer\n");
            }
#endif
        }
    }

    template<u32 bits>
    inline void draw(const u8 *data, Point2D pos, f32 alpha, f32 rotate, f32 scale = f32(1), Point2D anchor = {0.5, 0.5}){
        pos -= camera;
        draw<8>(data, round(pos.x), round(pos.y), alpha, rotate, scale, anchor.x, anchor.y);
    }
}
