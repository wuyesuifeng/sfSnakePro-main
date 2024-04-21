#include "Game.h"
#include "inout/ShareMemory.h"
#include "inout/ReadConf.cpp"

int main()
{

    char *path = readConf::getPath();
    shm::ShareMemory share = shm::ShareMemory(path);
    free(path);
    int *in = share.getReadPos(),
        *out = share.getWritePos();
        
    sfSnake::Game game;
    game.run();

    return 0;
}