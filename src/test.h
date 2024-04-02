#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#include <vector>
#include <memory>
#include <cmath>

#include "screen/Screen.h"
#include "element/TitleSprite.h"

namespace sfSnake
{
    class Game
    {
    public:
        Game();
        ~Game() = default;

        void run();

    private:
        void handleInput();
        void update(sf::Time delta);
        void render();
    };

    /**
     * @brief set any shape's origin to its middle.
     * @param shape any shape
     *
     * @code {.c++}
     * setOriginMiddle(titleSprite_);
     * @endcode
     *
     * @return sf::FloatRect shapeBounds
     *
     */
    template <typename T>
    inline sf::FloatRect setOriginMiddle(T &shape)
    {
        sf::FloatRect shapeBounds = shape.getLocalBounds();
        shape.setOrigin(shapeBounds.left + shapeBounds.width / 2,
                        shapeBounds.top + shapeBounds.height / 2);
        return shapeBounds;
    }

    /**
     * @brief calculate the distance between two nodes
     *
     * @param node sf::Vector2
     *
     * @return double
     */
    template <typename T1, typename T2>
    inline double dis(
        sf::Vector2<T1> node1,
        sf::Vector2<T2> node2) noexcept
    {
        return std::sqrt(
            std::pow(
                (
                    static_cast<double>(node1.x) -
                    static_cast<double>(node2.x)),
                2) +
            std::pow(
                (
                    static_cast<double>(node1.y) -
                    static_cast<double>(node2.y)),
                2));
    }

    /**
     * @brief calculate the length of a vector
     *
     * @param node sf::Vector2
     *
     * @return double
     */
    template <typename T>
    inline double length(sf::Vector2<T> node) noexcept
    {
        return std::sqrt(
            std::pow(static_cast<double>(node.x), 2) +
            std::pow(static_cast<double>(node.y), 2));
    }
}