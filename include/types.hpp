#pragma once

#include <cstdint>

#define FIXED_POINTS_USE_NAMESPACE
#define FIXED_POINTS_NO_RANDOM

#include <FixedPoints/FixedPoints.h>

using f32 = FixedPoints::SFixed<23, 8>;
using uptr = std::uintptr_t;
using u32 = std::uint32_t;
using s32 = std::int32_t;
using u16 = std::uint16_t;
using s16 = std::int16_t;
using u8 = std::uint8_t;
using s8 = std::int8_t;
using c8 = char;

enum class BitmapFormat {
    Indexed1BPP,
    Indexed2BPP,
    Indexed4BPP,
    Indexed8BPP,
    FullColor
};

inline constexpr s32 round(f32 v){
    return (v.getInternal() + (1 << 7)) >> 8;
}

inline constexpr s32 floor(f32 v){
    return static_cast<s32>(v.getInternal()) >> 8;
}

inline constexpr s32 ceil(f32 v){
    s32 i = v.getInternal();
    if(i & 0xFF) i += 1 << 8;
    return i >> 8;
}

inline u32 random(u32 seed = 0) {
    static u32 _rngState = 0;
    if (seed) _rngState = seed;
    extern u32 getTime();
    if (!_rngState)
        _rngState = 1013904223 + 1664525 * getTime();
    _rngState ^= _rngState << 17;
    _rngState ^= _rngState >> 13;
    _rngState ^= _rngState << 5;
    return _rngState;
}

inline s32 random(s32 min, s32 max) {
    if (min >= max)
        return min;
    return (random() % (max - min)) + min;
}
