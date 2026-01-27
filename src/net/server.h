#pragma once

#include "socket_wrapper.hpp"

class Server
{
public:
    static Server& GetInstance()
    {
        static Server instance;
        return instance;
    }
private:
    Server();
    ~Server();

public:
    bool Init();
    void Start();
    void Release();

    void EnableIPv6();
    void EnableSSL();
    void SetListenPort(const short port);


private:
    bool Init6();
    void DoRead();

    bool InitSSL();
    void DoRead6();

private:
    bool ipv6_enabled_;
    bool ssl_enabled_;

    short listen_port_;

    SocketWrapper socket_;
    SocketWrapper socket6_;
};
