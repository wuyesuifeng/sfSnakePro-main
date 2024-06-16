#include <SFML/Graphics.hpp>

#include <memory>
#include <iostream>
#include <math.h>

#include "element/Snake.h"
#include "Game.h"
#include "element/Fruit.h"

#include "screen/GameOverScreen.h"

using namespace sfSnake;

const int Snake::InitialSize = 5;

static const float VISION_X_HALF = VISION_X_SUM / 2,
                    VISION_HALF_WIDTH = VISION_PIXEL_WIDTH * VISION_X_HALF,
                    VISION_HALF_WIDTH2 = VISION_HALF_WIDTH - VISION_PIXEL_WIDTH;

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

float culAngle(sf::Vector2f recDirection) {
    float angle =
        std::acos(recDirection.y / length(recDirection)) /
        PI * 180.0;
    if (recDirection.x > 0)
        angle = -angle;
        
    return angle;
}

void Snake::update(sf::Time delta)
{
    move();
    toWindow(path_.front(), direction_, abs(tan(culAngle(direction_) * PI / 180.0f)));
    look();
    checkSelfCollisions();
}

float culSelfCollisionDis(float radius) {
    return 2.0f * radius;
}

void Snake::look() {
    SnakePathNode head = path_.front();

    float angle = culAngle(direction_),
            radian = angle * PI / 180.0f,
            cosR = cos(radian),
            sinR = sin(radian),
            tanVal = abs(tan(radian)),
            moveY = cosR * VISION_PIXEL_WIDTH,
            moveX = sinR * VISION_PIXEL_WIDTH,
            interX = 1.0f,
            visionPadding = VISION_PIXEL_WIDTH + culSelfCollisionDis(nodeRadius_);

    sf::Vector2f center = sf::Vector2f(head.x + direction_.x * visionPadding, head.y + direction_.y * visionPadding);
    
    cosR = abs(cosR);
    sinR = abs(sinR);

    sf::Vector2f pos;
    for (int x = -VISION_X_HALF, i = 0; x < VISION_X_HALF; x++, i++) {
        for (int y = 0; y < VISION_Y_SUM; y++) {
            pos = center + sf::Vector2f(-moveX * y, moveY * y) + sf::Vector2f(moveY * x, moveX * x);
            toWindow(pos, direction_, tanVal, cosR, sinR, x + VISION_X_HALF, head);
            vision &v = vision_[i][y];
            v.pos = pos;
            v.color = VISION_DEF_COLOR;
        }
    }
}

void Snake::checkFruitCollisions(std::deque<Fruit> &fruits)
{
    auto toRemove = fruits.end();
    SnakePathNode headnode = path_.front();

    for (auto i = fruits.begin(); i != fruits.end(); ++i)
    {
        if (dis(
                i -> shape_.getPosition(), headnode) <
            nodeRadius_ + i -> shape_.getRadius()) {
            toRemove = i;
        }

        for (int x = 0, y = 0; x < VISION_X_SUM; x++, y = 0) {
            for (; y < VISION_Y_SUM; y++) {
                vision &v = vision_[x][y];
                if (dis(
                    i -> shape_.getPosition(), v.pos
                ) < i -> shape_.getRadius()) {
                    v.color = VISION_CHECK_COLOR;
                }
            }
        }
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
    if (score_ > InitialSize || score > 0) {
        tailOverlap_ += score * 10;
        score_ += score;
    }
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

    if (tailOverlap_ < 0) {
        do {
            path_.pop_front();
            tailOverlap_++;  
        } while (tailOverlap_ < 0);
    } else {
        int times = speedup_ ? 2 : 1;
        for (int i = 1; i <= times; i++)
        {
            path_.push_front(SnakePathNode(
                headNode.x + direction_.x * i * nodeRadius_ / 5.0,
                headNode.y + direction_.y * i * nodeRadius_ / 5.0));
            if (tailOverlap_) {
                tailOverlap_--;
            } else {
                path_.pop_back();
            }
        }
    }
}

void Snake::checkSelfCollisions()
{
    SnakePathNode head = path_.front();
    int count = 0;

    for (auto i = path_.begin(); i != path_.end(); i++, count++) {

        for (int x = 0, y = 0; x < VISION_X_SUM; x++, y = 0) {
            for (; y < VISION_Y_SUM; y++) {
                vision &v = vision_[x][y];
                if (dis(v.pos, *i) < culSelfCollisionDis(nodeRadius_)) {
                    v.color = VISION_HARM_COLOR;
                }
            }
        }

        if (count >= 30 && dis(head, *i) < culSelfCollisionDis(nodeRadius_))
        {
            dieSound_.stop();
            dieSound_.play();
            hitSelf_ = true;

            *(in + 1) = Game::HIS_XY;
            *(in + 2) = Game::HIS_XY;
            return;
        }
    }
    hitSelf_ = false;
}

bool inWindow(SnakePathNode &node)
{
    return node.x >= 0 &&
            node.x <= Game::GlobalVideoMode.width &&
            node.y >= 0 &&
            node.y <= Game::GlobalVideoMode.height;
}

float culTimes(float x, float dx, int total) {
    return dx > 0.0f ? x / dx : (total - x) / -dx;
}

float culTimes2(float x, float dx, int total) {
    return dx < 0.0f ? x / -dx : (total - x) / dx;
}

void culOutWindowPos(float &pos, float &pos2, float &hpos, float &hpos2, float dir, float dir2, unsigned int total, unsigned int total2, float tanVal, float sin, float cos, int num, float subsidy) {
    float posPlus = pos - total;

    if (culTimes(hpos, dir, total) < culTimes(hpos2, dir2, total2) && culTimes2(hpos, dir, total) < culTimes2(hpos2, dir2, total2)) {
        pos2 = pos2 - pos / tanVal + posPlus / tanVal;
        pos = posPlus;
    } else {
        pos = pos - pos2 * tanVal + posPlus;
        pos2 = posPlus / tanVal;

        if (num > -1) {
            pos = pos + VISION_PIXEL_WIDTH / sin * num - subsidy / sin;
            pos2 = pos2 + VISION_PIXEL_WIDTH / cos * num - subsidy / cos;
        }
    }

}

bool transCoord(float &dir, float &coord, float &coord2, int border) {
    if (dir < 0) {
        coord = border - coord;
        coord2 = border - coord2;
        dir = -dir;
        return true;
    }
    return false;
}

void reverse(float &x, float &y, float hx, float hy, float dx, float dy, int width, int height, float tanVal, float sin, float cos, int num, float subsidy) {
    const bool transX = transCoord(dx, x, hx, width),
                transY = transCoord(dy, y, hy, height);

    culOutWindowPos(x, y, hx, hy, dx, dy, width, height, tanVal, sin, cos, num, subsidy);

    if (transX) {
        x = width - x;
    }

    if (transY) {
        y = height - y;
    }
}

SnakePathNode Snake::toWindow(sf::Vector2f &node, SnakePathNode dir, float tanVal) {
    return toWindow(node, dir, tanVal, 0, 0, -1, node);
}

SnakePathNode Snake::toWindow(sf::Vector2f &node, SnakePathNode dir, float tanVal, float sin, float cos, int num, SnakePathNode head) {
    bool negativeX = node.x < 0,
            negativeY = node.y < 0,
            beyondX = negativeX || node.x > Game::GlobalVideoMode.width,
            beyondY = negativeY || node.y > Game::GlobalVideoMode.height;
    if (beyondX) {
        if (dir.y == 0) {
            node.x = negativeX ? node.x + Game::GlobalVideoMode.width : node.x - Game::GlobalVideoMode.width;
        } else if (culTimes2(head.x, dir.x, Game::GlobalVideoMode.width) > culTimes2(head.y, dir.y, Game::GlobalVideoMode.height)) {

            float subsidy = VISION_HALF_WIDTH;
            if (num > -1 && ((dir.x > 0 && dir.y < 0) || (dir.x < 0 && dir.y > 0))) {
                num = abs(num + 1 - VISION_X_SUM);
                subsidy = VISION_HALF_WIDTH2;
            }
            
            reverse(node.y, node.x, head.y, head.x, dir.y, dir.x, 
                    Game::GlobalVideoMode.height, Game::GlobalVideoMode.width, 1.0f / tanVal, cos, sin, num, subsidy);
        } else {

            float subsidy = VISION_HALF_WIDTH;
            if (num > -1 && ((dir.x > 0 && dir.y > 0) || (dir.x < 0 && dir.y < 0))) {
                num = abs(num + 1 - VISION_X_SUM);
                subsidy = VISION_HALF_WIDTH2;
            }

            reverse(node.x, node.y, head.x, head.y, dir.x, dir.y, 
                Game::GlobalVideoMode.width, Game::GlobalVideoMode.height, tanVal, sin, cos, num, subsidy);
        }
    } else if (beyondY) {
        if (dir.x == 0) {
            node.y = negativeY ? node.y + Game::GlobalVideoMode.height : node.y - Game::GlobalVideoMode.height;
        } else if (culTimes2(head.x, dir.x, Game::GlobalVideoMode.width) > culTimes2(head.y, dir.y, Game::GlobalVideoMode.height)) {

            float subsidy = VISION_HALF_WIDTH;
            if (num > -1 && ((dir.x > 0 && dir.y < 0) || (dir.x < 0 && dir.y > 0))) {
                num = abs(num + 1 - VISION_X_SUM);
                subsidy = VISION_HALF_WIDTH2;
            }
            
            reverse(node.y, node.x, head.y, head.x, dir.y, dir.x, 
                    Game::GlobalVideoMode.height, Game::GlobalVideoMode.width, 1.0f / tanVal, cos, sin, num, subsidy);
        } else {

            float subsidy = VISION_HALF_WIDTH;
            if (num > -1 && ((dir.x > 0 && dir.y > 0) || (dir.x < 0 && dir.y < 0))) {
                num = abs(num + 1 - VISION_X_SUM);
                subsidy = VISION_HALF_WIDTH2;
            }

            reverse(node.x, node.y, head.x, head.y, dir.x, dir.y, 
                Game::GlobalVideoMode.width, Game::GlobalVideoMode.height, tanVal, sin, cos, num, subsidy);
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
    *(out + 3) = direction_.x;
    *(out + 4) = direction_.y;

    SnakePathNode lastSnakeNode, lastMiddleNode, nowSnakeNode;
    float angle;
    sf::Vector2f body;
    SnakePathNode wNowHeadNode;

    lastSnakeNode = *path_.begin();
    wNowHeadNode = lastSnakeNode;
    headSprite.setPosition(wNowHeadNode);
    angle = culAngle(direction_);
    headSprite.setRotation(angle);
    
    sf::RectangleShape shape = sf::RectangleShape();
    shape.setSize(sf::Vector2f(VISION_PIXEL_WIDTH, VISION_PIXEL_WIDTH));
    shape.setRotation(angle);
    vision v;
    for (int x = 0, y; x < VISION_X_SUM; x++, y = 0) {
        for (; y < VISION_Y_SUM; y++) {
            v = vision_[x][y];
            shape.setFillColor(sf::Color(v.color));
            shape.setPosition(v.pos);
            window.draw(shape);
            *(out + 4 + (x + 1) * (y + 1)) = v.color == VISION_DEF_COLOR ? 0 : v.color == VISION_CHECK_COLOR ? 1 : 2;
        }
    }

    renderNode(wNowHeadNode, headSprite, window, 3);

    count = 5;
    for (auto i = path_.begin() + 5, end = path_.end();
            i < end;
            i += 5, count += 5, j += 2)
    {
        body = *i;

        if (count % 2)
            lastMiddleNode = body;
        else
        {
            nowSnakeNode = body;
            angle = culAngle(nowSnakeNode - lastSnakeNode);
            nodeMiddle.setRotation(angle);

            lastSnakeNode = nowSnakeNode;

            renderNode(nowSnakeNode, nodeShape, window, 0);
            renderNode(lastMiddleNode, nodeMiddle, window, 0);
        }
    }
}

template <typename T>
void Snake::renderNode(sf::Vector2f &nowPosition, T &shape, sf::RenderWindow &window, int offset)
{
    shape.setPosition(nowPosition);
    window.draw(shape);
}