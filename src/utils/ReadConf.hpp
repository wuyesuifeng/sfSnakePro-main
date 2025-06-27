#pragma once

#include <vector>

#include "def.h"

namespace utils {
    struct config_data {
        TYPE_VOL maxVitality, vitalityStepCnt,
            eatDelight, bitePain, vitalityPain, visionBlankVol, visionBodyVol,
            visionFruitVol, speedLevel1, speedLevel2, delightConsum, speedVitalityDiff;
        unsigned int visionFruitPos, visionBodyPos, waitTime;
        int initialSize, visionXSum, visionYSum, maxFruit, minFruit, healthTick;
        char *gamePath;
        long heath;
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