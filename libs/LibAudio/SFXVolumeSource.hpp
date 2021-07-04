#pragma once

namespace Audio {
    class SFX8VolumeSource {
        u32 len;
        const u8 *head;

        static void copy(u8 *buffer, void *ptr){
            auto& state = *reinterpret_cast<SFX8VolumeSource*>(ptr);
            u32 len = std::min<u32>(512, state.len * state.slowdown);
            u32 i=0, j=0, r=state.slowdown;
            for(i=0; i<len; ++i){
                s32 sample = state.head[j];
                if (!--r) {
                    r = state.slowdown;
                    j++;
                }
                buffer[i] = (state.volume*(sample - 128) >> 8) + 128;
            }
            for(; i<512; ++i)
                buffer[i] = 128;
            state.head += len;
            state.len -= len;
        }

        static void mix(u8 *buffer, void *ptr){
            auto& state = *reinterpret_cast<SFX8VolumeSource*>(ptr);
            u32 len = std::min<u32>(512, state.len);
            u32 i=0, j=0, r=state.slowdown;
            for(i=0; i<len; ++i){
                s32 sample = state.head[j];
                if (!--r) {
                    r = state.slowdown;
                    j++;
                }
                buffer[i] = Audio::mix(buffer[i], (state.volume*(sample - 128) >> 8) + 128);
            }

            state.head += len;
            state.len -= len;
        }

    public:
        u8 slowdown;
        u8 volume;

        template<u32 channel = 1>
        static SFX8VolumeSource& play(const u8 *data, u32 len){
            static SFX8VolumeSource state;
            state.len = len;
            state.head = data;
            state.volume = 255;
            state.slowdown = 1;
            connect(channel, &state, channel == 0 ? copy : mix);
            return state;
        }

        template<u32 channel = 1, u32 len = 0>
        static SFX8VolumeSource& play(const u8 (&data)[len]){
            return play<channel>(data, len);
        }
    };

    template<u32 channel = 1, u32 len = 0>
    inline SFX8VolumeSource& play(const u8 (&data)[len], u8 volume, u8 slowdown = 1){
        auto& src = SFX8VolumeSource::play<channel>(data, len);
        src.volume = volume;
        src.slowdown = slowdown;
        return src;
    }
}
