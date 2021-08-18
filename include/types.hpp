#pragma once

#include <cstdint>
#include <type_traits>
#include <array>
#include <algorithm>
#include <string_view>
#include <tuple>


using uptr = std::uintptr_t;
using u64 = std::uint64_t;
using s64 = std::int64_t;
using u32 = std::uint32_t;
using s32 = std::int32_t;
using u16 = std::uint16_t;
using s16 = std::int16_t;
using u8 = std::uint8_t;
using s8 = std::int8_t;
using c8 = char;

template<u32 size = 64> struct _umin { using type = u64; };
template<> struct _umin<4> { using type = u32; };
template<> struct _umin<2> { using type = u16; };
template<> struct _umin<1> { using type = u8; };

template<u64 maxVal>
using umin = typename _umin<(maxVal >> 32) ? 8 :
                                (maxVal >> 16) ? 4 :
                                (maxVal >> 8) ? 2 :
                                1>::type;

template<u32 size = 64> struct _smin { using type = s64; };
template<> struct _smin<4> { using type = s32; };
template<> struct _smin<2> { using type = s16; };
template<> struct _smin<1> { using type = s8; };

template<u64 maxVal>
using smin = typename _smin<(maxVal >> 31) ? 8 :
                                (maxVal >> 15) ? 4 :
                                (maxVal >> 7) ? 2 :
                                1>::type;

#define decl_cast(type, value) static_cast<decltype(type)>(value)

constexpr u32 operator "" _hash(const char *str, std::size_t len){
    u32 v1 = 5381, v2 = 2166136261;
    while(len--){
        u32 c = *str++;
        v1 = (v1 * 251) ^ c;
        v2 = (v2 ^ c) * 16777619;
    }
    return v1 * 13 + v2;
}

class StringInfo {
    const std::string_view str;
    const u32 hash;
    const u32 length;

    static constexpr std::tuple<const std::string_view, u32, u32> strprops(const std::string_view view) {
        u32 v1 = 5381, v2 = 2166136261;
        for(auto c : view){
            v1 = (v1 * 251) ^ c;
            v2 = (v2 ^ c) * 16777619;
        }
        u32 v = v1 * 13 + v2;
        return std::tuple<const std::string_view, u32, u32>{view, v, view.end() - view.begin()};
    }

public:
    constexpr StringInfo(const std::tuple<const std::string_view, u32, u32>& t) :
        str(std::get<0>(t)), hash(std::get<1>(t)), length(std::get<2>(t)) {}

    constexpr StringInfo(const std::string_view view) :
        StringInfo(strprops(view)) {}

    constexpr StringInfo(const char *str) :
        StringInfo(strprops(str)) {}

    operator u32 () const { return hash; }

    operator const std::string_view () const { return str; }

    u32 size() const { return length; }
};

// https://nilsdeppe.com/posts/for-constexpr

template <auto start, auto end, auto inc, class Func>
constexpr void for_constexpr(Func&& f) {
    if constexpr (start < end) {
        f(std::integral_constant<decltype(start), start>());
        for_constexpr<start + inc, end, inc>(f);
    }
}

template <auto start, auto end, class Func>
constexpr void for_constexpr(Func&& f) {
    for_constexpr<start, end, 1>(std::forward(f));
}


// https://stackoverflow.com/questions/35941045/can-i-obtain-c-type-names-in-a-constexpr-way
/*
template <typename T> constexpr std::string_view type_name();

template <>
constexpr std::string_view type_name<void>()
{ return "void"; }

namespace detail {

using type_name_prober = void;

template <typename T>
constexpr std::string_view wrapped_type_name()
{
#ifdef __clang__
    return __PRETTY_FUNCTION__;
#elif defined(__GNUC__)
    return __PRETTY_FUNCTION__;
#elif defined(_MSC_VER)
    return __FUNCSIG__;
#else
#error "Unsupported compiler"
#endif
}

constexpr std::size_t wrapped_type_name_prefix_length() {
    return wrapped_type_name<type_name_prober>().find(type_name<type_name_prober>());
}

constexpr std::size_t wrapped_type_name_suffix_length() {
    return wrapped_type_name<type_name_prober>().length()
        - wrapped_type_name_prefix_length()
        - type_name<type_name_prober>().length();
}

} // namespace detail

template <typename T>
constexpr std::string_view type_name() {
    constexpr auto wrapped_name = detail::wrapped_type_name<T>();
    constexpr auto prefix_length = detail::wrapped_type_name_prefix_length();
    constexpr auto suffix_length = detail::wrapped_type_name_suffix_length();
    constexpr auto type_name_length = wrapped_name.length() - prefix_length - suffix_length;
    return wrapped_name.substr(prefix_length, type_name_length);
}
*/

constexpr inline u32 nextPowerOfTwo(u32 v){
    v |= v >> 16;
    v |= v >> 8;
    v |= v >> 4;
    v |= v >> 2;
    v |= v >> 1;
    return v + 1;
}

constexpr inline bool isPowerOfTwo(u32 v){
    if (!v) return true;
    while (!(v & 1)) v >>= 1;
    return v == 1;
}

constexpr inline u32 countTrailingZeros(u32 v){
    u32 i = 0;
    for (; i<32; ++i){
        if ((v >> i) & 1) {
            break;
        }
    }
    return i;
}

#ifndef NO_FLOAT

#include <math.h>

using f32 = float;

inline constexpr s32 f32ToS24q8(f32 f){
    return f * 256;
}

inline constexpr f32 s24q8ToF32(s32 s){
    return s / 256.0f;
}

inline constexpr f32 PI = 3.1415926535897932384626433832795028841971f;
inline constexpr f32 TAU = 3.1415926535897932384626433832795028841971f * 2.0f;

inline constexpr f32 toRadians(f32 deg){
    return deg * PI / 180.0f;
}

inline constexpr f32 tweenTo(f32 t, f32 v, s32 speed) {
    return t - s24q8ToF32((f32ToS24q8(t) - f32ToS24q8(v)) >> speed);
}

#else

#include "fixed.hpp"

#endif

f32 inline angleDelta(f32 a1, f32 a2) {
    // while (a1 > 2*PI) a1 -= 2*PI;
    // while (a1 < 0) a1 += 2*PI;
    // while (a2 > 2*PI) a2 -= 2*PI;
    // while (a2 < 0) a2 += 2*PI;

    f32 a = a1 - a2;
    f32 b = a2 - a1;

    while (a > 2*PI) a -= TAU;
    while (a < 0) a += TAU;
    while (b > TAU) b -= TAU;
    while (b < 0) b += TAU;

    return a < b ? -a : b;
}

#include "Point2D.h"
#include "Point3D.h"

struct Size2D {
    f32 w, h;
};

struct Size3D : public Size2D {
    f32 d;
};

#ifdef random
#undef random
#endif
#define random femto_random

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
