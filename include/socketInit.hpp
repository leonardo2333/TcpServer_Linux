#ifndef SOCKETINIT_H_
#define SOCKETINIT_H_

#ifdef _WIN32
#include<windows.h>
#include<WinSock2.h>
#pragma comment(lib,"ws2_32.lib")
class socketInit
{
public:
    socketInit()
    {
        WORD ver=MAKEWORD(2,2);
        WSADATA dat;
        WSAStartup(ver,&dat);
    }

    ~socketInit()
    {
        WSACleanup();
    }
}
#endif
#endif