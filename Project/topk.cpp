#include "config.h"
using namespace std;

void TopK::update(const string& w) {
    // 删除旧节点（如果有）
    if (pos.find(w) != pos.end()) {
        auto it = pos[w];
        S.erase(it);
    }
    // 插入新节点
    if (words_count[w] == 0) return;
    auto newi = S.insert({w, words_count[w]}).first;
    // .first提取了指向插入元素的迭代器
    pos[w] = newi;
}

vector<pair<string, lli>> TopK::getTopK() {
    vector<pair<string, lli>> result;
    auto it = S.begin();
    for (lli i = 0; i < K && it != S.end();i++, it++) {
        result.push_back({it -> word, it -> cnt});
    }
    return result;
}