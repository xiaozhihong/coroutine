#include "log.h"
#include "socket_util.h"
#include "util.h"

#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

using namespace std;

int SocketUtil::CreateSocket(const int& type)
{
    int ret = socket(AF_INET, type, 0);

    if (ret < 0)
    {
        LogErr << PrintErr("socket", ret) << endl;
    }
    
    return ret;
}

int SocketUtil::CreateTcpSocket()
{
    return CreateSocket(SOCK_STREAM);
}

int SocketUtil::IpPortToSocketAddr(const string& ip, const uint16_t& port, sockaddr_in& addr)
{
    int ret = 0;

    bzero(&addr, sizeof(addr));

    uint32_t net = 0;
    ret = IpStrToNet(ip, net);

    if (ret < 0)
    {
        return ret;
    }

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = net;
    addr.sin_port = htons(port);

    return 0;
}

int SocketUtil::SocketAddrToIpPort(const sockaddr_in& addr, string& ip, uint16_t& port)
{
    uint32_t net = addr.sin_addr.s_addr;
    int ret = IpNetToStr(net, ip);

    if (ret < 0)
    {
        return -1;
    }

    port = ntohs(addr.sin_port);

    return 0;
}

int SocketUtil::Bind(const int& fd, const string& ip, const uint16_t& port)
{
    sockaddr_in addr;
    int ret = IpPortToSocketAddr(ip, port, addr);

    if (ret < 0)
    {
        return ret;
    }

    ret = bind(fd, (sockaddr*)&addr, sizeof(addr));

    if (ret < 0)
    {
        LogErr << PrintErr("bind", ret) << endl;
    }

    return ret;
}

int SocketUtil::Listen(const int& fd)
{
    int ret = listen(fd, 64);

    if (ret < 0)
    {
        LogErr << PrintErr("listen", ret) << endl;
    }

    return ret;
}

int SocketUtil::Connect(const int& fd, const string& ip, const uint16_t& port)
{
    sockaddr_in addr;
    int ret = IpPortToSocketAddr(ip, port, addr);

    if (ret < 0)
    {
        return ret;
    }

    ret = connect(fd, (sockaddr*)&addr, sizeof(addr));

    if (ret < 0)
    {
        LogErr << PrintErr("connect", ret) << endl;
    }

    return ret;
}

int SocketUtil::Accept(const int& fd, string& ip, uint16_t& port)
{ 
    sockaddr_in addr;
    bzero(&addr, sizeof(addr));
    socklen_t addr_len = sizeof(addr);
    int ret = accept(fd, (sockaddr*)&addr, &addr_len);

    if (ret < 0)
    {
        LogErr << PrintErr("accpet", ret) << endl;
        return -1;
    }

    SocketAddrToIpPort(addr, ip, port);

    return ret;
}

int SocketUtil::SetBlock(const int& fd, const bool& block)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0)
    {
        LogErr << PrintErr("fcntl", flags) << endl;
        return flags;
    }

    if (block)
    {
        if (flags & O_NONBLOCK == 0)
        {
            return 0;
        }

        flags &= (~O_NONBLOCK);
    }
    else
    {
        if (flags & O_NONBLOCK)
        {
            return 0;
        }

        flags |= O_NONBLOCK;
    }

    int ret = fcntl(fd, F_SETFL, flags);
    if (ret < 0)
    {
        LogErr << PrintErr("fcntl", ret) << endl;
    }

    return ret;
}

int SocketUtil::ReuseAddr(const int& fd)
{
    int op = 1;

    int ret = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &op, sizeof(op));

    if (ret < 0)
    {
        LogErr << PrintErr("setsockopt", ret) << endl;
    }

    return ret;
}

int SocketUtil::GetError(const int& fd, int& err)
{
    socklen_t err_len = sizeof(err);
    int ret = getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &err_len);

    if (ret < 0)
    {
        LogErr << PrintErr("getsockopt", ret) << endl;
    }

    return ret;
}

int SocketUtil::IpNetToStr(const uint32_t& net, string& str)
{
    in_addr in = {0};
    in.s_addr = net;

    char out_buf[INET6_ADDRSTRLEN] = {0};

    const char* pc = inet_ntop(AF_INET, &in, out_buf, sizeof(out_buf));

    if (pc == NULL)
    {
        LogErr << PrintErr("inet_ntop", -1) << endl;
        return -1;
    }

    str.assign(pc);

    return 0;
}

int SocketUtil::IpHostToStr(const uint32_t& host, string& str)
{
    return IpNetToStr(be32toh(host), str);
}

int SocketUtil::IpStrToNet(const string& str, uint32_t& net)
{
    in_addr out = {0};

    int ret = inet_pton(AF_INET, str.c_str(), &out);

    if (ret <= 0)
    {
        LogErr << PrintErr("inet_pton", ret) << endl;
        return -1;
    }

    net = out.s_addr;

    return 0;
}

int SocketUtil::IpStrToHost(const string& str, uint32_t& host)
{
    uint32_t net = 0;
    int ret = IpStrToNet(str, net);

    host = be32toh(net);

    return ret;
}

int SocketUtil::GetIpStrByHost(const std::string& host, std::string& ip)
{
    addrinfo ai; 
    memset(&ai, 0, sizeof(addrinfo));

    addrinfo* result = NULL;

    ai.ai_family = AF_INET;
    ai.ai_socktype = SOCK_DGRAM;

    int ret = getaddrinfo(host.c_str(), NULL, &ai, &result);
    if (ret != 0)
    {   
        LogErr << PrintErr("getaddrinfo", ret) << endl;
    }   
    else
    {   
        for (addrinfo* rp = result; rp != NULL; rp = rp->ai_next) 
        {   
            sockaddr_in* p = (sockaddr_in*)rp->ai_addr;
            ret = IpNetToStr(p->sin_addr.s_addr, ip);

            if (ret == 0)
            {
                break;
            }
        }   

        freeaddrinfo(result);
    }   

    return ret;
}
