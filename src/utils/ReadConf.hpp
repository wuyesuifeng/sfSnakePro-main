#pragma once

#include <vector>

namespace utils {
    struct config_data {
      short eatDelight, bitePain, vitalityPain, initialSize, visionXSum, visionYSum;
      float maxVitality, minVitality, vitalityStep, vitalityStepCnt;
      int visionFruitPos, visionBodyPos;
      char *gamePath;
    };

    class ReadConf {
        private:
            config_data cfg;
        public:
            ReadConf();
            ~ReadConf();
            config_data getCfg();
    };
}