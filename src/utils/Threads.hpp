#pragma once

#include <thread>

namespace utils {
    class Threads {
        private:
            int cnt;
            int currentCnt;
            std::thread **threads;
            bool *threadsStatus;
            void join(int i);
        public:
            ~Threads();
            void init(int threadsCnt);
            void join();
            void add(std::thread *t) {
                threads[currentCnt] = t;
                threadsStatus[currentCnt] = true;
            }
            int getThreadsCnt() {
                return cnt;
            }
            void resetCurrentCnt() {
                currentCnt = 0;
            }
            void addCurrentCnt() {
                currentCnt++;
            }
            bool isFull() {
                return cnt == currentCnt;
            }
    };
    

    template<typename _Callable, typename... _Args>
    void addThread(Threads &threads, _Callable&& __f, _Args&&... __args) {
        if (threads.isFull()) {
            threads.join();
        }
        threads.add(new std::thread(__f, __args...));
        threads.addCurrentCnt();
    }
}