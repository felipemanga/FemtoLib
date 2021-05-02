#pragma once
#include "Femto"
#include "types.hpp"

namespace Graphics {
    namespace _graphicsInternal {
        inline void (*_print)(c8) = +[](c8){};
        inline u8 _textXScale, _textYScale;
    }

    inline s32 textX, textY;

    inline void setCursor(f32 x, f32 y){
        textX = round(x / f32(_graphicsInternal::_textXScale));
        textY = round(y / f32(_graphicsInternal::_textYScale));
    }

    inline void setCursor(Point2D point){
        point -= camera;
        setCursor(point.x, point.y);
    }

    inline void print( char c ){
        _graphicsInternal::_print(c);
    }

    inline void printD( char c ){
#ifndef RELEASE
        print(c);
#endif
    }

    inline void print( const char *str ){
        if (!str) return;
        for(u32 i=0; str[i]; ++i){
            _graphicsInternal::_print(str[i]);
        }
    }

    inline void printD( const char *str ){
#ifndef RELEASE
        print(str);
#endif
    }

    inline void print( char *str ){
        print(static_cast<const char*>(str));
    }

    inline void printD( char *str ){
#ifndef RELEASE
        print(str);
#endif
    }

    inline void printNUM(u32 number){
        if(number == 0){
            print('0');
            return;
        }
        char tmp[15];
        char *cursor = tmp + sizeof(tmp);
        *--cursor = 0;
        while(number && cursor > tmp){
            *--cursor = '0' + (number % 10);
            number /= 10;
        }
        print(cursor);
    }

    inline void printNUMD(unsigned int n){
#ifndef RELEASE
        printNUM(n);
#endif
    }

    template <typename Integer,
              typename std::enable_if<std::is_integral<Integer>::value, int>::type = 0
              >
    inline void print(Integer number){
        if (std::is_signed<Integer>::value && number < 0){
            print('-');
            number = -number;
        }
        printNUM(static_cast<unsigned int>(number));
    }

    template <typename Integer,
              typename std::enable_if<std::is_integral<Integer>::value, int>::type = 0
              >
    inline void printD(Integer number){
#ifndef RELEASE
        print(number);
#endif
    }

    inline void print(f32 number){
        auto whole = s32(number);
        print(whole);
        print('.');
        number -= whole;
        auto fraction = f32ToS24q8(number);
        if (!fraction) {
            print('0');
            return;
        }
        for (u32 i=0; i < 4 && fraction; ++i) {
            fraction *= 10;
            whole = floor(s24q8ToF32(fraction));
            fraction -= whole;
            print(whole);
        }
    }

    template <typename Pointer,
              typename std::enable_if<std::is_pointer<Pointer>::value, int>::type = 0
              >
    inline void print(Pointer ptr){
        auto v = reinterpret_cast<std::uintptr_t>(ptr);
        const char digits[] = "0123456789ABCDEF";
        for(int i=0; i<8; ++i){
            print(digits[v >> (32 - 4)]);
            v <<= 4;
        }
    }

    inline void print( bool b ){
        print(b ? "true" : "false");
    }

    inline void printD( bool b ){
#ifndef RELEASE
        print(b);
#endif
    }

    template <typename Enum,
              typename std::enable_if<std::is_enum<Enum>::value, int>::type = 0
              >
    inline void print(Enum e){
        print(static_cast<unsigned int>(e));
    }

    template <typename Enum,
              typename std::enable_if<std::is_enum<Enum>::value, int>::type = 0
              >
    inline void printD(Enum e){
#ifndef RELEASE
        print(e);
#endif
    }

    template <
        typename Arg,
        typename ... Args
        >
    void print(const Arg& arg, const Args& ... args){
        print(arg);
        if constexpr (sizeof...(args) != 0) {
            print(args...);
        }
    }

    template <
        typename Arg,
        typename ... Args,
        typename std::enable_if<sizeof...(Args), int>::type = 0
        >
    void printD(const Arg& arg, const Args& ... args){
#ifndef RELEASE
        print(arg);
        print(args...);
#endif
    }
};
