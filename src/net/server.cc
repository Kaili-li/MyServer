#if defined(_WIN32) || defined(_WIN64)  // Windows

#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

#else   // Linux or macOS

#include <sys/socket.h>
#include <arpa/inet.h>

#endif

#include "server.h"
#include "logging.h"
#include "event_wrapper.hpp"
#include "http_session.h"


Server::Server() : ipv6_enabled_(false), ssl_enabled_(false), listen_port_(80)
{}

Server::~Server()
{
    socket_.Close();

    if (ipv6_enabled_) {
        socket6_.Close();
    }
}


bool Server::Init()
{
#if defined(_WIN32) || defined(_WIN64)

    WSADATA wsa_data;
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0)
    {
        LOG(1) << "[ERROR]: WSAStart failed" << std::endl;
        return false;
    }

#endif

    return ::Init();
}

void Server::Start()
{

    socket_.ToNonBlockMode();

    struct sockaddr_in srv_addr{};
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    srv_addr.sin_port = htons(listen_port_);
    if (bind(socket_.GetSocket(), (struct sockaddr*)&srv_addr, sizeof(srv_addr)) != 0)
    {
        LOG(1) << "[ERROR]: socket bind error! " << std::endl;
        return;
    }
    std::cout << "[INFO]: IPv4 HTTP Server Running on " << listen_port_ << std::endl;

    if (listen(socket_.GetSocket(), SOMAXCONN) != 0)
    {
        LOG(1) << "[ERROR] listen socket failed " << std::endl;
        return;
    }


    // if (ipv6_enabled_ && !Init6()) {
    //     return;
    // }

    socket_.SetOnReadCallback([&](){ DoRead(); });
    EventStart(socket_.GetSocket(), kReadEvent, [&]{ DoRead();} );

    // if (ipv6_enabled_)
    // {
    //     socket6_.SetOnReadCallback([&](){ DoRead6();});
    //     socket6_.StartRead();
    // }
}

void Server::Release()
{
    socket_.Close();
}

void Server::EnableIPv6()
{
    ipv6_enabled_ = true;
}

void Server::EnableSSL()
{
    ssl_enabled_ = true;
}

void Server::SetListenPort(const short port)
{
    listen_port_ = port;
}


void Server::DoRead()
{
    struct sockaddr_in c_adr{};
    socklen_t c_adr_len = sizeof(c_adr);

    SOCKET cfd = accept(socket_.GetSocket(), (struct sockaddr*)&c_adr, &c_adr_len);
    LOG(1) << "[INFO]: Http Request from: " << inet_ntoa(c_adr.sin_addr) << std::endl;

    auto http_session = new HttpSession(cfd);
    http_session->Start();
}


bool Server::Init6()
{
    struct sockaddr_in6 srv_addr{};
    srv_addr.sin6_family = AF_INET6;
    srv_addr.sin6_port = htons(listen_port_);
    srv_addr.sin6_addr = in6addr_any;
    if (bind(socket6_.GetSocket(), (struct sockaddr*)&srv_addr, sizeof(srv_addr)) != 0)
    {
        LOG(4) << "[ERROR]: " << __FUNCTION__ << ", socket bind error! " << std::endl;
        return false;
    }
    std::cout << "[INFO]: IPv6 HTTP Server Running on: " << listen_port_ << std::endl;

    if (listen(socket6_.GetSocket(), SOMAXCONN) != 0)
    {
        LOG(1) << "[ERROR]: " << __FUNCTION__ << ", v6 socket listen failed " << std::endl;
        return false;
    }

    return true;
}


void Server::DoRead6()
{
    struct sockaddr_in6 c_addr{};
    socklen_t c_addr_len = sizeof(c_addr);
    SOCKET cfd = accept(socket6_.GetSocket(), (struct sockaddr*)&c_addr, &c_addr_len);

    char buf[64]{};
    inet_ntop(AF_INET6, &c_addr.sin6_addr, buf, 64);
    LOG(1) << "[INFO]: Recv HTTP Request from: " << buf << std::endl;

    auto v6_http_session = new HttpSession(cfd);
    v6_http_session->Start();
}




