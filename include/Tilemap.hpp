#pragma once

#include "Femto"

namespace Graphics {
    template <typename MapType, typename TileType>
    class Tilemap {
    public:
        const MapType& map;
        const TileType *tileSet;
        u32 width, height;

        constexpr Tilemap(const MapType& map, const TileType *tileSet, u32 width, u32 height) :
            map(map),
            tileSet(tileSet),
            width(width),
            height(height) {}

        const TileType& get(u32 x, u32 y) {
            x = x % width;
            y = y % height;
            u32 index = map[ x + y * width ];
            return tileSet[index];
        }
    };

    template <typename MapType>
    class Tilemap<MapType, u8> {
    public:
        using TileType = u8;
        const MapType& map;
        const TileType *tileSet;
        u32 width, height;
        u32 tileSize;

        constexpr Tilemap(const MapType& map, const TileType *tileSet, u32 width, u32 height) :
            map(map),
            tileSet(tileSet + 2),
            width(width),
            height(height),
            tileSize(tileSet[0]*tileSet[0]) {}

        const u8* get(u32 x, u32 y) {
            x = x % width;
            y = y % height;
            return tileSet + map[ x + y * width ] * tileSize;
        }
    };
}
