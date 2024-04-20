#include "Game.h"
#include "inout/ReadConf.cpp"

using namespace sfSnake;

int main()
{
    // Game game;
    // game.run();

    std::cout << readConf::getPath() << std::endl;

    return 0;
}