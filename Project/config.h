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
struct stamp {
    string timestamp;
    unordered_map<string, int> map;
};
extern deque<stamp> windows;
extern cppjieba::Jieba jieba;
extern std::unordered_set<std::string> stop_words;
extern string front_of_deque;
extern string back_of_deque;

bool loadstopwords(const std::string& filename);
std::vector<std::string> cleaner(const std::vector<std::string>& line);
vector<string> segmentation(std::string line, const std::string& motion);
lli time_sub(string t1, string t2);
#endif