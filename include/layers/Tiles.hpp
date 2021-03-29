#pragma once
#include "../Femto"

namespace Graphics {

    namespace layer {
        template <u32 _tileWidth, u32 _tileHeight, bool _isTransparent = true>
        class Tiles;
    }

    struct TileSource {
        using Callback = const u8 * (*)(u32 x, u32 y, void *data);
        Callback callback;
        void* data;

        TileSource() = default;
        TileSource(const TileSource& other) = default;

        TileSource(Callback cb, void *data = nullptr) : callback(cb), data(data) {}

        template<typename Clazz>
        TileSource(Clazz &provider){
            data = &provider;
            callback = [](u32 x, u32 y, void *data) -> const u8* {
                return reinterpret_cast<Clazz*>(data)->get(x, y);
            };
        }
    };

    namespace layer {

        template <u32 _tileWidth, u32 _tileHeight, bool _isTransparent>
        class Tiles {
        public:
            constexpr static bool isTransparent = _isTransparent;
            constexpr static u32 tileWidth = _tileWidth;
            constexpr static u32 tileHeight = _tileHeight;
            constexpr static u32 rowWidth = screenWidth / _tileWidth + 2;

            const u8 *row[rowWidth];
            TileSource source;
            s32 counter = screenHeight;
            s32 rowNum = 0;
            u32 dx;

            Tiles(const TileSource& source) : source(source) {
                bind();
            }

            void update(u16 *line, s32 y) {
                counter++;
                if (!y || counter >= s32(tileHeight)) {
                    if (!y) {
                        s32 cy = round(camera.y);
                        rowNum = cy / tileHeight;
                        counter = cy - rowNum * tileHeight;
                    }else{
                        rowNum++;
                        counter = 0;
                    }
                    s32 cx = round(camera.x);
                    s32 gx = cx / tileWidth;
                    dx = cx - gx * tileWidth;
                    for(u32 x = 0; x < rowWidth; ++x){
                        row[x] = source.callback(gx++, rowNum, source.data);
                    }
                }

                u32 dx = this->dx;
                auto end = line + screenWidth;
                for (u32 column = 0; column < rowWidth - 2; ++column){
                    if (isTransparent && !row[column]) continue;
                    auto tile = row[column] + counter * tileWidth + dx;
                    for (u32 x = dx; x < tileWidth; ++x, ++line) {
                        u32 color = *tile++;
                        if (!isTransparent || color) {
                            color = palette[color];
                            *line = color;
                        }
                    }
                    dx = 0;
                }

                for (u32 column = rowWidth - 2; column < rowWidth; ++column){
                    if (isTransparent && !row[column]) continue;
                    auto tile = row[column] + counter * tileWidth;
                    for (u32 x = 0; x < tileWidth && line != end; ++x, ++line) {
                        u32 color = *tile++;
                        if (!isTransparent || color) {
                            color = palette[color];
                            *line = color;
                        }
                    }
                }

            }

            void bind(){
                // instance = this;
                // _tileMap = +[](){};
            }
        };
    }
}
