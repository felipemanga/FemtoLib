#pragma once
#include "Femto"

extern "C" void flushLine16(u16 *line);
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
            auto obj = reinterpret_cast<Class*>(data);
            if constexpr (std::is_member_function_pointer<decltype(&Class::update)>::value) {
                reinterpret_cast<Class*>(data)->update(line, y);
            } else {
                auto update = obj->update;
                (obj->*update)(line, y);
            }
        }) {}

    template <typename Class>
    constexpr LineFiller(Class& obj) : LineFiller(&obj) {}
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

// default Init

#include "layers/solidColor.hpp"
#include "layers/DrawList.hpp"
#include "layers/RotoZoom.hpp"
#include "fonts/tiny5x7.hpp"

namespace Graphics {
    inline void init(u32 bgColor = 0){
        Graphics::palette = Graphics::generalPalette;
        static Graphics::layer::DrawList<200> drawList(fontTiny);
        static Graphics::ScanlineRenderer defaultGfx = {
            Graphics::layer::solidColor(bgColor),
            drawList
        };
    }
}
