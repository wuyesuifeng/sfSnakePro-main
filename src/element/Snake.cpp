#include <SFML/Graphics.hpp>

#include <memory>
#include <iostream>
#include <math.h>

#include "element/Snake.h"
#include "Game.h"
#include "element/Fruit.h"

#include "screen/GameOverScreen.h"

#include "utils/Time.hpp"

#define XY_CHAR_MAX 255
#define XY_CHAR_MIN 0
#define CHAR_PLUS 100
#define ANGLE_PLUS_THRESHOLD 180
#define ANGLE_PLUS_THRESHOLD2 60
#define ANGLE_PLUS_THRESHOLD3 120

using namespace sfSnake;

const int Snake::InitialSize = 5;

static const float VISION_X_HALF = VISION_X_SUM / 2,
                    VISION_HALF_WIDTH = VISION_PIXEL_WIDTH * VISION_X_HALF,
                    VISION_HALF_WIDTH2 = VISION_HALF_WIDTH - VISION_PIXEL_WIDTH;

float culAngle(sf::Vector2f recDirection) {
    float angle =
        std::acos(recDirection.y / length(recDirection)) /
        PI * 180.0;
    if (recDirection.x > 0)
        angle = -angle;
        
    return angle;
}

Snake::Snake()
    : hitSelf_(false),
      turning(utils::timestamp()),
      pain_(0),
      hurting(0),
      eating(0),
    //   speedup_(false),
      speed_(0),
      direction_(Direction(0, -1)),
      angle_(180),
      hisAngle_(angle_),
      headAngle_(angle_),
      radian(angle_ * PI / 180.0f),
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

    // pickupBuffer_.loadFromFile("assets/sounds/pickup.wav");
    // pickupSound_.setBuffer(pickupBuffer_);
    // pickupSound_.setVolume(30);

    // dieBuffer_.loadFromFile("assets/sounds/die.wav");
    // dieSound_.setBuffer(dieBuffer_);
    // dieSound_.setVolume(50);
    
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

    if (
        sf::Keyboard::isKeyPressed(sf::Keyboard::Up) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {

        direction_ = Direction(0, -1);

        angle_ = culAngle(direction_);
        radian = angle_ * PI / 180.0f;
    } else if (
        sf::Keyboard::isKeyPressed(sf::Keyboard::Down) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {

        direction_ = Direction(0, 1);

        angle_ = culAngle(direction_);
        radian = angle_ * PI / 180.0f;
    } else if (
        sf::Keyboard::isKeyPressed(sf::Keyboard::Left) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {

        direction_ = Direction(-1, 0);

        angle_ = culAngle(direction_);
        radian = angle_ * PI / 180.0f;
    } else if (
        sf::Keyboard::isKeyPressed(sf::Keyboard::Right) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {

        direction_ = Direction(1, 0);

        angle_ = culAngle(direction_);
        radian = angle_ * PI / 180.0f;
    }
        

    if (!Game::mouseButtonLocked)
    {
        if (
            sf::Mouse::isButtonPressed(sf::Mouse::Left) ||
            sf::Mouse::isButtonPressed(sf::Mouse::Right))
        {
            mousePosition = sf::Mouse::getPosition(window);
            handleInput(mousePosition, window);

            angle_ = culAngle(direction_);
            radian = angle_ * PI / 180.0f;
        }
    }

    // if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
    //     speedup_ = true;
    // else
    //     speedup_ = false;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
        speed_++;
    }
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

float parseAngle(float angle) {
    return angle < 0 ? 360 + angle : angle;
}

float parseAngle2(float angle) {
    return angle > ANGLE_PLUS_THRESHOLD ? angle - 360 : angle;
}

void Snake::update(sf::Time delta)
{
    float plus = 0;
    
    plus += (float) (*in) * ANGLE_PLUS_THRESHOLD2 / XY_CHAR_MAX + (float) in[1] * ANGLE_PLUS_THRESHOLD2 / XY_CHAR_MAX / XY_CHAR_MAX;

    if (in[2]) {
        speed_ = in[2] > 20 ? 2 : 1;
    }

    plus -= (float) in[4] * ANGLE_PLUS_THRESHOLD2 / XY_CHAR_MAX + (float) in[3] * ANGLE_PLUS_THRESHOLD2 / XY_CHAR_MAX / XY_CHAR_MAX;

    if (plus) {

        if (plus > ANGLE_PLUS_THRESHOLD3) {
            plus = ANGLE_PLUS_THRESHOLD3;
        } else if (plus < -ANGLE_PLUS_THRESHOLD3) {
            plus = -ANGLE_PLUS_THRESHOLD3;
        }

        angle_ = parseAngle2(angle_ + plus);

        // cout << angle_;
        float angle = parseAngle(angle_),
                headAngle = parseAngle(headAngle_);
        float angleTmp = angle - headAngle;
        if (angleTmp > ANGLE_PLUS_THRESHOLD) {
            angleTmp = angleTmp - 360;
        } else if (angleTmp < -ANGLE_PLUS_THRESHOLD) {
            angleTmp = 360 + angleTmp;
        }
        if (angleTmp > 0) {
            if (angleTmp > ANGLE_PLUS_THRESHOLD2) {
                angle_ = parseAngle2(headAngle + ANGLE_PLUS_THRESHOLD2);
                pain_ += angleTmp - ANGLE_PLUS_THRESHOLD2;
            }
            *(out + 2) = min(angleTmp, 255.0f);
            *(out + 3) = 0;
        } else if (angleTmp < 0) {
            if (angleTmp < -ANGLE_PLUS_THRESHOLD2) {
                angle_ = parseAngle2(headAngle - ANGLE_PLUS_THRESHOLD2);
                pain_ += ANGLE_PLUS_THRESHOLD2 - angleTmp;
            }
            *(out + 3) = -max(angleTmp, -255.0f);
            *(out + 2) = 0;
        } else {
            *(out + 2) = 0;
            *(out + 3) = 0;
        }

        // cout << "\t" << angle_ << endl;
        
        static unsigned long long now;
        now = utils::timestamp();
        static unsigned long long tmpDiff;
        tmpDiff = abs(((hisAngle_ > 0 && hisAngle_ > 0) || (hisAngle_ < 0 && hisAngle_ < 0)) ? hisAngle_ - angleTmp : hisAngle_ + angleTmp) * 500;
        if (now - turning > 200000) {
            turning = min(now - 200000 + tmpDiff, now);
        } else {
            turning = min(turning + tmpDiff, now);
        }

        radian = angle_ * PI / 180.0f;

        direction_.y += cos(radian) * headTexture.getSize().y;
        direction_.x -= sin(radian) * headTexture.getSize().y;

        static double directionSize;
        directionSize = length(direction_);
        direction_.x /= directionSize;
        direction_.y /= directionSize;

        hisAngle_ = angleTmp;

        // printf("angle_: %f\n", angle_);
    }
    
    move();
    toWindow(path_.front(), direction_, abs(tan(radian)));
    look();
    checkSelfCollisions();
}

float culSelfCollisionDis(float radius) {
    return 2.0f * radius;
}

void Snake::look() {
    SnakePathNode head = path_.front();

    float cosR = cos(radian),
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
        *out = min(*out + CHAR_PLUS, XY_CHAR_MAX);
        eating = utils::timestamp();
    } else {
        unsigned long long diff = (utils::timestamp() - eating) / 10;
        if (diff < CHAR_PLUS) {
            *out = max(*out, (unsigned char) (CHAR_PLUS - diff));
        } else {
            *out = 0;
        }
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
        // int times = speedup_ ? 2 : 1;
        if (speed_ > 0) {
            headAngle_ = angle_;
            for (int i = 1; i <= speed_; i++)
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
            speed_ = 0;
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
            pain_ += CHAR_PLUS;

            hurting = utils::timestamp();
            return;
        }
    }
    unsigned long long now = utils::timestamp(),
                        diff = (now - hurting) / 10;
    if (diff < CHAR_PLUS) {
        diff = CHAR_PLUS - diff + min((now - turning) / 2000, 100ull);
    } else {
        diff = min((now - turning) / 2000, 100ull);
    }
    if (diff) {
        pain_ += diff;
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
    *(out + 1) = min(pain_, XY_CHAR_MAX);
    pain_ = 0;
    unsigned char *out_tmp = out + 4;

    SnakePathNode lastSnakeNode, lastMiddleNode, nowSnakeNode;
    float angle = angle_;
    sf::Vector2f body;
    SnakePathNode wNowHeadNode;

    lastSnakeNode = *path_.begin();
    wNowHeadNode = lastSnakeNode;
    headSprite.setPosition(wNowHeadNode);
    headSprite.setRotation(angle);
    
    sf::RectangleShape shape = sf::RectangleShape();
    shape.setSize(sf::Vector2f(VISION_PIXEL_WIDTH, VISION_PIXEL_WIDTH));
    shape.setRotation(angle);
    vision v;
    for (int x = 0, y = 0; x < VISION_X_SUM; x++, y = 0) {
        for (; y < VISION_Y_SUM; y++, out_tmp++) {
            v = vision_[x][y];
            shape.setFillColor(sf::Color(v.color));
            shape.setPosition(v.pos);
            window.draw(shape);

            unsigned char *val;
            switch(v.color) {
                case VISION_HARM_COLOR:
                    val = out_tmp + VISION_HARM_POS;
                    *val = 10;
                    val = out_tmp + VISION_CHECK_POS;
                    *val = 0;
                    val = out_tmp;
                    *val = 0;
                    break;
                case VISION_CHECK_COLOR:
                    val = out_tmp + VISION_CHECK_POS;
                    *val = 80;
                    val = out_tmp + VISION_HARM_POS;
                    *val = 0;
                    val = out_tmp;
                    *val = 0;
                    break;
                default:
                    val = out_tmp;
                    *val = 10;
                    val = out_tmp + VISION_CHECK_POS;
                    *val = 0;
                    val = out_tmp + VISION_HARM_POS;
                    *val = 0;
            }
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