#ifndef TCP_SERVER_H_
#define TCP_SERVER_H_

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
#include<vector>
#include<set>
#include<map>
#include<queue>
#include<iostream>

class TcpServer;
class ThreadRecv;
class ClientObject;

class TalkingRoom
{
private:
    u_int roomID;
    std::set<ClientObject*> members;
    pthread_mutex_t mtx;
public:
    TalkingRoom();
    TalkingRoom(u_int rmID);
    ~TalkingRoom();
    void setroomID(u_int rmID){roomID=rmID;};
    u_int getroomID(){return roomID;}
    u_int getmembercount(){return members.size();}
    void joinroom(ClientObject* client);
    void leaveroom(ClientObject* client);
    void sendtoall(ClientObject* co,MessageHead* msg);

};

class ClientObject
{
public:
    ClientObject():lastpos(0){m_cs=INVALID_SOCKET;};
    ClientObject(SOCKET s):m_cs(s),lastpos(0){};
    virtual ~ClientObject(){if(m_cs!=INVALID_SOCKET)close(m_cs);};
    void onmsgproc(ThreadRecv* pthread,MessageHead* msg);
    void sendmsg(MessageHead* msg);
    SOCKET getSocket(){return m_cs;};
    void setSocket(SOCKET s){m_cs=s;};
    char* getbuffer(){return buffer;};
    int getlastpos(){return lastpos;};
    void setlastpos(int pos){lastpos=pos;};

private:
    SOCKET m_cs;
    char buffer[PKG_MAX_SIZE*2];
    int lastpos;
};

class ThreadRecv
{
public:
    ThreadRecv();
    ~ThreadRecv();
    void start();
    int getmapsize()const{return mapclient.size();}
    void clientbufferemplace(ClientObject*);
    //线程函数作为成员函数必须为静态
    static void* threadprocess(void* lp);
    TcpServer* getserver(){return server;}
    void setserver(TcpServer* sr){server=sr;}
private:
    TcpServer* server;
    pthread_mutex_t mtx;
    std::vector<ClientObject*> vecclientbuffer;//临时客户端，多线程共享
    std::map<SOCKET,ClientObject*> mapclient;//真实客户端
    pthread_t pthd;
    fd_set fd_recv;
    bool clientchange;//是否有客户端接入
    int max_sock;
};

class TcpServer
{
public:
    TcpServer();
    virtual ~TcpServer();
    void bindandlisten(unsigned short port);
    void onlisten();
    virtual void onmsgproc(ClientObject* pclient,MessageHead* msg)=0;
    void startthread(int count);
    fd_set getsocks()const noexcept
    {
        return socks;
    }
    SOCKET getsocket()const noexcept
    {
        return ss;
    }
    static bool IsRun;
private:
    int threadcount;
    #ifdef _WIN32
    socketInit MySocketInit;
    #endif
    SOCKET ss;
    fd_set socks;
    ThreadRecv* pthreadrecv;
    std::vector<ClientObject*> clientvec;//所有客户端
};

class MyTcpServer:public TcpServer
{
public:
    MyTcpServer():TcpServer(){}
    std::map<u_int,TalkingRoom*> talkingrooms;
    void onmsgproc(ClientObject* pclient,MessageHead* msg)override;
    void showrooms(ClientObject* co);
    void joinroom(ClientObject* co,int roomID);
    void createroom(ClientObject* co);
    void talk(ClientObject* co,MsgTalk* msg);
};

class RoomIDManager
{
    static RoomIDManager* instance;
    RoomIDManager(){};
    std::queue<u_int> IDs;
    //单例模式，使用该类的静态成员变量和私有化构造函数
public:
    
    void initRoomIDManager()
    {
        for(int i=1;i<100;++i)
            IDs.emplace(i);
    }
    int getRoomID()
    {
        int res=IDs.front();
        IDs.pop();
        return res;
    }
    void pushRoomID(int rmID)
    {
        IDs.emplace(rmID);
    }
    //声明静态获取私有化的该类指针的函数，在该函数内调用构造函数
    static RoomIDManager* getInstance()
    {
        if(instance==nullptr)
        {
            instance=new RoomIDManager();
            instance->initRoomIDManager();
        }
        return instance;
    }
};

#endif