#include "../includefile/config.h"

using namespace std;

// [ACTION]处理模块，获取文本流中的K值
int parseK(const string& line) {
    size_t pos = line.find("K=");
    if (pos == string::npos) return -1;
    pos += 2;
    size_t end = pos;
    while (end < line.size() && isdigit(line[end])) { end++; }
    return stoi(line.substr(pos, end - pos));
}

// 普通文本分割模块，分成时间戳和文本
bool parseLine(const string& line, EventToken& e) {
    if (line.empty()) return false;
    if (line.rfind("[ACTION]", 0) == 0) return false;
    size_t pos = line.find(']');
    if (pos == string::npos || pos + 1 >= line.size()) return false;
    e.timestamp = line.substr(0, pos + 1);
    e.words     = line.substr(pos + 1);
    while (!e.words.empty() && isspace(e.words[0])) { e.words.erase(e.words.begin()); }
    return true;
}


int main(int argc, char* argv[]) {
    const string dataDir = "data/";
    std::string inputFile = "input1.txt";
    std::string outputFile = "output.txt";
    std::string userDictFile = "";
    std::string extraStopFile = "";
    std::string motion = "Cut(HMM)"; 
    lli stride = 120;
    int udp_max_k = 50; 
    
    // 自定义命令行参数
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-i" && i + 1 < argc) inputFile = argv[++i];
        else if (arg == "-o" && i + 1 < argc) outputFile = argv[++i];
        else if (arg == "-s" && i + 1 < argc) stride = std::stoll(argv[++i]);
        else if (arg == "-m" && i + 1 < argc) motion = argv[++i];
        else if (arg == "-u" && i + 1 < argc) userDictFile = argv[++i];
        else if (arg == "-w" && i + 1 < argc) extraStopFile = argv[++i];
        else if (arg == "-k" && i + 1 < argc) udp_max_k = std::stoi(argv[++i]);
    }
    
    // UdpSender的定义在includefile/config.h中
    UdpSender sender("127.0.0.1", 9999);
    
    // 加载停用词和用户专用词
    if (!loadstopwords("dict/stop_words.utf8")) {
        std::cerr << "[ERROR] cannot load default stopwords.";
        return 0;
    }
    if (!extraStopFile.empty()) loadstopwords(extraStopFile);
    if (!userDictFile.empty()) LoadUserDictFile(userDictFile);
    
    // 路径健壮性处理，包含"/"直接使用；否则加上dataDir，默认为data/中的文件
    string finalInputPath = (inputFile.find('/') != string::npos) ? inputFile : dataDir + inputFile;
    string finalOutputPath = (outputFile.find('/') != string::npos) ? outputFile : dataDir + outputFile;

    vector<string> lines;
    if (!ReadUtf8Lines(finalInputPath, lines)) {
        if (!ReadUtf8Lines(inputFile, lines)) {
            std::cerr << "[ERROR] cannot open input file." << std::endl;
            sender.sendData("{\"error\": \"Cannot open input file\"}");
            return EXIT_FAILURE;
        }
    }
    
    std::ofstream out(finalOutputPath, std::ios::binary);
    if (!out.is_open()) return EXIT_FAILURE;


    // 主循环
    for (int i = 0; i < lines.size(); i++) {
        const string& sentence = lines[i];
        // 处理[ACTION]行
        if (sentence.rfind("[ACTION]", 0) == 0) {
            int K = parseK(sentence);
            if (K > 0) {
                topk.K = K;
                if (!windows.empty()) out << windows.back().timestamp << "\n";
                vector<pair<string, lli>> result = topk.getTopK();
                int count = 1;
                for (auto& kv : result) {
                    out << count++ << ":" << kv.first << "(出现" << kv.second << "次)\n";
                }
            }
            else cerr << "[ERROR] K must be positive in [ACTION]." << endl;
        }

        // 处理普通文本行
        
        EventToken e;
        if (!parseLine(sentence, e)) continue;

        if (i == 0 || windows.empty()) {
            init_win(e, motion);
        } else {
            update_win(e, stride, motion);
        }

        if (!windows.empty()) {
            // 保留原始的k值，避免影响文本数据流中的[ACTION]逻辑
            int original_K = topk.K;
            topk.K = udp_max_k;
            vector<pair<string, lli>> gui_result = topk.getTopK();
            topk.K = original_K;
            
            // 将数据处理为JSON，并发送前端
            string current_time = windows.back().timestamp;
            string json_data = generate_json(current_time, gui_result, udp_max_k);
            sender.sendData(json_data);
        }


    }
    sender.sendData("EOF");
    out.close();
    return 0;
}