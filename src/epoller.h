#ifndef __EPOLLER_H__
#define __EPOLLER_H__

#include <sys/epoll.h>

#include <map>
#include <set>
#include <vector>

class Epoller
{
public:
    Epoller();
    ~Epoller();

    int EnableRead(const int& fd, void* args);
    int EnableWrite(const int& fd, void* args);
    int EnableAll(const int& fd, void* args);
    int DisableRead(const int& fd);
    int DisableWrite(const int& fd);
    int DisableAll(const int& fd);

    void TimeoutAt(void* args, const uint64_t& ms);
    void TimeoutAfter(void* args, const uint64_t& ms);

    void Wait(const int& ms, std::vector<void*>& active, std::vector<void*>& timeout);

private:
    int EnableEvent(const int& fd, const uint32_t& event, void* args);
    int DisableEvent(const int& fd, const uint32_t& event);

    void TakeTimeout(std::set<void*>& timeout);

private:
    int epoller_fd_;

    std::multimap<uint64_t, void*> timeout_map_;

    std::map<int, uint32_t> fd_events_;
};

#endif // __EPOLLER_H__
