#pragma once
#ifdef _FEMTO_INTERNAL_
#include "Femto"

extern "C" {
    void flushLine16(u16 *line);
    // void flushLine(const uint16_t *palette, const uint8_t *line);
    // void flushLine2X(const uint16_t *palette, const uint8_t *line);
    // void pixelExpand(uint8_t* dest, const uint8_t *src, uint32_t count, uint32_t recolor, int32_t stride);
    // void pixelCopy(uint8_t* dest, const uint8_t *src, uint32_t count, uint32_t recolor=0);
    // void pixelCopyMirror(uint8_t* dest, const uint8_t *src, uint32_t count, uint32_t recolor=0);
    // void pixelCopySolid(uint8_t* dest, const uint8_t *src, uint32_t count, uint32_t recolor=0);
}

extern void (*updateDisplay)();

namespace Graphics {

struct LineFiller {
    using Update = void (*)(u16 *line, u32 y, uptr data);
    uptr data;
    Update update;

    LineFiller() = default;

    constexpr LineFiller(LineFiller* other) :
        data(other->data),
        update(other->update) {}

    constexpr LineFiller(uptr data, Update update) : data(data), update(update) {}

    template <typename Class>
    constexpr LineFiller(Class* obj) :
        data(reinterpret_cast<uptr>(obj)),
        update(+[](u16 *line, u32 y, uptr data){
                    reinterpret_cast<Class*>(data)->update(line, y);
                }) {}

    template <typename Class>
    constexpr LineFiller(Class& obj) : LineFiller(&obj) {}
};

namespace layer {

    inline LineFiller solidColor(u16 color){
        return {
            color,
            +[](u16 *line, u32 y, uptr color){
                 for(u32 x=0; x<screenWidth; ++x)
                     line[x] = color;
             }
        };
    };

    inline LineFiller NOP(){
        return {0, +[](u16*, u32, uptr){}};
    }

    class BlendAdd {
    public:
        LineFiller child;
        BlendAdd(const LineFiller &child) : child(child) {}
        void update(u16 *line, u32 y){
            u16 tmp[screenWidth + 16];
//            MemOps::set(tmp, 0, screenWidth*2);
            auto z = reinterpret_cast<u32*>(tmp) + (screenWidth+16)/2;
            for(s32 x = -((screenWidth + 16)/2); x; ++x)
                z[x] = 0;

            child.update(tmp + 8, y, child.data);
            for(u32 i=0; i<screenWidth; ++i){
                u32 in = tmp[i + 8];
                if (!in) continue;

                u32 out = line[i];
                const u32 RgB = 0b11111'000000'11111;
                out =  (out & RgB)
                    | ((out & ~RgB) << 16);

                in  =  (in & RgB)
                    | ((in & ~RgB) << 16);

                out += in;
                in = out & 0b11111'000000'11111'00000'111111'00000;
                if (in) {
                    out |= (in >> 1)
                        | (in >> 2)
                        | (in >> 3)
                        | (in >> 4)
                        | (in >> 5);
                    out &= 0b111111000001111100000011111;
                }
                out |= out >> 16;
                line[i] = out;
            }
        }
    };
};

namespace _graphicsInternal {
    inline void update(LineFiller* fillers, u32 count) {
        u16 line[screenWidth + 16];
        for(u32 y=0; y<screenHeight; ++y){
            for( u32 i=0; i<count; ++i){
                auto& filler = fillers[i];
                filler.update(line + 8, y, filler.data);
            }
            flushLine16(line + 8);
        }
    }

    inline void *instance;
}

template<typename ... Type>
class ScanlineRenderer {
public:
    LineFiller fillers[sizeof...(Type)];

    ScanlineRenderer(const Type& ... fillers) : fillers{const_cast<Type*>(&fillers)...} {
        bind();
    }

    void bind() {
        auto update = +[](){
            auto &instance = *reinterpret_cast<ScanlineRenderer*>(_graphicsInternal::instance);
            _graphicsInternal::update(instance.fillers, sizeof...(Type));
        };

        _graphicsInternal::instance = this;
        updateDisplay = update;
    }
};

}

#include "DrawList.hpp"
#endif
