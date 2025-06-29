#pragma once

#include <vector>

#include "def.h"

namespace utils {
    struct io_section {
        unsigned int index, cnt;
    };
    struct config_data {
        TYPE_VOL maxVitality, vitalityStepCnt,
            eatDelight, bitePain, vitalityPain, visionBlankVol, visionBodyVol,
            visionFruitVol, speedLevel1, speedLevel2, delightConsum, speedVitalityDiff;
        unsigned int visionFruitPos, visionBodyPos, waitTime;
        int initialSize, visionXSum, visionYSum, maxFruit, minFruit, healthTick;
        char *gamePath;
        io_section *sectionArr;
        unsigned int sectionArrLen;
        float heath;
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