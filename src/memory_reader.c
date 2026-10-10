#include "memory_reader.h"
#include "decode.h"
#include <tlhelp32.h>
#include <Psapi.h>
#include <stdio.h>
#include <Shlwapi.h>

const PATTERN_BYTE GAME_STATE_PATTERN[15] = {
    {0x89, false},
    {0x05, false},
    {0x00, true},
    {0x00, true},
    {0x00, true},
    {0x00, true},
    {0x48, false},
    {0x8D, false},
    {0x0D, false},
    {0x00, true},
    {0x00, true},
    {0x00, true},
    {0x00, true},
    {0x89, false},
    {0x2D, false},
};
const PATTERN_BYTE GAME_STATE_TABLE_PATTERN[12] = {
    {0x4C, false},
    {0x8D, false},
    {0x05, false},
    {0x00, true},
    {0x00, true},
    {0x00, true},
    {0x00, true},
    {0x33, false},
    {0xD2, false},
    {0x49, false},
    {0x8B, false},
    {0xC0, false},
};

const GAME_STATE_MAP_ITEM GAME_STATES_MAP[GAME_STATES_MAP_SIZE] = {
    {BOOT, "BOOT"},
    {ONLINEUPDATE, "ONLINEUPDATE"},
    {APPMAINFLOW, "APPMAINFLOW"},
    {TESTMODE_FLOW, "TESTMODE_FLOW"},
    {TITLEDEMO_FLOW, "TITLEDEMO_FLOW"},
    {TOTAL_RESULT_FLOW, "TOTAL_RESULT_FLOW"},
    {PROTOTYPE_FLOW, "PROTOTYPE_FLOW"},
    {SHOPCLOSING, "SHOPCLOSING"},
    {CAUTION, "CAUTION"},
    {GAMEOVER, "GAMEOVER"},
    {CONFIG_VERISON_CHECK, "CONFIG_VERISON_CHECK"},
    {TITLE_SCENE, "TITLE_SCENE"},
    {WARNING_SCENE, "WARNING_SCENE"},
    {MOVIE_SCENE, "MOVIE_SCENE"},
    {RESULT_SCENE, "RESULT_SCENE"},
    {T_RESULT_SCENE, "T_RESULT_SCENE"},
    {MUSICSELECT, "MUSICSELECT"},
    {DEMO_GAME, "DEMO_GAME"},
    {DEMO_TUTORIAL, "DEMO_TUTORIAL"},
    {CARD_ENTRY_SCENE, "CARD_ENTRY_SCENE"},
    {CARD_ENTRY_JOIN_SCENE, "CARD_ENTRY_JOIN_SCENE"},
    {CARD_ENTRY_NEWEAPASS_SCENE, "CARD_ENTRY_NEWEAPASS_SCENE"},
    {CARD_ENTRY_REGIST_SCENE, "CARD_ENTRY_REGIST_SCENE"},
    {CARD_ENTRY_AUTH_SCENE, "CARD_ENTRY_AUTH_SCENE"},
    {CARD_ENTRY_CHECKIN_SCENE, "CARD_ENTRY_CHECKIN_SCENE"},
    {CARD_ENTRY_INFORMATION_SCENE, "CARD_ENTRY_INFORMATION_SCENE"},
    {CARD_ENTRY_MODE_SELECT_SCENE, "CARD_ENTRY_MODE_SELECT_SCENE"},
    {CARD_ENTRY_ERROREND_SCENE, "CARD_ENTRY_ERROREND_SCENE"},
    {CARD_ENTRY_END_SCENE, "CARD_ENTRY_END_SCENE"},
    {NAME_ENTRY_SCENE, "NAME_ENTRY_SCENE"},
    {RADARMAP_SELECT_SCENE, "RADARMAP SELECT SCENE"},
    {CARD_OUT_SCENE, "CARD_OUT_SCENE"},
    {STATION_SCENE, "STATION_SCENE"},
    {SKILL_LEVEL_SELECT, "SKILL LEVEL SELECT"},
    {TUTORIAL, "TUTORIAL"},
    {INFORMATION_SCENE, "INFORMATION SCENE"},
    {IMPOSSIBLE_NOTES_CHECK, "IMPOSSIBLE_NOTES_CHECK"},
    {PROTOTYPE_VIEWER, "PROTOTYPE_VIEWER"},
    {SYSTEM_VOLUME_CHECK_TEST_MENU, "SYSTEM_VOLUME_CHECK_TEST_MENU"},
    {MYROOM_SCENE, "MYROOM_SCENE"},
    {ALTERNATIVE_GAME_SCENE, "ALTERNATIVE_GAME_SCENE"},
    {MEGAMIXBATTLE_SCENE, "MEGAMIXBATTLE_SCENE"},
    {TRAINING_GAME_SCENE, "TRAINING_GAME_SCENE"},
    {MAIN_TEST_MENU, "MAIN_TEST_MENU"},
    {IO_TEST_MENU, "IO_TEST_MENU"},
    {IO_INPUT_CHECK_TEST_MENU, "IO_INPUT_CHECK_TEST_MENU"},
    {IO_CAMERA_CHECK_TEST_MENU, "IO_CAMERA_CHECK_TEST_MENU"},
    {IO_TOUCH_PANEL_CHECK_TEST_MENU, "IO_TOUCH_PANEL_CHECK_TEST_MENU"},
    {IO_LAMP_CHECK_TEST_MENU, "IO_LAMP_CHECK_TEST_MENU"},
    {IO_MECHA_CHECK_TEST_MENU, "IO_MECHA_CHECK_TEST_MENU"},
    {SCREEN_TEST_MENU, "SCREEN_TEST_MENU"},
    {NETWORK_CHECK_MENU, "NETWORK_CHECK_MENU"},
    {SYSTEM_INFO_TEST_MENU, "SYSTEM_INFO_TEST_MENU"},
    {OSS_INFO_TEST_MENU, "OSS_INFO_TEST_MENU"},
    {TOHO_CAMPAIGN_CONFIRM_TEST_MENU, "TOHO_CAMPAIGN_CONFIRM_TEST_MENU"},
    {TOHO_CAMPAIGN_LIST_TEST_MENU, "TOHO_CAMPAIGN_LIST_TEST_MENU"},
    {GOODS_CAMPAIGN_LIST_TEST_MENU, "GOODS_CAMPAIGN_LIST_TEST_MENU"},
    {QC_TEST_MENU, "QC_TEST_MENU"}
};

DWORD currentProcessId = 0;
HANDLE currentProcessHandle;
MODULEINFO currentProcessInfo;
MODULEINFO avs2CoreInfo;

#define PATTERN_SIZE(pattern) sizeof(pattern) / sizeof(PATTERN_BYTE)
#define MAKE_PATTERN(pattern, offset, identifier, transformer) {pattern, PATTERN_SIZE(pattern), offset, identifier, NULL, transformer}
#define MAKE_OFFSET(offset, identifier, transformer) {NULL, 0, offset, identifier, NULL, transformer}

bool read_offset(void** addr, size_t extraOffset) {
    size_t offset = 0;
    size_t sizeRead = 0;
    if (!read_address(&offset, &sizeRead, *addr, 4)) {
        return false;
    }
    
    *addr = (void*)(((size_t)*addr) + extraOffset + offset);

    return true;
}

bool game_state_transformer(void** addr) {
    if (!read_offset(addr, 4)) {
        printf("Read address failed while transforming game state address\n");
        return false;
    }
    return true;
}

bool game_state_table_transformer(void** addr) {
    if (!read_offset(addr, 4)) {
        printf("Read address failed while transforming game state address\n");
        return false;
    }
    return true;
}

SEARCH_PATTERN searchPatterns[] = {
    MAKE_PATTERN(GAME_STATE_PATTERN, 2, PI_GAME_STATE, game_state_transformer),
    MAKE_PATTERN(GAME_STATE_TABLE_PATTERN, 3, PI_STATE_TABLE, game_state_table_transformer),
};
const unsigned char PATTERN_COUNT = sizeof(searchPatterns) / sizeof(SEARCH_PATTERN);

MEMORY_DATA MemoryData;


/* 
    Get module information from a module handle.
    Return false if GetModuleInformation fails, else true.
*/
bool get_module_info(HMODULE moduleHandle, MODULEINFO* output) {
    if (GetModuleInformation(currentProcessHandle, moduleHandle, output, sizeof(MODULEINFO)) == 0) {
        printf("Failed to get module information with error code %d\n", GetLastError());
        return false;
    }
    return true;
}

/*
    Get needed module info (for sv6c.exe and avs2-core.dll).
    Return false if any of the steps fail, else true.
*/
bool init_module_info() {
    if (currentProcessHandle == NULL) {
        printf("Attempted to get module info before opening process\n");
        return false;
    }

    DWORD lpcbNeeded;
    if (EnumProcessModules(currentProcessHandle, NULL, 0, &lpcbNeeded) == 0) {
        printf("First EnumProcessModules call failed with error code %d\n", GetLastError());
        return false;
    }
    HMODULE* modules = malloc(lpcbNeeded);
    if (EnumProcessModules(currentProcessHandle, modules, lpcbNeeded, &lpcbNeeded) == 0) {
        printf("Second EnumProcessModules call failed with error code %d\n", GetLastError());
        free(modules);
        return false;
    }

    DWORD moduleCount = lpcbNeeded / sizeof(HMODULE);
    for (int i=0; i<moduleCount; i++) {
        char modFilePath[MAX_PATH];
        if (GetModuleFileNameExA(currentProcessHandle, modules[i], modFilePath, MAX_PATH) == 0) {
            printf("Failed to get module file name with error code %d\n", GetLastError());
            free(modules);
            return false;
        }
        PathStripPathA(modFilePath);

        if (strcmp(modFilePath, "sv6c.exe") == 0) {
            if (!get_module_info(modules[i], &currentProcessInfo)) {
                free(modules);
                return false;
            }
        } else if (strcmp(modFilePath, "avs2-core.dll") == 0) {
            if (!get_module_info(modules[i], &avs2CoreInfo)) {
                free(modules);
                return false;
            }
        }

        if (currentProcessInfo.lpBaseOfDll != NULL && avs2CoreInfo.lpBaseOfDll != NULL) {
            free(modules);
            return true;
        }
    }

    free(modules);

    printf("Unable to get module info for sv6c.exe and avs2-core.dll");
    return false;
}

/*
    Sets currentProcessId if it succeeds in finding the process.
    Returns false when any winapi calls fail or the process is not found, else true.
*/
bool set_current_process(char* processName) {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        printf("CreateToolhelp32Snapshot failed with error code %d\n", GetLastError());
        return false;
    }

    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);

    BOOL hResult = Process32First(hSnapshot, &pe);
    while (hResult) {
        if (strcmp(processName, pe.szExeFile) == 0) {
            currentProcessId = pe.th32ProcessID;
            break;
        }
        hResult = Process32Next(hSnapshot, &pe);
    }

    CloseHandle(hSnapshot);

    if (currentProcessId == 0) {
        printf("Failed to find process named \"%s\"\n", processName);
        return false;
    }
    return true;
}

/*
    Gets a handle to the current process (from currentProcessId).
    Returns false if current process is not set or OpenProcess fails, else true.
*/
bool open_process() {
    if (currentProcessId == 0) {
        printf("Attempted to open a process before setting the current process id\n");
        return false;
    }

    currentProcessHandle = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, 0, currentProcessId);
    if (currentProcessHandle == NULL) {
        printf("Process opening failed with error code %d\n", GetLastError());
        return false;
    }
    
    return true;
}

/*
    Closes handle to the current process (from currentProcessHandle).
    Returns false if currentProcessHandle is NULL or CloseHandle fails, else true.
*/
bool close_process() {
    if (currentProcessHandle == NULL) {
        printf("Attempted to close a process which is not open\n");
        return false;
    }

    if (!CloseHandle(currentProcessHandle)) {
        printf("Process handle closing failed with error code %d\n", GetLastError());
        return false;
    }

    return true;
}

/*
    Get information about an address page.
    If error 87 is raised by VirtualQueryEx, finished is set to true.
    Error 87 indicates that the last page has been reached.
    Returns false when an error is raised (including 87), else true.
*/
bool query_page(MEMORY_BASIC_INFORMATION* info, char* address, bool* finished) {
    if (VirtualQueryEx(currentProcessHandle, address, info, sizeof(MEMORY_BASIC_INFORMATION)) == 0) {
        DWORD err = GetLastError();
        if (err == 87) {
            *finished = true;
            return false;
        }
        printf("Page query failed with error code %d\n", err);
        return false;
    }
    return true;
}

/*
    Reads memory from an address into a buffer.
    Returns false if ReadProcessMemory fails, else true.
*/
bool read_address(void* buf, size_t* sizeBuf, void* address, size_t size) {
    if (ReadProcessMemory(currentProcessHandle, address, buf, size, sizeBuf)) {
        return true;
    }
    printf("Memory read failed with error code %d\n", GetLastError());
    return false;
}

/*
    Read a page and set the address for any patterns that match.
    Returns number of unfound patterns left or 0xFF when read_memory fails.
*/
unsigned char search_page(MEMORY_BASIC_INFORMATION info, char* addr) {
    char* buf = malloc(info.RegionSize);
    size_t bufSize;
    if (!read_address((void*)buf, &bufSize, addr, info.RegionSize)) {
        return 0xFF;
    }

    unsigned char unfound_patterns = 0;
    for (int i=0; i<PATTERN_COUNT; i++) {
        SEARCH_PATTERN* search = &searchPatterns[i];
        if (search->addr != NULL) continue;
        unfound_patterns++;
        
        size_t bufI = 0;
        size_t patternI = 0;
        while (bufI < bufSize) {
            if (search->pattern[patternI].wild || buf[bufI] == search->pattern[patternI].byte) {
                patternI++;
                if (patternI == search->size) {
                    // set addr to start of pattern
                    search->addr = (void*)(addr+bufI-patternI+1+search->offset);
                    if (search->transformer != NULL) {
                        if (!search->transformer(&search->addr)) {
                            return 0xFF;
                        }
                    }
                    unfound_patterns--;
                    printf("Found pattern %d at 0x%p\n", search->identifier, searchPatterns[i].addr);
                    break;
                }
            } else {
                patternI = 0;
            }
            bufI++;
        }
    }

    free(buf);
    return unfound_patterns;
}

/*
    Query through all pages and execute callback for readable pages.
    Returns false when a call to query_page fails or all the patterns were not found.
    Returns true when all patterns have been found.
*/
bool init_patterns() {
    if (&currentProcessInfo == NULL) {
        printf("Attempted to traverse pages before setting currentProcessInfo\n");
        return false;
    }

    for (int i=0; i<PATTERN_COUNT; i++) {
        SEARCH_PATTERN* search = &searchPatterns[i];
        if (search->pattern == NULL) {
            search->addr = (char*)currentProcessInfo.lpBaseOfDll + search->offset;
            if (search->transformer != NULL) {
                if (!search->transformer(&search->addr)) {
                    return false;
                }
            }
            printf("Initialized pattern %d at 0x%p\n", search->identifier, search->addr);
        }
    }

    char* addr = currentProcessInfo.lpBaseOfDll;
    MEMORY_BASIC_INFORMATION info;
    bool finished = false;
    
    while (true) {
        if (!query_page(&info, addr, &finished)) return false;
        if (finished) break;
        addr = info.BaseAddress;
        if (!(info.Protect & PAGE_NOACCESS || info.Protect & PAGE_GUARD || info.Protect == 0)) {
            unsigned char result = search_page(info, addr);
            if (result == 0xFF) return false;
            if (result == 0) return true;
        }
        addr += info.RegionSize;
        if ((size_t)addr >= (size_t)currentProcessInfo.lpBaseOfDll + currentProcessInfo.SizeOfImage) {
            break;
        }
    }
    return false;
}

// docs in memory_reader.h
bool memory_reader_init() {
    printf("Searching for process\n");
    if (!set_current_process("sv6c.exe")) return false;
    printf("Opening process\n");
    if (!open_process()) return false;
    printf("Getting module information\n");
    if (!init_module_info()) return false;
    printf("Searching pages for wanted memory data\n");
    if (!init_patterns()) return false;

    MemoryData.gameState = 0;
    MemoryData.gameStateTableSize = 0;
    MemoryData.gameStateName = "\0";

    printf("Memory reader finished initiating\n");
    return true;
}

// docs in memory_reader.h
bool memory_reader_cleanup() {
    printf("Cleaning up\n");
    memset(&MemoryData, 0, sizeof(MEMORY_DATA));
    if (!close_process()) return false;
    printf("Finished cleaning");
    return true;
}

/*
    Traverse a sequence of pointers + offsets and return the end address.
    Returns NULL if a call to read_address fails.
*/
void* get_ptr(void* baseAddr, int num, ...) {
    va_list args;
    va_start(args, num);

    size_t from = (size_t)baseAddr;
    size_t to;
    size_t sizeBuf;
    for (int i=0; i<num; i++) {
        if (!read_address(&to, &sizeBuf, (void*)(from+va_arg(args, int)), sizeof(size_t))) return NULL;
        if (to == 0) return NULL;
        from = to;
    }

    return (void*)from;
}

/*
    Decode text of a certain ui object (denoted by index i).
    Returns false if a call to decode_text fails or it fails to parse the text, else true.
*/
// bool do_decode(int i) {
//     // First strip the text style thingy
//     char* uiText = MemoryData.uiObjects[i].text;
//     if (MemoryData.uiObjects[i].text[0] == '[') {
//         int textI = 0;
//         while (textI < 512) {
//             textI++;
//             if (uiText[textI] == ']' && uiText[textI+1] != '[') {
//                 break;
//             }
//         }
//         if (textI == 512) {
//             printf("Failed to parse text of ui object %s", MemoryData.uiObjects[i].label);
//             uiText[0] = '\0';
//             return false;
//         }
//         char temp[512];
//         memcpy(temp, uiText+textI+1, 512-textI-1);
//         strcpy(uiText, temp);
//     }

//     // Now decode the stripped text
//     char outBuf[512];
//     if (!decode_text(MemoryData.uiObjects[i].text, outBuf, 512)) return false;
//     strcpy(MemoryData.uiObjects[i].text, outBuf);
//     return true;
// }

// docs in memory_reader.h
bool memory_reader_update() {
    // TODO: use char instead of string to identify pattern type
    for (int i=0; i<PATTERN_COUNT; i++) {
        SEARCH_PATTERN pattern = searchPatterns[i];

        size_t sizeBuf;
        switch (pattern.identifier) {
            case PI_GAME_STATE:
                if (!read_address(&MemoryData.gameStateId, &sizeBuf, pattern.addr, sizeof(unsigned char))) return false;
                continue;
            case PI_STATE_TABLE:
            {
                // only need to read this once
                if (MemoryData.gameStateTableSize > 0) {
                    continue;
                }

                unsigned char buf[0x158];
                while (true) {
                    void* addr = (void*)(((size_t) pattern.addr) + (0x158 * MemoryData.gameStateTableSize));
                    if (!read_address(buf, &sizeBuf, addr, sizeof(buf))) return false;

                    if (((unsigned int*)(buf + 0x4))[0] != 0x4) {
                        break;
                    }

                    GAME_STATE* gameState = &MemoryData.gameStateTable[MemoryData.gameStateTableSize];

                    gameState->id = ((unsigned int*)buf)[0];
                    void* namePtr = ((void**) (buf + 0x10))[0];
                    if (!read_address(gameState->name, &sizeBuf, namePtr, 64)) return false;
                    MemoryData.gameStateTableSize++;
                }
            }
        }
    }

    // set game state enum based on game state id
    bool foundState = false;
    for (int i=0; i<MemoryData.gameStateTableSize; i++) {
        if (MemoryData.gameStateTable[i].id == MemoryData.gameStateId) {
            MemoryData.gameStateName = MemoryData.gameStateTable[i].name;
            foundState = true;
            break;
        }
    }
    if (!foundState) {
        MemoryData.gameStateName = "UNKNOWN";
        MemoryData.gameState = GAME_STATE_UNKNOWN;
    } else {
        for (int i=0; i<GAME_STATES_MAP_SIZE; i++) {
            if (!strcmp(MemoryData.gameStateName, GAME_STATES_MAP[i].name)) {
                MemoryData.gameState = GAME_STATES_MAP[i].value;
            }
        }
    }

    return true;
}

// docs in memory_reader.h
DWORD memory_reader_process_id() {
    return currentProcessId;
}
