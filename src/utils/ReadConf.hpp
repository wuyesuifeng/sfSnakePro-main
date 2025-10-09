#pragma once

#include <vector>

#include "def.h"

namespace utils {
    struct io_section {
        unsigned int startIndex, endIndex;
    };
    struct config_data {
        TYPE_VOL maxVitality, vitalityStepCnt,
            eatDelight, bitePain, vitalityPain,
            visionBlankVol[3], visionBodyVol[3], visionFruitVol[3],
            brakeThreshold, delightConsum, painConsum, speedVitalityDiff;
        unsigned int waitTime, inputCnt, outputCnt, windowSize;
        unsigned int inputSectionArrLen, outputSectionArrLen, visionIndexes[3];
        int initialSize, maxFruit, minFruit, healthTick;
        float heath, visionAngle, visionDistance;
        char *gamePath;
        unsigned char threadsCnt;
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