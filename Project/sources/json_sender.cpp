#include "../includefile/config.h"
using namespace std;

string generate_json(const std::string& timestamp, const std::vector<std::pair<std::string, lli>>& result, int limit_k) {
    std::stringstream ss;
    ss << "{";
    ss << "\"time\": \"" << timestamp << "\",";
    ss << "\"top_k\": [";
    int count = 0;
    for (size_t i = 0; i < result.size() && count < limit_k; ++i, ++count) {
        ss << "{";
        ss << "\"word\": \"" << result[i].first << "\",";
        ss << "\"count\": " << result[i].second;
        ss << "}";
        if (i < result.size() - 1 && count < limit_k - 1) ss << ",";
    }
    ss << "]";
    ss << "}";
    return ss.str();
}