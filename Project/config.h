#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <vector>
#include <deque>
#include "cppjieba/Jieba.hpp"
struct EventToken {
    std::string timestamp;
    std::string words;
};

extern std::deque<EventToken> datastream;
extern cppjieba::Jieba jieba;
#endif