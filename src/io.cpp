#include "io.h"
#include "epoller.h"

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
