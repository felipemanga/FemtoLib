#pragma once
#include "Femto"
#include <algorithm>

namespace Graphics {
    namespace _drawListInternal{
        struct Cmd;
        using draw_t = void (*)(u16 *line, Cmd &cmd, u32 y);
        struct Cmd {
            const void *data;
            draw_t draw;
            umin<screenWidth> x;
            umin<screenHeight> y;
            umin<screenHeight> maxY;
            u8 b1;
            union {
                struct {
                    u8 b2, b3;
                    u16 s;
                };
                uptr udata;
            };
        };
        inline void *instance;
        inline void (*add)(const Cmd& s) = +[](const Cmd&){};
        inline void (*clear)() = +[](){};
        inline const u8 *font;

        inline void _print(c8 index) {
            if (!font) return;

            const u8* bitmap = font;
            u8 w = *bitmap;
            u8 h = *(bitmap + 1);
            u32 scale = (doubleFontSize ? 2 : 1);
            index -= font[2];
            if (u32(textX) > screenWidth) {
                textX = 0;
                textY += h * scale + linePadding;
            }

            uint8_t hbytes = ((h>>3) + ((h != 8) && (h != 16)));

            // add an offset to the pointer (fonts !)
            bitmap = bitmap + 4 + index * (w * hbytes + 1);
            uint32_t numBytes = *bitmap++; //first byte of char is the width in bytes

            if(u32(textY) >= screenHeight || textY + h*scale < 0 || u32(textX) >= screenWidth || textX + w*scale < 0) {
                textX += numBytes * scale + charPadding;
                return;
            }

            draw_t f;
            if( !doubleFontSize ){
                f = [](u16 *line, Cmd &s, u32 y){
                        int h = s.maxY - s.y;
                        auto bitmap = static_cast<const uint8_t *>(s.data);
                        int x = s.x;
                        u32 fg = s.s;
                        int numBytes = s.b1;
                        if( u32(s.b1 + x) > screenWidth )
                            numBytes = screenWidth - x;

                        uint8_t hbytes = ((h>>3) + ((h != 8) && (h != 16))) == 2;
                        for (int i = 0; i < numBytes; i++) {
                            uint32_t bitcolumn = *bitmap++;
                            if (hbytes)
                                bitcolumn |= (*bitmap++)<<8;
                            if (!(bitcolumn & (1<<y)))
                                continue;
                            line[x+i] = fg;
                        }
                    };
            } else {
                f = [](u16 *line, Cmd &s, u32 y){
                        int h = (s.maxY - s.y)>>1;
                        y >>= 1;
                        auto bitmap = static_cast<const uint8_t *>(s.data);
                        int x = s.x;
                        u32 c = s.s;
                        int numBytes = s.b1;
                        if( u32((s.b1<<1) + x) > screenWidth )
                            numBytes = (screenWidth - x) >> 1;

                        uint8_t hbytes = ((h>>3) + ((h != 8) && (h != 16))) == 2;

                        for (int i = 0; i < numBytes; i++) {
                            uint32_t bitcolumn = *bitmap++;
                            if (hbytes)
                                bitcolumn |= (*bitmap++)<<8;
                            if (!(bitcolumn & (1<<y)))
                                continue;
                            line[x+(i<<1)] = c;
                            line[x+(i<<1)+1] = c;
                        }
                    };
                h *= 2;
            }

            add(Cmd{
                    .data = bitmap,
                    .draw = f,
                    .x = decl_cast(Cmd::x, textX),
                    .y = decl_cast(Cmd::y, textY),
                    .maxY = decl_cast(Cmd::maxY, h),
                    .b1 = decl_cast(Cmd::b1, numBytes),
                    .s = decl_cast(Cmd::s, primaryColor)
                });

            textX += numBytes * scale + charPadding;
        }

        inline void blitBitPlane(u16 *, Cmd &s, u32 y){
        }

        template <bool isTransparent>
        inline void blit1BPP(u16 *line, Cmd &s, u32 y){
            auto data = static_cast<const u8*>(s.data);
            u32 color = s.udata & 0xFFFF;
            u32 secondary = s.udata >> 16;
            int w = data[0];
            auto src = data + 2 + y * ((w + (w&0x7?8:0)) >> 3);
            if(s.x < 0){
                src -= s.x >> 3;
                w += s.x;
            }else if(s.x > 0){
                line += s.x;
            }
            if(u32(s.x + w) >= screenWidth){
                w = (screenWidth) - s.x;
            }

            if(w&7) w += 8;
            w >>= 3;
            u32 alpha = s.b1;

            if (alpha != ((0xFF + 4) >> 3)) {
                while (w--) {
                    u32 b = *src++;
                    for (s32 i=7; i>=0; --i, b >>= 1) {
                        if (isTransparent) {
                            if (b&1) {
                                u32 bg = line[i];
                                bg = (bg | bg << 16) & 0x07e0f81f;
                                bg += (color - bg) * alpha >> 5;
                                bg &= 0x07e0f81f;
                                line[i] = (bg | bg >> 16);
                            }
                        } else {
                            u32 c = (b&1) ? color : secondary;
                            u32 bg = line[i];
                            bg = (bg | bg << 16) & 0x07e0f81f;
                            bg += (c - bg) * alpha >> 5;
                            bg &= 0x07e0f81f;
                            line[i] = (bg | bg >> 16);
                        }
                    }
                    line += 8;
                }
            } else {
                while(w--){
                    unsigned int b = *src++;
                    if(b&1) line[7] = color;
                    else if(!isTransparent) line[7] = secondary;
                    b >>= 1;
                    if(b&1) line[6] = color;
                    else if(!isTransparent) line[6] = secondary;
                    b >>= 1;
                    if(b&1) line[5] = color;
                    else if(!isTransparent) line[5] = secondary;
                    b >>= 1;
                    if(b&1) line[4] = color;
                    else if(!isTransparent) line[4] = secondary;
                    b >>= 1;
                    if(b&1) line[3] = color;
                    else if(!isTransparent) line[3] = secondary;
                    b >>= 1;
                    if(b&1) line[2] = color;
                    else if(!isTransparent) line[2] = secondary;
                    b >>= 1;
                    if(b&1) line[1] = color;
                    else if(!isTransparent) line[1] = secondary;
                    b >>= 1;
                    if(b&1) line[0] = color;
                    else if(!isTransparent) line[0] = secondary;

                    line += 8;
                }
            }
// #endif
        }

        template <bool isTransparent>
        inline void blit8BPP(u16 *line, Cmd &s, u32 y){
            extern void pixelCopy8BPPA(u16* dest, const u8* src, u32 count, const u16* palette, u32 alpha);
            extern void pixelCopy8BPPAS(u16* dest, const u8* src, u32 count, const u16* palette, u32 alpha);
            extern void pixelCopy8BPP(u16* dest, const u8* src, u32 count, const u16* palette);
            extern void pixelCopy8BPPS(u16* dest, const u8* src, u32 count, const u16* palette);

            auto data = static_cast<const u8*>(s.data);
            int w = data[0];
            const uint8_t *src = data + 2 + y * w;
            if(s.x < 0){
                src -= s.x;
                w += s.x;
            }else if(s.x > 0){
                line += s.x;
            }
            if(u32(s.x + w) >= screenWidth){
                w = screenWidth - s.x;
            }

            auto palette = reinterpret_cast<const u16 *>(s.udata);
            u32 alpha = s.b1;
            if (alpha != ((0xFF + 4) >> 3)) {
                if (isTransparent)
                    pixelCopy8BPPA(line, src, w, palette, alpha);
                else
                    pixelCopy8BPPAS(line, src, w, palette, alpha);
            } else {
                if (isTransparent)
                    pixelCopy8BPP(line, src, w, palette);
                else
                    pixelCopy8BPPS(line, src, w, palette);
            }
        }
/*


        inline void blitMirror1BPP(uint8_t *line, Cmd &s, int y){
            auto data = static_cast<const uint8_t*>(s.data);
            int w = s.b2;
            const uint8_t *src = data + (y * w >> 3);
            if(s.x < 0){
                src -= s.x >> 3;
                w += s.x;
            }else if(s.x > 0){
                line += s.x;
            }
            if(s.x + w >= screenWidth+8){
                w = (screenWidth+8) - s.x;
            }

            line += w - 8;
            w >>= 3;
#ifndef POK_SIM
            pixelExpand(line, src, w, s.b1, -8);
#else
            auto recolor = s.b1;
            while(w--){
                unsigned int b = *src++;
                if(b&1) line[0] = recolor; b >>= 1;
                if(b&1) line[1] = recolor; b >>= 1;
                if(b&1) line[2] = recolor; b >>= 1;
                if(b&1) line[3] = recolor; b >>= 1;
                if(b&1) line[4] = recolor; b >>= 1;
                if(b&1) line[5] = recolor; b >>= 1;
                if(b&1) line[6] = recolor; b >>= 1;
                if(b&1) line[7] = recolor;
                line -= 8;
            }
#endif
        }

        inline void blitMirror(uint8_t *line, Cmd &s, int y){
            auto data = static_cast<const uint8_t*>(s.data);

            int w = s.b2;
            const uint8_t *src = data + y * w + w - 1;
            if(s.x < 0){
                w += s.x;
                src += s.x;
            }else if(s.x > 0){
                line += s.x;
            }
            if(s.x + w >= screenWidth){
                w = screenWidth - s.x;
            }

            pixelCopyMirror(line, src - w + 1, w, s.b1);

            // while(w--){
            // if(*src)
            // *line = *src;
            // line++;
            // src--;
            // }
        }


        inline void blitFlip1BPP(uint8_t *line, Cmd &s, int y){
            auto data = static_cast<const uint8_t*>(s.data);
            int w = s.b2;
            int h = s.maxY - s.y;

            const uint8_t *src = data + ((h - 1 - y) * w >> 3);
            if(s.x < 0){
                src -= s.x >> 3;
                w += s.x;
            }else if(s.x > 0){
                line += s.x;
            }
            if(s.x + w >= screenWidth+8){
                w = (screenWidth+8) - s.x;
            }

            if(w&7) w += 8;
            w >>= 3;
#ifndef POK_SIM
            pixelExpand(line, src, w, s.b1, 8);
#else
            auto recolor = s.b1;
            while(w--){
                unsigned int b = *src++;
                if(b&1) line[7] = recolor; b >>= 1;
                if(b&1) line[6] = recolor; b >>= 1;
                if(b&1) line[5] = recolor; b >>= 1;
                if(b&1) line[4] = recolor; b >>= 1;
                if(b&1) line[3] = recolor; b >>= 1;
                if(b&1) line[2] = recolor; b >>= 1;
                if(b&1) line[1] = recolor; b >>= 1;
                if(b&1) line[0] = recolor;
                line += 8;
            }
#endif
        }

        inline void blitFlip(uint8_t *line, Cmd &s, int y){
            auto data = static_cast<const uint8_t*>(s.data);

            int w = s.b2;
            int h = s.maxY - s.y;
            const uint8_t *src = data + (h - 1 - y) * w;
            if(s.x < 0){
                src -= s.x;
                w += s.x;
            }else if(s.x > 0){
                line += s.x;
            }
            if(s.x + w >= screenWidth){
                w = screenWidth - s.x;
            }
            pixelCopy(line, src, w, s.b1);

            // while(w--){
            // if(*src)
            // *line = *src;
            // line++;
            // src++;
            // }
        }

        inline void blitFlipMirror1BPP(uint8_t *line, Cmd &s, int y){
            auto data = static_cast<const uint8_t*>(s.data);
            int w = s.b2;
            int h = s.maxY - s.y;
            const uint8_t *src = data + ((h - 1 - y) * w >> 3);
            if(s.x < 0){
                src -= s.x >> 3;
                w += s.x;
            }else if(s.x > 0){
                line += s.x;
            }
            if(s.x + w >= screenWidth+8){
                w = (screenWidth+8) - s.x;
            }

            line += w - 8;
            w >>= 3;
#ifndef POK_SIM
            pixelExpand(line, src, w, s.b1, -8);
#else
            auto recolor = s.b1;
            while(w--){
                unsigned int b = *src++;
                if(b&1) line[0] = recolor; b >>= 1;
                if(b&1) line[1] = recolor; b >>= 1;
                if(b&1) line[2] = recolor; b >>= 1;
                if(b&1) line[3] = recolor; b >>= 1;
                if(b&1) line[4] = recolor; b >>= 1;
                if(b&1) line[5] = recolor; b >>= 1;
                if(b&1) line[6] = recolor; b >>= 1;
                if(b&1) line[7] = recolor;
                line -= 8;
            }
#endif
        }

        inline void blitFlipMirror(uint8_t *line, Cmd &s, int y){
            auto data = static_cast<const uint8_t*>(s.data);

            int w = s.b2;
            int h = s.maxY - s.y;
            const uint8_t *src = data + (h - 1 - y) * w + w - 1;
            if(s.x < 0){
                w += s.x;
                src += s.x;
            }else if(s.x > 0){
                line += s.x;
            }
            if(s.x + w >= screenWidth){
                w = screenWidth - s.x;
            }

            pixelCopyMirror(line, src - w + 1, w, s.b1);

            // while(w--){
            // if(*src)
            // *line = *src;
            // line++;
            // src--;
            // }
        }
*/
    }

    namespace layer {

        template <u32 _capacity>
        class DrawList {
            using Cmd = _drawListInternal::Cmd;
            u32 _size = 0;
            u32 _begin = 0;
            u32 _end = 0;

            void add(const Cmd& s){
                auto& sb = cmdBuffer[_end];
                sb = s;
                s32 maxY = sb.maxY;
                maxY += sb.y;
                if(maxY < 0)
                    return;
                if(u32(maxY) > screenHeight)
                    maxY = screenHeight;
                sb.maxY = maxY;
                _end++;
                if(_end >= capacity) _end = 0;
                _size++;
                if (_size > capacity) {
                    _begin++;
                    _size--;
                    if (_begin >= capacity) {
                        _begin = 0;
                    }
                }
            }

            void clear(){
                _begin = _end = _size = 0;
            }

        protected:
            static constexpr u32 capacity = _capacity;
            Cmd cmdBuffer[capacity];
            u8 cmdsPerLine[screenHeight];

        public:

            DrawList(const u8 *font){
                bind();
                bindText(font);
            }

            DrawList(){
                bind();
            }

            void bind() {
                _drawListInternal::instance = this;

                _drawListInternal::add =
                    +[](const Cmd& s){
                         reinterpret_cast<DrawList*>(_drawListInternal::instance)->add(s);
                     };

                _drawListInternal::clear =
                    +[](){
                         reinterpret_cast<DrawList*>(_drawListInternal::instance)->clear();
                     };
            }

            void bindText(const u8 *font = _drawListInternal::font) {
                _drawListInternal::font = font;
                _graphicsInternal::_textXScale = 1;
                _graphicsInternal::_textYScale = 1;
                _graphicsInternal::_print = _drawListInternal::_print;
            }

            void update(u16 *line, u32 y){
                if(y == 0){
                    for(u32 i=0; i<screenHeight; ++i)
                        cmdsPerLine[i] = 0;

                    if (!_size)
                        return;

                    for(u32 i = _begin, j = 0; j != _size; ++i, ++j){
                        if (i >= capacity) i = 0;
                        auto& s = cmdBuffer[i];
                        for(u32 y=std::max(0, static_cast<int>(s.y)), my=std::min(screenHeight, static_cast<u32>(s.maxY)); y<my; ++y){
                            cmdsPerLine[y]++;
                        }
                    }
                }

                u32 max = cmdsPerLine[y];
                if (!max) return;
                for(u32 i = _begin, j = 0; j != _size; ++i, ++j){
                    if (i >= capacity) i = 0;
                    auto& s = cmdBuffer[i];
                    if( s.y > y ) continue;

                    u32 smaxY = s.maxY;
                    if( smaxY <= y ){
                        continue;
                    }

                    s.draw(line, s, y - s.y);
                    if(!--max) break;
                }
            }

        };

        template <u8 _capacity, u32 _cmdsPerLine>
        class FastDrawList {
            using Cmd = _drawListInternal::Cmd;
            u32 _size = 0;
            u32 _begin = 0;
            u32 _end = 0;

            void add(const Cmd& s){
                auto& sb = cmdBuffer[_end];
                sb = s;
                s32 maxY = sb.maxY;
                maxY += sb.y;
                if(maxY < 0)
                    return;
                if(maxY > screenHeight)
                    maxY = screenHeight;
                sb.maxY = maxY;

                for (u32 y=sb.y; y<maxY; ++y){
                    u32 i;
                    for(i=0; i<_cmdsPerLine - 1; ++i){
                        if(cmdsPerLine[y][i] == 0xFF) break;
                    }
                    cmdsPerLine[y][i] = _end;
                }

                _end++;
                if(_end >= capacity) _end = 0;
                _size++;
                if (_size > capacity) {
                    _begin++;
                    _size--;
                    if (_begin >= capacity) {
                        _begin = 0;
                    }
                }
            }

            void clear(){
                _begin = _end = _size = 0;
                for(u32 y = 0; y < screenHeight; ++y){
                    for (u32 i = 0; i < _cmdsPerLine; ++i) {
                        cmdsPerLine[y][i] = 0xFF;
                    }
                }
            }

        protected:
            static constexpr u32 capacity = _capacity;
            Cmd cmdBuffer[capacity];
            u8 cmdsPerLine[screenHeight][_cmdsPerLine];

        public:

            FastDrawList(const u8 *font){
                _drawListInternal::font = font;
                clear();
                bind();
                bindText();
            }

            FastDrawList(){
                clear();
                bind();
            }

            void bindText() {
                _graphicsInternal::_print = _drawListInternal::_print;
            }

            void bind() {
                _drawListInternal::instance = this;
                _drawListInternal::add = +[](const Cmd& s){
                                              reinterpret_cast<FastDrawList*>(_drawListInternal::instance)->add(s);
                                          };
                _drawListInternal::clear = +[](){
                                                reinterpret_cast<FastDrawList*>(_drawListInternal::instance)->clear();
                                            };
            }

            void update(u16 *line, u32 y){
                auto cmds = &cmdsPerLine[y][0];
                for(u32 i = 0; i < _cmdsPerLine; ++i){
                    if (cmds[i] == 0xFF) break;
                    auto& s = cmdBuffer[cmds[i]];
                    s.draw(line, s, y - s.y);
                }
            }

        };

    }

    inline void line(s32 x, s32 y, s32 ex, s32 ey, u32 color = Graphics::primaryColor) {
        using namespace _drawListInternal;
        if (y > ey) {
            std::swap(y, ey);
            std::swap(x, ex);
        }

        s32 dy = ey - y;
        s32 dx = ex - x;
        s32 error;
        draw_t f;
        if (dy > std::abs(dx)) {
            error = std::abs(dx << 15) / dy;
            f  = +[](u16 *line, Cmd &s, u32 y){
                      s32 x = s.x;
                      if (x >= 0 && u32(x) < screenWidth) {
                          line[x] = reinterpret_cast<uptr>(s.data);
                      }
                      if ((((y - 1 - s.y) * u32(s.s) + (1<<14)) >> 15) != (((y - s.y) * u32(s.s) + (1<<14)) >> 15)) {
                          s.x += s8(s.b2);
                      }
                  };
        } else if (dx) {
            error = (dy << 15) / std::abs(dx);
            f = +[](u16 *line, Cmd &s, u32 y){
                     s32 e = s.s;
                     s32 stride = s8(s.b2);
                     s32 acc = e * s.b1;
                     s32 end = (e * s.b1 + (1<<14)) >> 15;
                     for (;s.b1 && ((acc + (1<<14)) >> 15) == end; acc -= e){
                         s32 x = s.x;
                         s.b1--;
                         s.x += stride;
                         if (x >= 0 && u32(x) < screenWidth) {
                             line[x] = reinterpret_cast<uptr>(s.data);
                         };
                     }
                 };
        } else return;

        Cmd cmd{
                .data = reinterpret_cast<void*>(color),
                .draw = f,
                .x = decl_cast(Cmd::x, x),
                .y = decl_cast(Cmd::y, y),
                .maxY = decl_cast(Cmd::maxY, dy + 1),
                .b1 = decl_cast(Cmd::b1, std::abs(dx)),
        };
        cmd.b2 = decl_cast(Cmd::b2, (x < ex) ? 1 : -1);
        cmd.s = decl_cast(Cmd::s, error);
        add(cmd);
    }

    inline void line(Point2D start, Point2D end, u32 color = Graphics::primaryColor) {
        start -= camera;
        end -= camera;
        line(round(start.x), round(start.y), round(end.x), round(end.y), color);
    }

    inline void fillRect(s32 x, s32 y, s32 w, s32 h, u32 color = Graphics::primaryColor) {
        using namespace _drawListInternal;
        // LOG(x, y, w, h, color, "\n");

        if(x < 0){ w += x; x = 0; }
        if(u32(x + w) >= screenWidth) w = screenWidth - x;
        if(y < 0){ h += y; y = 0; }
        if(u32(y) >= screenHeight || y + h <= 0 || u32(x) >= screenWidth || (x + w) <= 0)
            return;

        draw_t f = [](u16 *line, Cmd &s, u32 y){
                       auto c = reinterpret_cast<uptr>(s.data);
                       int w = s.b1;
                       line += s.x;
                       while(w--){
                           *line++ = c;
                       }
                   };

        add(Cmd{
                .data = reinterpret_cast<void*>(color),
                .draw = f,
                .x = decl_cast(Cmd::x, x),
                .y = decl_cast(Cmd::y, y),
                .maxY = decl_cast(Cmd::maxY, h),
                .b1 = decl_cast(Cmd::b1, w)
            });
    }

    inline void fillRect(Point2D topLeft, Point2D bottomRight, u32 color = Graphics::primaryColor) {
        topLeft -= camera;
        bottomRight -= camera;
        fillRect(round(topLeft.x), round(topLeft.y), round(bottomRight.x - topLeft.x), round(bottomRight.y - topLeft.y), color);
    }

    inline void fillRect(Point2D topLeft, Size2D size, u32 color = Graphics::primaryColor) {
        topLeft -= camera;
        fillRect(round(topLeft.x), round(topLeft.y), round(size.w), round(size.h), color);
    }

    template <u32 bits, bool transparent = false>
    inline void draw(const u8 *data, s32 x = 0, s32 y = 0, f32 falpha = 1){
        using namespace _drawListInternal;

        if (x + data[0] <= 0 || u32(x) >= screenWidth) return;
        if (y + data[1] <= 0 || u32(y) >= screenHeight) return;

        u8 alpha = round(falpha * 255);
        alpha = (u32(alpha) + 4) >> 3;
        auto f = blitBitPlane;
        auto udata = reinterpret_cast<uptr>(palette);

        switch(bits){
        case 1:
            f = blit1BPP<transparent>;
            udata = primaryColor | (secondaryColor << 16);
            if (alpha != ((0xFF + 4) >> 3)) {
                udata = (udata | udata << 16) & 0x07e0f81f;
            }
            break;

        case 8:
            f = blit8BPP<transparent>;
            break;

        default:
            break;
        }

        Cmd cmd{
                .data = data,
                .draw = f,
                .x = decl_cast(Cmd::x, x),
                .y = decl_cast(Cmd::y, y),
                .maxY = data[1],
                .b1 = decl_cast(Cmd::b1, alpha)
                };
        cmd.udata = udata;
        add(cmd);
    }

    template <u32 bits, bool transparent = false>
    inline void draw(const u8 *data, Point2D topLeft, f32 falpha = 1){
        topLeft -= camera;
        draw<bits, transparent>(data, round(topLeft.x), round(topLeft.y), falpha);
    }

    inline void clear(){
        _drawListInternal::clear();
    }

}
