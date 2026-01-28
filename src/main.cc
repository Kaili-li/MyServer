#include <iostream>

#include "server.h"


int main()
{
    
    if (!Server::GetInstance().Init()) {
        std::cerr << "Server Init failed " << std::endl;
        return -1;
    }
    Server::GetInstance().SetListenPort(8000);

    // Server::GetInstance().EnableIPv6();

    Server::GetInstance().Start();

    return 0;
}
