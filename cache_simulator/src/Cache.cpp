#include "Cache.h"
#include <cstring>
CacheSet::CacheSet(){
    replacement.SetWays(CACHE_WAYS);
}
CacheSet::~CacheSet(){
    
}

CacheLine* CacheSet::Find(uint32_t tag){
    for(uint8_t way = 0; way < CACHE_WAYS; way++){
        if(lines[way].valid && lines[way].tag == tag){
            return &lines[way];
        }
    }
    return nullptr;
}
CacheLine* CacheSet::Replace(uint32_t tag, uint8_t* sourceData){

    uint8_t victim = replacement.GetVictim();

    // set new cache line attributes
    lines[victim].valid = true;
    lines[victim].tag = tag;

    std::memcpy(lines[victim].data.data(), sourceData, CACHE_LINE_SIZE);

    return &lines[victim];
}

void Cache::Initialize(MainMemory* memory){
    mainMemory = memory;
}
uint32_t Cache::Read(uint32_t address){
    AddressParts addressParts(address);

    CacheLine* line = sets[addressParts.setIndex].Find(addressParts.tag);

    if(line){   // cache hit
        // reinterpret the memory as a 32 bit integer
        return *reinterpret_cast<uint32_t*>(&line->data[addressParts.byteOffset]);
    }else{
        uint32_t lineStart = address & ~(CACHE_LINE_SIZE-1);
        std::array<uint8_t, CACHE_LINE_SIZE> buffer;
        mainMemory->Read(lineStart, CACHE_LINE_SIZE, buffer.data());
        CacheLine* newLine = sets[addressParts.setIndex].Replace(addressParts.tag, buffer.data());
        return *reinterpret_cast<uint32_t*>(&newLine->data[addressParts.byteOffset]);

    }
    return 0;
}
void Cache::Write(uint32_t address, uint32_t data){
    AddressParts addressParts(address);

    CacheLine* line  = sets[addressParts.setIndex].Find(addressParts.tag);

    if(line){   // Cache hit
        *reinterpret_cast<uint32_t*>(&line->data[addressParts.byteOffset]) = data;
    }
    mainMemory->Write(address, sizeof(uint32_t), reinterpret_cast<uint8_t*>(&data));
}