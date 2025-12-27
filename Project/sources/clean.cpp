#include "../includefile/config.h"
using namespace std;
// 以二进制读取，确保不影响编码带来的影响。 
bool ReadUtf8Lines(const std::string& filename, std::vector<std::string>& lines) {
    std::ifstream ifs(filename, std::ios::binary);
    if (!ifs.is_open()) {
        std::cerr << "[ERROR] Failed to open file: " << filename << std::endl;
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

// 加载停用词列表
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

// 判断是否为停用词
bool isstopwords(const string& word) {
    return stop_words.count(word) > 0;
}

// 对所给行进行停用词清洗
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

// 分词模块
vector<string> segmentation(string line, const string& motion) {

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
    else {
        cerr << "[ERROR] " << motion << "is not a motion";
    }
    sentence = cleaner(sentence);
    return sentence;
}

// int main() {
//     // 测试数据
//     vector<string> test_cases = {
//         "人工智能技术飞速，，。发展",  // 测试标点符号清理
//         "这是一个测试句子。",          // 测试普通句子
//         "12345，测试数字过滤！",       // 测试数字和标点
//         "Hello, 世界！",              // 测试中英文混合
//         "    前面有空格的句子",        // 测试前导空格
//         "句子后面有空格    ",          // 测试尾随空格
//         "重复的词语 重复的词语",       // 测试重复词语
//         "特殊符号@#￥%……&*（）",       // 测试特殊符号
//         "",                           // 测试空字符串
//         "单词",                       // 测试单个词语
//     };

//     // 加载停用词
//     if (!loadstopwords("dict/stop_words.utf8")) {
//         cerr << "error: failed to load stopwords" << endl;
//         return 0;
//     }

//     // 遍历测试数据并输出结果
//     for (const string& test_case : test_cases) {
//         cout << "Input: " << test_case << endl;
//         cout << "Output: " << segmentation(test_case, "Cut(HMM)") << endl;
//         cout << "---------------------------------" << endl;
//     }

//     return 0;
// }