#pragma once

#include <iostream>
#include <fstream>
#include <string.h>

#define DEBUG_CONF_PATH "/home/xy/workspace/sfSnakePro-main/build/conf"
#define CONF_PATH "conf"

using namespace std;

namespace utils {
    static char* getPath() {
        ifstream ifs;
        ifs.open(CONF_PATH, ios::in);

        if (!ifs.is_open()) {
            ifs.open(DEBUG_CONF_PATH, ios::in);
        }

        if (ifs.is_open()) {
            string buff, res;
            while (getline(ifs, buff)) {
                res += buff;
            }
            ifs.close();
            const int len = res.length();
            char* resChar = (char*) calloc(len, sizeof(char));
            memcpy(resChar, res.c_str(), len);
            return resChar;
        } else {
            throw "open conf failed";
        }

    }
}