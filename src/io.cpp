#include <netinet/in.h>
#include <arpa/nameser.h>
#include <resolv.h>

#include <unistd.h>

#include <iostream>
#include <vector>

#include "coroutine.h"
#include "epoller.h"
#include "io.h"
#include "log.h"
#include "socket_util.h"
#include "util.h"

__thread bool event_loop_quit = false;
__thread Epoller* g_epoller = NULL;
__thread res_state g_thread_res_state = NULL;

Epoller* get_epoller()
{
    if (g_epoller == NULL)
    {
        g_epoller = new Epoller();
    }

    return g_epoller;
}

res_state get_thread_res_state()
{
    if (g_thread_res_state == NULL)
    {   
        g_thread_res_state = (res_state)malloc(sizeof(struct __res_state));
    }   

    if (g_thread_res_state->options & RES_INIT)
    {   
        res_ninit(g_thread_res_state);
    }   

    return g_thread_res_state;
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
            get_epoller()->EnableWrite(fd, get_cur_ctx());
            LogDebug << LOG_PREFIX << "cid=" << get_cid() << " connect yield" << std::endl;
            Yield();
            get_epoller()->DisableWrite(fd);
            get_epoller()->EnableRead(fd, get_cur_ctx());
            LogDebug << LOG_PREFIX << "cid=" << get_cid() << " connect resume" << std::endl;
        }
    }

	int err = 0;
    ret = SocketUtil::GetError(fd, err);

    if (ret < 0 || err < 0)
    {   
        LogDebug << LOG_PREFIX << "connect " << ip << ":" << port << " failed" << std::endl;
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
                LogDebug << LOG_PREFIX << " read yield, cid=" << get_cid() << std::endl;
                get_epoller()->EnableRead(fd, get_cur_ctx());
                Yield();
                LogDebug << LOG_PREFIX << " read resume, cid=" << get_cid() << std::endl;
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
                get_epoller()->EnableWrite(fd, get_cur_ctx());
                Yield();
                get_epoller()->DisableWrite(fd);
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

int GetHostByName(const std::string& host, std::string& ip)
{
    int fd = SocketUtil::CreateUdpSocket();
    if (fd < 0)
    {
        return -1;
    }

    res_state rs = get_thread_res_state();
    int ret = res_ninit(rs);

    if (ret != 0)
    {
        LogErr << PrintErr("res_ninit", ret) << std::endl;
        return -1;
    }

    uint8_t dns_query_buf[1460];
    ret = res_nmkquery(rs, QUERY, host.c_str(), C_IN, T_A, NULL, 0, NULL, dns_query_buf, sizeof(dns_query_buf));

    if (ret < 0)
    {
        LogErr << PrintErr("res_nmkquery", ret) << std::endl;
        return -1;
    }

    //LogDebug << LOG_PREFIX << "request bin=" << BinToHex(dns_query_buf, ret) << std::endl;

    HEADER* h = (HEADER*)dns_query_buf;
    LogDebug << LOG_PREFIX << "res_nmkquery ret=" << ret 
             << ",id=" << h->id
             << ",nscount=" << rs->nscount
             << std::endl;

    for (int i = 0; i < rs->nscount; ++i)
    {
        sendto(fd, dns_query_buf, ret, 0, (struct sockaddr*)&(rs->nsaddr_list[i]), sizeof(struct sockaddr));
        get_epoller()->EnableRead(fd, get_cur_ctx());

        LogDebug << LOG_PREFIX << "cid=" << get_cid() << " query dns yield" << std::endl;
        Yield();
        LogDebug << LOG_PREFIX << "cid=" << get_cid() << " query dns resume" << std::endl;

        sockaddr_in in_addr;
        socklen_t in_addr_len = sizeof(in_addr);
        uint8_t dns_answer_buf[1460];
        ret = recvfrom(fd, dns_answer_buf, sizeof(dns_answer_buf), 0, (struct sockaddr*)&in_addr, &in_addr_len);

        if (ret < 0)
        {
            LogDebug << LOG_PREFIX << PrintErr("recvfrom", ret) << std::endl;
            return -1;
        }

        //LogDebug << LOG_PREFIX << "response bin=" << BinToHex(dns_answer_buf, ret) << std::endl;

        std::string dns_ip;
        uint16_t dns_port;
        SocketUtil::SocketAddrToIpPort(in_addr, dns_ip, dns_port);

        h = (HEADER*)dns_answer_buf;

        int qdcount = ntohs(h->qdcount);
        int ancount = ntohs(h->ancount);

        LogDebug << LOG_PREFIX << "recvfrom ret=" << ret 
                 << ",dns addr=" << dns_ip << ":" << dns_port
                 << ",id=" << h->id
                 << ",qdcount=" << qdcount
                 << ",ancount=" << ancount
                 << std::endl;

        uint8_t* dns_end = dns_answer_buf + ret;
        uint8_t* content = dns_answer_buf + sizeof(HEADER);

        while (qdcount > 0)
        {
            --qdcount;
            content += dn_skipname(content, dns_end) + QFIXEDSZ;
        }

        while (ancount > 0 && content < dns_end)
        {
            char tmp[1460];
            ret = dn_expand(dns_answer_buf, dns_end, content, tmp, sizeof(tmp));

            LogDebug << LOG_PREFIX << "dn_expand ret=" << ret << std::endl;

            if (ret < 0)
            {
                break;
            }
            --ancount;

            content += ret;
            uint16_t type = be16toh(*(uint16_t*)content);
            content += 8;
            ret = be16toh(*(uint16_t*)content);
            content += 2;

            LogDebug << LOG_PREFIX << "type=" << type << ",ret=" << ret << std::endl;

            if (type == T_CNAME)
            {
                content += ret;
                continue;
            }

            LogDebug << LOG_PREFIX << "COPY to in_addr" << std::endl;
            struct in_addr in;
            memcpy(&in, content, ret);

            SocketUtil::InAddrToIp(in, ip);

            LogDebug << LOG_PREFIX << "host=" << host << ",ip=" << ip << std::endl;

            content += ret;

            get_epoller()->DisableAll(fd);
			return 0;
        }
    }

    get_epoller()->DisableAll(fd);
	return -1;
}

void SleepMs(const int& ms)
{
    get_epoller()->TimeoutAfter(get_cur_ctx(), ms);
}
