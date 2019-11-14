﻿// PHZ
// 2018-5-15

#include "rtsplib/server/net/TcpSocket.h"
#include "rtsplib/server/net/Socket.h"
#include "rtsplib/server/net/SocketUtil.h"
#include "rtsplib/server/net/Logger.h"

using namespace xop;

TcpSocket::TcpSocket(SOCKET sockfd)
    : _sockfd(sockfd)
{
#ifdef _WIN32
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 0), &wsaData);
#endif
}

TcpSocket::~TcpSocket()
{
#ifdef _WIN32
	WSACleanup();
#endif
}

SOCKET TcpSocket::create()
{
    _sockfd = socket(AF_INET, SOCK_STREAM, 0);
    return _sockfd;
}

bool TcpSocket::bind(std::string ip, uint16_t port)
{
    struct sockaddr_in addr = {0};			  
    addr.sin_family = AF_INET;		  
    addr.sin_addr.s_addr = inet_addr(ip.c_str()); 
    addr.sin_port = htons(port);  

    if(::bind(_sockfd, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
    {
        LOG_DEBUG(" <socket=%d> bind <%s:%u> failed.\n", _sockfd, ip.c_str(), port);
        return false;
    }

    return true;
}

bool TcpSocket::listen(int backlog)
{
    if(::listen(_sockfd, backlog) == SOCKET_ERROR)
    {
        LOG_DEBUG("<socket=%d> listen failed.\n", _sockfd);
        return false;
    }

    return true;
}

SOCKET TcpSocket::accept()
{
    struct sockaddr_in addr = {0};
    socklen_t addrlen = sizeof addr;

    SOCKET clientfd = ::accept(_sockfd, (struct sockaddr*)&addr, &addrlen);

    return clientfd;
}

bool TcpSocket::connect(std::string ip, uint16_t port, int timeout)
{ 
    if(!SocketUtil::connect(_sockfd, ip, port, timeout))
    {
        LOG_DEBUG("<socket=%d> connect failed.\n", _sockfd);
        return false;
    }

    return true;
}

void TcpSocket::close()
{
#if defined(__linux) || defined(__linux__) 
    ::close(_sockfd);
#elif defined(WIN32) || defined(_WIN32)
    closesocket(_sockfd);
#else
	
#endif
    _sockfd = 0;
}

void TcpSocket::shutdownWrite()
{
    shutdown(_sockfd, SHUT_WR);
    _sockfd = 0;
}
