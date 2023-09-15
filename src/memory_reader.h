#ifndef MEMORY_READER_H
#define MEMORY_READER_H

#include <windows.h>
#include <stdbool.h>

static const unsigned char PATTERN_GAME_STATE[8] = {0x22, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0x2B, 0x00};

enum GAME_STATES {
    STATE_MUSIC_SELECT = 0x10,
    STATE_STAGE = 0x29,
    STATE_RESULTS = 0xE,
    STATE_TITLE = 0xB,
    STATE_COURSE_SELECT = 0x22,
    STATE_COURSE_RESULT = 0xF,
    STATE_MODE_SELECT = 0x1B,
    STATE_MENU_SELECT = 0x28,
    STATE_ENTRY = 0x18,
    STATE_STARTUP = 0xFF,
    STATE_LOADING = 0x00
};

enum PATTERN_IDENTIFIERS {
    PI_GAME_STATE,
    PI_UI_PTR,
    PI_USERDATA_PTR
};

typedef struct _UI_OBJECT {
    char _trash1[120];
    char label[128];
    char _trash2[36];
    char text[512];  // actually bigger than 512 in memory I think, but this is enough
} UI_OBJECT;

typedef struct _USERDATA {
    size_t _ptr1;
    int _unknown1;
    char str1[17]; // idk what these strings are, the names can be fixed when that's figured out
    char str2[13];
    char str3[17];
    char username[9];
    char _unknown2[212];
    unsigned int volforce;
} USERDATA;

typedef struct _MEMORY_DATA {
    unsigned char GameState;
    UI_OBJECT UiObjects[26];
    unsigned char uiObjCount;
    USERDATA UserData;
} MEMORY_DATA;

typedef struct _SEARCH_PATTERN {
    const char* pattern;
    const unsigned char size;
    const unsigned int offset;
    const unsigned char base;
    const unsigned char identifier;
    void* addr;
} SEARCH_PATTERN;

extern MEMORY_DATA MemoryData;

/*
    Looks for and attempts to open a handle to the SDVX process.
    Then finds all the memory addresses for the data we want.
*/
bool memory_reader_init();
/*
    Set memory of MemoryData to 0 and close handle to process.
    Returns false if closing process handle fails, else true.
*/
bool memory_reader_cleanup();
/*
    Update values of MemoryData.
    The values will otherwise not update.
*/
bool memory_reader_update();
/*
    Get the process id of the current process (sv6c.exe).
*/
DWORD memory_reader_process_id();

#endif  /* MEMORY_READER_H */