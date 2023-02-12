#include "tcp_server.hpp"

bool TcpServer::IsRun=true;
RoomIDManager* RoomIDManager::instance=nullptr;

#pragma region TcpServer

TcpServer::TcpServer()
    :threadcount(0)
    #ifdef _WIN32
    ,MySocketInit()
    #endif
{
    ss=socket(AF_INET,SOCK_STREAM,0);
    FD_ZERO(&socks);
}

TcpServer::~TcpServer()
{
    IsRun=false;
    close(ss);
    if(pthreadrecv)
    {
        delete[] pthreadrecv;
    }
    for(int i=0;i<clientvec.size();++i)
    {
        delete clientvec[i];
    }
}

void TcpServer::bindandlisten(unsigned short port)
{
    struct sockaddr_in addr;
    addr.sin_family=AF_INET;
    addr.sin_port=htons(port);
#ifdef _WIN32
    addr.sin_addr.s_un.s_addr=INADDR_ANY;    
#elif __linux__
    addr.sin_addr.s_addr=inet_addr("172.18.83.136");
#endif
    if(bind(ss,(sockaddr*)&addr,sizeof(sockaddr))==-1)
    {
        std::cout<<"error in binding address"<<std::endl;
        return;
    }
    if(listen(ss,10)==-1)
    {
        std::cout<<"error in listening"<<std::endl;
        return;
    } 
    FD_SET(ss,&socks);
}

void TcpServer::onlisten()
{ 
    fd_set tmp=socks;
    timeval tv={0,0};
    if(select(ss+1,&tmp,0,0,&tv)>0)
    {
        if(FD_ISSET(ss,&tmp))
        {
            sockaddr_in clientaddr;
            unsigned int adlen=sizeof(sockaddr_in);
            auto sclient=accept(ss,(sockaddr*)&clientaddr,&adlen);
            if(sclient==SOCKET_ERROR)
            {
                std::cout<<"error in rceiving socket"<<std::endl;
                return;
            }
            std::cout<<"connecting with client:["
                <<inet_ntoa(clientaddr.sin_addr)
                <<"]"<<std::endl;
            ClientObject* coptr=new ClientObject(sclient);
            clientvec.emplace_back(coptr);
            //分配给线程处理,查找客户端数量最少的线程
            ThreadRecv* bestthread=pthreadrecv;
            for(int j=1;j<threadcount;++j)
            {
                if(bestthread->getmapsize()>pthreadrecv[j].getmapsize())
                    bestthread=&pthreadrecv[j];
            }
            bestthread->clientbufferemplace(coptr);
        }
    }
}

void TcpServer::startthread(int count)
{
    threadcount=count;
    pthreadrecv=new ThreadRecv[count];
    for(int i=0;i<count;++i)
    {
        pthreadrecv[i].start();
        pthreadrecv[i].setserver(this);
    }
}

#pragma endregion TcpServer

#pragma region TalkingRoom

TalkingRoom::TalkingRoom()
{
    pthread_mutex_init(&mtx,NULL);
}

TalkingRoom::TalkingRoom(u_int rmID)
    :roomID(rmID)
{
    pthread_mutex_init(&mtx,NULL);
}

TalkingRoom::~TalkingRoom()
{
    pthread_mutex_destroy(&mtx);
}

void TalkingRoom::joinroom(ClientObject *client)
{
    pthread_mutex_lock(&mtx);
    if(members.size()<MAX_MEMBER_NUM)
        members.emplace(client);
    pthread_mutex_unlock(&mtx);
}

void TalkingRoom::leaveroom(ClientObject *client)
{
    pthread_mutex_lock(&mtx);
    auto it=members.find(client);
    if(it!=members.end())
        members.erase(it);
    pthread_mutex_unlock(&mtx);
}

void TalkingRoom::sendtoall(ClientObject *co, MessageHead *msg)
{
    for(auto it=members.begin();it!=members.end();++it)
    {
        if(*it!=co)
            (*it)->sendmsg(msg);
    }
}

#pragma endregion TalkingRoom

#pragma region ClientObject
void ClientObject::onmsgproc(ThreadRecv *pthread, MessageHead *msg)
{
    pthread->getserver()->onmsgproc(this,msg);
}

void ClientObject::sendmsg(MessageHead *msg)
{
    send(this->getSocket(),(const char*)msg,msg->msg_len,0);
}

#pragma endregion ClientObject

#pragma region ThreadRecv
ThreadRecv::ThreadRecv()
    :max_sock(0),
    vecclientbuffer()
{
    clientchange=true;
    pthread_mutex_init(&mtx,NULL);
}

ThreadRecv::~ThreadRecv()
{
    pthread_mutex_destroy(&mtx);
    void* res;
    if(pthread_join(pthd,&res))
        close(pthd);
}

void ThreadRecv::start()
{
    pthread_create(&pthd,NULL,threadprocess,(void*)this);
}

void ThreadRecv::clientbufferemplace(ClientObject *coptr)
{
    pthread_mutex_lock(&mtx);
    vecclientbuffer.emplace_back(coptr);
    pthread_mutex_unlock(&mtx);
}

void* ThreadRecv::threadprocess(void *lp)
{
    ThreadRecv* tr=(ThreadRecv*)lp;

    while(TcpServer::IsRun)
    {
        if(!tr->vecclientbuffer.empty())
        {
            pthread_mutex_lock(&tr->mtx);
            for(int s=tr->vecclientbuffer.size()-1;s>=0;--s)
            {
                tr->mapclient[tr->vecclientbuffer[s]->getSocket()]=tr->vecclientbuffer[s];
            }
            tr->vecclientbuffer.clear();
            tr->clientchange=true;
            pthread_mutex_unlock(&tr->mtx);
        }
        if(tr->clientchange)
        {
            auto s=tr->mapclient.size();
            auto it=tr->mapclient.begin();
            for(int i=0;i<s;++i,++it)
            {
                tr->max_sock=std::max(it->second->getSocket(),tr->max_sock);
                FD_SET(it->second->getSocket(),&tr->fd_recv);
            }
            tr->clientchange=false;
        }
        fd_set tmp=tr->fd_recv;
        timeval tv={1,0};
        if(select(tr->max_sock+1,&tmp,0,0,&tv)>0)
        {
            for(u_int i=0;i<FD_SETSIZE;++i)
            {
                if(FD_ISSET(i,&tmp))
                {
                    char buf[1024]={0};
                    auto iter=tr->mapclient.find(i);
                    if(iter!=tr->mapclient.end())
                    {
                        char* clientbuffer=iter->second->getbuffer(); 
                        int lastpos=iter->second->getlastpos();
                        int res=recv(i,clientbuffer+lastpos,PKG_MAX_SIZE-lastpos,0);
                        //std::cout<<res<<std::endl;//包大小
                        if(res>0)
                        {
                            MessageHead* phead=(MessageHead*)clientbuffer;
                            lastpos+=res;
                            //循环拆包
                            while(lastpos>=sizeof(MessageHead))
                            {
                                if(lastpos>=phead->msg_len)
                                {
                                    iter->second->onmsgproc(tr,phead);
                                    memcpy(clientbuffer,clientbuffer+phead->msg_len,lastpos-phead->msg_len);
                                    lastpos-=phead->msg_len;
                                }
                                else
                                {
                                    break;
                                }
                            }
                            iter->second->setlastpos(lastpos);
                            
                        }
                        else
                        {
                            std::cout<<"disconnected with client"<<i<<std::endl;
                            if(iter!=tr->mapclient.end())
                                tr->mapclient.erase(iter);
                            tr->clientchange=true;
                            break;
                        }
                    }
                }

            }
        }
    }
    return (void*)0;
}
#pragma endregion ThreadRecv

#pragma region MyTcpServer
void MyTcpServer::onmsgproc(ClientObject *pclient, MessageHead *msg)
{
    switch(msg->msg_type)
    {
        case Msg_Show:
        {
            showrooms(pclient);
        }
        break;
        case Msg_Join:
        {
            int roomID=((MsgJoin*)msg)->roomID;
            joinroom(pclient,roomID);
        }
        break;
        case Msg_Create:
        {
            createroom(pclient);
        }
        break;
        case Msg_Talk:
        {
            MsgTalk* msgtk=(MsgTalk*)msg;
            talk(pclient,msgtk);
            std::cout<<"[room"<<msgtk->roomID<<"][client"<<msgtk->speakerID<<"]:"
                <<((MsgTalk*)msg)->getbuffer()<<std::endl;
        }
            break;
        case Msg_Exit:
        {
            MsgExit* msgexit=(MsgExit*)msg;
            auto iter=talkingrooms.find(msgexit->roomID);
            if(iter!=talkingrooms.end())
                iter->second->leaveroom(pclient);
            if(iter->second->getmembercount()==0)
            {
                RoomIDManager::getInstance()->pushRoomID(iter->first);
                talkingrooms.erase(iter);
            }
                
        }
            break;
        // default:
        //     std::cout<<"invalid message type"<<std::endl;
        // break;
    }
}

void MyTcpServer::showrooms(ClientObject *co)
{
    MsgShowReply msgreply;
    msgreply.roomCount=talkingrooms.size();
    std::cout<<sizeof(MsgShowReply)<<std::endl;
    auto it=talkingrooms.begin();
    int i=0;
    while(it!=talkingrooms.end())
    {
        RoomInfo info={it->second->getroomID(),it->second->getmembercount(),MAX_MEMBER_NUM};
        msgreply.rooms[i++]=info;
        ++it;
    }
    co->sendmsg(&msgreply);
}

void MyTcpServer::joinroom(ClientObject *co, int roomID)
{
    MsgJoinReply msgreply;
    if(talkingrooms.count(roomID))
    {
        msgreply.result=true;
        msgreply.roomID=roomID;
        talkingrooms[roomID]->joinroom(co);
    }
    else
    {
        msgreply.result=false;
    }
    co->sendmsg((MessageHead*)&msgreply);
}

void MyTcpServer::createroom(ClientObject *co)
{
    RoomIDManager* rmIDManager=RoomIDManager::getInstance();
    //rmIDManager->initRoomIDManager();
    int roomID=rmIDManager->getRoomID();
    talkingrooms[roomID]=new TalkingRoom(roomID);
    talkingrooms[roomID]->joinroom(co);
    MsgCreateReply createreply=MsgCreateReply(true);
    createreply.roomID=roomID;
    co->sendmsg((MessageHead*)&createreply);
}

void MyTcpServer::talk(ClientObject *co, MsgTalk *msg)
{
    auto iter=talkingrooms.find(msg->roomID);
    if(iter!=talkingrooms.end())
        iter->second->sendtoall(co,(MessageHead*)msg);
}

#pragma endregion MyTcpServer
