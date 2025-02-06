#include "ShareMemory.h"
#include <stdio.h>
#include <stdlib.h>
#ifdef __linux
#include <cstring>
#endif

#define READ_SIZE sizeof(SHARE_DATA_TYPE) * READ_LEN
#define ME_PROJECT_ID 1
#define FLAG IPC_CREAT | 0777

using namespace utils;

ShareMemory::ShareMemory(char *xyExecFile, size_t writeSize) {
    int writeKey, readKey;

    char *tmp = NULL;
    tmp = getcwd(NULL, 0);
    const int len = 8 + strlen(tmp);
    char *me_path;
    me_path = (char *) calloc(len, len * sizeof(char));
    memcpy(me_path, tmp, len);
    free(tmp);

#ifdef _WIN32
    for (int i = 0; i < 128; i++) {
        if (me_path[i] == '\\') {
            me_path[i] = '/';
        } else if (!me_path[i]) {
            me_path[i] = '/';
            me_path[i + 1] = 'r';
            me_path[i + 2] = 'u';
            me_path[i + 3] = 'n';
            me_path[i + 4] = '.';
            me_path[i + 5] = 'e';
            me_path[i + 6] = 'x';
            me_path[i + 7] = 'e';
            break;
        }
    }

    puts(me_path);
    puts(xyExecFile);

    // ERROR_INVALID_HANDLE
    // 创建共享文件句柄 
	write = OpenFileMapping(FILE_MAP_ALL_ACCESS, true, me_path);

    if (!write) {
        puts("CreateFileMapping snake");
        // printf("\tme_path[61]: %d\n", me_path[61]);
        SetLastError(0);
        write = CreateFileMapping(
            INVALID_HANDLE_VALUE,   // 物理文件句柄
            NULL,   // 默认安全级别
            PAGE_READWRITE,   // 可读可写
            0,   // 高位文件大小
            writeSize,   // 低位文件大小
            me_path   // 共享内存名称
        );
        if (GetLastError()) {
            printf("\tCreateFileMapping write_path err: %d\n", GetLastError());
            throw "CreateFileMapping write_path failed";
        }
    }

    read = OpenFileMapping(FILE_MAP_ALL_ACCESS, true, xyExecFile);
    if (!read) {
        puts("CreateFileMapping ai");
        // printf("\txyExecFile[53]: %d\n", xyExecFile[53]);
        SetLastError(0);
        read = CreateFileMapping(
            INVALID_HANDLE_VALUE,   // 物理文件句柄
            NULL,   // 默认安全级别
            PAGE_READWRITE,   // 可读可写
            0,   // 高位文件大小
            READ_SIZE,   // 低位文件大小
            xyExecFile   // 共享内存名称
        );
        if (GetLastError()) {
            printf("\tCreateFileMapping read_path err: %d\n", GetLastError());
            throw "CreateFileMapping read_path failed";
        }
    }

    SetLastError(0);
    writePos = (SHARE_DATA_TYPE *) MapViewOfFile(
		write,            // 共享内存的句柄
		FILE_MAP_ALL_ACCESS, // 可读写许可
		0,
		0,
		0 		 // 填写 BIG_BUF_SIZE
	);

    if (GetLastError()) {
        printf("\tMapViewOfFile writePos err: %d\n", GetLastError());
        throw "MapViewOfFile writePos failed";
    }

    SetLastError(0);
    readPos = (SHARE_DATA_TYPE *) MapViewOfFile(
		read,            // 共享内存的句柄
		FILE_MAP_ALL_ACCESS, // 可读写许可
		0,
		0,
		0		 // 填写 BIG_BUF_SIZE
	);

    if (GetLastError()) {
        printf("\tMapViewOfFile readPos err: %d\n", GetLastError());
        throw "MapViewOfFile readPos failed";
    }
#else
    printErr(me_path);
    printErr(xyExecFile);

    if ((writeKey = ftok(me_path, ME_PROJECT_ID)) == -1) {
        throw "ftoke writeKey failed";
    }

    if ((readKey = ftok(xyExecFile, ME_PROJECT_ID)) == -1) {
        throw "ftoke readKey failed";
    }

    if ((writeId = shmget(writeKey, writeSize, FLAG)) == -1) {
        throw "shmget writeId failed";
    }

    if ((readId = shmget(readKey, READ_SIZE, FLAG)) == -1) {
        throw "shmget readId failed";
    }

    if ((writePos = (SHARE_DATA_TYPE *)shmat(writeId, NULL, 0)) == nullptr) {
      throw "shmat writePos failed";
    }

    if ((readPos = (SHARE_DATA_TYPE *)shmat(readId, NULL, 0)) == nullptr) {
      throw "shmat readPos failed";
    }
#endif

    free(me_path);

    *writePos = 1;
    *readPos = 1;
}

ShareMemory::~ShareMemory() {

    if (*readPos) {

        *writePos = 0;

#ifdef _WIN32
        UnmapViewOfFile(writePos);
        UnmapViewOfFile(readPos);
        CloseHandle(write);
        CloseHandle(read);
#else
        if (shmdt(readPos) == -1) {
            printErr("shmdt read memory failed");
        }

        if (shmdt(writePos) == -1) {
            printErr("shmdt write memory failed");
        }

        if (shmctl(readId, IPC_RMID, 0) == -1) {
            printErr("delete read memory failed");
        }

        if (shmctl(writeId, IPC_RMID, 0) == -1) {
            printErr("delete write memory failed");
        }
#endif
    }
}

SHARE_DATA_TYPE *ShareMemory::getReadPos() { return readPos + 1; }

SHARE_DATA_TYPE *ShareMemory::getWritePos() { return writePos + 1; }