#include "ShareMemory.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <unistd.h>
#include <iostream>
#include <string.h>

#define READ_SIZE 512
#define WRITE_SIZE 1024
#define ME_PROJECT_ID 1
#define FLAG IPC_CREAT | 0777
#define ME_EXEC_FILE "/proc/self/exe"

using namespace shm;

ShareMemory::ShareMemory(char *xyExecFile) {
    int writeKey, readKey;

    if ((writeKey = ftok(ME_EXEC_FILE, ME_PROJECT_ID)) == -1) {
        throw "ftoke writeKey failed";
    }

    if ((readKey = ftok(xyExecFile, ME_PROJECT_ID)) == -1) {
        throw "ftoke readKey failed";
    }

    if ((writeId = shmget(writeKey, WRITE_SIZE, FLAG)) == -1) {
        throw "shmget writeId failed";
    }

    if ((readId = shmget(readKey, READ_SIZE, FLAG)) == -1) {
        throw "shmget readId failed";
    }

    if ((writePos = (int*) shmat(writeId, NULL, 0)) == nullptr) {
        throw "shmat writePos failed";
    }

    if ((readPos = (int*) shmat(readId, NULL, 0)) == nullptr) {
        throw "shmat readPos failed";
    }
}

ShareMemory::~ShareMemory() {
    if (shmdt(readPos) == -1) {
        throw "shmdt read memory failed";
    }

    if (shmctl(readId, IPC_RMID, 0) == -1) {
        throw "delete read memory failed";
    }

    if (shmdt(writePos) == -1) {
        throw "shmdt write memory failed";
    }

    if (shmctl(writeId, IPC_RMID, 0) == -1) {
        throw "delete write memory failed";
    }
}

int* ShareMemory::getReadPos() {
    return readPos;
}

int* ShareMemory::getWritePos() {
    return writePos;
}