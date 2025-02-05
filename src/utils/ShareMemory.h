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

#define READ_LEN 3 + 2
#define SHARE_DATA_TYPE unsigned int

namespace utils {
    class ShareMemory {
        public:
            ShareMemory(char *xyExecFile, size_t writeSize);
            ~ShareMemory();
            unsigned char* getReadPos();
            unsigned char* getWritePos();
        private:
            unsigned char *writePos = nullptr;
            unsigned char *readPos = nullptr;
            int readSize;
            int writeSize;
            int visionFruitPos;
            int visionBodyPos;
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