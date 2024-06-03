#pragma once

#include <iostream>
#include <string.h>
#include <string>

namespace utils {
    class ShareMemory {
        public:
            ShareMemory(char *xyExecFile);
            ~ShareMemory();
            int* getReadPos();
            int* getWritePos();
        private:
            int *writePos = nullptr;
            int *readPos = nullptr;
            int writeId;
            int readId;
    };

    static void printErr(std::string val) {
        std::cout << val << std::endl;
    }
}