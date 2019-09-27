#include <sys/epoll.h>

#include <assert.h>
#include <signal.h>
#include <unistd.h>

#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

#include "coroutine.h"
#include "epoller.h"
#include "log.h"
#include "io.h"
#include "socket_util.h"

using namespace std;

void EchoRoutine(void* args)
{
    int fd = (uint64_t)args;
    bool error = false;

    get_epoller()->Add(fd, EPOLLIN, get_cur_ctx());

    while (! error)
    {
        uint8_t buf[1024];

        int ret = Read(fd, buf, sizeof(buf));

        LogDebug << "read " << ret << endl;

        if (ret > 0)
        {
            ret = WriteGivenSize(fd, buf, ret);

            if (ret < 0)
            {
                break;
            }
        }
        else if (ret < 0)
        {
            break;
        }
        else if (ret == 0)
        {
            LogDebug << "read EOF" << endl;
            break;
        }
    }
    
    get_epoller()->Del(fd);
    LogDebug << "close fd=" << fd << endl;
    close(fd);
}

void AcceptRoutine(void* args)
{
    int server_fd = SocketUtil::CreateTcpSocket();
    LogDebug << "server_fd=" << server_fd << endl;

    SocketUtil::SetBlock(server_fd, 0);
    SocketUtil::ReuseAddr(server_fd);
    SocketUtil::Bind(server_fd, "0.0.0.0", 8788);
    SocketUtil::Listen(server_fd);

    get_epoller()->Add(server_fd, EPOLLIN, get_cur_ctx());

    string client_ip = "";
    uint16_t client_port = 0;

    while (true)
    {
        int ret = SocketUtil::Accept(server_fd, client_ip, client_port);
        if (ret < 0)
        {
            if (errno == EAGAIN)
            {
                LogDebug << "-> accept yield" << endl;
                Yield(get_cur_ctx());
                LogDebug << "<- accept resume" << endl;
                continue;
            }
            else
            {
                LogDebug << "accept error" << endl;
                break;
            }
        }

        LogDebug << "accept " << client_ip << ":" << client_port << ", fd=" << ret << endl;
        CoroutineContext* echo_ctx = CreateCoroutine("EchoRoutine", EchoRoutine, (void*)ret);
        SocketUtil::SetBlock(ret, 0);
        Resume(echo_ctx);
    }
}

int main(int argc, char* argv[], char* env[])
{
    signal(SIGPIPE, SIG_IGN);

    CoroutineContext* accept_ctx = CreateCoroutine("AcceptRoutine", AcceptRoutine, NULL);
    Resume(accept_ctx);

    EventLoop();

    return 0;
}
