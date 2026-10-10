#include "memory_reader.h"
#include <stdio.h>


int main(int arc, char** argv) {
    if (!memory_reader_init()) return 1;
    if (!memory_reader_update()) return 1;
    printf("Game state: %s\n", MemoryData.gameStateName);
    printf("Found %d states\n", MemoryData.gameStateTableSize);
    for (int i=0; i<MemoryData.gameStateTableSize; i++) {
        printf("State %d: %s\n", MemoryData.gameStateTable[i].id, MemoryData.gameStateTable[i].name);
    }
}