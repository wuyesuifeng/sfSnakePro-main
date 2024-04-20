#pragma once

#include <iostream>
#include <string.h>

namespace shm {
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
}