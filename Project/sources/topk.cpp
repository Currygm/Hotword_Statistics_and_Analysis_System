#include "../includefile/config.h"
using namespace std;

// 更新Top-K set
void TopK::update(const string& w) {
    // 删除旧节点（如果有）
    if (pos.find(w) != pos.end()) {
        auto it = pos[w];
        // it是否超出S的范围，查看该元素是否在S中
        if (it != S.end() && S.find(*it) != S.end()) {  // 确保迭代器有效
            S.erase(it);
        } else {
            cerr << "[WARN] Iterator invalid or element not found in S for word: " << w << endl;
        }
        pos.erase(w);  // 从 pos 中移除无效的迭代器
    }

    // 插入新节点
    if (words_count[w] > 0) {
        auto newi = S.insert({w, words_count[w]}).first;
        pos[w] = newi;
    }
}

// 获取当前Top-K
vector<pair<string, lli>> TopK::getTopK() {
    vector<pair<string, lli>> result;
    auto it = S.begin();
    for (lli i = 0; i < K && it != S.end();i++, it++) {
        result.push_back({it -> word, it -> cnt});
    }
    return result;
}

// int main() {

//     // 加载停用词
//     if (!loadstopwords("dict/stop_words.utf8")) {
//         std::cerr << "[ERROR] cannot load default stopwords.";
//         return 0;
//     }
//     // 初始化测试数据
//     vector<EventToken> test_events = {
//         {"[0:00:01]", "人工智能 技术 高速 发展"},
//         {"[0:01:00]", "人工智能 正在 改变 世界"},
//         {"[0:02:00]", "机器学习 是 人工智能 的 核心"},
//         {"[0:05:00]", "深度学习 推动了 人工智能 的 进步"},
//         {"[0:10:00]", "人工智能 的 未来 充满 无限 可能"}
//     };

//     // 滑动窗口时间跨度为 300 秒（5 分钟）
//     lli stride = 300;
//     string motion = "Cut(HMM)";

//     // 设置 TopK 的 K 值
//     topk.K = 3;

//     // 初始化滑动窗口
//     init_win(test_events[0], motion);

//     // 模拟滑动窗口的更新
//     for (size_t i = 1; i < test_events.size(); ++i) {
//         cout << "Processing event: " << test_events[i].timestamp << " - " << test_events[i].words << endl;
//         update_win(test_events[i], stride, motion);

//         // 输出当前窗口的状态
//         cout << "Window front timestamp: " << windows.front().timestamp << endl;
//         cout << "Window back timestamp: " << windows.back().timestamp << endl;

//         // 输出窗口中的所有事件
//         cout << "Current events in the window:" << endl;
//         for (const auto& event : windows) {
//             cout << event.timestamp << " - " << event.words << endl;
//         }

//         // 输出当前窗口中的 TopK 结果
//         vector<pair<string, lli>> result = topk.getTopK();
//         cout << "Top " << topk.K << " words in the window:" << endl;
//         for (size_t j = 0; j < result.size(); ++j) {
//             cout << j + 1 << ": " << result[j].first << " (出现 " << result[j].second << " 次)" << endl;
//         }
//         cout << "---------------------------------" << endl;
//     }

//     return 0;
// }