#include "cppjieba/Jieba.hpp"
#include "config.h"
#include <vector>
#include <cstdlib>
#include <iostream>
#include <sstream>
using namespace std;
string Join(const vector<string>& items, const string& delim) {
    ostringstream oss; 
    // ostringstream能够高效构造字符串，尤其是在高频构造大量字符串的情况下
    // 将添加内容写入缓冲区。
    for (size_t i = 0; i < items.size(); ++i) {
        if (i) oss << delim;
        oss << items[i];
    }
    return oss.str();
}

string segmentation(string line, const string& motion) {
    vector<string> sentence;
    if (motion == "Cut(HMM)") {
        jieba.Cut(line, sentence, true);
    }
    else if (motion == "Cut(NoHMM)") {
        jieba.Cut(line, sentence, false);
    }
    else if (motion == "CutForSearch"){
        jieba.CutForSearch(line, sentence);
    }
    sentence = cleaner(sentence);
    return Join(sentence, "/");
}

// int main () {
//     string line("人工智能技术高速发展");
//     line = segmentation(line, "CutForSearch");
//     cout << line << endl;
//     return 0;
// }