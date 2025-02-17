#pragma once

#include <vector>

namespace utils {
    struct config_data {
      float maxVitality, minVitality, vitalityStep, vitalityStepCnt;
      int visionFruitPos, visionBodyPos, visionBlankVol, visionBodyVol, visionFruitVol;
      unsigned int outStartBit, angleStartBit, runStartBit;
      char *gamePath;
      short eatDelight, bitePain, vitalityPain, initialSize, visionXSum,
          visionYSum, speedLevel1, speedLevel2, eatDelightDuration;
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