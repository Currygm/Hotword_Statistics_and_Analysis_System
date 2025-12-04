#include "config.h"

using namespace std;
cppjieba::Jieba jieba(           // 创建一个jieba对象
    "dict/jieba.dict.utf8",      // 主词典
    "dict/hmm_model.utf8",       // HMM 模型
    "dict/user.dict.utf8",       // 用户词典
    "dict/idf.utf8",             // IDF 统计文件
    "dict/stop_words.utf8"       // 停用词表
);
deque<EventToken> datastream;