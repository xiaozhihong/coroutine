#ifndef __EPOLLER_H__
#define __EPOLLER_H__

#include <vector>

class Epoller
{
public:
    Epoller();
    ~Epoller();

    int Add(const int& fd, const int& events, void* args);
    int Del(const int& fd);

    void Wait(const int& ms, std::vector<void*>& active);

private:
    int epoller_fd_;
};

#endif // __EPOLLER_H__
