#pragma once

#define NAKED __attribute__((naked))

class MemOps {
public:
    static void NAKED copy(void *to, const void *from, std::size_t length) {
    __asm__ volatile (
        ".syntax unified" "\n"
        "cmp r2, 0" "\n"
        "beq 3f" "\n"

        "adds r0, r2" "\n"
        "adds r1, r2" "\n"

        // check if word-aligned
        "movs r3, r0" "\n"
        "orrs r3, r1" "\n"
        "orrs r3, r2" "\n"
        "lsls r3, 30" "\n"
        "beq 2f" "\n"

        // not aligned, do byte copy
        "rsbs r2, 0" "\n"
        "1: ldrb r3, [r1, r2]" "\n"
        "strb r3, [r0, r2]" "\n"
        "adds r2, 1" "\n"
        "bne 1b" "\n"
        "bx lr" "\n"

        // aligned, do word copy
        "2: rsbs r2, 0" "\n"
        "1: ldr r3, [r1, r2]" "\n"
        "str r3, [r0, r2]" "\n"
        "adds r2, 4" "\n"
        "bne 1b" "\n"
        "3: bx lr" "\n"
        );
    }

    static void NAKED set(void* dst, std::uint8_t val, std::size_t count) {
        __asm__ volatile (
            ".syntax unified" "\n"
            "rsbs r2, 0" "\n"
            "beq 2f" "\n"
            "subs r0, r2" "\n"
            "1: strb r1, [r0, r2]" "\n"
            "adds r2, 1" "\n"
            "bne 1b" "\n"
            "2: bx lr" "\n"
            );
    }

    static void NAKED set(void* dst, std::uint8_t val, std::size_t count, std::size_t stride) {
        __asm__ volatile (
            ".syntax unified"   "\n"
            "muls r2, r3"       "\n"
            "adds r0, r2"       "\n"
            "subs r0, r3"       "\n"
            "rsbs r2, 0"        "\n"
            "beq 2f"            "\n"
            "1: adds r2, r3"    "\n"
            "strb r1, [r0, r2]" "\n"
            "ble 1b"            "\n"
            "2: bx lr"          "\n"
        );
    }

};

#undef NAKED
