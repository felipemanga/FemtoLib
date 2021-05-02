#pragma once

#pragma GCC diagnostic ignored "-Wattributes"

namespace Audio {

    inline void setVolume(u32 v){
        v = 255 - (192 - v) * (192 - v) * 255 / 36864;
// #if defined(POKITTO) || defined(POKITTO_CLASSIC)
//         u32 hwVolume = v ? (v>>1) | 0xF : 0;
//         u32 swVolume = v ? (v | 0xF) + 1 : 0;
//         SoftwareI2C(P0_4, P0_5).write(0x5e, hwVolume);
//         audio_volume = swVolume;
// #else
        audio_volume = v;
// #endif
    }

    inline void mix(void* dst, const void* src, std::size_t count) {
        auto bdst = reinterpret_cast<u8*>(dst) + count;
        auto bsrc = reinterpret_cast<const u8*>(src) + count;
        for (int i = -count; i < 0; ++i)
            bdst[i] = bsrc[i];
    }

    template <u32 channelCount, u32 sampleRate>
    class Sink : public BaseSink<Sink<channelCount, sampleRate>, channelCount> {
        static void IRQ(){}

    public:
        void init(){
            // NVIC_SetVector((IRQn_Type)TIMER_32_0_IRQn, (uint32_t)IRQ);
            if(this->wasInit)
                return;
            this->wasInit = true;

            this->channels[0].source =
                +[](u8 *buffer, void *ptr){
                     MemOps::set(buffer, 128, 512);
                 };

            for(int i=1; i<channelCount; ++i){
                this->channels[i].source = nullptr;
            }

            for(int i=0; i<bufferCount; ++i){
                audio_state[i] = 0;
            }
            audio_playHead = 0;

            MemOps::set(audio_buffer, 128, 1024);
        }

        Sink() = default;
    };
}
