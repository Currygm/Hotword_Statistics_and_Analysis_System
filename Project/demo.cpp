// ============================= demo.cpp =====================================
// 目标：演示 cppjieba 在“文件输入 -> 分词/标注/关键词 -> 文件输出”流程。
// 示例主题：中山大学 计算机学院的学习与科研正能量场景（句子放在输入文件中，不直接写死在代码里）。
// 重要要求：为避免 Windows 控制台编码问题，代码中不直接向终端输出任何中文（只输出英文提示）。
// 如果需要查看中文结果，请打开输出文件（UTF-8 编码）。
// ----------------------------------------------------------------------------
// 功能概览：
// 1. 初始化 cppjieba 分词器（加载 5 类词典）
// 2. 从输入文件逐行读取中文句子
// 3. 执行：基本分词（含/不含 HMM）+ 搜索分词 + 词性标注 + 关键词抽取
// 4. 将结果写入输出文件，供分析对比
// ----------------------------------------------------------------------------
// 使用说明（假设当前工作目录为项目根）：
//   编译： g++ -std=c++11 -O2 -I. -I./cppjieba -o demo
//   运行： ./demo                # 使用默认 input/output 文件名
//   或   ： ./demo input.txt out.txt
// 默认输入： test_sentences.txt   默认输出： output.txt
// 请确保 dict/ 目录与可执行文件同级或路径设置正确。
// ----------------------------------------------------------------------------
// 教学要点速览：
// - Cut           : 基本分词（可选 HMM）
// - CutForSearch  : 搜索引擎模式，更细粒度，提高召回
// - Tag           : 词性标注，了解词语在句中的角色
// - Keyword (TF-IDF): 统计权重，抽取高权重词
// - InsertUserWord: 动态添加专有名词（如“中山大学计算机学院”）提升识别准确度
// ----------------------------------------------------------------------------

#include "cppjieba/Jieba.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdlib>

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

// 将 vector<string> 用分隔符拼接为单行字符串。
// 用途：输出分词结果时以统一的 "word1/word2/word3" 形式展示，便于肉眼对比差异。
std::string Join(const std::vector<std::string>& items, const std::string& delim) {
    std::ostringstream oss;
    for (size_t i = 0; i < items.size(); ++i) {
        if (i) oss << delim;
        oss << items[i];
    }
    return oss.str();
}

int main(int argc, char* argv[]) {
    // 步骤 1: 解析命令行参数（仅英文输出，避免控制台中文编码问题）
    std::string inputFile = "test_sentences.txt";   // 确保中文编码是UTF-8 
    std::string outputFile = "output.txt";          // default output filename
    if (argc >= 2) inputFile = argv[1];
    if (argc >= 3) outputFile = argv[2];

    // 步骤 2: 初始化 Jieba 分词器
    // 注意：路径应与运行时工作目录匹配；若可执行文件与 dict/ 不同层级，需要自行调整。
    cppjieba::Jieba jieba(
        "dict/jieba.dict.utf8",      // 主词典
        "dict/hmm_model.utf8",       // HMM 模型
        "dict/user.dict.utf8",       // 用户词典
        "dict/idf.utf8",             // IDF 统计文件
        "dict/stop_words.utf8"       // 停用词表
    );

    // 步骤 3: 读取输入文件。
    std::vector<std::string> lines;
    if (!ReadUtf8Lines(inputFile, lines)) {
        std::cerr << "[ERROR] cannot open input file: " << inputFile << std::endl;
        std::cerr << "[HINT ] create a UTF-8 file named '" << inputFile << "' with Chinese sentences." << std::endl;
        return EXIT_FAILURE;
    }
    if (lines.empty()) {
        std::cerr << "[WARN ] input file is empty." << std::endl;
    }

    // 步骤 4: 打开输出文件
    std::ofstream out(outputFile, std::ios::binary);
    if (!out.is_open()) {
        std::cerr << "[ERROR] cannot open output file: " << outputFile << std::endl;
        return EXIT_FAILURE;
    }

    // 输出文件头部信息。
    out << "===== cppjieba segmentation demo (Sun Yat-sen University CS School) =====\n";
    out << "InputFile: " << inputFile << "\n";
    out << "LineCount: " << lines.size() << "\n";
    out << "Note: Chinese sentences are read from input and processed; console prints remain English to avoid encoding issues.\n\n";

    // 步骤 5: 插入用户词（专有名词）以提升识别完整性
    // 不输出中文到终端，仅在文件中体现效果。
    jieba.InsertUserWord("中山大学计算机学院");

    // 步骤 6: 遍历每一行进行分词、标注、关键词抽取
    const size_t topk = 5; // 关键词抽取数量（可调）
    for (size_t idx = 0; idx < lines.size(); ++idx) {
        const std::string& sentence = lines[idx];
        out << "[LINE " << (idx + 1) << "] " << sentence << "\n";

        // 基本分词 (使用 HMM)
        std::vector<std::string> cutWords;
        jieba.Cut(sentence, cutWords, true);
        out << "  Cut(HMM): " << Join(cutWords, "/") << "\n";

        // 基本分词 (不使用 HMM) — 对比差异
        std::vector<std::string> cutWordsNoHMM;
        jieba.Cut(sentence, cutWordsNoHMM, false);
        out << "  Cut(NoHMM): " << Join(cutWordsNoHMM, "/") << "\n";

        // 搜索引擎模式分词（更细粒度）
        std::vector<std::string> searchWords;
        jieba.CutForSearch(sentence, searchWords);
        out << "  CutForSearch: " << Join(searchWords, "/") << "\n";

        // 词性标注
        std::vector<std::pair<std::string, std::string>> tagres;
        jieba.Tag(sentence, tagres);
        out << "  Tag: ";
        for (size_t i = 0; i < tagres.size(); ++i) {
            if (i) out << "/";
            out << tagres[i].first << "(" << tagres[i].second << ")";
        }
        out << "\n";

        // 关键词抽取 (TF-IDF) — 返回 Word 结构，含权重
        std::vector<cppjieba::KeywordExtractor::Word> keywordres;
        jieba.extractor.Extract(sentence, keywordres, topk);
        out << "  Keywords(TF-IDF): ";
        for (size_t i = 0; i < keywordres.size(); ++i) {
            if (i) out << ", ";
            out << keywordres[i].word << ":" << keywordres[i].weight;
        }
        out << "\n";

        // 关键词抽取 (另一接口) — 直接 pair<string,double>
        std::vector<std::pair<std::string, double>> keywordres2;
        jieba.extractor.Extract(sentence, keywordres2, topk);
        out << "  Keywords2: ";
        for (size_t i = 0; i < keywordres2.size(); ++i) {
            if (i) out << ", ";
            out << keywordres2[i].first << ":" << keywordres2[i].second;
        }
        out << "\n\n"; // 行间空行
    }

    out << "===== END =====\n";
    out.close();

    std::cout << "Processing finished. Output file generated: " << outputFile << std::endl;
    std::cout << "Open the file with a UTF-8 capable editor to view results." << std::endl;
    return EXIT_SUCCESS;
}

