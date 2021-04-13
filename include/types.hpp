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
        return {view, v, view.end() - view.begin()};
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

// template <typename Type>
// using DataReferenceType = typename std::conditional<
//     std::is_pointer_v<Type>,
//     Type,
//     typename std::conditional<
//         std::is_array_v<Type>,
//         std::remove_all_extents_t<Type>*,
//         Type&
//         >::type
//     >::type;

// template <typename DataType, typename LUTType>
// class LUT {
// public:
//     DataType data;
//     LUTType lut;

//     template <typename _DataType, typename _LUTType>
//     constexpr LUT(const _DataType &data, const _LUTType &lut) :
//         data(data),
//         lut(lut) {}

//     template <typename _DataType, typename _LUTType>
//     constexpr LUT(_DataType &&data, _LUTType &&lut) :
//         data(std::move(data)),
//         lut(std::move(lut)) {}

//     constexpr auto operator [] (u32 index) const {
//         return lut[data[index]];
//     }
// };

// template <typename DataType, typename LUTType>
// LUT(const DataType&, const LUTType&) -> LUT<
//     DataReferenceType<const DataType>,
//     DataReferenceType<const LUTType>
//     >;

// template <typename DataType, typename LUTType>
// class PageLUT {
// public:
//     DataType data;
//     LUTType lut;
//     u32 pageSize;

//     template <typename _DataType, typename _LUTType>
//     constexpr PageLUT(const _DataType &data, const _LUTType &lut, u32 pageSize) :
//         data(data),
//         lut(lut),
//         pageSize(pageSize) {}

//     template <typename _DataType, typename _LUTType>
//     constexpr PageLUT(_DataType &&data, _LUTType &&lut, u32 pageSize) :
//         data(std::move(data)),
//         lut(std::move(lut)),
//         pageSize(pageSize) {}

//     constexpr auto operator [] (u32 index) const {
//         return lut + data[index] * pageSize;
//     }
// };

// template <typename DataType, typename LUTType>
// PageLUT(const DataType&, const LUTType&, u32) -> PageLUT<
//     DataReferenceType<const DataType>,
//     DataReferenceType<const LUTType>
//     >;

// template <typename Type>
// class Data2D {
// public:
//     Type data;
//     u32 width;
//     u32 height;

//     auto operator [] (u32 index) const {
//         return data[index];
//     }

//     auto get(u32 x, u32 y) const {
//         x %= width; y %= height;
//         return data[y * width + x];
//     }
// };

// template <typename Type>
// Data2D(const Type& data, u32, u32) -> Data2D<
//     DataReferenceType<const Type>
//     >;

// template <typename Type>
// Data2D(Type&& data, u32, u32) -> Data2D<Type>;

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

inline constexpr f32 toRadians(f32 deg){
    return deg * PI / 180.0f;
}

#else

#define FIXED_POINTS_USE_NAMESPACE
#define FIXED_POINTS_NO_RANDOM
#include <FixedPoints/FixedPoints.h>
using f32 = FixedPoints::SFixed<23, 8>;

inline constexpr f32 PI = FixedPoints::Pi<f32>;

inline constexpr s32 f32ToS24q8(f32 f){
    return f.getInternal();
}

inline constexpr f32 s24q8ToF32(s32 s){
    return f32::fromInternal(s);
}

inline constexpr f32 sin(f32 rad) {
    using trig = Trig<(PI/2).getInternal(), 255>;
    return f32::fromInternal(trig::sin(rad.getInternal()));
}

inline constexpr f32 cos(f32 rad) {
    using trig = Trig<(PI/2).getInternal(), 255>;
    return f32::fromInternal(trig::cos(rad.getInternal()));
}

inline constexpr f32 toRadians(f32 deg){
    constexpr auto iPI = (PI.getInternal() << 8) / 180;
    return f32::fromInternal(deg.getInternal() * iPI >> 16);
}

inline constexpr s32 round(f32 v){
    return (v.getInternal() + (1 << 7)) >> 8;
}

inline constexpr f32 floor(f32 v){
    return f32::fromInternal(static_cast<s32>(v.getInternal()) >> 8 << 8);
}

inline constexpr s32 ceil(f32 v){
    s32 i = v.getInternal();
    if(i & 0xFF) i += 1 << 8;
    return i >> 8;
}

namespace std {
    inline constexpr f32 abs(f32 v) {
        return v < 0 ? -v : v;
    }
}

#endif

struct Point2D {
    f32 x, y;
    void set(f32 x, f32 y){
        this->x = x;
        this->y = y;
    }

    void rotateXY(f32 rad){
        f32 cr  = cos(rad);
        f32 sr  = sin(rad);
        f32 x   = cr * this->x - sr * this->y;
        this->y = sr * this->x + cr * this->y;
        this->x = x;
    }

    Point2D& operator += (const Point2D &other) {
        x += other.x;
        y += other.y;
        return *this;
    }

    Point2D& operator *= (const Point2D &other) {
        x *= other.x;
        y *= other.y;
        return *this;
    }

    Point2D& operator -= (const Point2D &other) {
        x -= other.x;
        y -= other.y;
        return *this;
    }


    Point2D operator + (const Point2D &other) {
        Point2D c{*this};
        c.x += other.x;
        c.y += other.y;
        return c;
    }

    Point2D operator * (const Point2D &other) {
        Point2D c{*this};
        c.x *= other.x;
        c.y *= other.y;
        return c;
    }

    Point2D operator - (const Point2D &other) {
        Point2D c{*this};
        c.x -= other.x;
        c.y -= other.y;
        return c;
    }

    template <typename IterableCollection, typename Other>
    static void add(IterableCollection &points, const Other& other) {
        for (auto& point : points) {
            point += other;
        }
    }

    template <typename IterableCollection>
    static void rotateXY(IterableCollection &points, f32 rad) {
        f32 cr = cos(rad);
        f32 sr = sin(rad);
        for (auto& point : points) {
            f32 x = cr * point.x - sr * point.y;
            point.y = sr * point.x + cr * point.y;
            point.x = x;
        }
    }
};


struct Point3D : public Point2D {
    f32 z;

    void rotateXZ(f32 rad){
        f32 cr = cos(rad);
        f32 sr = sin(rad);
        f32 x = cr * this->x - sr * z;
        z = cr * z + sr * this->x;
        this->x = x;
    }

    void rotateYZ(f32 rad){
        f32 cr = cos(rad);
        f32 sr = sin(rad);
        f32 y = cr * this->y - sr * z;
        z = cr * z + sr * this->y;
        this->y = y;
    }
};

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
