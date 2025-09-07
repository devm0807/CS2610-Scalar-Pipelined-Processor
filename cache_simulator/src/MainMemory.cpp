#include "MainMemory.h"

#include <array>
#include <iostream>
#include <iomanip>
#include <cstring>

MainMemory::MainMemory()
{
    memory = std::make_unique<std::array<uint8_t, MAIN_MEMORY_SIZE>>();

    memory.get()->fill(0x00);
}
MainMemory::~MainMemory() {}

void MainMemory::Read(uint32_t startAddress, uint8_t size, uint8_t *destination) {
    std::memcpy(destination, &memory.get()->at(startAddress), size);
}
void MainMemory::Write(uint32_t startAddress, uint8_t size, uint8_t *source) {
    std::memcpy(&memory.get()->at(startAddress), source, size);
}
void MainMemory::Print() {
    const uint8_t ROWS = 24;
    const uint8_t COLUMNS = 12;

    for(uint8_t row = 0; row < ROWS; row++){
        for(uint8_t column = 0; column < COLUMNS; column++){
            std::cout << "0x"
                      << std::hex << std::setw(2) << std::setfill('0')
                      << static_cast<int>(memory->at(row * COLUMNS + column))
                      << "\t";
        }
        std::cout << std::endl;
    }
    std::cout << std::dec;
}