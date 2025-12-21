#include "config.h"
using namespace std;

// 这里的stride以秒为单位
// 初始化滑动窗口
void init_win(EventToken e, const string& motion) {
    vector<string> seg = segmentation(e.words, motion);
    stamp new_st = {e.timestamp, seg};
    add_count(seg);
    windows.push_back(new_st);
    front_of_deque = e.timestamp;
    back_of_deque = e.timestamp;
}
// 对滑动窗口进行实时更新
void update_win(EventToken e, lli stride, const string& motion) {
    while (time_sub(front_of_deque, e.timestamp) > stride) {
        del_count(windows.front().words);
        windows.pop_front();
        if (!windows.empty()) {
            front_of_deque = windows.front().timestamp;
        }
        else break;
    }
    vector<string> seg = segmentation(e.words, motion);
    if (windows.empty()) {
        front_of_deque = e.timestamp;
    }
    back_of_deque = e.timestamp;
    stamp new_st = {e.timestamp, seg};
    windows.push_back(new_st);
    add_count(seg);
}

// int main () {
//     // 要记得初始化stop_words，否则无法加载停用词
//     if (!loadstopwords("dict/stop_words.utf8")) {
//         cout << "error";
//         return 0;
//     }
//     topk.K = 5;
//     const string s1 = "[0:00:01] 人工智能技术高速发展，。。，（）中山大学计算机学院";
//     const string s2 = "[0:01:40] 中山大学计算机学院计算机科学与技术";
//     const string s3 = "[0:06:30] 计算机科学与技术";
//     const string s4 = "[1:00:00] 系统架构，人工智能";
//     EventToken e1 = {.timestamp = s1.substr(0, 9), .words = s1.substr(10, 100)};
//     EventToken e2 = {.timestamp = s2.substr(0, 9), .words = s2.substr(10, 100)};
//     EventToken e3 = {.timestamp = s3.substr(0, 9), .words = s3.substr(10, 100)};
//     EventToken e4 = {.timestamp = s4.substr(0, 9), .words = s4.substr(10, 100)};
//     init_win(e1, "Cut(HMM)");
//     cout << windows.front().timestamp << endl;
//     vector<pair<string, lli>> result = topk.getTopK();
//     for (auto kv : result) {
//         cout << kv.first << " : " << kv.second << endl;
//     }
//     update_win(e2, 300, "Cut(HMM)");
//     update_win(e3, 300, "Cut(HMM)");
//     result = topk.getTopK();
//     cout << windows.front().timestamp << endl;
//     for (auto kv : result) {
//         cout << kv.first << " : " << kv.second << endl;
//     }
//     cout << windows.back().timestamp << endl;
//     update_win(e4, 300, "Cut(HMM)");
//     result = topk.getTopK();
//     cout << windows.front().timestamp << endl;

//     // 用迭代器遍历
//     for (auto kv : result) {
//         cout << kv.first << " : " << kv.second << endl;
//     }
//     cout << windows.back().timestamp << endl;
//     return 0;
// }