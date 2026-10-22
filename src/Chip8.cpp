#include "Chip8.h"
#include <stdio.h>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>

uint8_t chip8_fontset[80] =
{
    0xF0, 0x90, 0x90, 0x90, 0xF0, //0
    0x20, 0x60, 0x20, 0x20, 0x70, //1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, //2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, //3
    0x90, 0x90, 0xF0, 0x10, 0x10, //4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, //5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, //6
    0xF0, 0x10, 0x20, 0x40, 0x40, //7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, //8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, //9
    0xF0, 0x90, 0xF0, 0x90, 0x90, //A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, //B
    0xF0, 0x80, 0x80, 0x80, 0xF0, //C
    0xE0, 0x90, 0x90, 0x90, 0xE0, //D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, //E
    0xf0, 0x80, 0xf0, 0x80, 0x80  //f
};

// Initialize
void Chip8::initialize() {
    pc      = 0x200;    // Program counter starts at 0x200
    opcode  = 0;        // Reset current opcode
    I       = 0;        // Reset index register
    sp      = 0;        // Reset stack pointer

    // Clear display
    for (int i = 0; i < 2048; i++) {
        gfx[i] = 0;
    }

    // Clear stack, key and V registers
    for (int i = 0; i < 16; i++) {
        stack[i] = 0;
        key[i]   = 0;
        V[i]     = 0;
    }

    for (int i = 0; i < 4096; i++) {
        memory[i] = 0;
    }

    // Load fontset
    for(int i = 0; i < 80; i++) {
        memory[i] = chip8_fontset[i];
    }

    srand((unsigned int)time(NULL));
}

// Initialize and load the ROM file
bool Chip8::loadROM(const char* file_path) {
    initialize();
    
    // Open file stream
    FILE* file_ptr = fopen(file_path, "rb");
    size_t file_size = 0;

    if (file_ptr == nullptr) {
        std::cerr << "Failed to open rom" << std::endl;
        return false;
    }

    // Get the file size
    fseek(file_ptr, 0, SEEK_END);
    file_size = ftell(file_ptr);
    rewind(file_ptr);

    // Allocate memory to store the rom
    uint8_t* file_buffer = (uint8_t*)malloc(sizeof(char) * file_size);
    if (file_buffer == nullptr) {
        std::cerr << "Could not allocate memory for the ROM file" << std::endl;
        return false;
    }

    // Copy ROM into buffer
    size_t result = fread(file_buffer, sizeof(char), file_size, file_ptr);
    if (result != file_size) {
        std::cerr << "Failed to read ROM" << std::endl;
        fclose(file_ptr);
        free(file_buffer);
        return false;
    }

    // Check if the ROM size is not larger than the available memory
    if (file_size <= (4096 - START_ADDRESS)) {
        // Copy ROM buffer to memory
        for (int i = 0; i < file_size; i++) {
            memory[START_ADDRESS + i] = file_buffer[i]; // Load buffer into memory. Starting at 0x200 (512)
        }
    } else {
        std::cerr << "ROM is too large to fit into memory" << std::endl;
        fclose(file_ptr);
        free(file_buffer);
        return false;
    }

    // End the file stream and free ROM buffer
    fclose(file_ptr);
    free(file_buffer);
    return true;
}

void Chip8::emulateCycle() {
    // Fetch the opcode
    opcode = memory[pc] << 8 | memory[pc + 1];

    /// The & operator here works like a mask. When we have a opcode for example 0xA2F0. The real goal of the opcode
    /// is specified by the first cyfer of the code, in this example A (0x(A)2F0). The other 3 cyfers are more specific
    /// of what needs to happen for that specific goal. So now the switch statement can filter on the first cyfer.

    if (delay_timer > 0)
        delay_timer--;

    if (sound_timer > 0)
        sound_timer--;

    // Decode the opcode
    switch (opcode & 0xF000) {
        // (& 0xF000) this isolates the first 4 bytes of the 16 byte opcode
        // Look for 0x0NNN
        // For us specific 0x0ENN
        case 0x0000:
            switch (opcode & 0x0FF) {
                case 0x00E0: // (0x00E0) Clear the screen
                    memset(gfx, 0, sizeof(gfx));
                    drawFlag = true;
                    pc += 2;
                    break;
                case 0x00EE: // (0x00EE) Returns from subroutine
                    sp--;
                    pc = stack[sp];
                    pc += 2;
                    break;
                default:
                    printf("Unknown opcode [0x0000]: 0x%X\n", opcode);
            }
            break;

        case 0x1000: // (0x1NNN) Jumps to the address NNN
            pc = opcode & 0x0FFF;
            break;

        case 0x2000: // (0x2NNN) Calls subroutine at address NNN
            stack[sp] = pc;
            sp++;
            pc = opcode & 0x0FFF;
            break;

        case 0x3000: // (0x3XNN) Skips the next instruction if VX equals NN
            if (V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF))
                pc += 4; // Increase by 4 because we want to skip the next opcode
            else
                pc += 2;

            break;

        case 0x4000: // (0x4XNN) Skips the next instruction if VX does not equal NN
            if (V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF))
                pc += 4; // Increase by 4 because we want to skip the next opcode
            else
                pc += 2;

            break;

        case 0x5000: // (0x5XY0) // Skips the next instruction if VX equals VY
            if (V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4])
                pc += 4;
            else
                pc += 2;

            break;

        case 0x6000: // (0x6XNN) Sets VX to NN
            V[(opcode & 0x0F00) >> 8] = (opcode & 0x00FF);
            pc += 2;
            break;

        case 0x7000: // (0x7XNN) Adds NN to VX (carry flag is not changed)
            V[(opcode & 0x0F00) >> 8] += (opcode & 0x00FF);
            pc += 2;
            break;

        case 0x8000: // All opcodes starting with 0x8
            switch (opcode & 0x000F) {
                case 0x0000: //(0x8XY0) Sets VX to the value of VY
                    V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;

                case 0x0001: // (0x8XY1) Sets VX to VX or VY
                    V[(opcode & 0x0F00) >> 8] |= V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;

                case 0x0002: // (0x8XY2) Sets VX to VX and VY
                    V[(opcode & 0x0F00) >> 8] &= V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;

                case 0x0003: // (0x8XY3) Sets VX to VX xor VY
                    V[(opcode & 0x0F00) >> 8] ^= V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;

                case 0x0004:
                    // (0x8XY4) Adds VY to VX. VF is set to 1 when there's an overflow, and to 0 when there is not
                    if ((V[(opcode & 0x00F0) >> 4]) > ((0xFF) - V[(opcode & 0x0F00) >> 8]))
                        V[0xF] = 1;
                    else
                        V[0xF] = 0;

                    V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;

                case 0x0005: // (0x8XY5) VY is subtracted from VX. VF is set to 0 when there's an underflow,
                    // and 1 when there is not. (i.e. VF set to 1 if VX >= VY and 0 if not).
                    if (V[(opcode & 0x0F00) >> 8] >= V[(opcode & 0x00F0) >> 4])
                        V[0xF] = 1;
                    else
                        V[0xF] = 0;

                    V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;

                case 0x0006:
                    // (0x8XY6) Shifts VX to the right by 1, then stores the least significant bit of VX prior to the shift into VF
                    V[0xF] = (V[opcode & 0x0F00]) >> 8 & 0x01;
                    V[(opcode & 0x0F00) >> 8] = (opcode & 0x0F00) << 1;
                    pc += 2;
                    break;

                case 0x0007: // (0x8XY7) Sets VX to VY minus VX. VF is set to 0 when there's an underflow,
                    // and 1 when there is not. (i.e. VF set to 1 if VY >= VX)
                    if (V[(opcode & 0x00F0) >> 4] >= V[(opcode & 0x0F00) >> 8])
                        V[0xF] = 1;
                    else
                        V[0xF] = 0;

                    V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8];
                    pc += 2;
                    break;

                case 0x000E: // (0x8XYE) Shifts VX to the left by 1, then sets VF to 1 if the most significant bit
                    // of VX prior to that shift was set, or to 0 if it was unset
                    V[0xF] = (V[(opcode & 0x0F00) >> 8] & 0x80) >> 7;
                    V[(V[opcode & 0x0F00]) >> 8] <<= 1;
                    pc += 2;
                    break;

                default:
                    printf("Unknown opcode [0x0000]: 0x%X\n", opcode);
            }
            break;

        case 0x9000: // (0x9XY0) Skips the next instruction if VX does not equal VY
            if (V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4])
                pc += 4;
            else
                pc += 2;
            break;

        case 0xA000: // (0xANNN) Sets I to the address NNN
            I = opcode & 0x0FFF;
            pc += 2;
            break;

        case 0xB000: // (0xBNNN) Jumps to the address NNN plus V0
            pc = V[0x0] + (opcode & 0x0FFF);
            break;

        case 0xC000: // (0xCXNN) Sets VX to the result of a bitwise and operation on a random number (Typically: 0 to 255) and NN
            V[(opcode & 0x0F00) >> 8] &= rand() % 256;
            pc += 2;
            break;

        case 0xD000: { // (0xDXYN) Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels.
                       // Each row of 8 pixels is read as bit-coded starting from memory location I; I value does not change
                       // after the execution of this instruction. As described above, VF is set to 1 if any screen pixels are
                       // flipped from set to unset when the sprite is drawn, and to 0 if that does not happen
            uint16_t x = V[(opcode & 0x0F00) >> 8];
            uint16_t y = V[(opcode & 0x00F0) >> 4];
            uint16_t height = (opcode & 0x000F);
            uint16_t pixel;

            V[0xF] = 0;
            for (int yLine = 0; yLine < height; yLine++) {
                pixel = memory[I + yLine];
                for (int xLine = 0; xLine < 8; xLine++) {
                    if ((pixel & (0x80 >> xLine)) != 0) {
                        if (gfx[(x + xLine + ((y + yLine) * 64))] == 1)
                            V[0xF] = 1;
                        gfx[x + xLine + ((y + yLine) * 64)] ^= 1;
                    }
                }
            }

            drawFlag = true;
            pc += 2;
            break;
        }

        case 0xE000:
            switch (opcode & 0x00FF) {
                case 0x009E: // (0xEX9E) Skips the next instruction if the key stored in VX(only consider the lowest nibble) is pressed
                    if (key[V[(opcode & 0x0F00) >> 8]] != 0)
                        pc += 4;
                    else
                        pc += 2;

                    break;

                case 0x00A1: // (0xEXA1) Skips the next instruction if the key stored in VX(only consider the lowest nibble) is not pressed
                    if (key[V[(opcode & 0x0F00) >> 8]] == 0)
                        pc += 4;
                    else
                        pc += 2;

                    break;

                default:
                    printf("Unknown opcode [0x0000]: 0x%X\n", opcode);
            }
            break;

        case 0xF000:
            switch (opcode & 0x00FF) {
                case 0x0007: // (0xFX07) Sets VX to the value of the delay timer
                    V[(opcode & 0x0F00) >> 8] = delay_timer;
                    pc += 2;
                    break;

                case 0x000A: { // (0xFX0A) A key press is awaited, and then stored in VX
                               // (blocking operation, all instruction halted until next key event, delay and sound timers should continue processing)
                    bool key_pressed = false;
                    for (int i = 0; i < 16; i++) {
                        if (key[i] != 0) {
                            V[(opcode & 0x0F00) >> 8] = i;
                            key_pressed = true;
                            break;
                        }
                    }

                    if (!key_pressed)
                        return;

                    pc += 2;
                    break;
                }

                case 0x0015: // (0xFX15) Sets the delay timer to VX
                    delay_timer = V[(opcode & 0x0F00) >> 8];
                    pc += 2;
                    break;

                case 0x0018: // (0xFX18) Sets the sound timer to VX
                    sound_timer = V[(opcode & 0x0F00) >> 8];
                    pc += 2;
                    break;

                case 0x001E: // (0xFX1E) Adds VX to I. VF is not affected
                    I += V[(opcode & 0x0F00) >> 8];
                    pc += 2;
                    break;

                case 0x0029: // (0xFX29) Sets I to the location of the sprite for the character in VX
                             // (only consider the lowest nibble). Characters 0-F (in hexadecimal) are represented by a 4x5 font
                    I = V[(opcode & 0x0F00) >> 8] * 5;
                    pc += 2;
                    break;

                case 0x0033: // (0xFX33) Stores the binary-coded decimal representation of VX, with the hundreds digit in memory
                             // at location in I, the tens digit at location I+1, and the ones digit at location I+2
                    memory[I] = V[(opcode & 0x0F00) >> 8] / 100;
                    memory[I + 1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10;
                    memory[I + 2] = (V[(opcode & 0x0F00) >> 8] % 100) % 10;
                    pc += 2;
                    break;

                case 0x0055: // (0xFX55) Stores from V0 to VX (including VX) in memory, starting at address I.
                             // The offset from I is increased by 1 for each value written, but I itself is left unmodified
                    for (int i = 0; i <= ((opcode & 0x0F00) >> 8); i++) {
                        memory[I + i] = V[i];
                    }
                    pc += 2;
                    break;

                case 0x0065: // (0xFX65) Fills from V0 to VX (including VX) with values from memory, starting at address I.
                             // The offset from I is increased by 1 for each value read, but I itself is left unmodified
                    for (int i = 0; i <= ((opcode & 0x0F00) >> 8); i++) {
                        V[i] = memory[I + i];
                    }
                    pc += 2;
                    break;

                default:
                    printf("Unknown opcode [0x0000]: 0x%X\n", opcode);
            }
            break;
    }
}
