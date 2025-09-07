#include "ReplacementAlgorithm.h"

#include <cstdlib>

void RandomReplacement::SetWays(uint8_t numOfWays){
    ways = numOfWays;
}
uint8_t RandomReplacement::GetVictim() const {
    return rand() % ways;
}