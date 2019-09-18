#include <unistd.h>

#include <vector>

#include "coroutine.h"
#include "epoller.h"
#include "io.h"

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

void EpollWait(std::vector<CoroutineContext*>& actives)
{
    std::vector<void*> tmp;
    get_epoller()->Wait(100, tmp);

    for (const auto& ctx : tmp)
    {
        actives.push_back((CoroutineContext*)ctx);
    }
}

void EventLoop()
{
    while (! event_loop_quit)
    {   
        std::vector<CoroutineContext*> active_ctx;
        EpollWait(active_ctx);

        for (auto& ctx : active_ctx)
        {   
            Resume(ctx);
        }   
    }
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
                Yield(get_cur_ctx());
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
                Yield(get_cur_ctx());
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

    return nbytes;
}
