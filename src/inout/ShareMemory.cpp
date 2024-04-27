#include "ShareMemory.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <unistd.h>
#include <iostream>
#include <string.h>

#define READ_LEN 3
#define READ_SIZE sizeof(int) * READ_LEN
#define WRITE_SIZE 1024
#define ME_PROJECT_ID 1
#define FLAG IPC_CREAT | 0777

using namespace utils;

ShareMemory::ShareMemory(char *xyExecFile) {
    int writeKey, readKey;

    char me_path[128];
    getcwd(me_path, sizeof(me_path) - 1);

    if ((writeKey = ftok(me_path, ME_PROJECT_ID)) == -1) {
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

    *readPos = READ_LEN;
}

ShareMemory::~ShareMemory() {
    if (shmdt(readPos) == -1) {
        printErr("shmdt read memory failed");
    }

    if (shmctl(readId, IPC_RMID, 0) == -1) {
        printErr("delete read memory failed");
    }

    if (shmdt(writePos) == -1) {
        printErr("shmdt write memory failed");
    }

    if (shmctl(writeId, IPC_RMID, 0) == -1) {
        printErr("delete write memory failed");
    }
}

int* ShareMemory::getReadPos() {
    return readPos;
}

int* ShareMemory::getWritePos() {
    return writePos;
}