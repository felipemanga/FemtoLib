#pragma once

#include "Femto"
#include "fonts/tiny5x7.hpp"

namespace Graphics {
namespace layer {
namespace _monoInternal {
inline u8 fontWidth, fontHeight;
inline void* instance;
inline void (*_clear)() = +[](){};
}

template<const u8* font = fontTiny>
class MonoText {
    static constexpr u32 fontHeight = font[1];
    static constexpr u32 fontWidth = font[0];
    static constexpr u32 invFontHeight = (1 << 24) / fontHeight;
    static constexpr u32 invFontWidth = (1 << 24) / fontWidth;

public:
    static constexpr u32 columns = screenWidth / fontWidth;
    static constexpr u32 rows = screenHeight / fontHeight;
    static constexpr u32 hbytes = ((fontHeight>>3) + ((fontHeight != 8) && (fontHeight != 16)));
    static constexpr bool isTall = hbytes == 2;

    MonoText(){
        _monoInternal::instance = this;
        bindText();
        clear();
    }

    void bindText(){ bind(); }

    void bind(){
        _graphicsInternal::_textXScale = fontWidth;
        _graphicsInternal::_textYScale = fontHeight;
        _monoInternal::_clear =
            +[](){
                 reinterpret_cast<MonoText*>(_monoInternal::instance)->clear();
             };

        _graphicsInternal::_print =
            +[](char c){
                 reinterpret_cast<MonoText*>(_monoInternal::instance)->print(c);
             };
    }

    void clear(){
        textX = textY = 0;
#if defined(TARGET_LPC11U6X)
        __asm__ volatile (
            ".syntax unified" "\n"
            "1: subs %0, 1 \n"
            "strb %2, [%1, %0] \n"
            "bne 1b \n"
            :: "l" (sizeof(buffer)), "l" (buffer), "l" (0)
            : "cc"
            );
#else
        MemOps::set(buffer, 0, sizeof(buffer));
#endif
    }

    void clear(u32 start, u32 end = ~0){
        if(start > rows){
            return;
        }
        if(end >= rows){
            end = rows;
        }
        if(start >= end){
            return;
        }
        auto count = end - start;
        MemOps::set(buffer + start * columns, 0, count * columns);
    }

    void print(char ch){
        std::int32_t c = ch;

        if(c == '\n'){
            newline();
            return;
        }

        if(u32(textX) >= columns || u32(textY) >= rows){
            newline();
        }

        if(font[3] && c >= 'a' && c <= 'z'){
            c = (c - 'a') + 'A';
        }

        c -= font[2];

        if(c < 0) c = 0;

        buffer[textY * columns + textX++] = c;
    }

    void newline(){
        textX = 0;
        textY++;
        if(u32(textY) >= rows){
            textY = rows - 1;
            scrollUp();
        }
    }

    void scrollUp(){
        MemOps::copy(buffer, buffer + columns, (rows - 1) * columns);
        MemOps::set(buffer + (rows - 1) * columns, 0, columns);
    }

    void operator () (u16 *line, u32 y){
        u32 row = (y * invFontHeight + (1 << 20)) >> 24;
        u32 shift = y - row * fontHeight;
        if(row >= rows) return;
        const char *text = buffer + row * columns;
        for(u32 column = 0; column < columns; ++column){
            std::int32_t index = *text++;
            if(index == 0){
                line += fontWidth;
                continue;
            }

            auto bitmap = font + 4 + index * (fontWidth * hbytes + 1);
            u32 gliphWidth = *bitmap++;
            u32 offsetX = fontWidth/2 - gliphWidth/2;
            for(u32 localX = 0; localX < gliphWidth; ++localX, ++line){
                u32 bitcolumn = *bitmap++;
                if(isTall){
                    bitcolumn |= (*bitmap++) << 8;
                }
                if ((bitcolumn >> shift) & 1) {
                    line[offsetX] = primaryColor;
                }
            }
            line += fontWidth - gliphWidth;
        }
    }

    char buffer[columns * rows];
};

}

inline void clearText(){
    layer::_monoInternal::_clear();
}

}
