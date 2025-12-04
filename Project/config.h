#ifndef CONFIG_H
#define CONFIG_H
// 防止头文件被重复包含

#include <string>
#include <vector>
#include <deque>
#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_set>
#include <cstdlib>
#include "cppjieba/Jieba.hpp"
using namespace std;
typedef long long lli;
struct EventToken {
    std::string timestamp;
    std::string words;
};

extern std::deque<EventToken> datastream;
extern cppjieba::Jieba jieba;
extern unordered_set<string> stop_words;

vector<string> cleaner(const vector<string>& line);
string segmentation(string line, const string& motion);
#endif