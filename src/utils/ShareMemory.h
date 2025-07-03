#pragma once

#include <iostream>
#include <string>
#include "def.h"

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#define getcwd _getcwd
#else
#include <sys/shm.h>
#include <unistd.h>
#endif

namespace utils {
    class ShareMemory {
        public:
            ShareMemory(char *xyExecFile, unsigned int readCnt, unsigned int writeCnt);
            ~ShareMemory();
            TYPE_VOL* getReadPos();
            TYPE_VOL* getWritePos();
        private:
            TYPE_VOL *writePos = nullptr;
            TYPE_VOL *readPos = nullptr;
#ifdef _WIN32
            HANDLE read;
            HANDLE write;
#else
            int writeId;
            int readId;
#endif
    };

    static void printErr(std::string val) {
        std::cout << val << std::endl;
    }
}