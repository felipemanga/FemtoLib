#pragma once
#include "../Femto"

namespace Graphics {
    namespace layer {

        template <u32 _tileWidth, u32 _tileHeight, bool _isTransparent, typename TileSource>
        class Tiles {
        public:
            constexpr static bool isTransparent = _isTransparent;
            constexpr static u32 tileWidth = _tileWidth;
            constexpr static u32 tileHeight = _tileHeight;
            constexpr static u32 rowWidth = screenWidth / _tileWidth + 2;

            TileSource source;

            template<typename Other>
            constexpr Tiles(const Other& other) : source(other.source), row{} {}

            template<typename Other>
            constexpr Tiles(Other&& other) : source(std::move(other.source)), row{} {}

            constexpr Tiles(const TileSource& source) :
                source(source), row{} {}

            constexpr Tiles(TileSource&& source) :
                source(std::move(source)), row{} {}

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
                    dx = cx - gx * s32(tileWidth);
                    for(u32 x = 0; x < rowWidth; ++x){
                        row[x] = source.get(gx++, rowNum);
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

        private:
            const u8 *row[rowWidth];
            s32 counter = screenHeight;
            s32 rowNum = 0;
            u32 dx = 0;
        };

        template<
            u32 tileWidth, u32 tileHeight,
            typename IndexType, typename BitmapType
            >
        constexpr auto makeSimpleTilemap(IndexType&& mt, BitmapType&& tt, u32 mapWidth, u32 mapHeight) {
            Data2D map = {
                PageLUT{
                    std::forward<IndexType>(mt),
                    std::forward<BitmapType>(tt),
                    tileWidth*tileHeight
                },
                mapWidth, mapHeight
            };
            return Graphics::layer::Tiles<tileWidth, tileHeight, true, decltype(map)>(std::move(map));
        }
    }
}
