#pragma once

#include <string>
#include <vector>

using namespace std;

namespace utils {
    void trim(string &str);
    void split(string &str, const string &del, vector<std::string> *ret);
}