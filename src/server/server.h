#pragma once

#include "posix_socket.h"
#include "ssl_build_flag.h"
#include "ipv6_build_flag.h"

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
    void SetSSLListenPort(const short port);


private:
    bool Init6();
    void DoRead();

    bool InitSSL();
    void DoRead6();

private:
    bool ipv6_enabled_;
    bool ssl_enabled_;

    short listen_port_;
    short ssl_listen_port_;

    PosixSocket socket_;
    PosixSocket v6_socket_;
};
