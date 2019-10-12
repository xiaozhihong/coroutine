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

struct UserParam
{
    std::string ip;
    uint16_t port;
    std::string file;
};

void SendCoroutine(void* args)
{
    int fd = SocketUtil::CreateTcpSocket();
    SocketUtil::SetBlock(fd, 0);

    UserParam* param = (UserParam*)args;

    string file = param->file;
    string ip = param->ip;
    uint16_t port = param->port;

    int ret = Connect(fd, ip, port);

    bool error = false;
    uint64_t count = 0;
    while (! error)
    {
        ++count;
        uint64_t now_ms = GetNowInMillSecond();
        int file_fd = open(file.c_str(), O_RDONLY, 0664);

        if (file_fd < 0)
        {
            LogDebug << "open " << file << " failed." << endl;
            LogDebug << PrintErr("open", errno) << endl;
            break;
        }

        uint8_t prefix[1024];
        int prefix_len = snprintf((char*)prefix, sizeof(prefix), "count=%lu, now_ms=%lu\n", count, now_ms);
        if (WriteGivenSize(fd, prefix, prefix_len) <= 0)
        {
            LogDebug << "write prefix failed" << endl;
            break;
        }

        if (ReadGivenSize(fd, prefix, prefix_len) <= 0)
        {
            LogDebug << "read prefix " << prefix_len << " bytes failed" << endl;
            break;
        }

        while (true)
        {
            uint8_t buf[1024*64];

            ret = read(file_fd, buf, sizeof(buf));
            
            if (ret > 0)
            {
                if (WriteGivenSize(fd, buf, ret) <= 0)
                {
                    LogDebug << "write " << ret << " bytes failed" << endl;
                    error = true;
                    break;
                }

                if (ReadGivenSize(fd, buf, ret) <= 0)
                {
                    LogDebug << "read " << ret << " bytes failed" << endl;
                    error = true;
                    break;
                }
            }
            else
            {
                break;
            }
        }
        close(file_fd);
    }

    get_epoller()->DisableAll(fd);
}

int main(int argc, char* argv[], char* env[])
{
    signal(SIGPIPE, SIG_IGN);

    if (argc < 4)
    {
        LogDebug << "Usage ./main <ip> <port> <file>" << endl;
        return -1;
    }

    string ip = argv[1];
    uint16_t port = StrTo<uint16_t>(argv[2]);
    string file = argv[3];

    UserParam* param = new UserParam;
    param->ip = ip;
    param->port = port;
    param->file = file;
    CoroutineContext* ctx = CreateCoroutine("SendCoroutine", SendCoroutine, (void*)param);
    Resume(ctx);

    EventLoop();

    return 0;
}
