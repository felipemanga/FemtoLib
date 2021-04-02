#pragma once
#include "../Femto"

namespace Graphics {
    namespace layer {

        using TileCopy = Function<void(u16*, u32 x, u32 y, u32 width)>;
        using TileSource = Function<TileCopy(u32, u32)>;

        template <u32 _tileWidth, u32 _tileHeight>
        class Tiles {
        public:
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
                source(source), row{} {}

            void operator () (u16 *line, s32 y) {
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
                        row[x] = source(gx++, rowNum);
                    }
                }

                u32 dx = this->dx;
                auto end = line + screenWidth;
                for (u32 column = 0; column < rowWidth - 2; ++column){
                    if (row[column]) {
                        row[column](line, dx, counter, tileWidth);
                    }
                    line += tileWidth - dx;
                    dx = 0;
                }

                for (u32 column = rowWidth - 2; column < rowWidth && line < end; ++column){
                    if (row[column]) {
                        u16 tmp[tileWidth];
                        for(u32 x = 0; x < tileWidth; ++x)
                            tmp[x] = line[x];
                        row[column](tmp, 0, counter, tileWidth);
                        auto m = std::min<u32>(end - line, tileWidth);
                        for(u32 x = 0; x < m; ++x)
                            line[x] = tmp[x];
                    }
                    line += tileWidth;
                }
            }

        private:
            TileCopy row[rowWidth];
            s32 counter = screenHeight;
            s32 rowNum = 0;
            u32 dx = 0;
        };

        namespace _internal {
            Function<u32(u32, u32)> *getTile;
            Function<TileCopy(u32)> *getBitmap;
        }

        template<
            u32 tileWidth,
            u32 tileHeight
            >
        class Tilemap : public Graphics::layer::Tiles<tileWidth, tileHeight> {

            static constexpr u32 tileSize = tileWidth * tileHeight;

            Function<u32(u32, u32)> getTile;
            Function<TileCopy(u32)> getBitmap;

        public:
            Tilemap() :
                Graphics::layer::Tiles<tileWidth, tileHeight>(TileSource{this})
                { bind(); }

            using Tiles<tileWidth, tileHeight>::operator();

            TileCopy operator () (u32 x, u32 y){
                if (!getBitmap || !getTile)
                    return {};
                return getBitmap(getTile(x, y));
            }

            void bind(){
                _internal::getTile = &getTile;
                _internal::getBitmap = &getBitmap;
            }
        };
    }

    template<typename Map>
    void setTilemap(Map &map){
        *Graphics::layer::_internal::getTile = map;
    }

    void setTileset(const u8 *ts) {
        *Graphics::layer::_internal::getBitmap = {
            reinterpret_cast<uptr>(ts),
            [](uptr uts, u32 tileId) -> layer::TileCopy {
                auto ts = reinterpret_cast<const u8*>(uts);
                auto bmp = ts + tileId * (ts[0] * ts[0]);
                return {
                    reinterpret_cast<uptr>(bmp),
                    [](uptr ubmp, u16 *line, u32 x, u32 y, u32 width){
                        auto bmp = reinterpret_cast<const u8*>(ubmp);
                        bmp += y * width + x;
                        for( s32 i = width - x; i >= 0; --i ){
                            *line++ = palette[*bmp++];
                        }
                    }
                };
            }
        };
    }
}
