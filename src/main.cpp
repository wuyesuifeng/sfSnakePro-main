// #include "Game.h"
#include "inout/ShareMemory.h"
#include "inout/ReadConf.cpp"
#include <unistd.h>

using namespace std;

int main()
{

    char *path = utils::getPath();
    utils::ShareMemory share = utils::ShareMemory(path);
    free(path);
    int *in = share.getReadPos(),
        *out = share.getWritePos();

    while (true) {
        sleep(1);
        int pos = *in, pos2 = *(in + 1);
        if (pos == 8) {
            cout << pos << endl;
        }
        if (pos2 == 6) {
            cout << pos2 << endl;
        }
        *out = 58;
        *(out + 1) = 16;
        in = share.getReadPos();
        out = share.getWritePos();
    }
        
    // sfSnake::Game game;
    // game.run();

    return 0;
}