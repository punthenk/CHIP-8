#pragma once

#include <cstdint>

class Chip8 {
public:
    uint16_t stack[16];                         // The stack for the emulator
    uint16_t sp;                                // Stack pointer

    uint8_t memory[4096];                       // 4K Memory of the emulator
    uint8_t V[16];                              // 16th register used as 'carry flag'

    uint16_t pc;                                // Program counter
    uint16_t opcode;                            // Current operation code
    uint16_t I;                                 // Index register

    uint8_t delay_timer;                        // When set above 0 it will count down to 0
    uint8_t sound_timer;                        // When set above 0 it will count down to 0

    uint8_t gfx[64 * 32];                       // (64 * 32) 2048 pixels for the graphics
    uint8_t key[16];                            // Keep track of the current state of the key

    bool drawFlag = false;                      // Keep track of the current draw state

    void initialize();                          // Initialize registers and memory once
    void loadROM(const char* file_path);        // Load FILE into memory
    void emulateCycle();

private:
    const unsigned int START_ADDRESS = 0x200;   // Load the ROM memory starts at 0x200
};

