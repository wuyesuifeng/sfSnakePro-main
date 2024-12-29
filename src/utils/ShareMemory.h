#pragma once

#include <iostream>
#include <string>

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#define getcwd _getcwd
#else
#include <sys/shm.h>
#include <unistd.h>
#endif

#define READ_P_LEN 2
#define READ_LEN 5 + 1
#define VISION_CHECK_POS 800
#define VISION_HARM_POS 1600
#define WRITE_LEN 2408 + 1

namespace utils {
    class ShareMemory {
        public:
            ShareMemory(char *xyExecFile);
            ~ShareMemory();
            unsigned char* getReadPos();
            unsigned char* getWritePos();
        private:
            unsigned char *writePos = nullptr;
            unsigned char *readPos = nullptr;
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