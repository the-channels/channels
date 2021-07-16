#include <iostream>
#ifdef WIN32
#include <WinSock2.h>
#endif
#include "socket_hub.h"

int main(int argc, const char** argv)
{
    int port = 9493;

#ifdef WIN32
    // excuse me windows what the fuck?
    int iResult;
    WSADATA wsaData;
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0)
    {
        std::cout << "WSAStartup failed: " << iResult << std::endl;
        return 1;
    }
#endif

    for (int i = 1; i < argc; i++)
    {
        std::string arg(argv[i]);

        if (arg == "-h")
        {
            std::cout << "Usage:" << std::endl;
            std::cout << " -v: verbose" << std::endl;
            std::cout << " -p port: change listen port" << std::endl;
            return 0;
        }
        else if (arg == "-v")
        {
            SocketChannelHub::SetVerbose();
        }
        else if (arg == "-p")
        {
            if (i + 1 < argc)
            {
                i++;
                port = atoi(argv[i]);
            }
        }
    }

    std::cout << "Starting hub" << port << std::endl;

    SocketChannelHub hub(port);
    return hub.run();
}
