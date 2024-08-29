#pragma once

#include <iostream>
#include <string>

#define READ_P_LEN 4
#define READ_LEN 8
#define WRITE_LEN 802

namespace utils {
    class ShareMemory {
        public:
            ShareMemory(char *xyExecFile);
            ~ShareMemory();
            char* getReadPos();
            char* getWritePos();
        private:
            char *writePos = nullptr;
            char *readPos = nullptr;
            int writeId;
            int readId;
    };

    static void printErr(std::string val) {
        std::cout << val << std::endl;
    }
}