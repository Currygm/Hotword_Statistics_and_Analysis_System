#include "../includefile/config.h"
using namespace std;
// 以二进制读取，确保不影响编码带来的影响。
bool loadstopwords(const string& filename) {
    ifstream ifs(filename, ios::binary);
    string word;
    if (!ifs.is_open()) return false;
    while(getline(ifs, word)) {
        // Windows下，在二进制模式下getline()会保留'\r'需要手动清除。
        // 文件中可能存在空行，或者文件末尾可能有一个换行符导致空行，所以需要判断
        if(!word.empty() && word.back() == '\r') {
            word.pop_back();
        }
        if(!word.empty()) stop_words.insert(word);
    }
    return true;
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