#include "../includefile/config.h"
using namespace std;
UdpSender::UdpSender(const char* ip, int port) {
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = inet_addr(ip);
}
    
void UdpSender::sendData(const std::string& data) {
    sendto(sock, data.c_str(), data.length(), 0, 
        (struct sockaddr*)&serverAddr, sizeof(serverAddr));
}

UdpSender::~UdpSender() {
    close(sock);
}

string generate_json(const string& timestamp, const vector<pair<string, lli>>& result) {
    stringstream ss;
    ss << "{";
    ss << "\"time\": \"" << timestamp << "\",";
    ss << "\"top_k\": [";
    for (size_t i = 0; i < result.size(); ++i) {
        ss << "{";
        ss << "\"word\": \"" << result[i].first << "\",";
        ss << "\"count\": " << result[i].second;
        ss << "}";
        if (i < result.size() - 1) ss << ",";
    }
    ss << "]";
    ss << "}";
    return ss.str();
}