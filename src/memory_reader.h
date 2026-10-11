#ifndef MEMORY_READER_H
#define MEMORY_READER_H

#include <windows.h>
#include <stdbool.h>

typedef struct {
    const char byte;
    const bool wild;
} PATTERN_BYTE;

enum GAME_STATES {
    BOOT,
    ONLINEUPDATE,
    APPMAINFLOW,
    TESTMODE_FLOW,
    TITLEDEMO_FLOW,
    TOTAL_RESULT_FLOW,
    PROTOTYPE_FLOW,
    SHOPCLOSING,
    CAUTION,
    GAMEOVER,
    CONFIG_VERISON_CHECK,
    TITLE_SCENE,
    WARNING_SCENE,
    MOVIE_SCENE,
    RESULT_SCENE,
    T_RESULT_SCENE,
    MUSICSELECT,
    DEMO_GAME,
    DEMO_TUTORIAL,
    CARD_ENTRY_SCENE,
    CARD_ENTRY_JOIN_SCENE,
    CARD_ENTRY_NEWEAPASS_SCENE,
    CARD_ENTRY_REGIST_SCENE,
    CARD_ENTRY_AUTH_SCENE,
    CARD_ENTRY_CHECKIN_SCENE,
    CARD_ENTRY_INFORMATION_SCENE,
    CARD_ENTRY_MODE_SELECT_SCENE,
    CARD_ENTRY_ERROREND_SCENE,
    CARD_ENTRY_END_SCENE,
    NAME_ENTRY_SCENE,
    RADARMAP_SELECT_SCENE,
    CARD_OUT_SCENE,
    STATION_SCENE,
    SKILL_LEVEL_SELECT,
    TUTORIAL,
    INFORMATION_SCENE,
    IMPOSSIBLE_NOTES_CHECK,
    PROTOTYPE_VIEWER,
    SYSTEM_VOLUME_CHECK_TEST_MENU,
    MYROOM_SCENE,
    ALTERNATIVE_GAME_SCENE,
    MEGAMIXBATTLE_SCENE,
    TRAINING_GAME_SCENE,
    MAIN_TEST_MENU,
    IO_TEST_MENU,
    IO_INPUT_CHECK_TEST_MENU,
    IO_CAMERA_CHECK_TEST_MENU,
    IO_TOUCH_PANEL_CHECK_TEST_MENU,
    IO_LAMP_CHECK_TEST_MENU,
    IO_MECHA_CHECK_TEST_MENU,
    SCREEN_TEST_MENU,
    NETWORK_CHECK_MENU,
    SYSTEM_INFO_TEST_MENU,
    OSS_INFO_TEST_MENU,
    TOHO_CAMPAIGN_CONFIRM_TEST_MENU,
    TOHO_CAMPAIGN_LIST_TEST_MENU,
    GOODS_CAMPAIGN_LIST_TEST_MENU,
    QC_TEST_MENU,
    GAME_STATE_UNKNOWN
};

#define GAME_STATES_MAP_SIZE 58
typedef struct {
    unsigned char value;
    const char* name;
} GAME_STATE_MAP_ITEM;
extern const GAME_STATE_MAP_ITEM GAME_STATES_MAP[GAME_STATES_MAP_SIZE];

enum PATTERN_IDENTIFIERS {
    PI_GAME_STATE,
    PI_STATE_TABLE,
    PI_SCORE_RECORDS,
    PI_MUSIC_DB_MANAGER,
    PI_CURRENTLY_PLAYING,
};

typedef struct {
    unsigned int id;
    char name[64];
} GAME_STATE;

typedef struct {
    unsigned int stageNumber;
    unsigned int musicId;
    unsigned int difficulty;
    unsigned int playMode;
    unsigned int clear;
    unsigned int grade;
    unsigned int score;
    unsigned int exScore;
    unsigned int maxChain;
    unsigned int just;
    unsigned int critical;
    unsigned int nearr;
    unsigned int error;
    unsigned int btn;
    unsigned int lng;
} SCORE_RECORD;

typedef struct {
    int musicId;
    int version;
    int infVer;
    unsigned int genre;
    int bgNo;
    int demoPri;
    unsigned int distributionDate;
    unsigned int bpmMin;
    unsigned int bpmMax;
    unsigned short volume;
    int level[5];
    int price[5];
    int jacketPrint[5];
    int jacketMask[5];
    char label[8];
    char ascii[64];
    char titleName[256];
    char titleName2[256];
    char artistName[256];
    char titleYomigana[256];
    char artistYomigana[256];
    char illustrator[5][256];
    char effectedBy[5][256];
} MUSIC_ENTRY;

typedef struct {
    unsigned int gameStateId;  // the id used by the game
    unsigned int gameState;  // value from the GAME_STATES enum
    char* gameStateName;
    GAME_STATE gameStateTable[128];  // table of game state ids and names
    size_t gameStateTableSize;

    SCORE_RECORD* scoreRecords;
    size_t scoreRecordsSize;

    MUSIC_ENTRY* musicDb;
    size_t musicDbSize;

    int currentlyPlayingMusicId;
    int currentlyPlayingStage;
    int currentlyPlayingDifficulty;
} MEMORY_DATA;

typedef struct {
    const PATTERN_BYTE* pattern;
    const unsigned char size;
    const unsigned int offset;
    const unsigned char identifier;
    void* addr;
    bool (*transformer)(void** addr);
} SEARCH_PATTERN;

extern MEMORY_DATA MemoryData;

/*
    Looks for and attempts to open a handle to the SDVX process.
    Then finds all the memory addresses for the data we want.
    This function should only be called once unless cleanup is called
    before the second call.
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
/*
    Get the music entry information of the currently
    playing track. Returns a reference to the MUSIC_ENTRY
    object stored in MemoryData, so it may no longer
    be valid once update() is run again.

    If the track cannot be found, the returned pointer
    will be NULL.
*/
MUSIC_ENTRY* memory_reader_get_currently_playing();

// internals
bool read_address(void* buf, size_t* sizeBuf, void* address, size_t size);

#endif  /* MEMORY_READER_H */