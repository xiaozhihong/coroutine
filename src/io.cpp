#include <unistd.h>

#include <iostream>
#include <vector>

#include "coroutine.h"
#include "epoller.h"
#include "io.h"
#include "log.h"
#include "socket_util.h"

__thread bool event_loop_quit = false;
__thread Epoller* g_epoller = NULL;

Epoller* get_epoller()
{
    if (g_epoller == NULL)
    {
        g_epoller = new Epoller();
    }

    return g_epoller;
}

void EpollWait(std::vector<CoroutineContext*>& actives, std::vector<CoroutineContext*>& timeouts)
{
    std::vector<void*> tmp_actives;
    std::vector<void*> tmp_timeouts;
    get_epoller()->Wait(100, tmp_actives, tmp_timeouts);

    for (const auto& ctx : tmp_actives)
    {
        actives.push_back((CoroutineContext*)ctx);
    }

    for (const auto& ctx : tmp_timeouts)
    {
        timeouts.push_back((CoroutineContext*)ctx);
    }
}

void EventLoop()
{
    while (! event_loop_quit)
    {   
        std::vector<CoroutineContext*> active_ctx;
        std::vector<CoroutineContext*> timeout_ctx;
        EpollWait(active_ctx, timeout_ctx);

        for (auto& ctx : active_ctx)
        {   
            Resume(ctx);
        }   

        for (auto& ctx : timeout_ctx)
        {
            Resume(ctx);
        }
    }
}

int Connect(const int& fd, const std::string& ip, const uint16_t& port)
{
    int ret = SocketUtil::Connect(fd, ip, port);

    if (ret < 0)
    {
        if (errno == EINPROGRESS)
        {
            get_epoller()->Add(fd, EPOLLOUT, get_cur_ctx());
            LogDebug << LOG_PREFIX << " connect yield" << std::endl;
            Yield();
            get_epoller()->Add(fd, EPOLLIN, get_cur_ctx());
            LogDebug << LOG_PREFIX << " connect resume" << std::endl;
        }
    }

	int err = 0;
    ret = SocketUtil::GetError(fd, err);

    if (ret < 0 || err < 0)
    {   
        LogDebug << "connect " << ip << ":" << port << " failed" << std::endl;
        return -1;
    }   

    return 0;
}

int Read(const int& fd, uint8_t* data, const int& size)
{
    int ret = -1;
    while (true)
    {
        ret = read(fd, data, size);
        if (ret > 0)
        {
            break;
        }
        else if (ret == 0)
        {
            break;
        }
        else
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
            {
                LogDebug << LOG_PREFIX << " read yield" << std::endl;
                Yield();
                LogDebug << LOG_PREFIX << " read resume" << std::endl;
            }
            else
            {
                break;
            }
        }
    }

    return ret;
}

int Write(const int& fd, const uint8_t* data, const int& size)
{
    int ret = 0;

    while (true)
    {
        ret = write(fd, data, size);

        if (ret > 0)
        {
            break;
        }
        else
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
            {
                LogDebug << LOG_PREFIX << " write yield" << std::endl;
                Yield();
                LogDebug << LOG_PREFIX << " write resume" << std::endl;
            }
            else
            {
                break;
            }
        }
    }

    return ret;
}

int ReadGivenSize(const int& fd, uint8_t* data, const int& size)
{
    int nbytes = 0;
    while (nbytes != size)
    {
        int ret = Read(fd, data, size);

        if (ret > 0)
        {
            nbytes += ret;
        }
        else
        {
            return ret;
        }
    }

    return nbytes;
}

int WriteGivenSize(const int& fd, const uint8_t* data, const int& size)
{
    int nbytes = 0;
    get_epoller()->Add(fd, EPOLLOUT, get_cur_ctx());
    while (nbytes != size)
    {
        int ret = Write(fd, data, size);

        if (ret > 0)
        {
            nbytes += ret;
        }
        else
        {
            return ret;
        }
    }
    get_epoller()->Add(fd, EPOLLIN, get_cur_ctx());

    return nbytes;
}
