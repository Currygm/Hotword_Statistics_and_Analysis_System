#include "config.h"
using namespace std;

// 当滑动窗口满容量时删除队列首的元素
void del_count(vector<string> words) {
    for (lli i = 0; i < words.size(); i++) {
        words_count[words[i]]--;
        topk.update(words[i]);
        if (words_count[words[i]] == 0) {
            // 当某个键的值清零时删除该元素
            words_count.erase(words[i]);
        }
    }
}

// 读取到新的一行时更新计数器
void add_count(vector<string> words) {
    for (lli i = 0; i < words.size(); i++) {
        words_count[words[i]]++;
        topk.update(words[i]);
    }
}