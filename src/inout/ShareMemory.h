#pragma once

#include <iostream>
#include <string.h>

namespace utils {
    class ShareMemory {
        public:
            ShareMemory(char *xyExecFile);
            ~ShareMemory();
            int* getReadPos();
            int* getWritePos();
        private:
            int *writePos;
            int *readPos;
            int writeId;
            int readId;
    };

    static void printErr(char *val) {
        std::cout << val << std::endl;
    }
}