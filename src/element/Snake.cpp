

#include <math.h>

#include <SFML/Graphics.hpp>
#include <iostream>
#include <memory>

#include "element/Snake.h"
#include "Game.h"
#include "screen/GameOverScreen.h"
#include "utils/Time.hpp"

#ifdef __linux
#include <string.h>
#endif

#define ANGLE_PLUS_THRESHOLD 180
#define ANGLE_PLUS_THRESHOLD2 60
#define ANGLE_PLUS_THRESHOLD3 120
#define max std::max
#define min std::min
#define CUL_VISION_INDEX(x, y) x * VISION_Y_SUM + y

#define INPUT_CNT_LEFT 300
#define INPUT_CNT_RUN 300
#define INPUT_CNT_RIGHT 300

using namespace sfSnake;

static float VISION_X_HALF,
                VISION_HALF_WIDTH,
                VISION_HALF_WIDTH2;

static unsigned int speed_level1, speed_level2, vision_blank_vol, vision_fruit_vol, vision_body_vol;
// static unsigned int vision_blank_vol, vision_fruit_vol, vision_body_vol;

float culAngle(sf::Vector2f recDirection) {
  float angle = std::acos(recDirection.y / length(recDirection)) / PI * 180.0;
  if (recDirection.x > 0) angle = -angle;

  return angle;
}

Snake::Snake()
    : vision_((vision *) malloc(sizeof(vision) * Game::cfg.visionXSum * Game::cfg.visionYSum)),
      hitSelf_(false),
      pain_(0),
      delight_(0),
      turnLeft(0),
      turnRight(0),
      stuckLeft(0),
      stuckRight(0),
      turnDirection_(0),
      leftVitality(0),
      rightVitality(0),
      eating(0),
      //   speedup_(false),
      speed_(0),
      direction_(Direction(0, -1)),
      angle_(180),
      hisAngle_(angle_),
      bodyDir_(angle_),
      headAngle_(0),
      radian(angle_ * PI / 180.0f),
      nodeRadius_(Game::GlobalVideoMode.width / 100.0f),
      tailOverlap_(0u),
      nodeShape(nodeRadius_),
      nodeMiddle(sf::Vector2f(nodeRadius_ * std::sqrt(3), nodeRadius_)),
      score_(Game::cfg.initialSize) {
  snakeLen = 10 * Game::cfg.initialSize;
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
  threads.init(std::thread::hardware_concurrency() - 5);

  in = Game::share.getReadPos();
  out = Game::share.getWritePos();
}

Snake::~Snake() {
  threads.join();
  free(vision_);
}

void Snake::initNodes() {
  path_.push_back(SnakePathNode(Game::GlobalVideoMode.width / 2.0f,
                                Game::GlobalVideoMode.height / 2.0f));
  for (int i = 1; i <= snakeLen; i++) {
    path_.push_back(SnakePathNode(Game::GlobalVideoMode.width / 2.0f -
                                      direction_.x * i * nodeRadius_ / 5.0,
                                  Game::GlobalVideoMode.height / 2.0f -
                                      direction_.y * i * nodeRadius_ / 5.0));
  }
}

void Snake::handleInput(sf::RenderWindow &window) {
  static sf::Vector2i mousePosition;

  if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) ||
      sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
    direction_ = Direction(0, -1);

    angle_ = culAngle(direction_);
    radian = angle_ * PI / 180.0f;
  } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) ||
             sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
    direction_ = Direction(0, 1);

    angle_ = culAngle(direction_);
    radian = angle_ * PI / 180.0f;
  } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) ||
             sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
    direction_ = Direction(-1, 0);

    angle_ = culAngle(direction_);
    radian = angle_ * PI / 180.0f;
  } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) ||
             sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
    direction_ = Direction(1, 0);

    angle_ = culAngle(direction_);
    radian = angle_ * PI / 180.0f;
  }

  if (!Game::mouseButtonLocked) {
    if (sf::Mouse::isButtonPressed(sf::Mouse::Left) ||
        sf::Mouse::isButtonPressed(sf::Mouse::Right)) {
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
  static TYPE_VOL *leftPtr, 
                          *runPtr = in + INPUT_CNT_LEFT,
                          *rightPtr = runPtr + INPUT_CNT_RUN,
                          *endPtr = rightPtr + INPUT_CNT_RIGHT,
                          distance;
  leftPtr = in;

  static unsigned long long plusTmp;

  static float plus;
  plus = 0;
  distance = 0;

  plusTmp = 0;
  do {
    plusTmp += *leftPtr;
    leftPtr++;
  } while (leftPtr != runPtr);

  plus = (float) plusTmp / INPUT_CNT_LEFT;

  plusTmp = 0;
  do {
    plusTmp += *leftPtr;
    leftPtr++;
  } while (leftPtr != rightPtr);

  distance = (float) plusTmp / INPUT_CNT_RUN;

  if (distance > speed_level1) {
    speed_ = distance > speed_level2 ? 2 : 1;
  }

  plusTmp = 0;
  do {
    plusTmp += *leftPtr;
    leftPtr++;
  } while (leftPtr != endPtr);

  plus -= (float) plusTmp / INPUT_CNT_RIGHT;

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
        stuckRight = (headAngle_ - ANGLE_PLUS_THRESHOLD2) * 10;
        if (stuckRight > MAX_VITALITY) {
          stuckRight = MAX_VITALITY;
        }
        pain_ += stuckRight;
      } else {
        stuckRight = 0;
      }
      stuckLeft = 0;
    } else if (headAngle_ < 0) {
      if (headAngle_ < -ANGLE_PLUS_THRESHOLD2) {
        angle_ = parseAngle2(bodyDir_ - ANGLE_PLUS_THRESHOLD2);
        stuckLeft = -(ANGLE_PLUS_THRESHOLD2 + headAngle_) * 10;
        if (stuckLeft > MAX_VITALITY) {
          stuckLeft = MAX_VITALITY;
        }
        pain_ += stuckLeft;
      } else {
        stuckLeft = 0;
      }
      stuckRight = 0;
    } else {
      stuckRight = 0;
      stuckLeft = 0;
    }

    // cout << "\t" << angle_ << endl;

    static unsigned long long now;
    now = utils::timestamp();
    plus = ((hisAngle_ > 0 && headAngle_ > 0) || (hisAngle_ < 0 && headAngle_ < 0)
                ? hisAngle_ - headAngle_
                : hisAngle_ + headAngle_) *
           100;
    if (plus) {
      if (plus > 0) {
        if (plus < 1) {
          plus = 1;
        } else if (plus > MAX_VITALITY) {
          plus = MAX_VITALITY;
        }
        turnRight = plus;
        turnLeft = 0;
      } else {
        if (plus > -1) {
          plus = -1;
        } else if (plus < -MAX_VITALITY) {
          plus = -MAX_VITALITY;
        }
        turnLeft = -plus;
        turnRight = 0;
      }
    } else {
      turnRight = 0;
      turnLeft = 0;
    }

    radian = angle_ * PI / 180.0f;

    direction_.y += cos(radian) * headTexture.getSize().y;
    direction_.x -= sin(radian) * headTexture.getSize().y;

    static double directionSize;
    directionSize = length(direction_);
    direction_.x /= directionSize;
    direction_.y /= directionSize;

    hisAngle_ = headAngle_;

    // printf("angle_: %f\n", angle_);
  } else {
    stuckRight = 0;
    stuckLeft = 0;
    turnRight = 0;
    turnLeft = 0;

    angle = parseAngle(angle_);
    headAngle_ = angle - bodyDir_;
    if (headAngle_ > ANGLE_PLUS_THRESHOLD) {
      headAngle_ = headAngle_ - 360;
    } else if (headAngle_ < -ANGLE_PLUS_THRESHOLD) {
      headAngle_ = 360 + headAngle_;
    }
  }

  static float headAngle;
  if (headAngle_ > 0) {
    headAngle = abs(headAngle_);
    leftVitality = min(leftVitality + max((Game::cfg.maxVitality - leftVitality) / Game::cfg.vitalityStepCnt * headAngle, Game::cfg.vitalityStep), Game::cfg.maxVitality);
    rightVitality = max(rightVitality - max(Game::cfg.vitalityStep, (-rightVitality - Game::cfg.minVitality) / Game::cfg.vitalityStepCnt * headAngle), Game::cfg.minVitality);
  } else if (headAngle_ < 0) {
    headAngle = abs(headAngle_);
    leftVitality = max(leftVitality - max(Game::cfg.vitalityStep, (-leftVitality - Game::cfg.minVitality) / Game::cfg.vitalityStepCnt * headAngle), Game::cfg.minVitality);
    rightVitality = min(rightVitality + max((Game::cfg.maxVitality - rightVitality) / Game::cfg.vitalityStepCnt * headAngle, Game::cfg.vitalityStep), Game::cfg.maxVitality);
  } else {
    leftVitality = min(leftVitality + max((Game::cfg.maxVitality - leftVitality) / Game::cfg.vitalityStepCnt, Game::cfg.vitalityStep), Game::cfg.maxVitality);
    rightVitality = min(rightVitality + max((Game::cfg.maxVitality - rightVitality) / Game::cfg.vitalityStepCnt, Game::cfg.vitalityStep), Game::cfg.maxVitality);
  }

  if (leftVitality) {
    pain_ += abs(leftVitality) * Game::cfg.vitalityPain / Game::cfg.maxVitality;
  }
  if (rightVitality) {
    pain_ += abs(rightVitality) * Game::cfg.vitalityPain / Game::cfg.maxVitality;
  }

  look();
  checkSelfCollisions();
  move();
  toWindow(path_.front(), direction_, abs(tan(radian)));
}

float culSelfCollisionDis(float radius) { return 2.0f * radius; }

void Snake::look() {
  SnakePathNode head = path_.front();

  float cosR = cos(radian), sinR = sin(radian), tanVal = abs(tan(radian)),
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
    delight_ = min(delight_ + Game::cfg.eatDelight, MAX_VITALITY);
    eating = utils::timestamp();
    leftVitality = 0;
    rightVitality = 0;
  } else {
    unsigned long long diff = (utils::timestamp() - eating) / Game::cfg.eatDelightDuration;
    if (diff < Game::cfg.eatDelight) {
      delight_ += Game::cfg.eatDelight - diff;
    }
  }
}

void Snake::grow(int score) {
  tailOverlap_ += score * 10;
  score_ += score;
}

unsigned Snake::getScore() const { return score_; }

bool Snake::hitSelf() const { return hitSelf_; }

void Snake::move() {
  SnakePathNode &headNode = path_.front();

  if (speed_ > 0) {
    if (!hitSelf_) bodyDir_ = parseAngle(angle_);
    for (int i = 1; i <= speed_; i++) {
      if (hitSelf_) {
        if (path_.size() > snakeLen) {
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
    speed_ = 0;
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

void checkVisionY(short speed_, bool *hitSelf_, TYPE_VOL *pain_, SnakePathNode *head,
                  vision *vision_, SnakePathNode *i,
                  float nodeRadius_) {
  for (int x = 0, y = 0; x < VISION_X_SUM; x++) {
    for (y = 0; y < VISION_Y_SUM; y++) {
      size_t index = CUL_VISION_INDEX(x, y);
      if (dis2(vision_[index].pos, *i) < culSelfCollisionDis(nodeRadius_)) {
        vision_[index].color = VISION_HARM_COLOR;
      }
    }
  }

  if (speed_ && dis2(*head, *i) < culSelfCollisionDis(nodeRadius_)) {
    // dieSound_.stop();
    // dieSound_.play();
    *hitSelf_ = true;
    *pain_ += Game::cfg.bitePain * speed_;
  }
}

void Snake::checkSelfCollisions() {
  Direction dir = direction_;
  dir.x *= speed_ * 10;
  dir.y *= speed_ * 10;
  SnakePathNode head = path_.front() + dir;

  if (hitSelf_) {
    hitSelf_ = false;
  }
  for (auto i = path_.begin() + 15; i < path_.end(); i += 10) {
    utils::addThread(threads, checkVisionY, speed_, &hitSelf_, &pain_, &head,
                     (vision *)vision_, &(*i), nodeRadius_);
  }
  threads.join();
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

SnakePathNode Snake::toWindow(sf::Vector2f &node, SnakePathNode dir,
                              float tanVal) {
  return toWindow(node, dir, tanVal, 0, 0, -1, node);
}

SnakePathNode Snake::toWindow(sf::Vector2f &node, SnakePathNode dir,
                              float tanVal, float sin, float cos, int num,
                              SnakePathNode head) {
  bool negativeX = node.x < 0, negativeY = node.y < 0,
       beyondX = negativeX || node.x > Game::GlobalVideoMode.width,
       beyondY = negativeY || node.y > Game::GlobalVideoMode.height;
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

  static sf::Vector2u TextureSize = headTexture.getSize();
  static long xSize = TextureSize.x * 10, ySize = TextureSize.y * 10;
  if ((node.x < -xSize || node.x > Game::GlobalVideoMode.width + xSize) &&
      (node.y < -ySize || node.y > Game::GlobalVideoMode.height + ySize)) {
    reset();
  }

  return node;
}

void Snake::reset() {
  leftVitality = 0;
  rightVitality = 0;
  stuckLeft = 0;
  stuckRight = 0;
  headAngle_ = 0;
  angle_ = 180;
  hisAngle_ = angle_;
  bodyDir_ = angle_;
  direction_ = Direction(0, -1);
  path_.clear();
  initNodes();
  radian = angle_ *PI / 180.0f;
}

void Snake::render(sf::RenderWindow &window) {
  static TYPE_VOL *judgementPtr = in - 1;
  if (*judgementPtr) {
    reset();
    *judgementPtr = 0;
    return;
  }

  static int count, j, x, y;
  j = 7;
  x = 0;
  y = 0;

  static TYPE_VOL *out_tmp;
  out_tmp = out;

  static size_t fillSize = sizeof(TYPE_VOL) * Game::cfg.fillCount;

  // 将数据长度、存活状态、分数、窗口尺寸输出到共享内存中
  memset(out_tmp, delight_, fillSize);
  pain_ = max(min(pain_, MAX_VITALITY), MIN_VITALITY);
  static TYPE_VOL headAngle;
  headAngle = headAngle_;
  out_tmp += Game::cfg.fillCount;
  memset(out_tmp, pain_, fillSize);
  out_tmp += Game::cfg.fillCount;
  memset(out_tmp, headAngle_ < 0 ? -headAngle : 0, fillSize);
  out_tmp += Game::cfg.fillCount;
  memset(out_tmp, turnLeft, fillSize);
  out_tmp += Game::cfg.fillCount;
  memset(out_tmp, stuckLeft, fillSize);
  out_tmp += Game::cfg.fillCount;
  memset(out_tmp, (rightVitality + Game::cfg.vitalityPain), fillSize);

  static SnakePathNode lastSnakeNode, lastMiddleNode, nowSnakeNode;
  static float angle;
  angle = angle_;
  static sf::Vector2f body;
  static SnakePathNode wNowHeadNode;

  lastSnakeNode = *path_.begin();
  wNowHeadNode = lastSnakeNode;
  headSprite.setPosition(wNowHeadNode);
  headSprite.setRotation(angle);

  static sf::RectangleShape shape;
  shape = sf::RectangleShape();
  shape.setSize(sf::Vector2f(VISION_PIXEL_WIDTH, VISION_PIXEL_WIDTH));
  shape.setRotation(angle);
  static vision v;
  for (; x < VISION_X_SUM; x++, y = 0) {
    for (; y < VISION_Y_SUM; y++, out_tmp++) {
      v = vision_[CUL_VISION_INDEX(x, y)];
      shape.setFillColor(sf::Color(v.color));
      shape.setPosition(v.pos);
      window.draw(shape);

      TYPE_VOL *val;
      switch (v.color) {
        case VISION_HARM_COLOR:
          val = out_tmp + Game::cfg.visionBodyPos;
          *val = vision_body_vol;
          val = out_tmp + Game::cfg.visionFruitPos;
          *val = 0;
          val = out_tmp;
          *val = 0;
          break;
        case VISION_CHECK_COLOR:
          val = out_tmp + Game::cfg.visionFruitPos;
          *val = vision_fruit_vol;
          val = out_tmp + Game::cfg.visionBodyPos;
          *val = 0;
          val = out_tmp;
          *val = 0;
          break;
        default:
          val = out_tmp;
          *val = vision_blank_vol;
          val = out_tmp + Game::cfg.visionFruitPos;
          *val = 0;
          val = out_tmp + Game::cfg.visionBodyPos;
          *val = 0;
      }
    }
  }

  out_tmp += Game::cfg.visionBodyPos;

  memset(out_tmp, (leftVitality + Game::cfg.vitalityPain), fillSize);
  out_tmp += Game::cfg.fillCount;
  memset(out_tmp, stuckRight, fillSize);
  out_tmp += Game::cfg.fillCount;
  memset(out_tmp, turnRight, fillSize);
  out_tmp += Game::cfg.fillCount;
  memset(out_tmp, headAngle_ > 0 ? headAngle : 0, fillSize);
  out_tmp += Game::cfg.fillCount;
  memset(out_tmp, pain_, fillSize);
  out_tmp += Game::cfg.fillCount;
  memset(out_tmp, delight_, fillSize);
  pain_ = 0;
  delight_ = 0;

  renderNode(wNowHeadNode, headSprite, window, 3);

  count = 5;
  for (auto i = path_.begin() + 5, end = path_.end(); i < end;
       i += 5, count += 5, j += 2) {
    body = *i;

    if (count % 2)
      lastMiddleNode = body;
    else {
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
void Snake::renderNode(sf::Vector2f &nowPosition, T &shape,
                       sf::RenderWindow &window, int offset) {
  shape.setPosition(nowPosition);
  window.draw(shape);
}