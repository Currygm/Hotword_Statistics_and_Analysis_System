#include "../includefile/config.h"

using namespace std;
cppjieba::Jieba jieba(           // 创建一个jieba对象
    "dict/jieba.dict.utf8",      // 主词典
    "dict/hmm_model.utf8",       // HMM 模型
    "dict/user.dict.utf8",       // 用户词典
    "dict/idf.utf8",             // IDF 统计文件
    "dict/stop_words.utf8"       // 停用词表
);
deque<stamp> windows;
unordered_set<string> stop_words;   // 存放从文件中加载的停用词，具有唯一性，O(1)插入、删除、查找时间复杂度。
string front_of_deque;
string back_of_deque;
unordered_map<string, lli> words_count;
set<node, greater<node>> TopK::S;
unordered_map<string, set<node, greater<node>>::iterator> TopK::pos;   // 记录每个词在二叉树中的迭代器
TopK topk;