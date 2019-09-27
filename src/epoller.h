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

    int Add(const int& fd, const uint32_t& events, void* args);
    int Del(const int& fd);

    void TimeoutAt(void* args, const uint64_t& ms);
    void TimeoutAfter(void* args, const uint64_t& ms);

    void Wait(const int& ms, std::vector<void*>& active, std::vector<void*>& timeout);

private:
    int GetFd(const int& fd, uint32_t& events);
    void AddFd(const int& fd, const uint32_t& events);
    void ModFd(const int& fd, const uint32_t& events);
    void DelFd(const int& fd);

    void TakeTimeout(std::set<void*>& timeout);

private:
    int epoller_fd_;

    std::multimap<uint64_t, void*> timeout_map_;

    std::map<int, uint32_t> fd_events_;
};

#endif // __EPOLLER_H__
