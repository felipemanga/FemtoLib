#pragma once

namespace Audio {
    class SFX8VolumeSource {
        u32 len;
        const u8 *head;

        static void copy(u8 *buffer, void *ptr){
            auto& state = *reinterpret_cast<SFX8VolumeSource*>(ptr);
            u32 len = std::min<u32>(512, state.len);
            u32 i=0;
            for(i=0; i<len; ++i)
                buffer[i] = (state.volume*(s32(state.head[i]) - 128) >> 8) + 128;
            for(; i<512; ++i)
                buffer[i] = 128;
            state.head += len;
            state.len -= len;
        }

        static void mix(u8 *buffer, void *ptr){
            auto& state = *reinterpret_cast<SFX8VolumeSource*>(ptr);
            u32 len = std::min<u32>(512, state.len);
            u32 i=0;
            for(i=0; i<len; ++i){
                s32 s = (state.volume*(s32(state.head[i]) - 127) >> 8) + 127;
                buffer[i] = Audio::mix(buffer[i], s);
            }

            state.head += len;
            state.len -= len;
        }

    public:
        u8 volume;

        template<u32 channel = 1>
        static SFX8VolumeSource& play(const u8 *data, u32 len){
            static SFX8VolumeSource state;
            state.len = len;
            state.head = data;
            connect(channel, &state, channel == 0 ? copy : mix);
            return state;
        }

        template<u32 channel = 1, u32 len = 0>
        static SFX8VolumeSource& play(const u8 (&data)[len]){
            return play<channel>(data, len);
        }
    };

    template<u32 channel = 1, u32 len = 0>
    inline SFX8VolumeSource& play(const u8 (&data)[len], u8 volume){
        auto& src = SFX8VolumeSource::play<channel>(data, len);
        src.volume = volume;
        return src;
    }
}
