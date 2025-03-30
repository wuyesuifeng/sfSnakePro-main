#pragma once

#include <vector>

#include "def.h"

namespace utils {
    struct config_data {
        TYPE_VOL maxVitality, minVitality, vitalityStep, vitalityStepCnt,
            eatDelight, bitePain, vitalityPain, visionBlankVol, visionBodyVol,
            visionFruitVol, speedLevel1, speedLevel2;
        unsigned int visionFruitPos, visionBodyPos;
        char *gamePath;
        short initialSize, visionXSum, visionYSum, fillCount;
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