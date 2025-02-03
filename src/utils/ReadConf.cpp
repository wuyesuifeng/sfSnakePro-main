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

utils::ReadConf::ReadConf() {

  ifstream ifs;
  ifs.open(CONF_PATH, ios::in);

  if (!ifs.is_open()) {
    ifs.open(DEBUG_CONF_PATH, ios::in);
  }

  if (ifs.is_open()) {
    string buff;
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
        cfg.eatDelight = stoi(ret[1]);
      } else if (ret[0] == "bitePain") {
        utils::trim(ret[1]);
        cfg.bitePain = stoi(ret[1]);
      } else if (ret[0] == "maxVitality") {
        utils::trim(ret[1]);
        cfg.maxVitality = stof(ret[1]);
      } else if (ret[0] == "minVitality") {
        utils::trim(ret[1]);
        cfg.minVitality = stof(ret[1]);
      } else if (ret[0] == "vitalityStep") {
        utils::trim(ret[1]);
        cfg.vitalityStep = stof(ret[1]);
      } else if (ret[0] == "vitalityStepCnt") {
        utils::trim(ret[1]);
        cfg.vitalityStepCnt = stof(ret[1]);
      } else if (ret[0] == "vitalityPain") {
        utils::trim(ret[1]);
        cfg.vitalityPain = stoi(ret[1]);
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
        cfg.visionBlankVol = stoi(ret[1]);
      } else if (ret[0] == "visionBodyVol") {
        utils::trim(ret[1]);
        cfg.visionBodyVol = stoi(ret[1]);
      } else if (ret[0] == "visionFruitVol") {
        utils::trim(ret[1]);
        cfg.visionFruitVol = stoi(ret[1]);
      }
    }
    ifs.close();
    // memcpy(resChar, res.c_str(), res.length());
  } else {
    throw "open conf failed";
  }
}

utils::ReadConf::~ReadConf() {
}

utils::config_data utils::ReadConf::getCfg() {
    return cfg;
}