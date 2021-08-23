#pragma once

#include "Femto"
#include "types.hpp"
#include <algorithm>
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


    BitmapFrame(u8 *data, u32 w, u32 h, u32 frame = 0) :
        data(data),
        w(w),
        h(h),
        frame(frame){}

    BitmapFrame(u8 *data, u32 frame = 0) :
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

    Bitmap(u32 width, u32 height){
        ptr()[0] = width;
        ptr()[1] = height;
    }

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

    void fill(type value) {
        // auto w = width();
        // auto h = height();
        // Bitmap::array::fill(value);
        // ptr()[0] = w;
        // ptr()[1] = h;
        MemOps::set(ptr() + 2, value, count - 2);
    }

    void drawHLine8(s32 x, s32 y, s32 w, u32 color) {
        u32 bitmapWidth = width();
        if (u32(x + w) >= bitmapWidth) w = bitmapWidth - x - 1;
        if (x < 0){ w += x; x = 0; }
        if (w <= 0 || u32(x) >= bitmapWidth || u32(y) >= height()) return;

        auto pos = ptr() + 2 + x + y * bitmapWidth;
        pos += w;
        w = -w;
#if defined(TARGET_LPC11U6X)
        __asm__ volatile (
            ".syntax unified" "\n"
            "1: adds %0, 1 \n"
            "strb %2, [%1, %0] \n"
            "bne 1b \n"
            : "+l" (w)
            : "l" (pos), "l" (color)
            : "cc"
            );
#else
        while (w++ < 0) {
            pos[w] = color;
        }
#endif
    }

    void drawHLine4(s32 x, s32 y, s32 w, u32 color) {
        u32 bitmapWidth = width();
        if (u32(x + w) > bitmapWidth) w = bitmapWidth - x;
        if (x < 0){ w -= x; x = 0; }
        if (w <= 0 || u32(x) >= bitmapWidth || u32(y) >= height()) return;

        auto pos = ptr() + 2 + (x >> 1) + y * (bitmapWidth >> 1);
        if (x & 1) {
            set(x, y, color);
            w--;
        }

        color = color & 0xF;
        color |= color << 4;

        pos += w >> 1;
        for (s32 x = 0; x < w; x += 2) {
            *pos++ = color;
        }
    }

    void drawHLine(u32 x, u32 y, s32 w, u32 color) {
        if constexpr (_bpp == 8) drawHLine8(x, y, w, color);
        else if constexpr (_bpp == 4) drawHLine4(x, y, w, color);
    }

/* * /
    void fillTriangle( s32 x0, s32 y0,
                       s32 x1, s32 y1,
                       s32 x2, s32 y2,
                       u32 col ){
        s32 a, b, y, last, tmp;

        if constexpr (_bpp == 8) {
            col &= 0xFF;
            col |= col << 8;
            col |= col << 16;
        }

        a = width();
        b = height();
        if( x0 < 0 && x1 < 0 && x2 < 0 ) return;
        if( x0 >= a && x1 > a && x2 > a ) return;
        if( y0 < 0 && y1 < 0 && y2 < 0 ) return;
        if( y0 >= b && y1 > b && y2 > b ) return;

        // Sort coordinates by Y order (y2 >= y1 >= y0)
        if (y0 > y1) {
            tmp = y0; y0 = y1; y1 = tmp;
            tmp = x0; x0 = x1; x1 = tmp;
        }
        if (y1 > y2) {
            tmp = y2; y2 = y1; y1 = tmp;
            tmp = x2; x2 = x1; x1 = tmp;
        }
        if (y0 > y1) {
            tmp = y0; y0 = y1; y1 = tmp;
            tmp = x0; x0 = x1; x1 = tmp;
        }

        if (y0 == y2) { // Handle awkward all-on-same-line case as its own thing
            a = b = x0;
            if (x1 < a) a = x1;
            else if (x1 > b) b = x1;
            if (x2 < a) a = x2;
            else if (x2 > b) b = x2;
            drawHLine(a, y0, b - a + 1, col);
            return;
        }

        s32 dx01 = x1 - x0,
            dx02 = x2 - x0,
            dy02 = (1<<16) / (y2 - y0),
            dx12 = x2 - x1,
            sa = 0,
            sb = 0;

        // For upper part of triangle, find scanline crossings for segments
        // 0-1 and 0-2.  If y1=y2 (flat-bottomed triangle), the scanline y1
        // is included here (and second loop will be skipped, avoiding a /0
        // error there), otherwise scanline y1 is skipped here and handled
        // in the second loop...which also avoids a /0 error here if y0=y1
        // (flat-topped triangle).
        if (y1 == y2) last = y1; // Include y1 scanline
        else last = y1 - 1; // Skip it

        y = y0;

        if( y0 != y1 ){
            int dy01 = (1<<16) / (y1 - y0);

            for (y = y0; y <= last; y++) {
                a = x0 + ((sa * dy01) >> 16);
                b = x0 + ((sb * dy02) >> 16);
                sa += dx01;
                sb += dx02;
                // longhand:
                //    a = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
                //    b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);

                if (a > b){
                    tmp = a;
                    a = b;
                    b = tmp;
                }
                drawHLine(a, y, b - a, col);
            }
        }

        // For lower part of triangle, find scanline crossings for segments
        // 0-2 and 1-2.  This loop is skipped if y1=y2.
        if( y1 != y2 ){
            int dy12 = (1<<16) / (y2 - y1);

            sa = dx12 * (y - y1);
            sb = dx02 * (y - y0);
            for (; y <= y2; y++) {
                a = x1 + ((sa * dy12) >> 16);
                b = x0 + ((sb * dy02) >> 16);
                sa += dx12;
                sb += dx02;

                if (a > b){
                    tmp = a;
                    a = b;
                    b = tmp;
                }
                drawHLine(a, y, b - a, col);
            }
        }
    }
/*/
   // Original from http://www.sunshine2k.de/coding/java/TriangleRasterization/TriangleRasterization.html
    void fillTriangle(s32 x1, s32 y1, s32 x2, s32 y2, s32 x3, s32 y3, u32 color) {
	if (y1 > y2) {
            std::swap(y1, y2);
            std::swap(x1, x2);
        }

	if (y1 > y3) {
            std::swap(y1, y3);
            std::swap(x1, x3);
        }

	if (y2 > y3) {
            std::swap(y2, y3);
            std::swap(x2, x3);
        }

	s32 t1x = x1, t2x = x1, y = y1, minx, maxx, t1xp, t2xp;
	bool changed1 = false;
	bool changed2 = false;
	s32 signx1 = 1, signx2 = 1,
            dx1 = x2 - x1, dy1 = y2 - y1,
            dx2 = x3 - x1, dy2 = y3 - y1;
	s32 e1, e2;

        if (dx1 < 0) {
            dx1 = -dx1;
            signx1 = -1;
        }

        if (dx2 < 0) {
            dx2 = -dx2;
            signx2 = -1;
        }

        changed1 = dy1 > dx1;
	if (changed1)
            std::swap(dx1, dy1);

        changed2 = dy2 > dx2;
	if (changed2)
            std::swap(dy2,dx2);

	e2 = dx2 >> 1;

        // Flat top, just process the second half
        if (y1 != y2) {
            e1 = dx1 >> 1;
            for (s32 i = 0; i < dx1;) {
                t1xp = 0;
                t2xp = 0;

                if (t1x < t2x) {
                    minx = t1x;
                    maxx = t2x;
                } else {
                    minx = t2x;
                    maxx = t1x;
                }

                // process first line until y value is about to change
                while (i < dx1) {
                    i++;
                    e1 += dy1;
                    while (e1 >= dx1) {
                        e1 -= dx1;
                        if (!changed1)
                            goto next1;
                        t1xp=signx1;
                    }
                    if (changed1)
                        break;
                    t1x += signx1;
                }

                // Move line
            next1:

                // process second line until y value is about to change
                while (1) {
                    e2 += dy2;
                    while (e2 >= dx2) {
                        e2 -= dx2;
                        if (!changed2)
                            goto next2;
                        t2xp = signx2;
                    }
                    if (changed2)
                        break;
                    t2x += signx2;
                }

            next2:
                if (minx > t1x)
                    minx = t1x;
                if (minx > t2x)
                    minx = t2x;

                if (maxx < t1x)
                    maxx = t1x;
                if (maxx < t2x)
                    maxx = t2x;

                drawHLine(minx, y, maxx - minx, color);    // Draw line from min to max points found on the y
                // Now increase y
                if (!changed1)
                    t1x += signx1;
                t1x += t1xp;

                if (!changed2)
                    t2x += signx2;
                t2x += t2xp;

                y += 1;
                if (y == y2)
                    break;
            }
        }

	// Second half
	dx1 = x3 - x2;
        if (dx1 < 0) {
            dx1 = -dx1;
            signx1 = -1;
        } else
            signx1 = 1;

	dy1 = y3 - y2;
	t1x = x2;
        changed1 = dy1 > dx1;
	if (changed1)
            std::swap(dy1, dx1);

	e1 = dx1 >> 1;

	for (s32 i = 0; i <= dx1; i++) {
            t1xp = 0;
            t2xp = 0;
            if (t1x < t2x) {
                minx = t1x;
                maxx = t2x;
            } else {
                minx = t2x;
                maxx = t1x;
            }

	    // process first line until y value is about to change
            while (i < dx1) {
    		e1 += dy1;
                if (e1 >= dx1) {
                    e1 -= dx1;
                    if (!changed1)
                        break;
                    t1xp = signx1;
                }
                if (changed1)
                    break;
                t1x += signx1;
                i += i < dx1;
            }

            // process second line until y value is about to change
            while (t2x != x3) {
                e2 += dy2;
                while (e2 >= dx2) {
                    e2 -= dx2;
                    if (!changed2)
                        goto next4;
                    t2xp = signx2;
                }
                if (changed2)
                    break;
                t2x += signx2;
            }
	next4:

            if (minx > t1x)
                minx = t1x;
            if (minx > t2x)
                minx = t2x;

            if (maxx < t1x)
                maxx = t1x;
            if (maxx < t2x)
                maxx = t2x;

            drawHLine(minx, y, maxx - minx, color);    // Draw line from min to max points found on the y

            // Now increase y
            if (!changed1)
                t1x += signx1;

            t1x += t1xp;
            if (!changed2)
                t2x += signx2;
            t2x += t2xp;
            y += 1;
            if (y > y3)
                return;
	}
    }

    type* ptr() {
        return Bitmap::array::data();
    }

    void set(u32 x, u32 y, u32 value) {
        if constexpr (_bpp == 4) set4(x + y * width(), value);
    }

    void set4(u32 index, u32 value) {
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
