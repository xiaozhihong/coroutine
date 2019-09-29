#include <sys/epoll.h>

#include <assert.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>

#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

#include "coroutine.h"
#include "epoller.h"
#include "io.h"
#include "log.h"
#include "socket_util.h"
#include "util.h"

using namespace std;

void TimeoutCoroutine(void* args);

void Schedule(void* args)
{
    //while (true)
    for (int i = 0; i != 1000; ++i)
    {
        CoroutineContext* ctx = CreateCoroutine("TimeoutCoroutine", TimeoutCoroutine, NULL);
        cout << LOG_PREFIX << "jump to TimeoutCoroutine" << endl;
        Resume(ctx);

        uint64_t start = GetNowInMillSecond();
        cout << LOG_PREFIX << "start=" << start << endl;
        SleepMs(10);
        Yield();
        cout << LOG_PREFIX << "elapse=" << (GetNowInMillSecond() - start) << endl;
    }
}

void TimeoutCoroutine(void* args)
{
    SleepMs(1000);
    uint64_t start = GetNowInMillSecond();
    cout << LOG_PREFIX << "start=" << start << endl;
    Yield();
    cout << LOG_PREFIX << "elapse=" << (GetNowInMillSecond() - start) << endl;
}

int main(int argc, char* argv[], char* env[])
{
    CoroutineContext* ctx = CreateCoroutine("Schedule", Schedule, NULL);
    Resume(ctx);

    EventLoop();

    return 0;
}
