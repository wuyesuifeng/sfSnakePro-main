#include <iostream>
#include <fstream>
#include <string>

using namespace std;

namespace readConf {
    string getPath() {
        ifstream ifs;
        ifs.open("conf", ios::in);

        if (!ifs.is_open()) {
            ifs.open("/home/xy/workspace/sfSnakePro-main/build/conf", ios::in);
        }

        if (ifs.is_open()) {
            string buff, res;
            while (getline(ifs, buff)) {
                res += buff;
            }
            return res;
        }

        throw "open conf failed";
    }
}