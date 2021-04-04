#pragma once

#include "Femto"
#include <type_traits>

template<u32 _bpp>
class BitmapFrame {
public:
    static constexpr u32 bitsPerPixel = _bpp;
    const u8 * const data = nullptr;
    const u32 w = 0;
    const u32 h = 0;
    const u32 frame = 0;

    const u8 *ptr() const {
        return data + 2 + (w / (8 / _bpp)) * (h / (8 / _bpp)) * frame;
    }

    u32 width() const { return w; }

    u32 height() const { return h; }

    BitmapFrame() = delete;

    BitmapFrame(const BitmapFrame<_bpp> &other) :
        data(other.data),
        w(other.w),
        h(other.h),
        frame(other.frame){}

    BitmapFrame(BitmapFrame<_bpp>&& other) :
        data(other.data),
        w(other.w),
        h(other.h),
        frame(other.frame){}

    BitmapFrame(const u8 *data, u32 w, u32 h, u32 frame = 0) :
        data(data),
        w(w),
        h(h),
        frame(frame){}

    BitmapFrame(const u8 *data, u32 frame = 0) :
        data(data),
        w(data[0]),
        h(data[1]),
        frame(frame){}

    constexpr BitmapFrame(const BitmapFrame<_bpp> &other, u32 frame) :
        data(other.data),
        w(other.w),
        h(other.h),
        frame(frame) {}

    template<typename Bitmap>
    constexpr BitmapFrame(const Bitmap &bitmap, u32 frame = 0) :
        BitmapFrame(bitmap.ptr(), bitmap.width(), bitmap.height(), frame) {
    }
};

template<u32 _bpp, std::size_t count>
class Bitmap : public std::array<umin<(1 << _bpp) -1>, count> {
public:
    using type = umin<(1 << _bpp) -1>;

    constexpr static u32 bitsPerPixel = _bpp;
    constexpr static u32 pixelsPerByte = 8 / bitsPerPixel;
    constexpr static u32 mask = (bitsPerPixel << 1) - 1;

    constexpr Bitmap(const std::array<type, count>& in) :
        Bitmap::array(in) {}

    constexpr operator BitmapFrame<_bpp> () const {
        return BitmapFrame<_bpp>{*this, 0};
    }

    constexpr BitmapFrame<_bpp> operator [] (u32 index) const {
        return BitmapFrame<_bpp>{ *this, index };
    }

    constexpr operator Function<u32(u32 x, u32 y)> () const {
        return this;
    }

    constexpr u32 width() const {
        return ptr()[0];
    }

    constexpr u32 height() const {
        return ptr()[1];
    }

    constexpr u32 operator () (u32 x, u32 y) const {
        if (x >= width() || y >= height())
            return 0;
        u32 c = (*this)[2 + (x / pixelsPerByte) + y * (width() / pixelsPerByte)];
        u32 shift = ((pixelsPerByte - 1) - (x % pixelsPerByte)) * bitsPerPixel;
        return (c >> shift) & mask;
    }

    const type* ptr() const {
        return Bitmap::array::data();
    }

    void set(u32 index, u32 value) {
        u32 old = Bitmap::array::operator[](index >> 1);
        if (index & 1) {
            old &= 0x0F;
            value <<= 4;
        } else {
            old &= 0xF0;
        }
        Bitmap::array::operator[](index >> 1) = old | value;
    }
};

template <u32 BPP, typename ... Arg>
inline constexpr Bitmap<BPP, 2 + sizeof...(Arg)/(sizeof(umin<(1 << BPP) - 1>) * 8 / BPP)> make_Bitmap(u32 width, u32 height, Arg ... arg) {
    constexpr u32 mask = (1 << BPP) - 1;
    constexpr u32 ratio = sizeof(umin<(1 << BPP) - 1>) * 8 / BPP;
    constexpr u32 capacity = sizeof(umin<mask>) * 8;

    std::array<int, sizeof...(Arg)> in = {static_cast<int>(arg)...};
    std::array<umin<mask>, 2 + sizeof...(Arg)/ratio> out = {};

    u32 outPos = 0;
    u32 i = 0;
    u32 acc = 0;

    out[outPos++] = width;
    out[outPos++] = height;

    for(auto v : in){
        i+=BPP;
        acc |= v << (capacity - i);
        if (i >= capacity) {
            out[outPos++] = acc;
            acc = 0;
            i = 0;
        }
    }

    return out;
}

template <typename ... Arg>
inline constexpr std::array<u8, sizeof...(Arg)> toU8(Arg ... arg) {
    return std::array<u8, sizeof...(Arg)>{ static_cast<u8>(arg)... };
};

template<u32 bpp, std::size_t size>
BitmapFrame(const Bitmap<bpp, size> &) -> BitmapFrame<bpp>;
