#ifndef __UTIL_H__
#define __UTIL_H__

#include <assert.h>
#include <signal.h>
#include <string.h>
#include <vector>

#include <sys/time.h>

#include <iostream>
#include <sstream>
#include <string>

#define LogErr std::cerr<<__FILE__<<" | "<<__func__<<":"<<__LINE__<<" # "
#define LogInfo std::cout<<__FILE__<<" | "<<__func__<<":"<<__LINE__<<" # "
#define LogDebug std::cout<<__FILE__<<" | "<<__func__<<":"<<__LINE__<<" # "


#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

class IoBuffer;

inline void IgnoreSignalPipe()
{
    signal(SIGPIPE, SIG_IGN);
}

template<typename T>
static std::string ToStr(const T& t)
{   
    std::ostringstream os; 
    os << t;

    return os.str();
}   

template<typename T>
static T StrTo(const std::string& str)
{   
    T ret;
    std::istringstream is(str);

    is >> ret;

    return ret;
}   

inline bool IsIpStr(const std::string& str)
{
    for (const auto& ch : str)
    {
        if (ch == '.')
        {
            continue;
        }

        if (ch < '0' || ch > '9')
        {
            return false;
        }
    }

    return true;
}


std::string BinToHex(const std::string& str, const size_t& one_line_count = 16, const bool& print_ascii = true);
std::string BinToHex(const uint8_t* buf, const size_t& len, const size_t& one_line_count = 16, const bool& print_ascii = true);
std::vector<std::string> SplitStr(const std::string& input, const std::string& sep);

template<typename T>
inline T Max(const T& l, const T& r)
{
    return (l > r) ? l : r;
}

inline std::string PrintErr(const std::string& func, const int& ret)
{
    std::ostringstream os;
    os << "func<" << func << ">, ret:" << ret << ", err:[" << strerror(errno) << "]";

    return os.str();
}

inline uint64_t GetNowInMillSecond()
{
    uint64_t ms;

    timeval tv;

    int ret = gettimeofday(&tv, NULL);
    if (ret < 0)
    {
        LogErr << PrintErr("gettimeofday", ret) << std::endl;
    }

    ms = tv.tv_sec * 1000 + tv.tv_usec / 1000;

    return ms;
}

inline uint64_t GetNowInSecond()
{
    return GetNowInMillSecond() / 1000;
}

#endif // __UTIL_H__
