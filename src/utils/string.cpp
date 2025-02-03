#include "string.hpp"

using namespace std;

void utils::trim(string &str) {
    if (str.empty()) return;
    str.erase(0, str.find_first_not_of(" "));
    str.erase(str.find_last_not_of(" ") + 1);
}

void utils::split(string &str, const string &del, vector<string> *ret) {
    size_t last = 0;
	size_t index = str.find_first_of(del, last);

	while (index != std::string::npos) {
		ret->push_back(str.substr(last, index - last));
		last = index + 1;
		index = str.find_first_of(del, last);
	}

	if (index - last > 0) {
		ret->push_back(str.substr(last, index - last));
	}

}