#include <sys/epoll.h>

#include <unistd.h>

#include <iostream>

#include "epoller.h"
#include "util.h"

Epoller::Epoller()
{
    epoller_fd_ = epoll_create(64);
}

Epoller::~Epoller()
{
    close(epoller_fd_);
}

int Epoller::Add(const int& fd, const int& events, void* args)
{
	struct epoll_event event;
    event.events = events;
    event.data.ptr = (void*)args;

    int ret = epoll_ctl(epoller_fd_, EPOLL_CTL_ADD, fd, &event);
    if (ret < 0)
    {
        std::cout << PrintErr("epoll_ctl", errno) << std::endl;
    }

    return ret;
}

int Epoller::Del(const int& fd)
{
	struct epoll_event event;

    int ret = epoll_ctl(epoller_fd_, EPOLL_CTL_DEL, fd, &event);
    if (ret < 0)
    {
        std::cout << PrintErr("epoll_ctl", errno) << std::endl;
    }

    return ret;
}

void Epoller::Wait(const int& ms, std::vector<void*>& active)
{
    static epoll_event events[1024];
    int event_num = epoll_wait(epoller_fd_, events, sizeof(events), ms);

    for (int i = 0; i != event_num; ++i)
    {
        active.push_back(events[i].data.ptr);
    }
}
