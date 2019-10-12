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

int Epoller::EnableRead(const int& fd, void* args)
{
    return EnableEvent(fd, EPOLLIN, args);
}

int Epoller::EnableWrite(const int& fd, void* args)
{
    return EnableEvent(fd, EPOLLOUT, args);
}

int Epoller::EnableAll(const int& fd, void* args)
{
    return EnableEvent(fd, EPOLLIN | EPOLLOUT, args);
}

int Epoller::DisableRead(const int& fd)
{
    return DisableEvent(fd, EPOLLIN);
}

int Epoller::DisableWrite(const int& fd)
{
    return DisableEvent(fd, EPOLLOUT);
}

int Epoller::DisableAll(const int& fd)
{
    return DisableEvent(fd, EPOLLIN | EPOLLOUT);
}

void Epoller::Wait(const int& ms, std::vector<void*>& active, std::vector<void*>& timeout)
{
    static epoll_event events[1024];
    int ret = epoll_wait(epoller_fd_, events, sizeof(events), ms);

    std::set<void*> timeout_set;
    TakeTimeout(timeout_set);

    if (ret > 0)
    {
        LogDebug << LOG_PREFIX << ret << " events happen" << std::endl;
        for (int i = 0; i != ret; ++i)
        {
            void* data = events[i].data.ptr;
            active.push_back(data);
            timeout_set.erase(data);
        }
    }
    else if (ret < 0)
    {
        LogDebug << PrintErr("epoll_wait", errno) << std::endl;
    }

    for (const auto& item : timeout_set)
    {
        timeout.push_back(item);
    }

#if defined(DEBUG)
    LogDebug << LOG_PREFIX << ret << " event happend, " << timeout.size() << " event timeout" << ", waitting=" << timeout_map_.size() << std::endl;
#endif
}

int Epoller::EnableEvent(const int& fd, const uint32_t& event, void* args)
{
    if (event== 0)
    {
        return 0;
    }

    uint32_t& fd_event = fd_events_[fd];

    if (fd_event & event == event)
    {
        LogDebug << LOG_PREFIX << "fd=" << fd << ", fd_event=" << fd_event << ", event=" << event << ", no changed" << std::endl;
        return 0;
    }

    int op = (fd_event != 0) ? EPOLL_CTL_MOD : EPOLL_CTL_ADD;
    fd_event |= event;

    LogDebug << LOG_PREFIX << "fd=" << fd << ", fd_event=" << fd_event << std::endl;

	struct epoll_event ep_ev;
    ep_ev.events = fd_event;
    ep_ev.data.ptr = (void*)args;

    int ret = epoll_ctl(epoller_fd_, op, fd, &ep_ev);
    if (ret < 0)
    {
        LogDebug << PrintErr("epoll_ctl", errno) << std::endl;
    }

    return ret;
}

int Epoller::DisableEvent(const int& fd, const uint32_t& event)
{
    if (event== 0)
    {
        return 0;
    }

    uint32_t& fd_event = fd_events_[fd];

    if (fd_event & event == 0)
    {
        LogDebug << LOG_PREFIX << "fd=" << fd << ", fd_event=" << fd_event << ", event=" << event << ", fd no such event" << std::endl;
        return 0;
    }

    fd_event &= ~event;
    int op = (fd_event != 0) ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;

	struct epoll_event ep_ev;
    ep_ev.events = fd_event;

    int ret = epoll_ctl(epoller_fd_, op, fd, &ep_ev);
    if (ret < 0)
    {
        LogDebug << PrintErr("epoll_ctl", errno) << std::endl;
    }

    if (fd_event == 0)
    {
        LogDebug << LOG_PREFIX << "fd=" << fd << ", no event now, delete it" << std::endl;
        fd_events_.erase(fd);
    }

    return ret;
}


void Epoller::TimeoutAt(void* args, const uint64_t& ms)
{
    timeout_map_.insert(std::make_pair(ms, args));
}

void Epoller::TimeoutAfter(void* args, const uint64_t& ms)
{
    TimeoutAt(args, GetNowInMillSecond() + ms);
}

void Epoller::TakeTimeout(std::set<void*>& timeout)
{
    uint64_t now_ms = GetNowInMillSecond();
    auto iter_end = timeout_map_.lower_bound(now_ms);

    auto iter = timeout_map_.begin();
    while (iter != iter_end)
    {
        timeout.insert(iter->second);
        iter = timeout_map_.erase(iter);
    }
}
