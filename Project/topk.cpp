#include "config.h"
using namespace std;

void TopK::update(const string& w) {
    // 删除旧节点（如果有）
    if (pos.find(w) != pos.end()) {
        auto it = pos[w];
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

vector<pair<string, lli>> TopK::getTopK() {
    vector<pair<string, lli>> result;
    auto it = S.begin();
    for (lli i = 0; i < K && it != S.end();i++, it++) {
        result.push_back({it -> word, it -> cnt});
    }
    return result;
}