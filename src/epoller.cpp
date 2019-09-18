#include <sys/epoll.h>

#include <unistd.h>

#include <iostream>

#include "epoller.h"
#include "log.h"
#include "util.h"

Epoller::Epoller()
{
    epoller_fd_ = epoll_create(64);
}

Epoller::~Epoller()
{
    close(epoller_fd_);
}

int Epoller::Add(const int& fd, const uint32_t& events, void* args)
{
    uint32_t ev = 0;
    int fd_exist = GetFd(fd, ev);
    if (fd_exist > 0 && ev == events)
    {
        std::cout << LOG_PREFIX << "fd=" << fd << ",events=" << events << " no change, ignore" << std::endl;
        return 0;
    }

	struct epoll_event event;
    event.events = events;
    event.data.ptr = (void*)args;

    int ret = epoll_ctl(epoller_fd_, EPOLL_CTL_ADD, fd, &event);
    if (ret < 0)
    {
        std::cout << PrintErr("epoll_ctl", errno) << std::endl;
    }
    else
    {
        if (fd_exist)
        {
            ModFd(fd, events);
        }
        else
        {
            AddFd(fd, events);
        }
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
    else
    {
        DelFd(fd);
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

int Epoller::GetFd(const int& fd, uint32_t& events)
{
    auto iter = fd_events_.find(fd);

    if (iter == fd_events_.end())
    {
        return 0;
    }

    events = iter->second;

    return 1;
}

void Epoller::AddFd(const int& fd, const uint32_t& events)
{
    std::cout << LOG_PREFIX << "fd=" << fd << ",events=" << events << std::endl;
    fd_events_[fd] = events;
}

void Epoller::ModFd(const int& fd, const uint32_t& events)
{
    std::cout << LOG_PREFIX << "fd=" << fd << ",events=" << events << std::endl;
    fd_events_[fd] = events;
}

void Epoller::DelFd(const int& fd)
{
    std::cout << LOG_PREFIX << "fd=" << fd << std::endl;
    fd_events_.erase(fd);
}
