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
    // 默认路径前缀
    const string dataDir = "data/";

    std::string inputFile = "input1.txt"; // 默认文件名，稍后处理路径
    std::string outputFile = "output.txt";
    std::string motion = "Cut(HMM)";
    lli stride = 120;
    // 【修改点】：删除了 targetK 变量

    // 3. 解析参数 (删除了 -k 的解析)
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "-i" && i + 1 < argc) {
            inputFile = argv[++i]; // 先只拿参数值
        } 
        else if (arg == "-o" && i + 1 < argc) {
            outputFile = argv[++i];
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
        // 【修改点】：删除了 else if (arg == "-k") 的块
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

    // 初始化UDP发送器
    UdpSender sender("127.0.0.1", 9999);
    
    // 【修改点】：不再设置 topk.K = targetK，使用类内的默认值或在 ACTION 中设置

    // 4. 路径健壮性处理
    // 如果 inputFile 包含 "/" (是路径) 则直接使用，否则加上 dataDir
    string finalInputPath = (inputFile.find('/') != string::npos) ? inputFile : dataDir + inputFile;
    string finalOutputPath = (outputFile.find('/') != string::npos) ? outputFile : dataDir + outputFile;

    vector<string> lines;
    if (!ReadUtf8Lines(finalInputPath, lines)) {
        // 如果失败，尝试直接读取（兼容某些特殊情况）
        if (!ReadUtf8Lines(inputFile, lines)) {
            std::cerr << "[ERROR] cannot open input file: " << finalInputPath << std::endl;
            // 发送错误给前端让它停止转圈
            sender.sendData("{\"error\": \"Cannot open input file\"}");
            return EXIT_FAILURE;
        }
    }
    
    if (lines.empty()) {
        std::cerr << "[WARN ] input file is empty." << std::endl;
    }

    std::ofstream out(finalOutputPath, std::ios::binary);
    if (!out.is_open()) {
        std::cerr << "[ERROR] cannot open output file: " << finalOutputPath << std::endl;
        return EXIT_FAILURE;
    }

    if (!loadstopwords("dict/stop_words.utf8")) {
        std::cerr << "[ERROR] cannot load stopwords.";
        return 0;
    }

    // 主循环
    for (int i = 0; i < lines.size(); i++) {
        const string& sentence = lines[i];

        // ===== 1️⃣ 先判断 ACTION 行 =====
        if (sentence.rfind("[ACTION]", 0) == 0) {
            int K = parseK(sentence);
            if (K <= 0) continue;   
            
            topk.K = K; // 设置为文件指定的 K
            
            if (!windows.empty()) {
                out << windows.back().timestamp << "\n";
            } else {
                std::cerr << "[WARN] windows is empty, cannot access back()." << endl;
            }

            vector<pair<string, lli>> result = topk.getTopK();
            if (result.empty()) {
                continue;
            }
            int count = 1;
            for (auto& kv : result) {
                out << count++ << ":" << kv.first << "(出现" << kv.second << "次)\n";
            }
            continue;
        }

        // ===== 2️⃣ 普通文本行 =====
        EventToken e;
        if (!parseLine(sentence, e)) {
            continue;   
        }

        if (i == 0 || windows.empty()) {
            init_win(e, motion);
        } else {
            update_win(e, stride, motion);
        }

        // ===== 3️⃣ 发送给前端 =====
        if (!windows.empty()) {
            // 【核心修改点】：保存现场 -> 修改 -> 获取 -> 恢复现场
            
            // 1. 保存当前的 K (可能是 ACTION 设置的，也可能是默认的)
            int original_K = topk.K; 
            
            // 2. 强制设为 50 以满足前端“大数据量”需求
            topk.K = 50; 
            
            // 3. 获取 Top-50
            vector<pair<string, lli>> gui_result = topk.getTopK();
            
            // 4. 立即恢复原来的 K，保证不影响文件输出逻辑
            topk.K = original_K; 

            // 5. 发送
            string current_time = windows.back().timestamp;
            string json_data = generate_json(current_time, gui_result);
            sender.sendData(json_data);
        }
    }
    
    sender.sendData("EOF");
    out.close();
    cout << "Finished." << endl;
    return 0;
}