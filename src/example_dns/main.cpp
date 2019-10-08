#include <sys/epoll.h>

#include <netinet/in.h>
#include <arpa/nameser.h>
#include <resolv.h>

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

struct UserParam
{
    std::string domain;
};

void DnsQuery(void* args)
{
    UserParam* param = (UserParam*)args;

    const std::string& domain = param->domain;

    string ip;
    int ret = GetHostByName(domain, ip);

    LogDebug << "GetHostByName ret=" << ret << endl;
}

int main(int argc, char* argv[], char* env[])
{
    signal(SIGPIPE, SIG_IGN);

    if (argc < 2)
    {
        LogDebug << "Usage ./main <dns>" << endl;
        return -1;
    }

    int count = 1;
    if (argc == 3)
    {
        count = StrTo<int>(argv[2]);
    }

    LogDebug << "count=" << count << endl;

    string domain = argv[1];

    UserParam* param = new UserParam;
    param->domain = domain;

    for (int i = 0; i != count; ++i)
    {
        CoroutineContext* ctx = CreateCoroutine("DnsQuery", DnsQuery, (void*)param);
        Resume(ctx);
    }

    EventLoop();

    return 0;
}
