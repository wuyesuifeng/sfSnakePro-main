#pragma once

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <deque>

#include "Fruit.h"
#include "def.h"
#include "utils/ReadConf.hpp"
#include "utils/ShareMemory.h"
#include "utils/Threads.hpp"

#define PI 3.14159265358979323846f
#define VISION_X_SUM Game::cfg.visionXSum
#define VISION_Y_SUM Game::cfg.visionYSum
#define VISION_PIXEL_WIDTH 10.0f
#define VISION_DEF_COLOR 0x55c40f99
#define VISION_CHECK_COLOR 0xee60ec99
#define VISION_HARM_COLOR 0xf0321d99

#define ADD_SHARE_INDEX(index, fillCount) \
    do {                                  \
        if (index < fillCount) {          \
            index++;                      \
        } else {                          \
            index = 0;                    \
        }                                 \
    } while (0)

namespace sfSnake {
    typedef sf::Vector2f Direction;
    typedef sf::Vector2f SnakePathNode;

    struct vision {
        sf::Vector2f pos;
        sf::Uint32 color;
    };

    class Snake {
        public:
         Snake();

         ~Snake();

         void handleInput(sf::RenderWindow &window);
         void handleInput(sf::Vector2i mousePosition, sf::RenderWindow &window);
         void update(sf::Time delta);
         void render(sf::RenderWindow &window);

         void reset();

         void checkFruitCollisions(std::deque<Fruit> &fruits);

         bool hitSelf() const;

         unsigned getScore() const;

         void printhead() const;

         void grow(int score);

        private:
            utils::Threads threads_;
            void initNodes();
            void move(SnakePathNode headNode);

            void checkOutOfWindow();
            void checkSelfCollisions(SnakePathNode head);
            void look(SnakePathNode head);

            template <typename T>
            void renderNode(sf::Vector2f &nowPosition, T &shape, sf::RenderWindow &window,
                            int offset);

            bool toWindow(SnakePathNode &node, SnakePathNode dir, float radian);

            bool toWindow(SnakePathNode &node, SnakePathNode dir, float radian,
                          float sin, float cos, int num, sf::Vector2f head);

            long health_;
            TYPE_VOL *deathFlag_;

            unsigned int visionBodyPos_, visionFruitPos_;
            TYPE_VITALITY minVitality_;
            bool death_;

            bool hitSelf_;
            char turnDirection_;
            UPPER_TYPE_VOL pain_, delight_;
            // bool speedup_;
            short int speed_;

            float angle_, hisAngle_, bodyDir_, headAngle_, radian_, turnLeft_, turnRight_,
                stuckLeft_, stuckRight_, leftVitality_, rightVitality_;

            unsigned int shareIndex_, delightIndex_, painIndex_;

            Direction direction_;
            float nodeRadius_;
            std::deque<SnakePathNode> path_;
            vision *vision_;
            int tailOverlap_;

            sf::CircleShape nodeShape_;
            sf::RectangleShape nodeMiddle_;
            sf::Texture headTexture_;
            sf::Sprite headSprite_;
            int snakeLen_;
            unsigned int score_;

            sf::SoundBuffer pickupBuffer_;
            sf::Sound pickupSound_;

            sf::SoundBuffer dieBuffer_;
            sf::Sound dieSound_;

            TYPE_VOL *in_ = nullptr, *out_ = nullptr;
    };
}  // namespace sfSnake