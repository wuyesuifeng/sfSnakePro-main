#pragma once

#include <vector>

#include "def.h"

namespace utils {
    struct io_section {
        unsigned int startIndex, endIndex;
    };
    struct config_data {
        TYPE_VOL maxVitality, vitalityStepCnt,
            eatDelight, bitePain, vitalityPain, visionBlankVol, visionBodyVol,
            visionFruitVol, speedLevel1, speedLevel2, delightConsum, speedVitalityDiff;
        unsigned int waitTime, inputCnt, outputCnt;
        unsigned int inputSectionArrLen, outputSectionArrLen;
        int initialSize, visionXSum, visionYSum, maxFruit, minFruit, healthTick;
        float heath;
        char *gamePath;
        io_section *inputSectionArr, *outputSectionArr;
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