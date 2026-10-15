#include "Chip8.h"
#include <stdio.h>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <iostream>

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
void Chip8::loadROM(const char* file_name) {
    initialize();
    
    // Open file stream
    FILE* file_ptr = fopen(file_name, "rb");
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
    if (file_buffer == NULL) {
        std::cerr << "Could not allocate memory for the ROM file" << std::endl;
        return;
    }

    // Copy ROM into buffer
    size_t result = fread(file_buffer, sizeof(char), file_size, file_ptr);
    if (result != file_size) {
        std::cerr << "Failed to read ROM" << std::endl;
        return;
    }

    // Check if the ROM size is not larger then the available memory
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
