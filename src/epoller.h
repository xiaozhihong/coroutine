#ifndef __EPOLLER_H__
#define __EPOLLER_H__

#include <map>
#include <vector>

class Epoller
{
public:
    Epoller();
    ~Epoller();

    int Add(const int& fd, const uint32_t& events, void* args);
    int Del(const int& fd);

    void Wait(const int& ms, std::vector<void*>& active);

private:
    int GetFd(const int& fd, uint32_t& events);
    void AddFd(const int& fd, const uint32_t& events);
    void ModFd(const int& fd, const uint32_t& events);
    void DelFd(const int& fd);

private:
    int epoller_fd_;

    std::map<int, uint32_t> fd_events_;
};

#endif // __EPOLLER_H__
