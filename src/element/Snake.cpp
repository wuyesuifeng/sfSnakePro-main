

#include <math.h>

#include <SFML/Graphics.hpp>
#include <iostream>
#include <memory>

#include "element/Snake.h"
#include "Game.h"
#include "screen/GameOverScreen.h"
#include "utils/Time.hpp"

#define ANGLE_PLUS_THRESHOLD 180
#define ANGLE_PLUS_THRESHOLD2 80
#define ANGLE_PLUS_THRESHOLD3 160
#define max std::max
#define min std::min
#define CUL_VISION_INDEX(x, y) x *VISION_Y_SUM + y

using namespace sfSnake;

static float VISION_X_HALF,
    VISION_HALF_WIDTH,
    VISION_HALF_WIDTH2;

static TYPE_VOL speed_level1, speed_level2, vision_blank_vol, vision_fruit_vol, vision_body_vol;
// static unsigned int vision_blank_vol, vision_fruit_vol, vision_body_vol;

float culAngle(sf::Vector2f recDirection) {
    float angle = std::acos(recDirection.y / length(recDirection)) / PI * 180.0;
    if (recDirection.x > 0)
        angle = -angle;

    return angle;
}

Snake::Snake()
    : vision_((vision *)malloc(sizeof(vision) * Game::cfg.visionXSum * Game::cfg.visionYSum)),
      hitSelf_(false),
      pain_(0),
      delight_(0),
      turnLeft_(0),
      turnRight_(0),
      stuckLeft_(0),
      stuckRight_(0),
      leftVitality_(0),
      rightVitality_(0),
      speed_(0),
      direction_(Direction(0, -1)),
      angle_(180),
      hisAngle_(angle_),
      bodyDir_(angle_),
      headAngle_(0),
      radian_(angle_ * PI / 180.0f),
      nodeRadius_(Game::GlobalVideoMode.width / 100.0f),
      tailOverlap_(0u),
      nodeShape_(nodeRadius_),
      nodeMiddle_(sf::Vector2f(nodeRadius_ * std::sqrt(3), nodeRadius_)),
      score_(Game::cfg.initialSize) {

    snakeLen_ = 10 * Game::cfg.initialSize;

    health_ = Game::cfg.heath;

    death_ = Game::cfg.death;

    maxVitality_ = Game::cfg.maxVitality;

    minVitality_ = -maxVitality_;

    vitalityStepCnt_ = Game::cfg.vitalityStepCnt;

    vitalityPain_ = Game::cfg.vitalityPain;

    vitalityPain2_ = vitalityPain_ / maxVitality_;

    speedVitalityDiff_ = Game::cfg.speedVitalityDiff;

    healthVal_ = Game::cfg.heath;

    speedVitality_ = speedVitalityMax_ = maxVitality_ - speedVitalityDiff_;

    initNodes();

    nodeShape_.setFillColor(sf::Color(0xf1c40fff));

    nodeMiddle_.setFillColor(sf::Color(0x1c2833ff));

    setOriginMiddle(nodeShape_);
    setOriginMiddle(nodeMiddle_);

    headTexture_.loadFromFile("assets/image/snakeHeadImage.png");
    headTexture_.setSmooth(true);
    sf::Vector2u TextureSize = headTexture_.getSize();
    float headScale = nodeRadius_ / TextureSize.y * 2.6;
    headSprite_.setTexture(headTexture_);
    headSprite_.setScale(headScale, headScale);

    setOriginMiddle(headSprite_);

    VISION_X_HALF = VISION_X_SUM / 2;
    VISION_HALF_WIDTH = VISION_PIXEL_WIDTH * VISION_X_HALF;
    VISION_HALF_WIDTH2 = VISION_HALF_WIDTH - VISION_PIXEL_WIDTH;
    speed_level1 = Game::cfg.speedLevel1;
    speed_level2 = Game::cfg.speedLevel2;
    vision_blank_vol = Game::cfg.visionBlankVol;
    vision_fruit_vol = Game::cfg.visionFruitVol;
    vision_body_vol = Game::cfg.visionBodyVol;

    // pickupBuffer_.loadFromFile("assets/sounds/pickup.wav");
    // pickupSound_.setBuffer(pickupBuffer_);
    // pickupSound_.setVolume(30);

    // dieBuffer_.loadFromFile("assets/sounds/die.wav");
    // dieSound_.setBuffer(dieBuffer_);
    // dieSound_.setVolume(50);
    threads_.init(std::thread::hardware_concurrency() - 5);

    in_ = Game::share.getReadPos();
    out_ = Game::share.getWritePos();

    deathFlag_ = out_ - 1;
}

Snake::~Snake() {
    threads_.join();
    free(vision_);
}

void Snake::initNodes() {
    path_.push_back(SnakePathNode(Game::GlobalVideoMode.width / 2.0f,
                                  Game::GlobalVideoMode.height / 2.0f));
    for (int i = 1; i <= snakeLen_; i++) {
        path_.push_back(SnakePathNode(Game::GlobalVideoMode.width / 2.0f -
                                          direction_.x * i * nodeRadius_ / 5.0,
                                      Game::GlobalVideoMode.height / 2.0f -
                                          direction_.y * i * nodeRadius_ / 5.0));
    }
}

void Snake::handleInput(sf::RenderWindow &window) {

    if (*deathFlag_) {
        return;
    }

    static sf::Vector2i mousePosition;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
        direction_ = Direction(0, -1);

        angle_ = culAngle(direction_);
        radian_ = angle_ * PI / 180.0f;
    } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) ||
               sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
        direction_ = Direction(0, 1);

        angle_ = culAngle(direction_);
        radian_ = angle_ * PI / 180.0f;
    } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) ||
               sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
        direction_ = Direction(-1, 0);

        angle_ = culAngle(direction_);
        radian_ = angle_ * PI / 180.0f;
    } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) ||
               sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
        direction_ = Direction(1, 0);

        angle_ = culAngle(direction_);
        radian_ = angle_ * PI / 180.0f;
    }

    if (!Game::mouseButtonLocked) {
        if (sf::Mouse::isButtonPressed(sf::Mouse::Left) ||
            sf::Mouse::isButtonPressed(sf::Mouse::Right)) {
            mousePosition = sf::Mouse::getPosition(window);
            handleInput(mousePosition, window);

            angle_ = culAngle(direction_);
            radian_ = angle_ * PI / 180.0f;
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
    if (dis(mousePosition,
            sf::Vector2f(Game::GlobalVideoMode.width / 15.0f * 14.0f,
                         Game::GlobalVideoMode.width / 15.0f)) >
        Game::GlobalVideoMode.width / 16.0f) {
        static double directionSize;
        SnakePathNode front = path_.front();
        direction_ = static_cast<sf::Vector2f>(mousePosition) - front;
        directionSize = length(direction_);
        direction_.x /= directionSize;
        direction_.y /= directionSize;
    }
}

float parseAngle(float angle) { return angle < 0 ? 360 + angle : angle; }

float parseAngle2(float angle) {
    return angle > ANGLE_PLUS_THRESHOLD ? angle - 360 : angle;
}

void Snake::update(sf::Time delta) {

    // static long long sleeping;
    // static unsigned int waitTime = Game::cfg.waitTime;

    // if (*deathFlag_ || (sleeping && utils::timestamp() - sleeping < waitTime)) {
    //     return;
    // }

    if (*deathFlag_) {
        return;
    }

    static utils::io_section *section = Game::cfg.inputSectionArr;
    static TYPE_VOL *inputPtr,
        *leftPtrEnd = in_ + section[0].endIndex,
        *runPtrEnd = in_ + section[1].endIndex,
        *rightPtrEnd = in_ + section[2].endIndex;

    static UPPER_TYPE_VOL plusTmp, speedTmp, plus;
    plus = 0;

    plusTmp = 0;
    inputPtr = in_ + section[0].startIndex;
    do {
        plusTmp += *inputPtr;
        inputPtr++;
    } while (inputPtr != leftPtrEnd);

    plus = -plusTmp * (leftVitality_ + maxVitality_);

    speedTmp = 0;
    inputPtr = in_ + section[1].startIndex;
    do {
        speedTmp += *inputPtr;
        inputPtr++;
    } while (inputPtr != runPtrEnd);

    speedTmp *= speedVitality_ / maxVitality_;

    if (speedTmp > speed_level1) {
        speed_ += speedTmp > speed_level2 ? 2 : 1;
    }

    plusTmp = 0;
    inputPtr = in_ + section[2].startIndex;
    do {
        plusTmp += *inputPtr;
        inputPtr++;
    } while (inputPtr != rightPtrEnd);

    plus += plusTmp * (rightVitality_ + maxVitality_);

    // if (!plus && !speedTmp) {
    //     sleeping = utils::timestamp();
    //     return;
    // }
    // sleeping = 0;

    plus /= maxVitality_;

    static float angle;
    if (plus) {
        if (plus > ANGLE_PLUS_THRESHOLD3) {
            plus = ANGLE_PLUS_THRESHOLD3;
        } else if (plus < -ANGLE_PLUS_THRESHOLD3) {
            plus = -ANGLE_PLUS_THRESHOLD3;
        }

        angle_ = parseAngle2(angle_ + plus);

        // cout << angle_;
        angle = parseAngle(angle_);
        headAngle_ = angle - bodyDir_;
        if (headAngle_ > ANGLE_PLUS_THRESHOLD) {
            headAngle_ = headAngle_ - 360;
        } else if (headAngle_ < -ANGLE_PLUS_THRESHOLD) {
            headAngle_ = 360 + headAngle_;
        }
        if (headAngle_ > 0) {

            if (headAngle_ > ANGLE_PLUS_THRESHOLD2) {
                angle_ = parseAngle2(bodyDir_ + ANGLE_PLUS_THRESHOLD2);
                stuckRight_ = (headAngle_ - ANGLE_PLUS_THRESHOLD2) * 10;
                if (stuckRight_ > MAX_VOL) {
                    stuckRight_ = MAX_VOL;
                }
                pain_ += stuckRight_;
            } else {
                stuckRight_ = 0;
            }
            stuckLeft_ = 0;
        } else if (headAngle_ < 0) {
            if (headAngle_ < -ANGLE_PLUS_THRESHOLD2) {
                angle_ = parseAngle2(bodyDir_ - ANGLE_PLUS_THRESHOLD2);
                stuckLeft_ = -(ANGLE_PLUS_THRESHOLD2 + headAngle_) * 10;
                if (stuckLeft_ > MAX_VOL) {
                    stuckLeft_ = MAX_VOL;
                }
                pain_ += stuckLeft_;
            } else {
                stuckLeft_ = 0;
            }
            stuckRight_ = 0;
        } else {
            stuckRight_ = 0;
            stuckLeft_ = 0;
        }

        // cout << "\t" << angle_ << endl;

        plus = ((hisAngle_ > 0 && headAngle_ > 0) || (hisAngle_ < 0 && headAngle_ < 0)
                    ? hisAngle_ - headAngle_
                    : hisAngle_ + headAngle_) *
               100;

        if (plus) {
            if (plus > 0) {
                if (plus > MAX_VOL) {
                    plus = MAX_VOL;
                }
                turnRight_ = plus;
                turnLeft_ = 0;
            } else {
                if (plus < -MAX_VOL) {
                    plus = -MAX_VOL;
                }
                turnLeft_ = -plus;
                turnRight_ = 0;
            }
        } else {
            turnRight_ = 0;
            turnLeft_ = 0;
        }

        radian_ = angle_ * PI / 180.0f;

        direction_.y += cos(radian_) * headTexture_.getSize().y;
        direction_.x -= sin(radian_) * headTexture_.getSize().y;

        static double directionSize;
        directionSize = length(direction_);
        direction_.x /= directionSize;
        direction_.y /= directionSize;

        hisAngle_ = headAngle_;

        // printf("angle_: %f\n", angle_);
    } else {
        stuckRight_ = 0;
        stuckLeft_ = 0;
        turnRight_ = 0;
        turnLeft_ = 0;

        angle = parseAngle(angle_);
        headAngle_ = angle - bodyDir_;
        if (headAngle_ > ANGLE_PLUS_THRESHOLD) {
            headAngle_ = headAngle_ - 360;
        } else if (headAngle_ < -ANGLE_PLUS_THRESHOLD) {
            headAngle_ = 360 + headAngle_;
        }
    }

    if (turnRight_) {
        leftVitality_ = min(leftVitality_ + (maxVitality_ - leftVitality_) / vitalityStepCnt_, maxVitality_);
        rightVitality_ = max(rightVitality_ - turnRight_, minVitality_);
    } else if (turnLeft_) {
        leftVitality_ = max(leftVitality_ - turnLeft_, minVitality_);
        rightVitality_ = min(rightVitality_ + (maxVitality_ - rightVitality_) / vitalityStepCnt_, maxVitality_);
    } else {
        rightVitality_ = min(rightVitality_ + (maxVitality_ - rightVitality_) / vitalityStepCnt_, maxVitality_);
        leftVitality_ = min(leftVitality_ + (maxVitality_ - leftVitality_) / vitalityStepCnt_, maxVitality_);
    }

    if (leftVitality_) {
        pain_ += abs(leftVitality_) * vitalityPain2_;
    }
    if (rightVitality_) {
        pain_ += abs(rightVitality_) * vitalityPain2_;
    }

    if (speed_) {
        if (speedVitality_) {
            speedVitality_ = max(speedVitality_ - speedVitality_ * speed_ / vitalityStepCnt_, 0.0f);
        }
    } else {
        speedVitality_ = min(speedVitality_ + (maxVitality_ - speedVitality_) / vitalityStepCnt_, maxVitality_);
    }

    if (speedVitality_ > speedVitalityMax_) {
        pain_ += vitalityPain_ * (speedVitality_ - speedVitalityMax_) / speedVitalityDiff_;
    }

    Direction dir = direction_;
    static short int speed, outOfWin;
    speed = speed_ * 10;
    dir.x *= speed;
    dir.y *= speed;
    static SnakePathNode headNode;
    headNode = path_.front() + dir;
    outOfWin = toWindow(headNode, direction_, abs(tan(radian_)));
    look(headNode);
    checkSelfCollisions(headNode);
    if (hitSelf_ && outOfWin) {
        look(path_.front());
        checkSelfCollisions(path_.front());
        for (int i = 1; i <= speed_; i++) {
            if (path_.size() > snakeLen_) {
                path_.pop_back();
            }
        }
    } else {
        move(path_.front());
        toWindow(path_.front(), direction_, abs(tan(radian_)));
    }
}

float culSelfCollisionDis(float radius) { return 2.0f * radius; }

void Snake::look(SnakePathNode head) {

    float cosR = cos(radian_), sinR = sin(radian_), tanVal = abs(tan(radian_)),
          moveY = cosR * VISION_PIXEL_WIDTH, moveX = sinR * VISION_PIXEL_WIDTH;
    static float visionPadding = (VISION_PIXEL_WIDTH + nodeRadius_) / 2;

    sf::Vector2f center = sf::Vector2f(head.x + direction_.x * visionPadding,
                                       head.y + direction_.y * visionPadding);

    cosR = abs(cosR);
    sinR = abs(sinR);

    sf::Vector2f pos;
    for (int x = -VISION_X_HALF, i = 0; x < VISION_X_HALF; x++, i++) {
        for (int y = 0; y < VISION_Y_SUM; y++) {
            pos = center + sf::Vector2f(-moveX * y, moveY * y) +
                  sf::Vector2f(moveY * x, moveX * x);
            toWindow(pos, direction_, tanVal, cosR, sinR, x + VISION_X_HALF, head);
            vision &v = vision_[CUL_VISION_INDEX(i, y)];
            v.pos = pos;
            v.color = VISION_DEF_COLOR;
        }
    }
}

void Snake::checkFruitCollisions(std::deque<Fruit> &fruits) {
    auto toRemove = fruits.end();
    SnakePathNode headnode = path_.front();

    for (auto i = fruits.begin(); i != fruits.end(); ++i) {
        if (dis(i->shape_.getPosition(), headnode) <
            nodeRadius_ + i->shape_.getRadius()) {
            toRemove = i;
        }

        for (int x = 0, y = 0; x < VISION_X_SUM; x++, y = 0) {
            for (; y < VISION_Y_SUM; y++) {
                vision &v = vision_[CUL_VISION_INDEX(x, y)];
                if (dis(i->shape_.getPosition(), v.pos) < i->shape_.getRadius()) {
                    v.color = VISION_CHECK_COLOR;
                }
            }
        }
    }

    if (toRemove != fruits.end()) {
        // pickupSound_.play();
        grow(toRemove->score_);
        fruits.erase(toRemove);
        delight_ = Game::cfg.eatDelight;
        leftVitality_ = rightVitality_ = stuckLeft_ = stuckRight_ = headAngle_ = 0;
        if (speedVitality_ > speedVitalityMax_) {
            speedVitality_ = speedVitalityMax_;
        }

        if (death_) {
            static float maxHealth = min(5.0 * healthVal_, MAX_VITALITY);
            if (health_ < maxHealth) {
                health_ += healthVal_;
            }
        } else {
            health_ = healthVal_;
        }
    }
}

void Snake::grow(int score) {
    tailOverlap_ += score * 10;
    score_ += score;
}

unsigned Snake::getScore() const { return score_; }

bool Snake::hitSelf() const { return hitSelf_; }

void Snake::move(SnakePathNode headNode) {

    if (speed_ > 0) {
        if (!hitSelf_)
            bodyDir_ = parseAngle(angle_);
        for (int i = 1; i <= speed_; i++) {
            if (hitSelf_) {
                if (path_.size() > snakeLen_) {
                    path_.pop_back();
                }
            } else {
                path_.push_front(
                    SnakePathNode(headNode.x + direction_.x * i * nodeRadius_ / 5.0,
                                  headNode.y + direction_.y * i * nodeRadius_ / 5.0));
                if (tailOverlap_) {
                    tailOverlap_--;
                } else {
                    path_.pop_back();
                }
            }
        }
    }
}

double dis2(sf::Vector2<float> node1,
            sf::Vector2<float> node2) noexcept {
    return std::sqrt(
        std::pow((static_cast<double>(node1.x) - static_cast<double>(node2.x)),
                 2) +
        std::pow((static_cast<double>(node1.y) - static_cast<double>(node2.y)),
                 2));
}

void checkVisionY(short speed, bool *hitSelf, UPPER_TYPE_VOL *pain, SnakePathNode *head,
                  vision *vision, SnakePathNode *i,
                  float nodeRadius, float *health, bool death) {
    for (int x = 0, y = 0; x < VISION_X_SUM; x++) {
        for (y = 0; y < VISION_Y_SUM; y++) {
            size_t index = CUL_VISION_INDEX(x, y);
            if (dis2(vision[index].pos, *i) < culSelfCollisionDis(nodeRadius)) {
                vision[index].color = VISION_HARM_COLOR;
            }
        }
    }

    if (speed && dis2(*head, *i) < culSelfCollisionDis(nodeRadius)) {
        // dieSound_.stop();
        // dieSound_.play();
        *hitSelf = true;
        if (death) {
            *pain += Game::cfg.bitePain * speed;
        } else {
            static TYPE_VOL tmp;
            tmp = Game::cfg.bitePain * speed;
            *pain += tmp;
            *health -= tmp;
        }
    }
}

void Snake::checkSelfCollisions(SnakePathNode head) {

    if (*deathFlag_) {
        return;
    }

    if (hitSelf_) {
        hitSelf_ = false;
    }
    for (auto i = path_.begin() + 15; i < path_.end(); i += 10) {
        utils::addThread(threads_, checkVisionY, speed_, &hitSelf_, &pain_, &head,
                         (vision *)vision_, &(*i), nodeRadius_, &health_, death_);
    }
    threads_.join();
}

bool inWindow(SnakePathNode &node) {
    return node.x >= 0 && node.x <= Game::GlobalVideoMode.width && node.y >= 0 &&
           node.y <= Game::GlobalVideoMode.height;
}

float culTimes(float x, float dx, int total) {
    return dx > 0.0f ? x / dx : (total - x) / -dx;
}

float culTimes2(float x, float dx, int total) {
    return dx < 0.0f ? x / -dx : (total - x) / dx;
}

void culOutWindowPos(float &pos, float &pos2, float &hpos, float &hpos2,
                     float dir, float dir2, unsigned int total,
                     unsigned int total2, float tanVal, float sin, float cos,
                     int num, float subsidy) {
    float posPlus = pos - total;

    if (culTimes(hpos, dir, total) < culTimes(hpos2, dir2, total2) &&
        culTimes2(hpos, dir, total) < culTimes2(hpos2, dir2, total2)) {
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

void reverse(float &x, float &y, float hx, float hy, float dx, float dy,
             int width, int height, float tanVal, float sin, float cos, int num,
             float subsidy) {
    const bool transX = transCoord(dx, x, hx, width),
               transY = transCoord(dy, y, hy, height);

    culOutWindowPos(x, y, hx, hy, dx, dy, width, height, tanVal, sin, cos, num,
                    subsidy);

    if (transX) {
        x = width - x;
    }

    if (transY) {
        y = height - y;
    }
}

bool Snake::toWindow(sf::Vector2f &node, SnakePathNode dir,
                     float tanVal) {
    return toWindow(node, dir, tanVal, 0, 0, -1, node);
}

bool Snake::toWindow(sf::Vector2f &node, SnakePathNode dir,
                     float tanVal, float sin, float cos, int num,
                     SnakePathNode head) {
    bool negativeX = node.x < 0, negativeY = node.y < 0,
         beyondX = negativeX || node.x > Game::GlobalVideoMode.width,
         beyondY = negativeY || node.y > Game::GlobalVideoMode.height;

    sf::Vector2f nodeTmp = node;
    if (beyondX) {
        if (dir.y == 0) {
            node.x = negativeX ? node.x + Game::GlobalVideoMode.width
                               : node.x - Game::GlobalVideoMode.width;
        } else if (culTimes2(head.x, dir.x, Game::GlobalVideoMode.width) >
                   culTimes2(head.y, dir.y, Game::GlobalVideoMode.height)) {
            float subsidy = VISION_HALF_WIDTH;
            if (num > -1 && ((dir.x > 0 && dir.y < 0) || (dir.x < 0 && dir.y > 0))) {
                num = abs(num + 1 - VISION_X_SUM);
                subsidy = VISION_HALF_WIDTH2;
            }

            reverse(node.y, node.x, head.y, head.x, dir.y, dir.x,
                    Game::GlobalVideoMode.height, Game::GlobalVideoMode.width,
                    1.0f / tanVal, cos, sin, num, subsidy);
        } else {
            float subsidy = VISION_HALF_WIDTH;
            if (num > -1 && ((dir.x > 0 && dir.y > 0) || (dir.x < 0 && dir.y < 0))) {
                num = abs(num + 1 - VISION_X_SUM);
                subsidy = VISION_HALF_WIDTH2;
            }

            reverse(node.x, node.y, head.x, head.y, dir.x, dir.y,
                    Game::GlobalVideoMode.width, Game::GlobalVideoMode.height, tanVal,
                    sin, cos, num, subsidy);
        }
    } else if (beyondY) {
        if (dir.x == 0) {
            node.y = negativeY ? node.y + Game::GlobalVideoMode.height
                               : node.y - Game::GlobalVideoMode.height;
        } else if (culTimes2(head.x, dir.x, Game::GlobalVideoMode.width) >
                   culTimes2(head.y, dir.y, Game::GlobalVideoMode.height)) {
            float subsidy = VISION_HALF_WIDTH;
            if (num > -1 && ((dir.x > 0 && dir.y < 0) || (dir.x < 0 && dir.y > 0))) {
                num = abs(num + 1 - VISION_X_SUM);
                subsidy = VISION_HALF_WIDTH2;
            }

            reverse(node.y, node.x, head.y, head.x, dir.y, dir.x,
                    Game::GlobalVideoMode.height, Game::GlobalVideoMode.width,
                    1.0f / tanVal, cos, sin, num, subsidy);
        } else {
            float subsidy = VISION_HALF_WIDTH;
            if (num > -1 && ((dir.x > 0 && dir.y > 0) || (dir.x < 0 && dir.y < 0))) {
                num = abs(num + 1 - VISION_X_SUM);
                subsidy = VISION_HALF_WIDTH2;
            }

            reverse(node.x, node.y, head.x, head.y, dir.x, dir.y,
                    Game::GlobalVideoMode.width, Game::GlobalVideoMode.height, tanVal,
                    sin, cos, num, subsidy);
        }
    }

    static sf::Vector2u TextureSize = headTexture_.getSize();
    static long xSize = TextureSize.x * 10, ySize = TextureSize.y * 10;
    if ((node.x < -xSize || node.x > Game::GlobalVideoMode.width + xSize) &&
        (node.y < -ySize || node.y > Game::GlobalVideoMode.height + ySize)) {
        reset();
    }

    return beyondX || beyondY;
}

void Snake::reset() {
    snakeLen_ = 10 * Game::cfg.initialSize;
    health_ = Game::cfg.heath;
    speedVitality_ = speedVitalityMax_;
    leftVitality_ = rightVitality_ = speed_ = delight_ = pain_ = stuckLeft_ = stuckRight_ = headAngle_ = turnRight_ = turnLeft_ = 0;
    angle_ = hisAngle_ = bodyDir_ = 180;
    radian_ = angle_ * PI / 180.0f;
    direction_ = Direction(0, -1);
    path_.clear();
    initNodes();
    hitSelf_ = false;

    for (int i = 0; i < Game::cfg.outputCnt; i++) {
        out_[i] = 0;
    }
}

void Snake::render(sf::RenderWindow &window) {

    if (*deathFlag_) {
        return;
    }

    pain_ = max(min(pain_, MAX_ENC), MIN_ENC);

    static short heaelthTick = Game::cfg.healthTick;
    if (death_) {
        health_ -= pain_ + heaelthTick;
    }

    if (health_ <= 0) {
        reset();

        if (death_) {
            *deathFlag_ = 1;
        }
        return;
    }

    static int count, i, j, x, y;
    j = 7;

    static utils::io_section *outSectionArr = Game::cfg.outputSectionArr,
                             outSection;
    static TYPE_VOL headAngle, angleTmp;

    outSection = outSectionArr[0]; // LTurn
    turnLeft_ *= maxVitality_;
    for (i = outSection.startIndex; i < outSection.endIndex; i++) {
        out_[i] = turnLeft_;
    }

    outSection = outSectionArr[1]; // LAngle
    headAngle = headAngle_ * maxVitality_;
    angleTmp = headAngle_ < 0 ? min(-headAngle, MAX_VOL) : 0;
    for (i = outSection.startIndex; i < outSection.endIndex; i++) {
        out_[i] = angleTmp;
    }

    outSection = outSectionArr[2]; // LStuck
    stuckLeft_ = min(stuckLeft_ * maxVitality_, MAX_VOL);
    for (i = outSection.startIndex; i < outSection.endIndex; i++) {
        out_[i] = stuckLeft_;
    }

    outSection = outSectionArr[4]; // pain
    for (i = outSection.startIndex; i < outSection.endIndex; i++) {
        out_[i] = pain_;
    }

    outSection = outSectionArr[5]; // RStuck
    stuckRight_ = min(stuckRight_ * maxVitality_, MAX_VOL);
    for (i = outSection.startIndex; i < outSection.endIndex; i++) {
        out_[i] = stuckRight_;
    }

    outSection = outSectionArr[6]; // RAngle
    angleTmp = headAngle_ > 0 ? min(headAngle, MAX_VOL) : 0;
    for (i = outSection.startIndex; i < outSection.endIndex; i++) {
        out_[i] = angleTmp;
    }

    outSection = outSectionArr[7]; // RTurn
    turnRight_ *= maxVitality_;
    for (i = outSection.startIndex; i < outSection.endIndex; i++) {
        out_[i] = turnRight_;
    }

    outSection = outSectionArr[9]; // delight
    for (i = outSection.startIndex; i < outSection.endIndex; i++) {
        out_[i] = delight_;
    }

    static TYPE_VOL *out_tmp;
    out_tmp = out_;

    static unsigned int selfVIndex = outSectionArr[3].startIndex,
                        fruitVIndex = outSectionArr[8].startIndex;

    static SnakePathNode lastSnakeNode, lastMiddleNode, nowSnakeNode;
    static float angle;
    angle = angle_;
    static sf::Vector2f body;
    static SnakePathNode wNowHeadNode;

    lastSnakeNode = *path_.begin();
    wNowHeadNode = lastSnakeNode;
    headSprite_.setPosition(wNowHeadNode);
    headSprite_.setRotation(angle);

    static sf::RectangleShape shape;
    shape = sf::RectangleShape();
    shape.setSize(sf::Vector2f(VISION_PIXEL_WIDTH, VISION_PIXEL_WIDTH));
    shape.setRotation(angle);
    static vision v;
    for (x = 0; x < VISION_X_SUM; x++) {
        for (y = 0; y < VISION_Y_SUM; y++, out_tmp++) {
            v = vision_[CUL_VISION_INDEX(x, y)];
            shape.setFillColor(sf::Color(v.color));
            shape.setPosition(v.pos);
            window.draw(shape);

            TYPE_VOL *val;
            switch (v.color) {
            case VISION_HARM_COLOR:
                val = out_tmp + selfVIndex;
                *val = vision_body_vol;
                val = out_tmp + fruitVIndex;
                *val = 0;
                break;
            case VISION_CHECK_COLOR:
                val = out_tmp + fruitVIndex;
                *val = vision_fruit_vol;
                val = out_tmp + selfVIndex;
                *val = 0;
                break;
            }
        }
    }

    if (delight_ > 0) {
        delight_ = max(delight_ - Game::cfg.delightConsum, 0.0);
    }

    renderNode(wNowHeadNode, headSprite_, window, 3);

    count = 5;
    for (auto i = path_.begin() + 5, end = path_.end(); i < end;
         i += 5, count += 5, j += 2) {
        body = *i;

        if (count % 2)
            lastMiddleNode = body;
        else {
            nowSnakeNode = body;
            angle = culAngle(nowSnakeNode - lastSnakeNode);
            nodeMiddle_.setRotation(angle);

            lastSnakeNode = nowSnakeNode;

            renderNode(nowSnakeNode, nodeShape_, window, 0);
            renderNode(lastMiddleNode, nodeMiddle_, window, 0);
        }
    }

    speed_ = 0;
}

template <typename T>
void Snake::renderNode(sf::Vector2f &nowPosition, T &shape,
                       sf::RenderWindow &window, int offset) {
    shape.setPosition(nowPosition);
    window.draw(shape);
}