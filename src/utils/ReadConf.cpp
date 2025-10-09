#include "ReadConf.hpp"

#include <fstream>
#include <iostream>
#include <string>

#include "string.hpp"

#ifdef __linux
#include <cstring>
#endif

#define DEBUG_CONF_PATH "/home/xy/workspace/sfSnakePro-main/build/conf"
#define CONF_PATH "conf.cfg"

using namespace std;

unsigned int setSections(utils::config_data &cfg, string &input_section, string &input_section_dict, utils::io_section **sectionArrPtr, unsigned int &sectionArrLen) {
    unsigned int count = 0;
    vector<string> ret, ret2, ret3;
    utils::split(input_section, ",", &ret);
    int len = ret.size();
    utils::split(input_section_dict, ",", &ret3);
    if (len != ret3.size()) {
        throw "Count of input_section failed to compare count of input_section_dict";
    }
    sectionArrLen = len;
    utils::io_section *sectionArr = (utils::io_section *)malloc(sizeof(utils::io_section) * len);
    *sectionArrPtr = sectionArr;
    string name;
    for (unsigned int i = 0, j, startIndex, cnt; i < len; i++) {
        name = ret3[i];

        if (name == "Vision1") {
            cfg.visionIndexes[0] = i;
        } else if (name == "Vision2") {
            cfg.visionIndexes[1] = i;
        } else if (name == "Vision3") {
            cfg.visionIndexes[2] = i;
        }

        for (j = 0, startIndex = 0; j < len; j++) {
            utils::io_section &section = sectionArr[j];
            input_section = ret[j];
            ret2.clear();
            utils::trim(input_section);
            utils::split(input_section, " ", &ret2);
            if (ret2.size()) {
                if (name == ret2[0]) {
                    section.endIndex = count += stoi(ret2[1]);
                    section.startIndex = startIndex;
                    break;
                } else {
                    cnt = stoi(ret2[1]);
                }
                startIndex += cnt;
            }
        }
    }
    return count;
}

utils::ReadConf::ReadConf() {

    ifstream ifs;
    ifs.open(CONF_PATH, ios::in);

    if (!ifs.is_open()) {
        ifs.open(DEBUG_CONF_PATH, ios::in);
    }

    if (ifs.is_open()) {
        string buff;
        string input_section, input_section_dict, output_section, output_section_dict;
        while (getline(ifs, buff)) {
            vector<string> ret;
            utils::split(buff, "=", &ret);
            utils::trim(ret[0]);
            if (ret[0] == "gamePath") {
                utils::trim(ret[1]);
                const int len = ret[1].length();
                char *resChar = (char *)calloc(len, len * sizeof(char));
                memcpy(resChar, ret[1].c_str(), len);
                cfg.gamePath = resChar;
            } else if (ret[0] == "eatDelight") {
                utils::trim(ret[1]);
                cfg.eatDelight = STO_FUNC_VOL(ret[1]);
            } else if (ret[0] == "bitePain") {
                utils::trim(ret[1]);
                cfg.bitePain = STO_FUNC_VOL(ret[1]);
            } else if (ret[0] == "maxVitality") {
                utils::trim(ret[1]);
                cfg.maxVitality = stof(ret[1]);
            } else if (ret[0] == "vitalityPain") {
                utils::trim(ret[1]);
                cfg.vitalityPain = STO_FUNC_VOL(ret[1]);
            } else if (ret[0] == "vitalityStepCnt") {
                utils::trim(ret[1]);
                cfg.vitalityStepCnt = stoi(ret[1]);
            } else if (ret[0] == "initialSize") {
                utils::trim(ret[1]);
                cfg.initialSize = stoi(ret[1]);
            } else if (ret[0] == "visionBlankVol") {
                utils::trim(ret[1]);
                buff = ret[1];
                ret.clear();
                utils::split(buff, ",", &ret);
                cfg.visionBlankVol[0] = STO_FUNC_VOL(ret[0]);
                cfg.visionBlankVol[1] = STO_FUNC_VOL(ret[1]);
                cfg.visionBlankVol[2] = STO_FUNC_VOL(ret[2]);
            } else if (ret[0] == "visionBodyVol") {
                utils::trim(ret[1]);
                buff = ret[1];
                ret.clear();
                utils::split(buff, ",", &ret);
                cfg.visionBodyVol[0] = STO_FUNC_VOL(ret[0]);
                cfg.visionBodyVol[1] = STO_FUNC_VOL(ret[1]);
                cfg.visionBodyVol[2] = STO_FUNC_VOL(ret[2]);
            } else if (ret[0] == "visionFruitVol") {
                utils::trim(ret[1]);
                buff = ret[1];
                ret.clear();
                utils::split(buff, ",", &ret);
                cfg.visionFruitVol[0] = STO_FUNC_VOL(ret[0]);
                cfg.visionFruitVol[1] = STO_FUNC_VOL(ret[1]);
                cfg.visionFruitVol[2] = STO_FUNC_VOL(ret[2]);
            } else if (ret[0] == "brakeThreshold") {
                utils::trim(ret[1]);
                cfg.brakeThreshold = STO_FUNC_VOL(ret[1]);
            } else if (ret[0] == "delightConsum") {
                utils::trim(ret[1]);
                cfg.delightConsum = STO_FUNC_VOL(ret[1]);
            } else if (ret[0] == "painConsum") {
                utils::trim(ret[1]);
                cfg.painConsum = STO_FUNC_VOL(ret[1]);
            } else if (ret[0] == "maxFruit") {
                utils::trim(ret[1]);
                cfg.maxFruit = stoi(ret[1]);
            } else if (ret[0] == "minFruit") {
                utils::trim(ret[1]);
                cfg.minFruit = stoi(ret[1]);
            } else if (ret[0] == "heath") {
                utils::trim(ret[1]);
                cfg.heath = stof(ret[1]);
            } else if (ret[0] == "death") {
                utils::trim(ret[1]);
                cfg.death = stoi(ret[1]);
            } else if (ret[0] == "healthTick") {
                utils::trim(ret[1]);
                cfg.healthTick = stoi(ret[1]);
            } else if (ret[0] == "waitTime") {
                utils::trim(ret[1]);
                cfg.waitTime = stoi(ret[1]);
            } else if (ret[0] == "speedVitalityDiff") {
                utils::trim(ret[1]);
                cfg.speedVitalityDiff = stof(ret[1]);
            } else if (ret[0] == "input_section") {
                utils::trim(ret[1]);
                input_section = ret[1];
            } else if (ret[0] == "input_section_dict") {
                utils::trim(ret[1]);
                input_section_dict = ret[1];
            } else if (ret[0] == "output_section") {
                utils::trim(ret[1]);
                output_section = ret[1];
            } else if (ret[0] == "output_section_dict") {
                utils::trim(ret[1]);
                output_section_dict = ret[1];
            } else if (ret[0] == "visionAngle") {
                utils::trim(ret[1]);
                cfg.visionAngle = stof(ret[1]);
            } else if (ret[0] == "visionDistance") {
                utils::trim(ret[1]);
                cfg.visionDistance = stof(ret[1]);
            } else if (ret[0] == "threadsCnt") {
                utils::trim(ret[1]);
                cfg.threadsCnt = stoi(ret[1]);
            } else if (ret[0] == "windowSize") {
                utils::trim(ret[1]);
                cfg.windowSize = stoi(ret[1]);
            }
        }
        ifs.close();

        if (!input_section.empty() && !input_section_dict.empty()) {
            cfg.outputCnt = setSections(cfg, input_section, input_section_dict, &cfg.outputSectionArr, cfg.outputSectionArrLen);
        } else {
            cfg.outputSectionArr = nullptr;
        }

        if (!output_section.empty() && !output_section_dict.empty()) {
            cfg.inputCnt = setSections(cfg, output_section, output_section_dict, &cfg.inputSectionArr, cfg.inputSectionArrLen);
        } else {
            cfg.inputSectionArr = nullptr;
        }

        // cout << "debug here" << endl;
        // memcpy(resChar, res.c_str(), res.length());
    } else {
        throw "open conf failed";
    }
}

utils::ReadConf::~ReadConf() {
    if (cfg.inputSectionArr) {
        free(cfg.inputSectionArr);
    }
    if (cfg.outputSectionArr) {
        free(cfg.outputSectionArr);
    }
}

utils::config_data utils::ReadConf::getCfg() {
    return cfg;
}