

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
#define STD_MAX std::max
#define STD_MIN std::min

#define MAX_VISION_ANGLE 180.0f

#define INITIAL_ANGLE 0

#define VISION_SECTION Game::cfg.outputSectionArr[Game::cfg.visionIndexes[0]]

using namespace sfSnake;

// static unsigned int vision_blank_vol, vision_fruit_vol, vision_body_vol;

#define INIT_TRIANGLE(triangle, sinVal, cosVal, visionDistance, visionPadding) \
    do {                                                                       \
        triangle.setPointCount(4);                                             \
        triangle.setPoint(0, sf::Vector2f(0, visionPadding));                  \
        triangle.setPoint(1, sf::Vector2f(sinVal, cosVal));                    \
        triangle.setPoint(2, sf::Vector2f(0, visionDistance));                 \
        triangle.setPoint(3, sf::Vector2f(-sinVal, cosVal));                   \
        triangle.setFillColor(sf::Color(VISION_DEF_COLOR));                    \
    } while (0)

float culAngle(sf::Vector2f recDirection) {
    float angle = std::acos(recDirection.y / length(recDirection)) / PI * 180.0;
    if (recDirection.x > 0)
        return -angle;
    return angle;
}

float culAngleABS(sf::Vector2f recDirection) {
    return std::acos(recDirection.y / length(recDirection)) / PI * 180.0;
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
      visionSumOdd_(visionSum_ % 2),
      visionRangeSum_(visionSum_ / 2 + visionSumOdd_),
      visionRange_(nullptr),
      visionRangeEnd_(nullptr),
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
      hitSelf_(false),
      outOfBounds_(false),
      visionOutOfBounds_(false),
      pain_(0),
      delight_(0),
      turnLeft_(0),
      turnRight_(0),
      injureLeft_(0),
      injureRight_(0),
      leftVitality_(0),
      rightVitality_(0),
      speed_(0),
      direction_(Direction(0, 1)),
      angle_(INITIAL_ANGLE),
      angleHis_(angle_),
      headAngleHis_(angle_),
      bodyDir_(angle_),
      headAngle_(0),
      radianToAngle_(180.f / PI),
      radian_(angle_ / radianToAngle_),
      nodeRadius_(Game::GlobalVideoMode.width / 100.0f),
      nodeRadius2_(nodeRadius_ / 5.f),
      tailOverlap_(0u),
      nodeShape_(nodeRadius_),
      nodeMiddle_(sf::Vector2f(nodeRadius_ * std::sqrt(3), nodeRadius_)),
      score_(Game::cfg.initialSize) {

    visionRange_ = (el_diff_range *)malloc(sizeof(el_diff_range) * visionRangeSum_);
    visionTriangle_ = new el_triangle[visionSum_];

    visionRangeEnd_ = visionRange_ + visionRangeSum_;
    visionTriangleEnd_ = visionTriangle_ + visionSum_;
    visionTriangleLast_ = visionTriangleEnd_ - 1;

    elAngleRange_ = visionAngle_ / visionSum_;

    float halfElRange = elAngleRange_ / 2;
    float visionDistance = visionDistance_;
    float visionPadding_ = (VISION_PIXEL_WIDTH + nodeRadius_) / 2;
    float sinVal = sin(halfElRange) * visionDistance,
          cosVal = cos(halfElRange) * visionDistance + visionPadding_;

    visionDistance += visionPadding_;

    el_triangle *visionTriangleLast = visionTriangleLast_,
                *visionTriangleStart = visionTriangle_;

    float startRange = halfVisionAngle_;
    if (visionSumOdd_) {
        el_diff_range *visionRangeStart = visionRangeEnd_ - 1;
        while (visionRangeStart < visionRangeEnd_) {
            visionTriangleLast->color = VISION_DEF_COLOR;
            INIT_TRIANGLE(visionTriangleLast->triangle, sinVal, cosVal, visionDistance, visionPadding_);
            visionTriangleLast->triangle.rotate(startRange);
            visionTriangleLast--;
            visionTriangleStart->color = VISION_DEF_COLOR;
            INIT_TRIANGLE(visionTriangleStart->triangle, sinVal, cosVal, visionDistance, visionPadding_);
            visionTriangleStart->triangle.rotate(-startRange);
            visionTriangleStart++;

            visionRangeStart->minAngle = startRange;
            visionRangeStart->maxAngle = startRange -= elAngleRange_;
            visionRangeStart++;
        }
        visionRangeStart->maxAngle = halfElRange;
        visionRangeStart->minAngle = 0;
        INIT_TRIANGLE(visionTriangleLast->triangle, sinVal, cosVal, visionDistance, visionPadding_);
    } else {
        el_diff_range *visionRangeStart = visionRange_;
        while (visionRangeStart != visionRangeEnd_) {
            visionTriangleLast->color = VISION_DEF_COLOR;
            INIT_TRIANGLE(visionTriangleLast->triangle, sinVal, cosVal, visionDistance, visionPadding_);
            visionTriangleLast->triangle.rotate(startRange);
            visionTriangleLast--;
            visionTriangleStart->color = VISION_DEF_COLOR;
            INIT_TRIANGLE(visionTriangleStart->triangle, sinVal, cosVal, visionDistance, visionPadding_);
            visionTriangleStart->triangle.rotate(-startRange);
            visionTriangleStart++;

            visionRangeStart->minAngle = startRange;
            visionRangeStart->maxAngle = startRange -= elAngleRange_;
            visionRangeStart++;
        }
    }

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
    if (visionRange_) {
        free(visionRange_);
    }
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

    static TYPE_VOL speed_level1 = Game::cfg.speedLevel1,
                    speed_level2 = Game::cfg.speedLevel2;

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
                injureRight_ = (headAngle_ - ANGLE_PLUS_THRESHOLD2) * 10;
                if (injureRight_ > MAX_VOL) {
                    injureRight_ = MAX_VOL;
                }
                pain_ += injureRight_;
            } else {
                injureRight_ = 0;
            }
            injureLeft_ = 0;
        } else if (headAngle_ < 0) {
            if (headAngle_ < -ANGLE_PLUS_THRESHOLD2) {
                angle_ = parseAngle2(bodyDir_ - ANGLE_PLUS_THRESHOLD2);
                injureLeft_ = -(ANGLE_PLUS_THRESHOLD2 + headAngle_) * 10;
                if (injureLeft_ > MAX_VOL) {
                    injureLeft_ = MAX_VOL;
                }
                pain_ += injureLeft_;
            } else {
                injureLeft_ = 0;
            }
            injureRight_ = 0;
        } else {
            injureRight_ = 0;
            injureLeft_ = 0;
        }

        // cout << "\t" << angle_ << endl;

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

        // printf("angle_: %f\n", angle_);
    } else {
        injureRight_ = 0;
        injureLeft_ = 0;
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

    if (turnRight_) {
        leftVitality_ = STD_MIN(leftVitality_ + (maxVitality_ - leftVitality_) / vitalityStepCnt_, maxVitality_);
        rightVitality_ = STD_MAX(rightVitality_ - turnRight_, minVitality_);
    } else if (turnLeft_) {
        leftVitality_ = STD_MAX(leftVitality_ - turnLeft_, minVitality_);
        rightVitality_ = STD_MIN(rightVitality_ + (maxVitality_ - rightVitality_) / vitalityStepCnt_, maxVitality_);
    } else {
        rightVitality_ = STD_MIN(rightVitality_ + (maxVitality_ - rightVitality_) / vitalityStepCnt_, maxVitality_);
        leftVitality_ = STD_MIN(leftVitality_ + (maxVitality_ - leftVitality_) / vitalityStepCnt_, maxVitality_);
    }

    if (leftVitality_) {
        pain_ += abs(leftVitality_) * vitalityPain2_;
    }
    if (rightVitality_) {
        pain_ += abs(rightVitality_) * vitalityPain2_;
    }

    if (speed_) {
        if (speedVitality_) {
            speedVitality_ = STD_MAX(speedVitality_ - speedVitality_ * speed_ / vitalityStepCnt_, 0.0f);
        }
    } else {
        speedVitality_ = STD_MIN(speedVitality_ + (maxVitality_ - speedVitality_) / vitalityStepCnt_, maxVitality_);
    }

    if (speedVitality_ > speedVitalityMax_) {
        pain_ += vitalityPain_ * (speedVitality_ - speedVitalityMax_) / speedVitalityDiff_;
    }

    static float distance;

    if (speed_) {

        visionOutOfBounds_ = false;

        static SnakePathNode headPos;
        headPos = path_.front();
        headPos = SnakePathNode(headPos.x + direction_.x * speed_ * nodeRadius2_,
                                headPos.y + direction_.y * speed_ * nodeRadius2_);

        headPos_ = &headPos;
        distance = dis(centerPos_, headPos);

        look(distance);
        checkSelfCollisions();
        if (hitSelf_ && distance > windowRadius_) {
            headPos_ = &path_.front();
            distance = dis(centerPos_, *headPos_);
            look(distance);
            for (int i = 1; i <= speed_; i++) {
                if (path_.size() > snakeLen_) {
                    path_.pop_back();
                }
            }
        } else {
            move(path_.front());
            headPos_ = &path_.front();
            distance = dis(centerPos_, *headPos_);
            if (distance > windowRadius_) {
                if (!outOfBounds_) {
                    outOfBounds_ = true;
                    static float angleA, angleB, hypotenuse, posAngle;
                    posAngle = culAngleABS(*headPos_ - centerPos_);
                    angleA = abs(90.f - angleABS_);
                    angleB = abs(angleA - abs(posAngle - 90.f));
                    hypotenuse = cos(angleB / radianToAngle_) * windowDiameter_;
                    angleA = angleA / radianToAngle_;

                    if (angle_ < 0) {
                        headPos_->x -= cos(angleA) * hypotenuse;
                    } else if (angle_ > 0) {
                        headPos_->x += cos(angleA) * hypotenuse;
                    }
                    if (angleABS_ > 90) {
                        headPos_->y += sin(angleA) * hypotenuse;
                    } else if (angleABS_ < 90) {
                        headPos_->y -= sin(angleA) * hypotenuse;
                    }
                }
            } else if (outOfBounds_) {
                outOfBounds_ = false;
            }
            // toWindow(headPos_, dir, abs(tan(radian_)));
        }
    } else {
        headPos_ = &path_.front();
        distance = dis(centerPos_, *headPos_);
        look(distance);
    }
}

float culSelfCollisionDis(float radius) { return 2.0f * radius; }

void Snake::look(float distance) {

    if (distance + visionDistance_ > windowRadius_) {
        static float angleA, posAngle;
        angleA = acos(-(pow(distance, 2) + pow(visionDistance_, 2) - pow(windowRadius_, 2)) / 2 / distance / visionDistance_) * 180 / PI;
        posAngle = culAngle(*headPos_ - centerPos_);
        // std::cout << culAngle(headPos_ - centerPos_) << std::endl;
        static float max[2], min[2];
        static bool maxOutOfBound, minOutOfBound;
        
        *max = posAngle + angleA + halfVisionAngle_;
        *min = posAngle - angleA - halfVisionAngle_;

        if (*max > 180) {
            max[1] = *max - 360;
            *max = 180;
            maxOutOfBound = true;
        } else {
            maxOutOfBound = false;
        }

        if (*min < -180) {
            min[1] = 360 - *min;
            *min = -180;
            minOutOfBound = true;
        } else {
            minOutOfBound = false;
        }

        if ((angle_ < *max && angle_ > *min) ||
            (maxOutOfBound && angle_ < max[1] && angle_ > -180) ||
            (minOutOfBound && angle_ < 180 && angle_ > min[1])) {

            if (angle_ == 90 || angle_ == 180 || angle_ == 180 || angle_ == -90) {
                headOutPos_.x = windowSize_ - headPos_->x;
                headOutPos_.y = windowSize_ - headPos_->y;
            }
            visionOutOfBounds_ = true;
        }
    }

    // static sf::Vector2f pos;
    // static float len, x, y;
    // len = sqrt(pow(dir.x, 2) + pow(dir.y, 2));
    // x = x / len * visionDistance_;
    // y = y / len * visionDistance_;
    // pos = headPos_;
    // pos.y += y;
    // pos.x += x;
    // if (visionOutOfBounds_ = toWindow(pos, dir, tanVal, head)) {
    //     pos.y -= y;
    //     pos.x -= x;
    //     headOutPos_ = pos;
    // }
}

#define CHANGE_DISTANCE(visionTrianglePtr, mtx, len, color) \
    do {                                                    \
        mtx.lock();                                         \
        if (visionTrianglePtr->distance > len) {            \
            visionTrianglePtr->distance = len;              \
            mtx.unlock();                                   \
            visionTrianglePtr->color = color;               \
        } else {                                            \
            mtx.unlock();                                   \
        }                                                   \
    } while (0)

#define DO_COMPARE_DISTANCE(angleDiff, len, visionRange, visionRangeEnd, mtx, visionTrianglePtr, color, code) \
    do {                                                                                                      \
        while (visionRange < visionRangeEnd) {                                                                \
            if (angleDiff <= visionRange->maxAngle || angleDiff >= visionRange->minAngle) {                   \
                CHANGE_DISTANCE(visionTrianglePtr, mtx, len, color);                                          \
                break;                                                                                        \
            }                                                                                                 \
            visionRange++;                                                                                    \
            code;                                                                                             \
        }                                                                                                     \
    } while (0)

#define COMPARE_DISTANCE(onRight, angleDiff, len, visionRange, visionRangeEnd, threads, visionTriangleStart, visionTriangleLast, color)        \
    do {                                                                                                                                       \
        if (onRight) {                                                                                                                         \
            DO_COMPARE_DISTANCE(angleDiff, len, visionRange, visionRangeEnd, threads->mtx, visionTriangleLast, color, visionTriangleLast--);   \
        } else {                                                                                                                               \
            DO_COMPARE_DISTANCE(angleDiff, len, visionRange, visionRangeEnd, threads->mtx, visionTriangleStart, color, visionTriangleStart++); \
        }                                                                                                                                      \
    } while (0)

void checkVision(sf::Vector2f pos, float angle, float radius, el_diff_range *visionRange, sf::Vector2f *center,
                 bool outOfBounds, float visionDistance, el_diff_range *visionRangeEnd, sf::Vector2f *outCenter,
                 utils::Threads *threads, el_triangle *visionTriangleStart, el_triangle *visionTriangleLast, sf::Uint32 color) {

    float len = dis(pos, *center);

    float angleDiff = culAngle(pos) - angle;

    bool onRight = angleDiff > 0;

    if (!onRight) {
        angleDiff = -angleDiff;
    }

    if (outOfBounds) {
        if (len < visionDistance) {
            COMPARE_DISTANCE(onRight, angleDiff, len, visionRange, visionRangeEnd, threads, visionTriangleStart, visionTriangleLast, color);
        } else {
            len = dis(pos, *outCenter);
            if (len < visionDistance) {
                COMPARE_DISTANCE(onRight, angleDiff, len, visionRange, visionRangeEnd, threads, visionTriangleStart, visionTriangleLast, color);
            }
        }
    } else if (len < visionDistance) {
        COMPARE_DISTANCE(onRight, angleDiff, len, visionRange, visionRangeEnd, threads, visionTriangleStart, visionTriangleLast, color);
    }
}

void Snake::checkFruitCollisions(std::deque<Fruit> &fruits) {
    auto toRemove = fruits.end();
    SnakePathNode headnode = path_.front();

    static float fruitRadius = fruits.begin()->shape_.getRadius();

    for (auto i = fruits.begin(); i != fruits.end(); ++i) {

        const sf::Vector2f &pos = i->shape_.getPosition();

        utils::addThread(threads_, checkVision, pos, angle_, fruitRadius, visionRange_, headPos_, visionOutOfBounds_, visionDistance_,
                         visionRangeEnd_, &headOutPos_, &threads_, visionTriangle_, visionTriangleLast_, VISION_CHECK_COLOR);

        if (dis(pos, headnode) <
            nodeRadius_ + i->shape_.getRadius()) {
            toRemove = i;
        }
    }

    if (toRemove != fruits.end()) {
        // pickupSound_.play();
        grow(toRemove->score_);
        fruits.erase(toRemove);
        delight_ = Game::cfg.eatDelight;
        leftVitality_ = rightVitality_ = injureLeft_ = injureRight_ = headAngle_ = 0;
        if (speedVitality_ > speedVitalityMax_) {
            speedVitality_ = speedVitalityMax_;
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
        if (!hitSelf_)
            bodyDir_ = parseAngle(angle_);
        for (int i = 1; i <= speed_; i++) {
            if (hitSelf_) {
                if (path_.size() > snakeLen_) {
                    path_.pop_back();
                }
            } else {
                path_.push_front(
                    SnakePathNode(headNode.x + direction_.x * i * nodeRadius2_,
                                  headNode.y + direction_.y * i * nodeRadius2_));
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

void Snake::checkSelfCollisions() {

    if (*deathFlag_) {
        return;
    }

    if (hitSelf_) {
        hitSelf_ = false;
    }
    for (auto i = path_.begin() + 15; i < path_.end(); i += 10) {

        utils::addThread(threads_, checkVision, *i, angle_, nodeRadius_, visionRange_, headPos_, visionOutOfBounds_, visionDistance_,
                         visionRangeEnd_, &headOutPos_, &threads_, visionTriangle_, visionTriangleLast_, VISION_CHECK_COLOR);

        if (speed_) {
            if (dis2(*headPos_, *i) < culSelfCollisionDis(nodeRadius_)) {
                // dieSound_.stop();
                // dieSound_.play();
                hitSelf_ = true;
                if (death_) {
                    pain_ += Game::cfg.bitePain * speed_;
                } else {
                    static TYPE_VOL tmp;
                    tmp = Game::cfg.bitePain * speed_;
                    pain_ += tmp;
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

void culOutWindowPos(float &pos, float &pos2, float &hpos, float &hpos2,
                     float dir, float dir2, unsigned int total,
                     unsigned int total2, float tanVal) {
    float posPlus = pos - total;

    if (culTimes(hpos, dir, total) < culTimes(hpos2, dir2, total2) &&
        culTimes2(hpos, dir, total) < culTimes2(hpos2, dir2, total2)) {
        pos2 = pos2 - pos / tanVal + posPlus / tanVal;
        pos = posPlus;
    } else {
        pos = pos - pos2 * tanVal + posPlus;
        pos2 = posPlus / tanVal;
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
             int width, int height, float tanVal) {
    const bool transX = transCoord(dx, x, hx, width),
               transY = transCoord(dy, y, hy, height);

    culOutWindowPos(x, y, hx, hy, dx, dy, width, height, tanVal);

    if (transX) {
        x = width - x;
    }

    if (transY) {
        y = height - y;
    }
}

bool Snake::toWindow(sf::Vector2f &node, SnakePathNode dir,
                     float tanVal) {
    return toWindow(node, dir, tanVal, node);
}

bool Snake::toWindow(sf::Vector2f &node, SnakePathNode dir,
                     float tanVal, SnakePathNode head) {
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

            reverse(node.y, node.x, head.y, head.x, dir.y, dir.x,
                    Game::GlobalVideoMode.height, Game::GlobalVideoMode.width,
                    1.0f / tanVal);
        } else {
            reverse(node.x, node.y, head.x, head.y, dir.x, dir.y,
                    Game::GlobalVideoMode.width, Game::GlobalVideoMode.height, tanVal);
        }
    } else if (beyondY) {
        if (dir.x == 0) {
            node.y = negativeY ? node.y + Game::GlobalVideoMode.height
                               : node.y - Game::GlobalVideoMode.height;
        } else if (culTimes2(head.x, dir.x, Game::GlobalVideoMode.width) >
                   culTimes2(head.y, dir.y, Game::GlobalVideoMode.height)) {

            reverse(node.y, node.x, head.y, head.x, dir.y, dir.x,
                    Game::GlobalVideoMode.height, Game::GlobalVideoMode.width,
                    1.0f / tanVal);
        } else {

            reverse(node.x, node.y, head.x, head.y, dir.x, dir.y,
                    Game::GlobalVideoMode.width, Game::GlobalVideoMode.height, tanVal);
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
    leftVitality_ = rightVitality_ = speed_ = delight_ = pain_ = injureLeft_ = injureRight_ = headAngle_ = turnRight_ = turnLeft_ = 0;
    angle_ = angleHis_ = INITIAL_ANGLE;
    headAngle_ = headAngleHis_ = bodyDir_ = 0;
    radian_ = angle_ / radianToAngle_;
    direction_ = Direction(0, 1);
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

    pain_ = STD_MAX(STD_MIN(pain_, MAX_ENC), MIN_ENC);

    static int heaelthTick = Game::cfg.healthTick;
    if (death_) {
        static float maxHealth = STD_MIN(5.0 * healthVal_, MAX_VITALITY);
        health_ -= pain_ + heaelthTick - delight_;
        if (health_ > maxHealth) {
            health_ = maxHealth;
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
    static TYPE_VOL headAngle, angleTmp;

    outSection = outSectionArr[0]; // LTurn
    turnLeft_ *= maxVitality_;
    for (i = outSection.startIndex; i < outSection.endIndex; i++) {
        out_[i] = turnLeft_;
    }

    outSection = outSectionArr[1]; // LAngle
    headAngle = headAngle_ * maxVitality_;
    angleTmp = headAngle_ < 0 ? STD_MIN(-headAngle, MAX_VOL) : 0;
    for (i = outSection.startIndex; i < outSection.endIndex; i++) {
        out_[i] = angleTmp;
    }

    outSection = outSectionArr[2]; // LStuck
    injureLeft_ = STD_MIN(injureLeft_ * maxVitality_, MAX_VOL);
    for (i = outSection.startIndex; i < outSection.endIndex; i++) {
        out_[i] = injureLeft_;
    }

    outSection = outSectionArr[4]; // RStuck
    injureRight_ = STD_MIN(injureRight_ * maxVitality_, MAX_VOL);
    for (i = outSection.startIndex; i < outSection.endIndex; i++) {
        out_[i] = injureRight_;
    }

    outSection = outSectionArr[5]; // RAngle
    angleTmp = headAngle_ > 0 ? STD_MIN(headAngle, MAX_VOL) : 0;
    for (i = outSection.startIndex; i < outSection.endIndex; i++) {
        out_[i] = angleTmp;
    }

    outSection = outSectionArr[6]; // RTurn
    turnRight_ *= maxVitality_;
    for (i = outSection.startIndex; i < outSection.endIndex; i++) {
        out_[i] = turnRight_;
    }

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
    while (visionStart < visionTriangleEnd_) {
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

    if (delight_ > 0) {
        delight_ = STD_MAX(delight_ - Game::cfg.delightConsum, 0.0);
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