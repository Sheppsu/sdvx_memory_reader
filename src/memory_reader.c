#include "memory_reader.h"
#include "decode.h"
#include <tlhelp32.h>
#include <Psapi.h>
#include <stdio.h>
#include <Shlwapi.h>


DWORD currentProcessId = 0;
HANDLE currentProcessHandle;
MODULEINFO currentProcessInfo;
MODULEINFO avs2CoreInfo;

// NOTE: pattern search isn't implemented for base 1 (avs2core) yet
SEARCH_PATTERN searchPatterns[] = {
    {PATTERN_GAME_STATE, 8, 0, 0, PI_GAME_STATE, NULL},
    {NULL, 0, 0x146348, 1, PI_UI_PTR, NULL},
    {NULL, 0, 0x799740, 0, PI_USERDATA_PTR, NULL}
};
const unsigned char PATTERN_COUNT = 3;

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
        SEARCH_PATTERN pattern = searchPatterns[i];
        if (pattern.addr != NULL) continue;
        unfound_patterns++;
        
        size_t bufI = 0;
        size_t patternI = 0;
        while (bufI < bufSize) {
            if (buf[bufI] == pattern.pattern[patternI]) {
                patternI++;
                if (patternI == pattern.size) {
                    searchPatterns[i].addr = (void*)(addr+bufI+1+pattern.offset);
                    unfound_patterns--;
                    printf("Found pattern %d at 0x%p\n", pattern.identifier, searchPatterns[i].addr);
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
        SEARCH_PATTERN pattern = searchPatterns[i];
        if (pattern.pattern == NULL) {
            searchPatterns[i].addr = (char*)(pattern.base == 1 ? avs2CoreInfo.lpBaseOfDll : currentProcessInfo.lpBaseOfDll) + pattern.offset;
            printf("Found pattern %d at 0x%p\n", pattern.identifier, searchPatterns[i].addr);
        }
    }

    // TODO: implement pattern search for avs2core base patterns (if ever needed)
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
bool do_decode(int i) {
    // First strip the text style thingy
    char* uiText = MemoryData.uiObjects[i].text;
    if (MemoryData.uiObjects[i].text[0] == '[') {
        int textI = 0;
        while (textI < 512) {
            textI++;
            if (uiText[textI] == ']' && uiText[textI+1] != '[') {
                break;
            }
        }
        if (textI == 512) {
            printf("Failed to parse text of ui object %s", MemoryData.uiObjects[i].label);
            uiText[0] = '\0';
            return false;
        }
        char temp[512];
        memcpy(temp, uiText+textI+1, 512-textI-1);
        strcpy(uiText, temp);
    }

    // Now decode the stripped text
    char outBuf[512];
    if (!decode_text(MemoryData.uiObjects[i].text, outBuf, 512)) return false;
    strcpy(MemoryData.uiObjects[i].text, outBuf);
    return true;
}

/*
    Populates the items of the MemoryData.UiObjects array with values.
    Returns false if a call to read_address or do_decode fails, else true.
*/
bool populate_ui_objects(void* addr) {
    MemoryData.uiObjCount = 0;
    size_t uiObjPtrs[26];
    size_t sizeBuf;
    if (!read_address(&uiObjPtrs, &sizeBuf, addr, sizeof(size_t)*26)) return true;
    for (int i=0; i<26; i++) {
        // Get UI objects
        // Returns when a null ptr is encountered (list likely ended short of 26 for whatever reason)
        if (uiObjPtrs[i] == 0) return true;
        if (!read_address(MemoryData.uiObjects+i, &sizeBuf, (void*)uiObjPtrs[i], sizeof(UI_OBJECT))) return false;
        if (!do_decode(i)) return false;
        MemoryData.uiObjCount++;
    }
    return true;
}

// docs in memory_reader.h
bool memory_reader_update() {
    // TODO: use char instead of string to identify pattern type
    for (int i=0; i<PATTERN_COUNT; i++) {
        SEARCH_PATTERN pattern = searchPatterns[i];

        size_t sizeBuf;
        switch (pattern.identifier) {
            case PI_GAME_STATE:
                if (!read_address(&MemoryData.gameState, &sizeBuf, pattern.addr, sizeof(unsigned char))) return false;
                continue;
            case PI_UI_PTR:
            {
                if (MemoryData.gameState != STATE_MUSIC_SELECT) continue;
                char* uiPtrs = get_ptr(pattern.addr, 3, 0, 0x174, 0x158);
                if (uiPtrs == NULL) continue;
                if (!populate_ui_objects(uiPtrs+0x118)) return false;
                continue;
            }
            case PI_USERDATA_PTR:
            {
                if (MemoryData.gameState == STATE_STARTUP || MemoryData.gameState == STATE_LOADING || MemoryData.gameState == STATE_TITLE) continue;
                void* ptr = get_ptr(pattern.addr, 1, 0);
                if (ptr == NULL) return false;
                if (!read_address(&MemoryData.userData, &sizeBuf, ptr, sizeof(USERDATA)));
                continue;
            }
        }
    }

    return true;
}

// docs in memory_reader.h
DWORD memory_reader_process_id() {
    return currentProcessId;
}
