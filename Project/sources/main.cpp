#include "../includefile/config.h"
using namespace std;

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
    const string dataDir = "data/";
    std::string inputFile = "input1.txt";
    std::string outputFile = "output.txt";
    std::string userDictFile = "";
    std::string extraStopFile = "";
    
    // 默认模式
    std::string motion = "Cut(HMM)"; 
    lli stride = 120;
    
    // 控制 UDP 发送的最大数量 (对应前端自定义最大K)
    int udp_max_k = 50; 

    // 解析参数
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-i" && i + 1 < argc) inputFile = argv[++i];
        else if (arg == "-o" && i + 1 < argc) outputFile = argv[++i];
        else if (arg == "-s" && i + 1 < argc) stride = std::stoll(argv[++i]);
        else if (arg == "-m" && i + 1 < argc) motion = argv[++i]; // 支持 Cut(HMM), CutForSearch 等
        
        // 新增参数
        else if (arg == "-u" && i + 1 < argc) userDictFile = argv[++i]; // 用户词典路径
        else if (arg == "-w" && i + 1 < argc) extraStopFile = argv[++i]; // 额外停用词路径
        else if (arg == "-k" && i + 1 < argc) udp_max_k = std::stoi(argv[++i]); // UDP发送限制
    }

    // 初始化UDP发送器
    UdpSender sender("127.0.0.1", 9999);
    
    // 1. 加载默认停用词
    if (!loadstopwords("dict/stop_words.utf8")) {
        std::cerr << "[ERROR] cannot load default stopwords.";
        return 0;
    }

    // 2. 加载用户自定义停用词
    if (!extraStopFile.empty()) {
        loadstopwords(extraStopFile);
    }

    // 3. 加载用户自定义专用词 (Jieba)
    if (!userDictFile.empty()) {
        LoadUserDictFile(userDictFile);
    }

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
            // 保存旧 K
            int original_K = topk.K;
            
            // 设置为前端要求的最大限制 (例如用户想看 Top 100，这里就得设 >= 100)
            topk.K = udp_max_k; 
            
            vector<pair<string, lli>> gui_result = topk.getTopK();
            topk.K = original_K; // 恢复

            string current_time = windows.back().timestamp;
            // 传入 udp_max_k 限制 JSON 大小
            string json_data = generate_json(current_time, gui_result, udp_max_k);
            sender.sendData(json_data);
        }
    }
    
    sender.sendData("EOF");
    out.close();
    cout << "Finished." << endl;
    return 0;
}