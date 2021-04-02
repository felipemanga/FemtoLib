#pragma once

#include <cstdint>
#include <type_traits>
#include <array>

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

template<typename Func_t>
class Function {

    template<typename Type> struct helper;

    template<typename Ret, typename ... Args>
    struct helper<Ret(Args...)> {
        using ptr = Ret (*) (uptr, Args...);
    };

    using Func = typename helper<Func_t>::ptr;

    uptr data = 0;
    Func func = nullptr;

public:
    constexpr Function() = default;

    constexpr Function(Function&& other) :
        data(other.data),
        func(other.func) {}

    constexpr Function(const Function &other) :
        data(other.data),
        func(other.func) {}

    constexpr Function(Function* other) :
        data(other->data),
        func(other->func) {}

    constexpr Function(uptr data, Func func) :
        data{data},
        func{func} {}

    constexpr Function(Func_t* unwrapped) :
        data{reinterpret_cast<uptr>(unwrapped)},
        func{[](uptr data, auto ... args){
            return reinterpret_cast<Func_t*>(data)(std::forward<decltype(args)>(args)...);
        }} {}

    constexpr Function& operator = (Function&& other) {
        data = other.data;
        func = other.func;
        return *this;
    }

    constexpr Function& operator = (const Function& other) {
        data = other.data;
        func = other.func;
        return *this;
    }

    template <typename Class>
    constexpr Function(const Class* obj) :
        data{reinterpret_cast<uptr>(obj)},
        func{[](uptr data, auto ... args){
            return (*reinterpret_cast<Class*>(data))(std::forward<decltype(args)>(args)...);
        }} {}

    template <typename Class, std::enable_if_t<!std::is_pointer<Class>::value, int> = 0>
    constexpr Function(const Class& obj) : Function(&obj) {}

    template <typename ... Args>
    auto operator () (Args&& ... args)  const {
        return func(data, std::forward<Args>(args)...);
    }

    operator bool () const {
        return func != nullptr;
    }
};

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

template<std::size_t count>
class Bitmap4BPP : public std::array<u8, count> {
public:
    constexpr Bitmap4BPP(const std::array<u8, count>& in) : Bitmap4BPP::array(in) {}

    constexpr operator const u8* () const {
        return ptr();
    }

    constexpr operator Function<u32(u32 x, u32 y)> () const {
        return this;
    }

    const u32 operator () (u32 x, u32 y) {
        return (*this)[2 + x + y * ptr()[0]];
    }

    constexpr u32 operator [] (u32 index) const {
        if (index < 2)
            return Bitmap4BPP::array::operator[](index);
        return index & 1
            ? Bitmap4BPP::array::operator[](index >> 1) & 0xF
            : Bitmap4BPP::array::operator[](index >> 1) >> 4;
    }

    const u8* ptr() const {
        return Bitmap4BPP::array::data();
    }

    void set(u32 index, u32 value) {
        u32 old = Bitmap4BPP::array::operator[](index >> 1);
        if (index & 1) {
            old &= 0x0F;
            value <<= 4;
        } else {
            old &= 0xF0;
        }
        Bitmap4BPP::array::operator[](index >> 1) = old | value;
    }
};

template <typename ... Arg>
inline constexpr Bitmap4BPP<sizeof...(Arg)/2> toU4(Arg ... arg) {
    std::array<int, sizeof...(Arg)> in = {static_cast<int>(arg)...};
    std::array<u8, sizeof...(Arg)/2> out = {};
    u32 i = 0;
    u32 acc = 0;
    for(auto v : in){
        if (!(i & 1)) {
            acc = v << 4;
        } else {
            acc |= v;
            out[i >> 1] = acc;
        }
        i++;
    }
    return out;
}

template <typename ... Arg>
inline constexpr std::array<u8, sizeof...(Arg)> toU8(Arg ... arg) {
    return std::array<u8, sizeof...(Arg)>{ static_cast<u8>(arg)... };
};

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
