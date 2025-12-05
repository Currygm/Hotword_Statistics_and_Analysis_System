#include "config.h"
using namespace std;

// 所有时间的格式都是[0:00:00]
// stoi是std中的函数，不是string类型的
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

