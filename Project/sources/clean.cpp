#include "../includefile/config.h"
using namespace std;
// 以二进制读取，确保不影响编码带来的影响。 
bool ReadUtf8Lines(const std::string& filename, std::vector<std::string>& lines) {
    std::ifstream ifs(filename, std::ios::binary);
    if (!ifs.is_open()) {
        return false;
    }
    std::string line;
    while (std::getline(ifs, line)) {
        // 处理 Windows 下 CRLF 的 \r
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (!line.empty()) {
            lines.push_back(line);
        }
    }
    return true;
}

bool loadstopwords(const string& filename) {
    vector<string> words;
    if (ReadUtf8Lines(filename, words)) {
        for (const auto& w : words) {
            stop_words.insert(w);
        }
    }
    return true;
}

// 加载用户专用词表
void LoadUserDictFile(const string& filepath) {
    vector<string> words;
    if (ReadUtf8Lines(filepath, words)) {
        for (const auto& w : words) {
            jieba.InsertUserWord(w); 
        }
    }
}

bool isstopwords(const string& word) {
    return stop_words.count(word) > 0;
}

vector<string> cleaner(const vector<string>& line) {
    vector<string> result;
    lli length = line.size();
    for(lli i = 0; i < length; i++) {
        if (!isstopwords(line[i])) {
            result.push_back(line[i]);
        }
    }
    return result;
}

// int main () {
//     const string s = "人工智能技术飞速，，。发展";
//     if (!loadstopwords("dict/stop_words.utf8")) {
//         cout << "error";
//         return 0;
//     }
//     cout << segmentation(s, "Cut(HMM)") << endl;
//     return 0;
// }