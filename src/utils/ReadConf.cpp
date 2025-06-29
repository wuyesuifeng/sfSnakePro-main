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

void setSections(utils::config_data &cfg, string &input_section, string &input_section_dict) {
    vector<string> ret, ret2, ret3;
    utils::split(input_section, ",", &ret);
    int len = ret.size();
    utils::split(input_section_dict, ",", &ret3);
    if (len != ret3.size()) {
        throw "Count of input_section failed to compare count of input_section_dict";
    }
    cfg.sectionArrLen = len;
    utils::io_section *sectionArr = (utils::io_section *)malloc(sizeof(utils::io_section) * len);
    cfg.sectionArr = sectionArr;
    string name;
    for (unsigned int i = 0, j; i < len; i++) {
        name = ret3[i];
        for (j = 0; j < len; j++) {
            utils::io_section &section = sectionArr[j];
            input_section = ret[j];
            ret2.clear();
            utils::trim(input_section);
            utils::split(input_section, " ", &ret2);

            if (ret2.size() && name == ret2[0]) {
                section.cnt = stoi(ret2[2]);
                section.index = j;
            }
        }
    }
}

utils::ReadConf::ReadConf() {

    ifstream ifs;
    ifs.open(CONF_PATH, ios::in);

    if (!ifs.is_open()) {
        ifs.open(DEBUG_CONF_PATH, ios::in);
    }

    if (ifs.is_open()) {
        string buff;
        string input_section, input_section_dict;
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
            } else if (ret[0] == "visionFruitPos") {
                utils::trim(ret[1]);
                cfg.visionFruitPos = stoi(ret[1]);
            } else if (ret[0] == "visionBodyPos") {
                utils::trim(ret[1]);
                cfg.visionBodyPos = stoi(ret[1]);
            } else if (ret[0] == "visionXSum") {
                utils::trim(ret[1]);
                cfg.visionXSum = stoi(ret[1]);
            } else if (ret[0] == "visionYSum") {
                utils::trim(ret[1]);
                cfg.visionYSum = stoi(ret[1]);
            } else if (ret[0] == "visionBlankVol") {
                utils::trim(ret[1]);
                cfg.visionBlankVol = STO_FUNC_VOL(ret[1]);
            } else if (ret[0] == "visionBodyVol") {
                utils::trim(ret[1]);
                cfg.visionBodyVol = STO_FUNC_VOL(ret[1]);
            } else if (ret[0] == "visionFruitVol") {
                utils::trim(ret[1]);
                cfg.visionFruitVol = STO_FUNC_VOL(ret[1]);
            } else if (ret[0] == "speedLevel1") {
                utils::trim(ret[1]);
                cfg.speedLevel1 = STO_FUNC_VOL(ret[1]);
            } else if (ret[0] == "speedLevel2") {
                utils::trim(ret[1]);
                cfg.speedLevel2 = STO_FUNC_VOL(ret[1]);
            } else if (ret[0] == "delightConsum") {
                utils::trim(ret[1]);
                cfg.delightConsum = STO_FUNC_VOL(ret[1]);
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
            }
        }
        ifs.close();

        if (!input_section.empty() && !input_section_dict.empty()) {
            setSections(cfg, input_section, input_section_dict);
        } else {
            cfg.sectionArr = nullptr;
        }
        cout << "debug here" << endl;
        // memcpy(resChar, res.c_str(), res.length());
    } else {
        throw "open conf failed";
    }
}

utils::ReadConf::~ReadConf() {
    if (cfg.sectionArr) {
        free(cfg.sectionArr);
    }
}

utils::config_data utils::ReadConf::getCfg() {
    return cfg;
}