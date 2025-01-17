#include "Threads.hpp"
#include <cstdlib>

using namespace utils;

void Threads::init(int threadsCnt) {
    cnt = threadsCnt;
    resetCurrentCnt();
    threads = (std::thread **) malloc(sizeof(std::thread) * cnt);
    threadsStatus = (bool *) malloc(sizeof(bool) * cnt);
    for (int i = 0; i < cnt; i++) {
        threadsStatus[i] = false;
    }
}

Threads::~Threads() {
    join();
    free(threads);
}

void Threads::join() {
    for (int i = 0; i < cnt; i++) {
        join(i);
    }
    resetCurrentCnt();
}

void Threads::join(int i) {
    std::thread *t = threads[i];
    bool &status = threadsStatus[i];
    if (status && t -> joinable()) {
        t -> join();
        status = false;
        delete t;
    }
}