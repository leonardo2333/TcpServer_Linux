#include "tcp_client.hpp"

TcpClient::TcpClient()
    :roomID(0)
    #ifdef _WIN32
    ,MySocketInit()
    #endif
{
    //建立socket
    sclient=socket(
        AF_INET,//协议族类型(AF_INET:IPv4,AF_INET6:IPv6,...)
        SOCK_STREAM,//socket类型(SOCK_STREAM:流式面向连接(TCP),SOCK_DGRAM:数据报(UDP),...)
        IPPROTO_TCP//协议类型(IPPROTO_TCP:TCP,IPPROTO_UDP:UDP,...)
    );
    if(sclient==SOCKET_ERROR)
    {
        std::cout<<"error in creating socket"<<std::endl;
        return;
    }

    std::cout<<"connecting with server..."<<std::endl;

}

TcpClient::~TcpClient()
{
    close(sclient);
    close(sendmsghandle);
}

void TcpClient::connectserver(const char *ip, unsigned short port)
{
    sockaddr_in sockad_in;
    sockad_in.sin_family=AF_INET;
    sockad_in.sin_port=htons(port);
    sockad_in.sin_addr.s_addr=inet_addr(ip);

    if(connect(sclient,(const sockaddr*)&sockad_in,sizeof(sockad_in))==SOCKET_ERROR)
    {
        std::cout<<"error in connecting server"<<std::endl;
        close(sclient);
        return;
    };
}

void TcpClient::recvmsg()
{
    fd_set socks;
    FD_ZERO(&socks);        
    FD_SET(sclient,&socks);
    char buf[1024]={};
    while(true)
    {
        fd_set tmp=socks;
        if(select(sclient+1,&socks,0,0,0)>0)
        {
            if(recv(getsock(),buf,1024,0)>0) 
            {
                onmsgproc((MessageHead*)buf);
            }
            else
            {
                std::cout<<"disconnected with server"<<std::endl;
                break;
            }
        }
    }
}

void TcpClient::sendmsg(MessageHead *msghead)
{
    send(getsock(),(const char*)msghead,msghead->msg_len,0);
}

void TcpClient::registersend()
{
    pthread_create(&sendmsghandle,0,sendmsgthread,(void*)this);
}

void TcpClient::onmsgproc(MessageHead *msg)
{
    switch (msg->msg_type)
    {
        case Msg_Show_Reply:
        {    
            MsgShowReply* msgreply=(MsgShowReply*)msg;
            if(msgreply->roomCount==0)
            {
                std::cout<<"there is no existing room"<<std::endl;
                return;
            }
            std::cout<<"=========================================="<<std::endl;
            std::cout<<"                 Room Info                "<<std::endl;
            std::cout<<"=========================================="<<std::endl;
            for(int i=0;i<msgreply->roomCount;++i)
            {
                std::cout<<"RoomID:"<<msgreply->rooms[i].roomID<<"\t|"
                    <<"CurMember:"<<msgreply->rooms[i].onlineNum<<"/"<<msgreply->rooms[i].totalNum<<std::endl;
                if(i==msgreply->roomCount-1)
                    break;
                std::cout<<"------------------------------------------"<<std::endl;
            }
            std::cout<<"=========================================="<<std::endl;
        }
        break;
        case Msg_Create_Reply:
        {
            MsgCreateReply* msgreply=(MsgCreateReply*)msg;
            if(msgreply->result)
            {
                roomID=msgreply->roomID;
                std::cout<<"Succeed in creating talkingroom "<<roomID<<std::endl;
            }
            else
            {
                std::cout<<"Failed to create talkingroom "<<roomID<<std::endl;
            }
            
        }
        break;
        case Msg_Join_Reply:
        {
            MsgJoinReply* msgreply=(MsgJoinReply*)msg;
            if(msgreply->result)
            {
                roomID=msgreply->roomID;
                std::cout<<"Succeed in joining talkingroom "<<roomID<<std::endl;
            }
            else
            {
                std::cout<<"Failed to join talkingroom "<<msgreply->roomID<<std::endl;
            }
            
        }
        break;
        case Msg_Talk:
        {
            MsgTalk* msgtk=(MsgTalk*)msg;
            std::cout<<"[room"<<msgtk->roomID<<"][client"<<msgtk->speakerID<<"]:"
                <<((MsgTalk*)msg)->getbuffer()<<std::endl;
        }
            break;
        default:
            std::cout<<"error in parsing message"<<std::endl;
            break;
    }
}

void *TcpClient::sendmsgthread(void *lp)
{
    TcpClient* MyTcpClient=(TcpClient*)lp;
    SOCKET sclient=MyTcpClient->getsock();
    std::cout<<"show\tjoin\tcreate\ttalk\texit"<<std::endl;
    char buf[1000];
    while(true)
    {  
        std::cin.getline(buf,1000);
        if(strcmp(buf,"show")==0)
        {
            MsgShow msg;
            MyTcpClient->sendmsg(&msg);
        }
        else if(strncmp(buf,"join",4)==0)
        {
            MsgJoin msg(atoi(buf+5));
            MyTcpClient->sendmsg(&msg);
        }
        else if(strcmp(buf,"create")==0)
        {
            MsgCreate msg;
            MyTcpClient->sendmsg(&msg);
        }
        else if(strcmp(buf,"talk")==0)
        {
            while(true)
            {
                std::cout<<"[self]:";
                MsgTalk msg(MyTcpClient->getroomID(),MyTcpClient->getsock());
                std::cin>>msg.getbuffer();
                if(strcmp(msg.getbuffer(),"esc")==0)
                {
                    break;
                }
                MyTcpClient->sendmsg((MessageHead*)&msg);
            }
            MsgExit exitmsg(MyTcpClient->getroomID());
            MyTcpClient->sendmsg(&exitmsg);
            MyTcpClient->setroomID(0);
        }
        else if(strcmp(buf,"exit")==0)
        {
            break;
        }    
    }
    return (void*)0;
}
