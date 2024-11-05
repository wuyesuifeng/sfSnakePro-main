#pragma once

#include <iostream>
#include <string>

#define READ_P_LEN 2
#define READ_LEN 5 + 1
#define VISION_CHECK_POS 800
#define VISION_HARM_POS 1600
#define WRITE_LEN 2402 + 1

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
            int writeId;
            int readId;
    };

    static void printErr(std::string val) {
        std::cout << val << std::endl;
    }
}