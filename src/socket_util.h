#ifndef __SOCKET_UTIL_H__
#define __SOCKET_UTIL_H__

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <string>

class SocketUtil
{
public:
    static int CreateSocket(const int& type);
    static int CreateTcpSocket();
    static int IpPortToSocketAddr(const std::string& ip, const uint16_t& port, sockaddr_in& addr);
    static int SocketAddrToIpPort(const sockaddr_in& addr, std::string& ip, uint16_t& port);
    static int Bind(const int& fd, const std::string& ip, const uint16_t& port);
    static int Listen(const int& fd);
    static int Connect(const int& fd, const std::string& ip, const uint16_t& port);
    static int Accept(const int& fd, std::string& ip, uint16_t& port);
    static int SetBlock(const int& fd, const bool& block);
    static int ReuseAddr(const int& fd);
    static int GetError(const int& fd, int& err);
    static int IpNetToStr(const uint32_t& net, std::string& str);
    static int IpHostToStr(const uint32_t& host, std::string& str);
    static int IpStrToNet(const std::string& str, uint32_t& net);
    static int IpStrToHost(const std::string& str, uint32_t& host);
    static int GetIpStrByHost(const std::string& host, std::string& ip);
};

#endif // __SOCKET_UTIL_H__
