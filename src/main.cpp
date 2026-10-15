#include "Chip8.h"
#include <iostream>

Chip8 chip8;

int main(int argc, char *argv[]) {
    char* paramater_file_path;

    if (argc > 1) {
        paramater_file_path = argv[1];
    } else {
        std::cerr << "Give the ROM file path as paramater" << std::endl;
        return 1;
    }

    chip8.loadROM(paramater_file_path);

    return 0;
}
