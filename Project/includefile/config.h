#ifndef CONFIG_H
#define CONFIG_H

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
#include <iomanip>
#include <chrono>
#include <thread>

// 跨平台 Socket 兼容代码
#ifdef _WIN32
    // Windows 平台
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib") 
    
    // 不要再 define close closesocket 了，会冲突！
    
    typedef int socklen_t;
#else
    // Linux / macOS 平台
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <unistd.h>     
    #include <cstring>      
#endif

#include "cppjieba/Jieba.hpp"

using namespace std;
typedef long long lli;

// 结构体定义
struct EventToken {
    std::string timestamp;
    std::string words;
};

struct stamp {
    string timestamp;
    vector<string> words;
};

struct node {
    string word;
    lli cnt;
    bool operator>(const node& other) const{
        if (cnt != other.cnt) {
            return cnt > other.cnt;
        }
        return word < other.word;
    }
};

class TopK {
public:
    lli K = 0;
    static set<node, greater<node>> S;
    static unordered_map<string, set<node, greater<node>>::iterator> pos;
    void update(const string& w);
    vector<pair<string, lli>> getTopK();
};

// UDP 发送类
class UdpSender {
    int sock;
    struct sockaddr_in serverAddr;
#ifdef _WIN32
    bool wsa_initialized = false;
#endif

public:
    UdpSender(const char* ip, int port) {
#ifdef _WIN32
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) == 0) {
            wsa_initialized = true;
        }
#endif
        sock = socket(AF_INET, SOCK_DGRAM, 0);
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        serverAddr.sin_addr.s_addr = inet_addr(ip);
    }

    void sendData(const std::string& data) {
        if (sock < 0) return;
        sendto(sock, data.c_str(), (int)data.length(), 0, 
               (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    }

    ~UdpSender() {
        if (sock >= 0) {
#ifdef _WIN32
            closesocket(sock); // Windows 专用关闭函数
#else
            close(sock);       // Linux 专用关闭函数
#endif
        }
#ifdef _WIN32
        if (wsa_initialized) WSACleanup();
#endif
    }
};

extern deque<stamp> windows;
extern cppjieba::Jieba jieba;
extern std::unordered_set<std::string> stop_words;
extern string front_of_deque;
extern string back_of_deque;
extern unordered_map<string, lli> words_count;
extern TopK topk;

void LoadUserDictFile(const string& filepath);
bool ReadUtf8Lines(const std::string& filename, vector<string>& lines);
bool loadstopwords(const std::string& filename);
vector<string> cleaner(const vector<string>& line);
vector<string> segmentation(string line, const string& motion);
lli time_sub(string t1, string t2);
void del_count(vector<string> words);
void add_count(vector<string> words);
void init_win(EventToken e, const string& motion);
void update_win(EventToken e, lli stride, const string& motion);
string generate_json(const std::string& timestamp, const std::vector<std::pair<std::string, lli>>& result, int limit_k);
#endif