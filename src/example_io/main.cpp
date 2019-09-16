#include <sys/epoll.h>

#include <signal.h>
#include <unistd.h>

#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

#include "coroutine.h"
#include "epoller.h"
#include "io.h"
#include "socket_util.h"

using namespace std;

void EchoRoutine(void* args)
{
    int fd = (uint64_t)args;
    while (true)
    {
        uint8_t buf[1024];
        int ret = read(fd, buf, sizeof(buf));

        if (ret > 0)
        {
            cout << "read " << ret << " bytes" << endl;

            write(fd, buf, ret);
        }
        else if (ret < 0)
        {
            if (errno == EAGAIN)
            {
                //cout << "-> yield" << endl;
                Yield(get_cur_ctx());
                //cout << "<- resume" << endl;
                continue;
            }

            break;
        }
        else if (ret == 0)
        {
            cout << "read EOF" << endl;
            break;
        }
    }
    
    get_epoller()->Del(fd);
    cout << "close fd=" << fd << endl;
    close(fd);
}

void AcceptRoutine(void* args)
{
    int server_fd = SocketUtil::CreateTcpSocket();
    cout << "server_fd=" << server_fd << endl;

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
                cout << "-> yield" << endl;
                Yield(get_cur_ctx());
                cout << "<- resume" << endl;
                continue;
            }
            else
            {
                return;
            }
        }

        cout << "accept " << client_ip << ":" << client_port << ", fd=" << ret << endl;
        CoroutineContext* echo_ctx = CreateCoroutine("EchoRoutine", EchoRoutine, (void*)ret);
        SocketUtil::SetBlock(ret, 0);
        get_epoller()->Add(ret, EPOLLIN | EPOLLOUT, echo_ctx);
    }
}


int main(int argc, char* argv[], char* env[])
{
    signal(SIGPIPE, SIG_IGN);

    CoroutineContext* accept_ctx = CreateCoroutine("AcceptRoutine", AcceptRoutine, NULL);
    Resume(accept_ctx);

    while (true)
    {
        vector<CoroutineContext*> active_ctx;
        EpollWait(active_ctx);

        if (active_ctx.empty())
        {
            //cout << "timeout, no events" << endl;
        }

        for (auto& ctx : active_ctx)
        {
            Resume(ctx);
        }
    }

    return 0;
}
