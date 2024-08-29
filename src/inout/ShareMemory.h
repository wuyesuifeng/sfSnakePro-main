#pragma once

#include <iostream>
#include <string>

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