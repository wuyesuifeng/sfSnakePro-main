#pragma once

#include <vector>

#include "def.h"

namespace utils {
    struct config_data {
      TYPE_VOL maxVitality, minVitality, vitalityStep, vitalityStepCnt,
          eatDelight, bitePain, vitalityPain, visionBlankVol, visionBodyVol,
          visionFruitVol;
      unsigned int visionFruitPos, visionBodyPos;
      short initialSize, visionXSum, visionYSum, speedLevel1, speedLevel2,
          eatDelightDuration, fillCount;
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