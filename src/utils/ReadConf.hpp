#pragma once

#include <vector>

#include "def.h"

namespace utils {
    struct config_data {
        TYPE_VOL maxVitality, minVitality, vitalityStep, vitalityStepCnt,
            eatDelight, bitePain, vitalityPain, visionBlankVol, visionBodyVol,
            visionFruitVol, speedLevel1, speedLevel2, delightConsum;
        unsigned int visionFruitPos, visionBodyPos;
        char *gamePath;
        long heath;
        short initialSize, visionXSum, visionYSum, maxFruit, minFruit, healthTick;
        bool death;
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