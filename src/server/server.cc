#include <sys/socket.h>
#include <arpa/inet.h>
#include <iostream>

#include "server.h"
#include "event.h"
#include "http_session.h"



Server::Server() : ipv6_enabled_(false), ssl_enabled_(false), listen_port_(80), ssl_listen_port_(443)
{}

Server::~Server()
{
    socket_.Close();

    if (ipv6_enabled_) {
        v6_socket_.Close();
    }
}


bool Server::Init()
{
    struct sockaddr_in srv_addr{};
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    srv_addr.sin_port = htons(listen_port_);
    if (bind(socket_.GetSocket(), (struct sockaddr*)&srv_addr, sizeof(srv_addr)) != 0)
    {
        std::cerr << "[ERROR]: socket bind error! " << std::endl;
        return false;
    }
    std::cout << "[INFO]: IPv4 HTTP Server Running on " << listen_port_ << std::endl;

    if (listen(socket_.GetSocket(), SOMAXCONN) != 0)
    {
        std::cerr << "[ERROR] listen socket failed " << std::endl;
        return false;
    }


    if (ipv6_enabled_ && !Init6()) {
        return false;
    }


    StartEventLoop();

    return true;
}

void Server::Start()
{
    socket_.SetOnReadCallback([&](){ DoRead(); });
    socket_.StartRead();

    if (ipv6_enabled_)
    {
        v6_socket_.SetOnReadCallback([&](){ DoRead6();});
        v6_socket_.StartRead();
    }


#if BUILDFLAG(SSL)
    ssl_socket_.StartRead([this](){ DoRead(); });
#endif


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

void Server::SetSSLListenPort(const short port)
{
    ssl_listen_port_ = port;
}

void Server::DoRead()
{
    struct sockaddr_in c_adr{};
    socklen_t c_adr_len = sizeof(c_adr);

    Socket cfd = accept(socket_.GetSocket(), (struct sockaddr*)&c_adr, &c_adr_len);
    std::cout << "\n[LOG]: Http Request from: " << inet_ntoa(c_adr.sin_addr) << std::endl;

    auto http_session = new HttpSession(PosixSocket(cfd));
    http_session->Start();
}


bool Server::Init6()
{
    struct sockaddr_in6 srv_addr{};
    srv_addr.sin6_family = AF_INET6;
    srv_addr.sin6_port = htons(listen_port_);
    srv_addr.sin6_addr = in6addr_any;
    if (bind(v6_socket_.GetSocket(), (struct sockaddr*)&srv_addr, sizeof(srv_addr)) != 0)
    {
        std::cerr << "[ERROR]: " << __FUNCTION__ << ", socket bind error! " << std::endl;
        return false;
    }
    std::cout << "[INFO]: IPv6 HTTP Server Running on: " << listen_port_ << std::endl;

    if (listen(v6_socket_.GetSocket(), SOMAXCONN) != 0)
    {
        std::cerr << "[ERROR]: " << __FUNCTION__ << ", v6 socket listen failed " << std::endl;
        return false;
    }

    return true;
}


void Server::DoRead6()
{
    struct sockaddr_in6 c_addr{};
    socklen_t c_addr_len = sizeof(c_addr);
    Socket cfd = accept(v6_socket_.GetSocket(), (struct sockaddr*)&c_addr, &c_addr_len);

    char buf[64]{};
    inet_ntop(AF_INET6, &c_addr.sin6_addr, buf, 64);
    std::cout << "\n[INFO]: Recv HTTP Request from: " << buf << std::endl;

    auto v6_http_session = new HttpSession(PosixSocket(cfd));
    v6_http_session->Start();
}




