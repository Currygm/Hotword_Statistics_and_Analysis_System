#ifndef CONFIG_H
#define CONFIG_H
// 防止头文件被重复包含

#include <string>
#include <vector>
#include <deque>
#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_set>
#include <unordered_map>
#include <queue>
#include <cstdlib>
#include <set>
#include <sys/socket.h> // Linux Socket
#include <arpa/inet.h>  // inet_addr
#include <unistd.h>     // close
#include <cstring>      // strlen
#include <thread>       // sleep_for
#include <iomanip>
#include <chrono>
#include "cppjieba/Jieba.hpp"
using namespace std;
typedef long long lli;
// 获取文本时每一帧文本
struct EventToken {
    std::string timestamp;
    std::string words;
};
// 储存在哈希表中的每一帧文本(已分词) 
struct stamp {
    string timestamp;
    vector<string> words;
};
// 在二叉树中统计词频
struct node {
    string word;
    lli cnt;
    // 自定义比较器，降序时用>，升序时用<
    // 由于set或map遵循严格弱排序，如果认为a > b为false，并且b > a也为false，则认为二者相等
    // 假如没有字典序的比较，如果出现了cnt相等的情形
    // set就会认为两个元素是相等的，从而忽略后续插入的元素。
    // 严格弱排序比较器的元素关系：反自反性，反对称性，传递性，唯一性
    bool operator>(const node& other) const{
        if (cnt != other.cnt) {
            return cnt > other.cnt;
        }
        return word < other.word;
    }
};
// 平衡二叉树实现TopK
class TopK {
public:
    lli K = 0;
    static set<node, greater<node>> S;
    static unordered_map<string, set<node, greater<node>>::iterator> pos;
    void update(const string& w);
    vector<pair<string, lli>> getTopK();
};

class UdpSender {
    int sock;
    struct sockaddr_in serverAddr;
public:
    UdpSender(const char* ip, int port);
    void sendData(const std::string& data);
    ~UdpSender();
};

extern string generate_json(const string& timestamp, const vector<pair<string, lli>>& result);
extern deque<stamp> windows;
extern cppjieba::Jieba jieba;
extern std::unordered_set<std::string> stop_words;
extern string front_of_deque;
extern string back_of_deque;
extern unordered_map<string, lli> words_count;
extern TopK topk;

bool loadstopwords(const std::string& filename);
std::vector<std::string> cleaner(const std::vector<std::string>& line);
vector<string> segmentation(std::string line, const std::string& motion);
lli time_sub(string t1, string t2);
void del_count(vector<string> words);
void add_count(vector<string> words);
void init_win(EventToken e, const string& motion);
void update_win(EventToken e, lli stride, const string& motion);
#endif