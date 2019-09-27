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
        LogDebug << LOG_PREFIX << "fd=" << fd << ",events=" << events << " no change, ignore" << std::endl;
        return 0;
    }

	struct epoll_event event;
    event.events = events;
    event.data.ptr = (void*)args;

    int op = EPOLL_CTL_ADD;
    if (fd_exist)
    {
        op = EPOLL_CTL_MOD;
    }

    int ret = epoll_ctl(epoller_fd_, op, fd, &event);
    if (ret < 0)
    {
        LogDebug << PrintErr("epoll_ctl", errno) << std::endl;
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
        LogDebug << PrintErr("epoll_ctl", errno) << std::endl;
    }
    else
    {
        DelFd(fd);
    }

    return ret;
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
    LogDebug << LOG_PREFIX << "fd=" << fd << ",events=" << events << std::endl;
    fd_events_[fd] = events;
}

void Epoller::ModFd(const int& fd, const uint32_t& events)
{
    LogDebug << LOG_PREFIX << "fd=" << fd << ",events=" << events << std::endl;
    fd_events_[fd] = events;
}

void Epoller::DelFd(const int& fd)
{
    LogDebug << LOG_PREFIX << "fd=" << fd << std::endl;
    fd_events_.erase(fd);
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
