#include "Chip8.h"
#include <stdio.h>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>

unsigned char chip8_fontset[80] =
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
}

// Initialize and load the ROM file
void Chip8::loadROM(const char* file_path) {
    initialize();
    
    // Open file stream
    FILE* file_ptr = fopen(file_path, "rb");
    size_t file_size = 0;

    if (file_ptr == NULL) {
        std::cerr << "Failed to open rom" << std::endl;
        return;
    }

    // Get the file size
    fseek(file_ptr, 0, SEEK_END);
    file_size = ftell(file_ptr);
    rewind(file_ptr);

    // Allocate memory to store the rom
    char* file_buffer = (char*)malloc(sizeof(char) * file_size);
    if (file_buffer == nullptr) {
        std::cerr << "Could not allocate memory for the ROM file" << std::endl;
        return;
    }

    // Copy ROM into buffer
    size_t result = fread(file_buffer, sizeof(char), file_size, file_ptr);
    if (result != file_size) {
        std::cerr << "Failed to read ROM" << std::endl;
        return;
    }

    // Check if the ROM size is not larger than the available memory
    if ((4096 - 512) > file_size) {
        // Copy ROM buffer to memory
        for (int i = 0; i < file_size; i++) {
            memory[START_ADDRESS + i] = (size_t)file_buffer[i]; // Load buffer into memory. Starting at 0x200 (512)
        }
    } else {
        std::cerr << "ROM is too large to fit into memory" << std::endl;
        return;
    }

    // End the file stream and free ROM buffer
    fclose(file_ptr);
    free(file_buffer);
}

void Chip8::emulateCycle() {
    // Fetch the opcode
    opcode = memory[pc] << 8 | memory[pc + 1];

    // Decode the opcode
    switch (opcode & 0xF000) { // (& 0xF000) this isolates the first 4 bytes of the 16 byte opcode
        case 0xA000: // ANNN sets I to the address NNN
            I = opcode & 0x0FFF;
            pc += 2;
            break;
        case 0x2000: // Calls subroutine at address NNN
            stack[sp] = pc;
            sp++;
            pc = opcode & 0x0FFF;
            break;
        case 0x0004: // Adds the value of VY to VX
            if(V[(opcode & 0x00F0) >> 4] > (0xFF - V[(opcode & 0x0F00) >> 8]))
                V[0xF] = 1; // carry because the sum was bigger then 255
            else
                V[0xF] = 0;

            V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
            pc += 2;
            break;
        case 0x0033:
            memory[I]     = V[(opcode & 0x0F00) >> 8] / 100;
            memory[I + 1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10;
            memory[I + 2] = (V[(opcode & 0x0F00) >> 8] % 100) % 10;
            pc += 2;
            break;
        case 0x0000:
            switch(opcode & 0x000F)
            {
                case 0x0000: // 0x00E0: Clears the screen
                    // Execute opcode
                    break;

                case 0x000E: // 0x00EE: Returns from subroutine
                    // Execute opcode
                    break;

                default:
                    printf("Unknown opcode [0x0000]: 0x%X\n", opcode);
            }
            break;
        default:
            std::cout << "Unknown opcode: 0x" + std::to_string(opcode) + "\n";
    }

    if (delay_timer > 0)
        delay_timer--;

    if (sound_timer > 0) {
        if (sound_timer == 1)
            std::cout << "BEEP!\n";

        sound_timer--;
    }
}
