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
#define VISION_PIXEL_WIDTH 10.0f
#define VISION_DEF_COLOR 0x55c40f99
#define VISION_CHECK_COLOR 0xee60ec99
#define VISION_HARM_COLOR 0xf0321d99

namespace sfSnake {
    typedef sf::Vector2f Direction;
    typedef sf::Vector2f SnakePathNode;

    class el_triangle {
        public:
            sf::ConvexShape triangle;
            double distance;
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

        void setAngle();

    private:
        utils::Threads threads_;
        void initNodes();
        void move(SnakePathNode headNode);

        void checkOutOfWindow();
        void checkSelfCollisions();
        void lookSelf();
        void look(float distance, SnakePathNode pos);

        template <typename T>
        void renderNode(sf::Vector2f &nowPosition, T &shape, sf::RenderWindow &window,
                        int offset);

        float health_, healthVal_;
        TYPE_VOL *deathFlag_;

        unsigned int visionSum_, windowSize_, halfWindowSize_, windowDiameter_, windowRadius_, windowRadiusPow_;

        float visionAngle_, halfVisionAngle_, visionElAngle_, visionDistance_;

        bool death_,
            hitSelf_,
            visionOutOfBounds_;

        TYPE_VOL stuckLeft_, stuckRight_, eatLeft_, eatRight_;
        // bool speedup_;
        short int speed_;

        float angle_, angleABS_, angleHis_, bodyDir_, headAngle_, headAngleHis_, radian_, turnLeft_, turnRight_, radianToAngle_;

        Direction direction_;
        float nodeRadius_, nodeRadius2_, nodeDiameter_;
        std::deque<SnakePathNode> path_;
        el_triangle *visionTriangle_, *visionTriangleEnd_;
        double elAngleRange_;
        SnakePathNode *headPos_, headOutPos_, centerPos_;
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
} // namespace sfSnake