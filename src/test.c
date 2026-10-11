#include "memory_reader.h"
#include <stdio.h>


int main(int arc, char** argv) {
    SetConsoleOutputCP(CP_UTF8);

    if (!memory_reader_init()) return 1;
    if (!memory_reader_update()) return 1;
    printf("Game state: %s\n", MemoryData.gameStateName);
    printf("Found %d states\n", MemoryData.gameStateTableSize);
    printf("Found %d score records\n", MemoryData.scoreRecordsSize);
    printf("Found %d music entries\n", MemoryData.musicDbSize);
    MUSIC_ENTRY* currentlyPlaying = memory_reader_get_currently_playing();
    if (currentlyPlaying == NULL) {
        printf("Nothing playing\n");
    } else {
        printf(
            "Currently playing %s - %s, stage %d, difficulty %d\n",
            currentlyPlaying->artistName,
            currentlyPlaying->titleName,
            MemoryData.currentlyPlayingStage,
            MemoryData.currentlyPlayingDifficulty
        );
    }
	
	printf("Completed successfully\n");
}