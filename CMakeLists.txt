cmake_minimum_required(VERSION 3.16)
project(chip-8)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(chip-8
    src/main.cpp
    src/Chip8.cpp
)

target_include_directories(chip-8 PRIVATE src)
