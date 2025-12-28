

#include <cmath>

#include <SFML/Graphics.hpp>
#include <iostream>
#include <memory>

#include "element/Snake.h"
#include "Game.h"
#include "screen/GameOverScreen.h"
#include "utils/Time.hpp"

#define ANGLE_PLUS_THRESHOLD 180
#define ANGLE_PLUS_THRESHOLD2 60
#define ANGLE_PLUS_THRESHOLD3 160
#define STD_MAX std::max
#define STD_MIN std::min

#define MAX_VISION_ANGLE 180.0f

#define INITIAL_ANGLE 0

#define VISION_SECTION Game::cfg.outputSectionArr[Game::cfg.visionIndexes[0]]

using namespace sfSnake;

float culAngle(sf::Vector2f recDirection) {
    float angle = std::acos(recDirection.y / length(recDirection)) / PI * 180.f;
    if (recDirection.x > 0)
        return -angle;
    return angle;
}

float culAngleABS(sf::Vector2f recDirection) {
    return std::acos(recDirection.y / length(recDirection)) / PI * 180.f;
}

float culRadian(sf::Vector2f recDirection) {
    float result = std::acos(recDirection.y / length(recDirection));
    if (recDirection.x > 0)
        return -result;
    return result;
}

float culRadianABS(sf::Vector2f recDirection) {
    return std::acos(recDirection.y / length(recDirection));
}

void Snake::setAngle() {
    angleABS_ = culAngleABS(direction_);
    angle_ = direction_.x > 0 ? -angleABS_ : angleABS_;
}

Snake::Snake()
    : visionSum_(VISION_SECTION.endIndex - VISION_SECTION.startIndex),
      visionTriangle_(nullptr),
      visionAngle_(STD_MIN(MAX_VISION_ANGLE, Game::cfg.visionAngle)),
      halfVisionAngle_(visionAngle_ / 2),
      visionElAngle_(visionAngle_ / visionSum_),
      visionDistance_(Game::cfg.visionDistance),
      windowSize_(Game::cfg.windowSize),
      halfWindowSize_(windowSize_ / 2),
      centerPos_(sf::Vector2f(halfWindowSize_, halfWindowSize_)),
      windowDiameter_(sqrt(pow(windowSize_, 2) + pow(windowSize_, 2))),
      windowRadius_(windowDiameter_ / 2),
      windowRadiusPow_(pow(windowRadius_, 2)),
      hitSelf_(false),
      visionOutOfBounds_(false),
      stuckRight_(0),
      stuckLeft_(0),
      eatRight_(0),
      eatLeft_(0),
      turnLeft_(0),
      turnRight_(0),
      speed_(0),
      direction_(Direction(0, 1)),
      angle_(INITIAL_ANGLE),
      angleABS_(abs(angle_)),
      angleHis_(angle_),
      headAngleHis_(angle_),
      bodyDir_(angle_),
      headAngle_(0),
      radianToAngle_(180.f / PI),
      radian_(angle_ / radianToAngle_),
      nodeRadius_(Game::GlobalVideoMode.width / 100.0f),
      nodeRadius2_(nodeRadius_ / 5.f),
      nodeDiameter_(nodeRadius_ * 2),
      tailOverlap_(0u),
      nodeShape_(nodeRadius_),
      nodeMiddle_(sf::Vector2f(nodeRadius_ * std::sqrt(3), nodeRadius_)),
      score_(Game::cfg.initialSize) {

    visionTriangle_ = new el_triangle[visionSum_];

    visionTriangleEnd_ = visionTriangle_ + visionSum_;

    elAngleRange_ = visionAngle_ / visionSum_;

    float visionPadding_ = (VISION_PIXEL_WIDTH + nodeRadius_) / 2;

    float halfElRange = elAngleRange_ / 2 / radianToAngle_;
    float visionDistance = visionDistance_;
    float sinVal = sin(halfElRange) * visionDistance,
          cosVal = cos(halfElRange) * visionDistance + visionPadding_;
    visionDistance += visionPadding_;

    el_triangle *visionTriangleStart = visionTriangle_;

    double startRange = halfVisionAngle_;
    while (visionTriangleStart != visionTriangleEnd_) {
        visionTriangleStart->color = VISION_DEF_COLOR;
        visionTriangleStart->distance = MAX_DISTANCE;
        visionTriangleStart->triangle.setPointCount(4);
        visionTriangleStart->triangle.setPoint(0, sf::Vector2f(0, visionPadding_));
        visionTriangleStart->triangle.setPoint(1, sf::Vector2f(sinVal, cosVal));
        visionTriangleStart->triangle.setPoint(2, sf::Vector2f(0, visionDistance));
        visionTriangleStart->triangle.setPoint(3, sf::Vector2f(-sinVal, cosVal));
        visionTriangleStart->triangle.rotate(startRange);
        visionTriangleStart++;
        startRange -= elAngleRange_;
    }

    snakeLen_ = 10 * Game::cfg.initialSize + 1;

    health_ = Game::cfg.heath;

    death_ = Game::cfg.death;

    healthVal_ = Game::cfg.heath;

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

    // pickupBuffer_.loadFromFile("assets/sounds/pickup.wav");
    // pickupSound_.setBuffer(pickupBuffer_);
    // pickupSound_.setVolume(30);

    // dieBuffer_.loadFromFile("assets/sounds/die.wav");
    // dieSound_.setBuffer(dieBuffer_);
    // dieSound_.setVolume(50);
    threads_.init(Game::cfg.threadsCnt);

    in_ = Game::share.getReadPos();
    out_ = Game::share.getWritePos();

    deathFlag_ = out_ - 1;
}

Snake::~Snake() {
    threads_.join();
    if (visionTriangle_) {
        delete[] visionTriangle_;
    }
}

void Snake::initNodes() {
    path_.push_back(SnakePathNode(centerPos_.x, centerPos_.y));
    for (int i = 1; i <= snakeLen_; i++) {
        path_.push_back(SnakePathNode(centerPos_.x -
                                          direction_.x * i * nodeRadius2_,
                                      centerPos_.y -
                                          direction_.y * i * nodeRadius2_));
    }

    headPos_ = &path_.front();
}

void Snake::handleInput(sf::RenderWindow &window) {

    if (*deathFlag_) {
        return;
    }

    static sf::Vector2i mousePosition;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
        direction_ = Direction(0, -1);

        setAngle();
        radian_ = angle_ / radianToAngle_;
    } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) ||
               sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
        direction_ = Direction(0, 1);

        setAngle();
        radian_ = angle_ / radianToAngle_;
    } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) ||
               sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
        direction_ = Direction(-1, 0);

        setAngle();
        radian_ = angle_ / radianToAngle_;
    } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) ||
               sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
        direction_ = Direction(1, 0);

        setAngle();
        radian_ = angle_ / radianToAngle_;
    }

    if (!Game::mouseButtonLocked) {
        if (sf::Mouse::isButtonPressed(sf::Mouse::Left) ||
            sf::Mouse::isButtonPressed(sf::Mouse::Right)) {
            mousePosition = sf::Mouse::getPosition(window);
            handleInput(mousePosition, window);

            setAngle();
            radian_ = angle_ / radianToAngle_;
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

    if (*deathFlag_) {
        return;
    }

    static utils::io_section *section = Game::cfg.inputSectionArr;
    static TYPE_VOL *inputPtr,
        *leftPtrEnd = in_ + section[0].endIndex,
        *runPtrEnd = in_ + section[1].endIndex,
        *rightPtrEnd = in_ + section[2].endIndex;

    static UPPER_TYPE_VOL plus;
    plus = 0;

    inputPtr = in_ + section[0].startIndex;
    do {
        plus -= *inputPtr;
        inputPtr++;
    } while (inputPtr != leftPtrEnd);

    speed_ = 0;
    inputPtr = in_ + section[1].startIndex;
    do {
        speed_ += *inputPtr;
        inputPtr++;
    } while (inputPtr != runPtrEnd);

    static TYPE_VOL brakeThreshold = Game::cfg.brakeThreshold;

    speed_ = speed_ > brakeThreshold ? 0 : 1;

    plus = 0;
    inputPtr = in_ + section[2].startIndex;
    do {
        plus += *inputPtr;
        inputPtr++;
    } while (inputPtr != rightPtrEnd);

    static float angle;
    if (plus) {
        if (plus > ANGLE_PLUS_THRESHOLD3) {
            plus = ANGLE_PLUS_THRESHOLD3;
        } else if (plus < -ANGLE_PLUS_THRESHOLD3) {
            plus = -ANGLE_PLUS_THRESHOLD3;
        }

        angle = parseAngle(parseAngle2(angle_ + plus));

        headAngle_ = angle - bodyDir_;
        if (headAngle_ > ANGLE_PLUS_THRESHOLD) {
            headAngle_ = headAngle_ - 360;
        } else if (headAngle_ < -ANGLE_PLUS_THRESHOLD) {
            headAngle_ = 360 + headAngle_;
        }
        if (headAngle_ > 0) {
            if (headAngle_ > ANGLE_PLUS_THRESHOLD2) {
                angle = parseAngle2(bodyDir_ + ANGLE_PLUS_THRESHOLD2);
                stuckRight_ += (headAngle_ - ANGLE_PLUS_THRESHOLD2) * 10;
            }
        } else if (headAngle_ < 0) {
            if (headAngle_ < -ANGLE_PLUS_THRESHOLD2) {
                angle = parseAngle2(bodyDir_ - ANGLE_PLUS_THRESHOLD2);
                stuckLeft_ -= (ANGLE_PLUS_THRESHOLD2 + headAngle_) * 10;
            }
        }

        angle_ = angle;
        angleABS_ = abs(angle);

        plus = ((headAngleHis_ > 0 && headAngle_ > 0) || (headAngleHis_ < 0 && headAngle_ < 0)
                    ? headAngleHis_ - headAngle_
                    : headAngleHis_ + headAngle_) *
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

        radian_ = angle_ / radianToAngle_;

        direction_.y += cos(radian_) * headTexture_.getSize().y;
        direction_.x -= sin(radian_) * headTexture_.getSize().y;

        static double directionSize;
        directionSize = length(direction_);
        direction_.x /= directionSize;
        direction_.y /= directionSize;

        headAngleHis_ = headAngle_;
    } else {
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

    static float angleDiff;
    angleDiff = angle_ - angleHis_;

    if (angleDiff) {

        static el_triangle *visionTriangleStart;
        visionTriangleStart = visionTriangle_;
        while (visionTriangleStart < visionTriangleEnd_) {
            visionTriangleStart->triangle.rotate(angleDiff);
            visionTriangleStart++;
        }

        angleHis_ = angle_;
    }

    static float distance;
    static SnakePathNode pos;

    if (speed_) {

        static SnakePathNode headPos;
        headPos = path_.front();
        headPos = SnakePathNode(headPos.x + direction_.x * speed_ * nodeRadius2_,
                                headPos.y + direction_.y * speed_ * nodeRadius2_);

        headPos_ = &headPos;
        distance = dis(centerPos_, headPos);

        checkSelfCollisions();

        if (hitSelf_ && distance > windowRadius_) {
            headPos_ = &path_.front();
            distance = dis(centerPos_, *headPos_);
            for (int i = 1; i <= speed_; i++) {
                if (path_.size() > snakeLen_) {
                    path_.pop_back();
                }
            }
            pos = *headPos_ - centerPos_;
        } else {
            move(path_.front());
            headPos_ = &path_.front();
            distance = dis(centerPos_, *headPos_);
            pos = *headPos_ - centerPos_;
        }
    } else {
        headPos_ = &path_.front();
        distance = dis(centerPos_, *headPos_);
        pos = *headPos_ - centerPos_;
    }
    look(distance, pos);

    if (distance > windowRadius_) {
        distance = dis(centerPos_, headOutPos_);
        if (distance <= windowRadius_) {
            headPos_->x = headOutPos_.x;
            headPos_->y = headOutPos_.y;
        } else {
            std::cout << distance - windowRadius_ << std::endl;
            reset();
        }
    }
    lookSelf();
}

void Snake::look(float distance, SnakePathNode pos) {

    if (distance + visionDistance_ > windowRadius_) {

        // std::cout << culAngle(headPos_ - centerPos_) << std::endl;
        static float max[2], min[2], posAngle;
        static bool maxOutOfBound, minOutOfBound;

        posAngle = culAngle(pos);

        *max = posAngle + halfVisionAngle_;
        *min = posAngle - halfVisionAngle_;

        if (*max > 180) {
            max[1] = *max - 360;
            *max = 180;
            maxOutOfBound = true;
        } else {
            maxOutOfBound = false;
        }
        if (*min < -180) {
            min[1] = *min + 360;
            *min = -180;
            minOutOfBound = true;
        } else {
            minOutOfBound = false;
        }

        if ((angle_ <= *max && angle_ >= *min) ||
            (maxOutOfBound && angle_ <= max[1] && angle_ >= -180) ||
            (minOutOfBound && angle_ <= 180 && angle_ >= min[1])) {

            if (!angleABS_) {
                headOutPos_.y = headPos_->y - sqrtf(windowRadiusPow_ - powf(pos.x, 2)) * 2;
                headOutPos_.x = headPos_->x;
            } else if (angleABS_ == 180) {
                headOutPos_.y = headPos_->y + sqrtf(windowRadiusPow_ - powf(pos.x, 2)) * 2;
                headOutPos_.x = headPos_->x;
            } else {
                static float hypotenuse, radianA, cosVal, sinVal, posX;
                radianA = abs(90.f - angleABS_) / radianToAngle_;
                posX = direction_.x <= 0 ? pos.x : -pos.x;
                hypotenuse = abs(pos.y) + tanf(radianA) * posX;
                cosVal = cosf(radianA);
                sinVal = sinf(radianA);
                hypotenuse = sqrtf(windowRadiusPow_ - powf(hypotenuse * cosVal, 2)) * 2;

                if (angle_ < 0) {
                    headOutPos_.x = headPos_->x - cosVal * hypotenuse;
                } else if (angle_ > 0) {
                    headOutPos_.x = headPos_->x + cosVal * hypotenuse;
                }

                if (angleABS_ >= 90) {
                    headOutPos_.y = headPos_->y + sinVal * hypotenuse;
                } else if (angleABS_ < 90) {
                    headOutPos_.y = headPos_->y - sinVal * hypotenuse;
                }
            }

            visionOutOfBounds_ = true;
        } else {
            visionOutOfBounds_ = false;
        }
    }
}

void checkVision(sf::Vector2f pos, float distance, float angle, float itemRadius, float halfVisionAngle,
                 float visionAngle, float radianToAngle, float visionElAngle,
                 sf::Vector2f *headPos, utils::Threads *threads,
                 el_triangle *visionTriangleStart, el_triangle *visionTriangleEnd, sf::Uint32 color) {

    float posAngle = culAngle(pos - *headPos),
          itemAngle = atanf(itemRadius / distance) * radianToAngle;

    float max, min, max2, min2, maxDiff, minDiff;
    bool compareRes = false;

    max = posAngle + itemAngle;
    if (max > 180) {
        max -= 360;
    }
    min = posAngle - itemAngle;
    if (min < -180) {
        min += 360;
    }

    itemAngle *= 2;

    max2 = angle + halfVisionAngle;
    if (max2 > 180) {
        max2 -= 360;
    }
    min2 = angle - halfVisionAngle;
    if (min2 < -180) {
        min2 += 360;
    }

    maxDiff = max - max2;
    minDiff = min - min2;

    if (maxDiff > 0) {
        if (compareRes = maxDiff <= itemAngle) {
            if (minDiff < -180) {
                minDiff = 360 + minDiff;
            }
        }
    } else if (compareRes = -maxDiff <= visionAngle) {
        visionTriangleStart -= (long)(maxDiff / visionElAngle);
        if (minDiff < -180) {
            minDiff = 360 + minDiff;
        }
    }

    if (minDiff > 0) {
        if (minDiff <= visionAngle) {
            visionTriangleEnd -= (long)(minDiff / visionElAngle);
            compareRes = true;
            if (maxDiff > 180) {
                maxDiff = 360 - maxDiff;
                if (maxDiff <= visionAngle) {
                    visionTriangleStart += (long)(maxDiff / visionElAngle);
                }
            }
        }
    } else if (-minDiff <= itemAngle) {
        compareRes = true;
        if (maxDiff > 180) {
            maxDiff = 360 - maxDiff;
            if (maxDiff <= visionAngle) {
                visionTriangleStart += (long)(maxDiff / visionElAngle);
            }
        }
    }

    if (compareRes) {
        while (visionTriangleStart < visionTriangleEnd) {
            threads->mtx.lock();
            if (visionTriangleStart->distance > distance) {
                visionTriangleStart->distance = distance;
                visionTriangleStart->color = color;
                threads->mtx.unlock();
            } else {
                threads->mtx.unlock();
            }
            visionTriangleStart++;
        }
    }
}

void Snake::checkFruitCollisions(std::deque<Fruit> &fruits) {
    static SnakePathNode headnode;
    headnode = path_.front();

    static float fruitRadius = fruits.begin()->shape_.getRadius(),
                 padding = fruitRadius * 2 + nodeRadius2_,
                 distance;

    for (auto i = fruits.begin(); i != fruits.end(); ++i) {

        const sf::Vector2f &pos = i->shape_.getPosition();

        distance = dis(pos, *headPos_) - padding;

        if (distance < visionDistance_) {
            utils::addThread(threads_, checkVision, pos, distance, angle_, fruitRadius, halfVisionAngle_,
                             visionAngle_, radianToAngle_, visionElAngle_,
                             headPos_, &threads_, visionTriangle_, visionTriangleEnd_, VISION_CHECK_COLOR);
        }
        if (visionOutOfBounds_) {
            distance = dis(pos, headOutPos_) - padding;
            if (distance < visionDistance_) {
                utils::addThread(threads_, checkVision, pos, distance, angle_, fruitRadius, halfVisionAngle_,
                                 visionAngle_, radianToAngle_, visionElAngle_,
                                 &headOutPos_, &threads_, visionTriangle_, visionTriangleEnd_, VISION_CHECK_COLOR);
            }
        }

        if (dis(pos, headnode) <
            nodeRadius_ + i->shape_.getRadius()) {
            // pickupSound_.play();
            grow(i->score_);
            fruits.erase(i);
            static float angleDiff;
            angleDiff = culAngle(pos - *headPos_) - angle_;
            if (angleDiff > 180) {
                angleDiff -= 360;
            } else if (angleDiff < -180) {
                angleDiff += 360;
            }
            if (angleDiff > 0) {
                eatRight_ = Game::cfg.eatDelight;
            } else if (angleDiff < 0) {
                eatLeft_ = Game::cfg.eatDelight;
            } else {
                eatRight_ = eatLeft_ = Game::cfg.eatDelight;
            }
            // headAngle_ = 0;
        }
    }
    threads_.join();
}

void Snake::grow(int score) {
    tailOverlap_ += score * 10;
    score_ += score;
}

unsigned Snake::getScore() const { return score_; }

bool Snake::hitSelf() const { return hitSelf_; }

void Snake::move(SnakePathNode headNode) {

    if (speed_ > 0) {
        static int i;
        if (hitSelf_) {
            i = 0;
            if (path_.size() > snakeLen_) {
                do {
                    path_.pop_back();
                    i++;
                } while (i < speed_ && path_.size() > snakeLen_);
            }
        } else {
            bodyDir_ = parseAngle(angle_);
            i = 1;
            do {
                path_.push_front(
                    SnakePathNode(headNode.x + direction_.x * i * nodeRadius2_,
                                  headNode.y + direction_.y * i * nodeRadius2_));
                if (tailOverlap_) {
                    tailOverlap_--;
                } else {
                    path_.pop_back();
                }
                i++;
            } while (i <= speed_);
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

void Snake::lookSelf() {

    if (*deathFlag_) {
        return;
    }

    if (hitSelf_) {
        hitSelf_ = false;
    }

    static float distance;

    for (auto i = path_.begin() + nodeRadius_; i < path_.end(); i += 10) {

        distance = dis(*i, *headPos_);

        if (distance < visionDistance_) {
            utils::addThread(threads_, checkVision, *i, distance, angle_, nodeRadius_, halfVisionAngle_,
                             visionAngle_, radianToAngle_, visionElAngle_,
                             headPos_, &threads_, visionTriangle_, visionTriangleEnd_, VISION_HARM_COLOR);
        }

        if (visionOutOfBounds_) {
            distance = dis(*i, headOutPos_);
            if (distance < visionDistance_) {
                utils::addThread(threads_, checkVision, *i, distance, angle_, nodeRadius_, halfVisionAngle_,
                                 visionAngle_, radianToAngle_, visionElAngle_,
                                 &headOutPos_, &threads_, visionTriangle_, visionTriangleEnd_, VISION_HARM_COLOR);
            }
        }
    }
    threads_.join();
}

void Snake::checkSelfCollisions() {

    if (*deathFlag_) {
        return;
    }

    if (hitSelf_) {
        hitSelf_ = false;
    }

    for (auto i = path_.begin() + nodeRadius_; i < path_.end(); i += 10) {

        if (dis2(*headPos_, *i) < nodeDiameter_) {

            static float angleDiff;
            angleDiff = culAngle(*i - *headPos_) - angle_;
            if (angleDiff > 180) {
                angleDiff -= 360;
            } else if (angleDiff < -180) {
                angleDiff += 360;
            }
            if (angleDiff > 0) {
                if (angleDiff < 90) {
                    hitSelf_ = true;
                    // dieSound_.stop();
                    // dieSound_.play();
                    if (death_) {
                        stuckRight_ = Game::cfg.bitePain * speed_;
                    } else {
                        static TYPE_VOL tmp;
                        tmp = Game::cfg.bitePain * speed_;
                        stuckRight_ = tmp;
                        health_ -= tmp;
                    }
                }
            } else if (angleDiff < 0) {
                if (angleDiff > -90) {
                    hitSelf_ = true;
                    // dieSound_.stop();
                    // dieSound_.play();
                    if (death_) {
                        stuckLeft_ = Game::cfg.bitePain * speed_;
                    } else {
                        static TYPE_VOL tmp;
                        tmp = Game::cfg.bitePain * speed_;
                        stuckLeft_ = tmp;
                        health_ -= tmp;
                    }
                }
            } else {
                hitSelf_ = true;
                // dieSound_.stop();
                // dieSound_.play();
                if (death_) {
                    stuckRight_ = stuckLeft_ = Game::cfg.bitePain * speed_;
                } else {
                    static TYPE_VOL tmp;
                    tmp = Game::cfg.bitePain * speed_;
                    stuckRight_ = stuckLeft_ = tmp;
                    health_ -= tmp;
                }
            }
        }
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

void Snake::reset() {
    snakeLen_ = 10 * Game::cfg.initialSize;
    health_ = Game::cfg.heath;

    speed_ = eatLeft_ = eatRight_ = stuckLeft_ = stuckRight_ = headAngle_ = turnRight_ = turnLeft_ = headAngleHis_ = bodyDir_ = 0;

    float angleDiff;
    angleDiff = INITIAL_ANGLE - angleHis_;
    if (angleDiff) {
        el_triangle *visionTriangleStart;
        visionTriangleStart = visionTriangle_;
        while (visionTriangleStart < visionTriangleEnd_) {
            visionTriangleStart->triangle.rotate(angleDiff);
            visionTriangleStart++;
        }
    }

    angle_ = angleHis_ = INITIAL_ANGLE;
    angleABS_ = abs(angle_);
    radian_ = angle_ / radianToAngle_;
    direction_ = Direction(0, 1);
    setAngle();
    path_.clear();
    initNodes();
    hitSelf_ = false;
    visionOutOfBounds_ = false;

    for (int i = 0; i < Game::cfg.outputCnt; i++) {
        out_[i] = 0;
    }
}

void Snake::render(sf::RenderWindow &window) {

    if (*deathFlag_) {
        return;
    }

    stuckLeft_ = STD_MIN(stuckLeft_, MAX_VOL);
    stuckRight_ = STD_MIN(stuckRight_, MAX_VOL);
    eatRight_ = STD_MIN(eatRight_, MAX_VOL);
    eatLeft_ = STD_MIN(eatLeft_, MAX_VOL);

    static int healthTick = Game::cfg.healthTick;
    if (death_) {
        health_ -= stuckRight_ + stuckLeft_ + healthTick - eatRight_ - eatLeft_;
        if (health_ > healthVal_) {
            health_ = healthVal_;
        }
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
    static TYPE_VOL angleTmp;

    outSection = outSectionArr[0]; // LTurn
    for (i = outSection.startIndex; i < outSection.endIndex; i++) {
        out_[i] = turnLeft_;
    }

    // if (turnLeft_) {
    //     std::cout << "turnLeft_: " << turnLeft_ << std::endl;
    // }

    outSection = outSectionArr[1]; // LAngle
    angleTmp = headAngle_ < 0 ? STD_MIN(-headAngle_, MAX_VOL) : 0;
    for (i = outSection.startIndex; i < outSection.endIndex; i++) {
        out_[i] = angleTmp;
    }

    // if (angleTmp) {
    //     std::cout << "LAngle: " << angleTmp << std::endl;
    // }

    outSection = outSectionArr[2]; // LStuck
    for (i = outSection.startIndex; i < outSection.endIndex; i++) {
        out_[i] = stuckLeft_;
    }

    // if (stuckLeft_) {
    //     std::cout << "stuckLeft_: " << stuckLeft_ << std::endl;
    // }

    outSection = outSectionArr[3]; // LStuck
    for (i = outSection.startIndex; i < outSection.endIndex; i++) {
        out_[i] = eatLeft_;
    }

    // if (eatLeft_) {
    //     std::cout << "eatLeft_: " << eatLeft_ << std::endl;
    // }

    outSection = outSectionArr[7]; // RStuck
    for (i = outSection.startIndex; i < outSection.endIndex; i++) {
        out_[i] = eatRight_;
    }

    // if (eatRight_) {
    //     std::cout << "eatRight_: " << eatRight_ << std::endl;
    // }

    outSection = outSectionArr[8]; // RStuck
    for (i = outSection.startIndex; i < outSection.endIndex; i++) {
        out_[i] = stuckRight_;
    }

    // if (stuckRight_) {
    //     std::cout << "stuckRight_: " << stuckRight_ << std::endl;
    // }

    outSection = outSectionArr[9]; // RAngle
    angleTmp = headAngle_ > 0 ? STD_MIN(headAngle_, MAX_VOL) : 0;
    for (i = outSection.startIndex; i < outSection.endIndex; i++) {
        out_[i] = angleTmp;
    }

    // if (angleTmp) {
    //     std::cout << "RAngle: " << angleTmp << std::endl;
    // }

    outSection = outSectionArr[10]; // RTurn
    for (i = outSection.startIndex; i < outSection.endIndex; i++) {
        out_[i] = turnRight_;
    }

    // if (turnRight_) {
    //     std::cout << "turnRight_: " << turnRight_ << std::endl;
    // }

    static TYPE_VOL *out_tmp;
    out_tmp = out_;

    static unsigned int vision1Index = outSectionArr[Game::cfg.visionIndexes[0]].startIndex,
                        vision2Index = outSectionArr[Game::cfg.visionIndexes[1]].startIndex,
                        vision3Index = outSectionArr[Game::cfg.visionIndexes[2]].startIndex;

    static SnakePathNode lastSnakeNode, lastMiddleNode, nowSnakeNode;
    static sf::Vector2f body;
    static SnakePathNode wNowHeadNode;

    lastSnakeNode = *path_.begin();
    wNowHeadNode = lastSnakeNode;
    headSprite_.setPosition(wNowHeadNode);
    headSprite_.setRotation(angle_);

    static TYPE_VOL *vision_blank_vol = Game::cfg.visionBlankVol,
                    *vision_fruit_vol = Game::cfg.visionFruitVol,
                    *vision_body_vol = Game::cfg.visionBodyVol;

    static el_triangle *visionStart;
    visionStart = visionTriangle_;

    while (visionStart != visionTriangleEnd_) {
        visionStart->triangle.setFillColor(sf::Color(visionStart->color));
        visionStart->triangle.setPosition(*headPos_);
        window.draw(visionStart->triangle);
        if (visionOutOfBounds_) {
            visionStart->triangle.setPosition(headOutPos_);
            window.draw(visionStart->triangle);
        }

        switch (visionStart->color) {
        case VISION_HARM_COLOR:
            *(out_tmp + vision1Index) = vision_body_vol[0];
            *(out_tmp + vision2Index) = vision_body_vol[1];
            *(out_tmp + vision3Index) = vision_body_vol[2];
            break;
        case VISION_CHECK_COLOR:
            *(out_tmp + vision1Index) = vision_fruit_vol[0];
            *(out_tmp + vision2Index) = vision_fruit_vol[1];
            *(out_tmp + vision3Index) = vision_fruit_vol[2];
            break;
        default:
            *(out_tmp + vision1Index) = vision_blank_vol[0];
            *(out_tmp + vision2Index) = vision_blank_vol[1];
            *(out_tmp + vision3Index) = vision_blank_vol[2];
        }

        visionStart->color = VISION_DEF_COLOR;
        visionStart->distance = MAX_DISTANCE;

        out_tmp++;
        visionStart++;
    }

    if (eatRight_ > 0) {
        eatRight_ = STD_MAX(eatRight_ - Game::cfg.delightConsum, 0.f);
    }

    if (eatLeft_ > 0) {
        eatLeft_ = STD_MAX(eatLeft_ - Game::cfg.delightConsum, 0.f);
    }

    if (stuckRight_ > 0) {
        stuckRight_ = STD_MAX(stuckRight_ - Game::cfg.painConsum, 0.f);
    }

    if (stuckLeft_ > 0) {
        stuckLeft_ = STD_MAX(stuckLeft_ - Game::cfg.painConsum, 0.f);
    }

    renderNode(wNowHeadNode, headSprite_, window, 3);

    count = 5;
    static float angle;
    angle = angle_;
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