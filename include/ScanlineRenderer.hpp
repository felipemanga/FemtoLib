#pragma once
#include "Femto"
#include "types.hpp"
#include <tuple>

extern "C" void flushLine16(u16 *line);
extern void (*updateDisplay)();

namespace Graphics {

    using LineFiller = Function<void(u16*, u32)>;

    namespace _graphicsInternal {
        inline void update(LineFiller* fillers, u32 count) {
            u16 line[screenWidth + 16];
            for(u32 y=0; y<screenHeight; ++y){
                for( u32 i=0; i<count; ++i){
                    fillers[i](line + 8, y);
                }
                flushLine16(line + 8);
            }
        }

        inline void *instance;
    }

    class DynamicRenderer {
    public:
        LineFiller *fillers;
        const u32 fillerCount;

        template<u32 count>
        DynamicRenderer(std::array<LineFiller, count> &fillers) : fillers{fillers.data()}, fillerCount(count) { bind(); }

        template<u32 count>
        DynamicRenderer(LineFiller (&fillers)[count]) : fillers{fillers}, fillerCount(count) { bind(); }

        DynamicRenderer(LineFiller *fillers, u32 count) : fillers{fillers}, fillerCount(count) { bind(); }

        ~DynamicRenderer(){
            if (_graphicsInternal::instance == this) {
                _graphicsInternal::instance = nullptr;
                if (updateDisplay == update) {
                    updateDisplay = +[](){};
                }
            }
        }

        static void update(){
            auto &instance = *reinterpret_cast<DynamicRenderer*>(_graphicsInternal::instance);
            _graphicsInternal::update(instance.fillers, instance.fillerCount);
        };

        void bind() {
            _graphicsInternal::instance = this;
            updateDisplay = update;
        }
    };

    template <typename ... Args>
    class Renderer : DynamicRenderer {
        std::tuple<Args...> args;
        LineFiller fillers[sizeof...(Args)];

    public:
        Renderer() :
            DynamicRenderer(fillers),
            fillers{std::get<Args>(args)...} {}

        template <u32 num>
        auto get(){
            return std::get<num>(args);
        }

        template <typename Type>
        void bind() {
            std::get<Type>(args).bind();
        }

        template <u32 num>
        void bind() {
            std::get<num>(args).bind();
        }
    };

}

// default Init

#include "layers/solidColor.hpp"
#include "layers/DrawList.hpp"
#include "fonts/tiny5x7.hpp"

namespace Graphics {
    template<u32 bgColor = colorFromRGB(0x1155AA), u32 spriteCount = 100, const u8* font = fontTiny>
    inline void init(){
        using Type = Graphics::Renderer<
            Graphics::layer::SolidColor<bgColor>,
            Graphics::layer::DrawList<spriteCount, font>
            >;
        static __attribute__((align(alignof(Type)))) u8 defaultGfx[sizeof(Type)];
        new (defaultGfx) Type();
    }
}
