#pragma once

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

inline constexpr f32 sqrt( f32 x ){
    u32 t = 0, q = 0, b = 0x40000000, r = x.getInternal();
    while( b > (uint) 0x40 ){
        t = q;
        t += b;
        if( r >= t )
        {
            r -= t;
            q += b<<1;
        }
        r <<= 1;
        b >>= 1;
    }
    q >>= 12;
    x = f32::fromInternal(q);
    return x;
}

inline f32 atan2(f32 fy, f32 fx) {
    constexpr s32 QUARTER = ((u32)(3.141592654f / 2.0 * (1L << 8)));
    constexpr u32 MAXITER = 9;
    static const u8 arctantab[] = {
        201, 119, 63, 32, 16, 8, 4, 2, 1, 1
    };

    s32 x = fx.getInternal(),
        y = fy.getInternal();

    if ((x == 0) && (y == 0))
        return 0;

// discombobulization
    {
        s32 theta = 0;
        s32 yi = 0;
        s32 tmpx = x, tmpy = y;
        const uint8_t *arctanptr = arctantab;

        /* Get the vector into the right half plane */
        if (tmpx < 0) {
            tmpx = -tmpx;
            tmpy = -tmpy;
            theta = 2 * QUARTER;
        }

        if (tmpy > 0)
            theta = - theta;

        if (tmpy < 0) {    /* Rotate positive */
            yi = tmpy + (tmpx << 1);
            tmpx  = tmpx - (tmpy << 1);
            tmpy  = yi;
            theta -= 283; // *arctanptr++;  /* Subtract angle */
        } else {      /* Rotate negative */
            yi = tmpy - (tmpx << 1);
            tmpx  = tmpx + (tmpy << 1);
            tmpy  = yi;
            theta += 283; // *arctanptr++;  /* Add angle */
        }

        for (u32 i = 0; i <= MAXITER; i++) {
            if (tmpy < 0) {    /* Rotate positive */
                yi = tmpy + (tmpx >> i);
                tmpx  = tmpx - (tmpy >> i);
                tmpy  = yi;
                theta -= *arctanptr++;
            } else {      /* Rotate negative */
                yi = tmpy - (tmpx >> i);
                tmpx  = tmpx + (tmpy >> i);
                tmpy  = yi;
                theta += *arctanptr++;
            }
        }

        x = tmpx;
        y = theta;
    }

    return f32::fromInternal(y);

}
