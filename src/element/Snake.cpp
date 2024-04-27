#include <SFML/Graphics.hpp>

#include <memory>
#include <iostream>

#include "element/Snake.h"
#include "Game.h"
#include "element/Fruit.h"

#include "screen/GameOverScreen.h"

using namespace sfSnake;

const int Snake::InitialSize = 5;

Snake::Snake()
    : hitSelf_(false),
      speedup_(false),
      direction_(Direction(0, -1)),
      nodeRadius_(Game::GlobalVideoMode.width / 100.0f),
      tailOverlap_(0u),
      nodeShape(nodeRadius_),
      nodeMiddle(sf::Vector2f(nodeRadius_ * std::sqrt(3), nodeRadius_)),
      score_(InitialSize),
      hisX(Game::HIS_XY),
      hisY(Game::HIS_XY)
{
    initNodes();

    nodeShape.setFillColor(sf::Color(0xf1c40fff));

    nodeMiddle.setFillColor(sf::Color(0x1c2833ff));

    setOriginMiddle(nodeShape);
    setOriginMiddle(nodeMiddle);

    headTexture.loadFromFile("assets/image/snakeHeadImage.png");
    headTexture.setSmooth(true);
    sf::Vector2u TextureSize = headTexture.getSize();
    float headScale = nodeRadius_ / TextureSize.y * 2.6;
    headSprite.setTexture(headTexture);
    headSprite.setScale(headScale, headScale);

    setOriginMiddle(headSprite);

    pickupBuffer_.loadFromFile("assets/sounds/pickup.wav");
    pickupSound_.setBuffer(pickupBuffer_);
    pickupSound_.setVolume(30);

    dieBuffer_.loadFromFile("assets/sounds/die.wav");
    dieSound_.setBuffer(dieBuffer_);
    dieSound_.setVolume(50);

    
    in = Game::share.getReadPos();
    out = Game::share.getWritePos();
}

void Snake::initNodes()
{
    path_.push_back(SnakePathNode(
        Game::GlobalVideoMode.width / 2.0f,
        Game::GlobalVideoMode.height / 2.0f));
    for (int i = 1; i <= 10 * InitialSize; i++)
    {
        path_.push_back(SnakePathNode(
            Game::GlobalVideoMode.width / 2.0f -
                direction_.x * i * nodeRadius_ / 5.0,
            Game::GlobalVideoMode.height / 2.0f -
                direction_.y * i * nodeRadius_ / 5.0));
    }
}

void Snake::handleInput(sf::RenderWindow &window)
{
    static sf::Vector2i mousePosition;

    mousePosition.x = *(in + 1);
    mousePosition.y = *(in + 2);
    if (mousePosition.x != hisX || mousePosition.y != hisY) {
        handleInput(mousePosition, window);
    }

    if (
        sf::Keyboard::isKeyPressed(sf::Keyboard::Up) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::W))
        direction_ = Direction(0, -1);
    else if (
        sf::Keyboard::isKeyPressed(sf::Keyboard::Down) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::S))
        direction_ = Direction(0, 1);
    else if (
        sf::Keyboard::isKeyPressed(sf::Keyboard::Left) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::A))
        direction_ = Direction(-1, 0);
    else if (
        sf::Keyboard::isKeyPressed(sf::Keyboard::Right) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::D))
        direction_ = Direction(1, 0);

    if (!Game::mouseButtonLocked)
    {
        if (
            sf::Mouse::isButtonPressed(sf::Mouse::Left) ||
            sf::Mouse::isButtonPressed(sf::Mouse::Right))
        {
            mousePosition = sf::Mouse::getPosition(window);
            handleInput(mousePosition, window);
        }
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
        speedup_ = true;
    else
        speedup_ = false;
}

void Snake::handleInput(sf::Vector2i mousePosition, sf::RenderWindow &window) {
    if (
            dis(mousePosition,
                sf::Vector2f(
                    Game::GlobalVideoMode.width / 15.0f * 14.0f,
                    Game::GlobalVideoMode.width / 15.0f)) >
            Game::GlobalVideoMode.width / 16.0f)
    {

        static double directionSize;
        SnakePathNode front = path_.front();
        direction_ =
            static_cast<sf::Vector2f>(mousePosition) - front;
        directionSize = length(direction_);
        direction_.x /= directionSize;
        direction_.y /= directionSize;
    }
}

void Snake::update(sf::Time delta)
{
    move();
    toWindow(path_.front(), direction_);
    checkSelfCollisions();
}

void Snake::checkFruitCollisions(std::deque<Fruit> &fruits)
{
    auto toRemove = fruits.end();
    SnakePathNode headnode = path_.front();

    for (auto i = fruits.begin(); i != fruits.end(); ++i)
    {
        if (dis(
                i -> shape_.getPosition(), headnode) <
            nodeRadius_ + i -> shape_.getRadius())
            toRemove = i;
    }

    if (toRemove != fruits.end())
    {
        pickupSound_.play();
        grow(toRemove->score_);
        fruits.erase(toRemove);
    }
}

void Snake::grow(int score)
{
    tailOverlap_ += score * 10;
    score_ += score;
}

unsigned Snake::getScore() const
{
    return score_;
}

bool Snake::hitSelf() const
{
    return hitSelf_;
}

void Snake::move()
{
    SnakePathNode &headNode = path_.front();
    int times = speedup_ ? 2 : 1;
    for (int i = 1; i <= times; i++)
    {
        path_.push_front(SnakePathNode(
            headNode.x + direction_.x * i * nodeRadius_ / 5.0,
            headNode.y + direction_.y * i * nodeRadius_ / 5.0));
        if (tailOverlap_)
            tailOverlap_--;
        else
            path_.pop_back();
    }
}

void Snake::checkSelfCollisions()
{
    SnakePathNode head = path_.front();
    int count = 0;

    for (auto i = path_.begin(); i != path_.end(); i++, count++) {
        if (count >= 30 && dis(head, *i) < 2.0f * nodeRadius_)
        {
            dieSound_.play();
            sf::sleep(sf::seconds(dieBuffer_.getDuration().asSeconds()));
            hitSelf_ = true;

            *(in + 1) = Game::HIS_XY;
            *(in + 2) = Game::HIS_XY;
            break;
        }
    }
}

bool inWindow(SnakePathNode &node)
{
    return node.x >= 0 &&
            node.x <= Game::GlobalVideoMode.width &&
            node.y >= 0 &&
            node.y <= Game::GlobalVideoMode.height;
}

void culOutWindowPos(float &pos, float &pos2, float dir, float dir2, unsigned int total, unsigned int total2) {
    
    float times = std::abs(total / dir),
            times2 = dir2 > 0 ? pos2 / dir2 : std::abs((total2 - pos2) / dir2);
    
    if (times > times2) {
        pos = pos - dir * times2;
        if (pos < 0) {
            pos = 0;
        } else if (pos > total) {
            pos = total;
        }
        pos2 = dir2 > 0 ? 0 : total2;
    } else {
        pos2 = pos2 - dir2 * times;
        if (pos2 < 0) {
            pos2 = 0;
        } else if (pos2 > total2) {
            pos2 = total2;
        }
        pos = dir > 0 ? 0 : total;
    }
}

SnakePathNode Snake::toWindow(SnakePathNode &node, SnakePathNode dir)
{
    if (node.x < 0 || node.x > Game::GlobalVideoMode.width) {
        if (dir.y == 0) {
            node.x = node.x < 0 ? node.x + Game::GlobalVideoMode.width : node.x - Game::GlobalVideoMode.width;
        } else {
            culOutWindowPos(node.x, node.y, dir.x, dir.y, 
                Game::GlobalVideoMode.width, Game::GlobalVideoMode.height);
        }
    }

    if (node.y < 0 || node.y > Game::GlobalVideoMode.height) {
        if (dir.x == 0) {
            node.y = node.y < 0 ? node.y + Game::GlobalVideoMode.height : node.y - Game::GlobalVideoMode.height;
        } else {
            culOutWindowPos(node.y, node.x, dir.y, dir.x, 
                Game::GlobalVideoMode.height, Game::GlobalVideoMode.width);
        }
    }
        
    return node;
}

void Snake::render(sf::RenderWindow &window)
{
    int count,
        j = 7;

    // 将数据长度、存活状态、分数、窗口尺寸输出到共享内存中
    *(out + 1) = hitSelf_ ? 0 : 1;
    *(out + 2) = score_;
    *(out + 3) = Game::GlobalVideoMode.width;
    *(out + 4) = Game::GlobalVideoMode.height;
    *(out + 5) = direction_.x * 99;
    *(out + 6) = direction_.y * 99;

    SnakePathNode lastSnakeNode, lastMiddleNode, nowSnakeNode;
    float angle;
    sf::Vector2f recDirection, body;
    SnakePathNode wNowHeadNode;

    lastSnakeNode = *path_.begin();
    wNowHeadNode = lastSnakeNode;
    headSprite.setPosition(wNowHeadNode);
    recDirection = direction_;
    angle =
        std::acos(recDirection.y / length(recDirection)) /
        3.14159265358979323846 * 180.0;
    if (direction_.x > 0)
        angle = -angle;
    headSprite.setRotation(angle);

    renderNode(wNowHeadNode, headSprite, window, 3);

    count = 5;
    for (auto i = path_.begin() + 5, end = path_.end();
            i < end;
            i += 5, count += 5, j += 2)
    {
        body = *i;

        // 将蛇身坐标输出到共享内存中
        *(out + j) = body.x * 99;
        *(out + j + 1) = body.y * 99;

        if (count % 2)
            lastMiddleNode = body;
        else
        {
            nowSnakeNode = body;

            recDirection = nowSnakeNode - lastSnakeNode;
            angle =
                std::acos(recDirection.y / length(recDirection)) /
                3.14159265358979323846 * 180.0;
            if (recDirection.x > 0)
                angle = -angle;
            nodeMiddle.setRotation(angle);

            lastSnakeNode = nowSnakeNode;

            renderNode(nowSnakeNode, nodeShape, window, 0);
            renderNode(lastMiddleNode, nodeMiddle, window, 0);
        }
    }
    *out = j - 2;
}

template <typename T>
void Snake::renderNode(sf::Vector2f &nowPosition, T &shape, sf::RenderWindow &window, int offset)
{
    shape.setPosition(nowPosition);
    window.draw(shape);
}