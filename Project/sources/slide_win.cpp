#include "../includefile/config.h"
using namespace std;

// 所有时间的格式都是[0:00:00]
// 读取时，分，秒
lli geth(string t) {
    t = t.substr(1, 1);
    return stoi(t);
}
lli getm(string t) {
    t = t.substr(3, 2);
    return stoi(t);
}
lli gets(string t) {
    t = t.substr(6, 2);
    return stoi(t);
}

// 计算每个时间点的秒数
lli get_time(string t) {
    lli th = geth(t);
    lli tm = getm(t);
    lli ts = gets(t);
    return th * 60 * 60 + tm * 60 + ts;
}

// 计算两个时间节点的时间差，返回的结果以秒s作为单位
lli time_sub(string t1, string t2) {
    lli ti1 = get_time(t1);
    lli ti2 = get_time(t2);
    return ti1 > ti2? (ti1 - ti2) : (ti2 - ti1);
}
// 初始化滑动窗口，在传入第一行时调用
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

// int main() {
//     // 初始化测试数据
//     vector<EventToken> test_events = {
//         {"[0:00:01]", "事件1"},
//         {"[0:01:00]", "事件2"},
//         {"[0:02:00]", "事件3"},
//         {"[0:05:00]", "事件4"},
//         {"[0:10:00]", "事件5"}
//     };

//     // 滑动窗口时间跨度为 300 秒（5 分钟）
//     lli stride = 300;
//     string motion = "Cut(HMM)";

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
//         cout << "---------------------------------" << endl;
//     }

//     return 0;
// }