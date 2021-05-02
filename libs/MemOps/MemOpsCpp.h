#pragma once

#include <cstdint>
#include <cstring>

// C++ versions of ARM assembler MemOps - Jonne

class MemOps {
public:
    static void copy(void *to, const void *from, std::size_t length) {
        std::memcpy(to, from, length);
    }

    static void set(void* dst, std::uint8_t val, std::size_t count) {
        //std::memset ( dst, val, count );
        auto d = static_cast<uint8_t*>(dst);
        for(std::size_t i = 0; i < count; i++) {
            d[i] = val;
        }
    }

    static void set(void* dst, std::uint8_t val, std::size_t count, std::size_t stride) {
        auto d = static_cast<std::uint8_t*>(dst);
        for (std::size_t i = 0; i < count; i += stride) {
            *d = val;
            d += stride;
        }

        /*
          __asm__ volatile (
          ".syntax unified"   "\n" // for example, count = 10, stride = 2, r0 points to dst[0]
          "muls r2, r3"       "\n" // r2 = count * stride, r2 = 5*2 = 10
          "adds r0, r2"       "\n" // add r2 to dst, so dst points to array end , r0 = dst[0] + 10
          "subs r0, r3"       "\n" // sub stride from dst, so r0 = dst[0] + 10 - 2 = dst[8]
          "rsbs r2, 0"        "\n" // r2 = -r2 , r2 = -10
          "beq 2f"            "\n" // Branch to return if r2 is equal to 0 , relative jump forward ("f") to local label 2
          "1: adds r2, r3"    "\n" // add stride to r2, r2 = -8 ... r2 becomes -6 ... until r2 = 0
          "strb r1, [r0, r2]" "\n" // r0 is pointing to dst[8] + offset (-8), dst = data[0] ... dst = data[2] ... until dst[8]
          "ble 1b"            "\n" // Signed integer comparison between gave less than or equal, r2 was -8 so relative jump back "b" to label 1
          "2: bx lr"          "\n" // return from function, absolute jump
          );
        */
    }

};
