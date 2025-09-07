#pragma once
#include <cstdint>
class RandomReplacement{
public:
    void SetWays(uint8_t numOfWays);
    uint8_t GetVictim() const;
private:
    uint8_t ways = 0;
};