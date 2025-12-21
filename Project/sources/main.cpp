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

int parseK(const string& line) {
    size_t pos = line.find("K=");
    if (pos == string::npos) return -1;

    pos += 2; // 跳过 "K="
    size_t end = pos;

    while (end < line.size() && isdigit(line[end])) {
        end++;
    }

    return stoi(line.substr(pos, end - pos));
}

bool parseLine(const string& line, EventToken& e) {
    if (line.empty()) return false;

    // ACTION 行提前处理
    if (line.rfind("[ACTION]", 0) == 0) {
        return false;
    }

    size_t pos = line.find(']');
    if (pos == string::npos) {
        return false;
    }

    if (pos + 1 >= line.size()) {
        return false;
    }

    e.timestamp = line.substr(0, pos + 1);
    e.words     = line.substr(pos + 1);

    // trim 前导空格
    while (!e.words.empty() && isspace(e.words[0])) {
        e.words.erase(e.words.begin());
    }

    return true;
}


int main(int argc, char* argv[]) {
    // 定义数据文件夹前缀，相对于main.out的路径，因为最后是由main.out执行的
    const string dataDir = "data/";

    // 1. 设置默认值（也加上 data/ 前缀）
    std::string inputFile = dataDir + "input1.txt";
    std::string outputFile = dataDir + "output.txt";
    std::string motion = "Cut(HMM)";
    lli stride = 120;

    // 2. 循环解析参数 (从 1 开始，因为 0 是程序名)
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "-i" && i + 1 < argc) {
            inputFile = dataDir + argv[++i]; // 获取下一个参数并跳过它
        } 
        else if (arg == "-o" && i + 1 < argc) {
            outputFile = dataDir + argv[++i];
        } 
        else if (arg == "-m" && i + 1 < argc) {
            motion = argv[++i];
        } 
        else if (arg == "-s" && i + 1 < argc) {
            try {
                stride = std::stoll(argv[++i]);
            } catch (...) {
                std::cerr << "错误: -s 后面必须接数字" << std::endl;
            }
        } 
        else if (arg == "--help" || arg == "-h") {
            std::cout << "用法: 程序名 [选项]\n"
                      << "选项:\n"
                      << "  -i <文件>    输入文件 (默认: input1.txt)\n"
                      << "  -o <文件>    输出文件 (默认: output.txt)\n"
                      << "  -m <动作>    动作类型 (默认: Cut(HMM))\n"
                      << "  -s <数值>    步长 (默认: 120)\n";
            return 0;
        }
        else {
            std::cerr << "未知参数: " << arg << std::endl;
        }
    }
    
    vector<string> lines;
    if (!ReadUtf8Lines(inputFile, lines)) {
        std::cerr << "[ERROR] cannot open input file: " << inputFile << std::endl;
        std::cerr << "[HINT ] create a UTF-8 file named '" << inputFile << "' with Chinese sentences." << std::endl;
        return EXIT_FAILURE;
    }
    if (lines.empty()) {
        std::cerr << "[WARN ] input file is empty." << std::endl;
    }

    std::ofstream out(outputFile, std::ios::binary);
    if (!out.is_open()) {
        std::cerr << "[ERROR] cannot open output file: " << outputFile << std::endl;
        return EXIT_FAILURE;
    }

    if (!loadstopwords("dict/stop_words.utf8")) {
        std::cerr << "[ERROR] cannot load stopwords.";
        return 0;
    }
    for (int i = 0; i < lines.size(); i++) {
    const string& sentence = lines[i];

    // ===== 1️⃣ 先判断 ACTION 行 =====
    if (sentence.rfind("[ACTION]", 0) == 0) {

        int K = parseK(sentence);
        if (K <= 0) continue;   // 非法 ACTION，直接跳过

        topk.K = K;

        // ⚠️ 只有 windows 非空，才能访问 back()
        if (!windows.empty()) {
            out << windows.back().timestamp << "\n";
        } else {
            std::cerr << "[WARN] windows is empty, cannot access back()." << endl;
        }

        vector<pair<string, lli>> result = topk.getTopK();
        if (result.empty()) {
            std::cerr << "[WARN] TopK result is empty, skipping output." << endl;
            continue;
        }
        int count = 1;
        for (auto& kv : result) {
            out << count++ << ":"
                << kv.first
                << "(出现" << kv.second << "次)\n";
        }
        continue;
    }

    // ===== 2️⃣ 普通文本行 =====
    EventToken e;
    if (!parseLine(sentence, e)) {
        std::cerr << "[WARN] Failed to parse line: " << sentence << endl;
        continue;   // 非法普通行，直接跳过
    }

    if (i == 0 || windows.empty()) {
        init_win(e, motion);
    } else {
        update_win(e, stride, motion);
    }
}
    out.close();
    return 0;
}