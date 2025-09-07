#pragma once

#include "MainMemory.h"
#include "Cache.h"

class MemorySystem {
public:
    MemorySystem();
    ~MemorySystem();

    uint32_t Read(uint32_t address);
    void Write(uint32_t address, uint32_t data);
    void PrintMainMemory();

private:
    MainMemory mainMemory;
    Cache cache;
};