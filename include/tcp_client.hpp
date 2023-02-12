#ifndef TCP_CLIENT_H_
#define TCP_CLIENT_H_

#ifdef _WIN32
#include "../include/socketInit.hpp"
#elif __linux__
#include<unistd.h>
#include<arpa/inet.h>
#include<sys/socket.h>
// #include<sys/types.h>
#include<netinet/in.h>
#include<string.h>
#include<pthread.h>

#endif

#include "macros.hpp"
#include "message_type.hpp"
#include<iostream>

class TcpClient
{
public:
    TcpClient();
    virtual ~TcpClient();
    void connectserver(const char* ip,unsigned short port);
    void recvmsg();
    void sendmsg(MessageHead*);
    void registersend();
    SOCKET getsock()const noexcept
    {
        return sclient;
    }
    HANDLE getsendhandle()const noexcept
    {
        return sendmsghandle;
    }
    void onmsgproc(MessageHead* msg);
    int getroomID(){return roomID;}
    void setroomID(int rmID){roomID=rmID;}
    static void* sendmsgthread(void* lp);
private:
    #ifdef _WIN32
    socketInit MySocketInit;
    #endif
    SOCKET sclient;
    pthread_t sendmsghandle;
    int roomID;
};

#endif