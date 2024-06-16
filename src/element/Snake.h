#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <deque>

#include "Fruit.h"

#define PI 3.14159265358979323846f
#define VISION_X_SUM 20
#define VISION_Y_SUM 40
#define VISION_PIXEL_WIDTH 10.0f
#define VISION_DEF_COLOR 0x55c40f99
#define VISION_CHECK_COLOR 0xee60ec99
#define VISION_HARM_COLOR 0xf0321d99

namespace sfSnake
{
    typedef sf::Vector2f Direction;
    typedef sf::Vector2f SnakePathNode;

    struct vision {
        sf::Vector2f pos;
        sf::Uint32 color;
    };

    class Snake
    {
    public:
        Snake();

        void handleInput(sf::RenderWindow &window);
        void handleInput(sf::Vector2i mousePosition, sf::RenderWindow &window);
        void update(sf::Time delta);
        void render(sf::RenderWindow &window);

        void checkFruitCollisions(std::deque<Fruit> &fruits);

        bool hitSelf() const;

        unsigned getScore() const;

        void printhead() const;

        void grow(int score);

    private:
        void initNodes();
        void move();

        void checkOutOfWindow();
        void checkSelfCollisions();
        void look();

        template <typename T>
        void renderNode(sf::Vector2f &nowPosition, T &shape, sf::RenderWindow &window, int offset);

        SnakePathNode toWindow(SnakePathNode &node, SnakePathNode dir, float radian);

        SnakePathNode toWindow(SnakePathNode &node, SnakePathNode dir, float radian, float sin, float cos, int num, sf::Vector2f head);

        bool hitSelf_;
        bool speedup_;

        Direction direction_;
        float nodeRadius_;
        std::deque<SnakePathNode> path_;
        vision vision_[VISION_X_SUM][VISION_Y_SUM];
        int tailOverlap_;

        sf::CircleShape nodeShape;
        sf::RectangleShape nodeMiddle;
        sf::Texture headTexture;
        sf::Sprite headSprite;

        static const int InitialSize;
        unsigned int score_;

        sf::SoundBuffer pickupBuffer_;
        sf::Sound pickupSound_;

        sf::SoundBuffer dieBuffer_;
        sf::Sound dieSound_;

        int *in = nullptr, *out = nullptr;
        int hisX, hisY;
    };
}