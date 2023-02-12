#ifndef MSG_TYPE_H_
#define MSG_TYPE_H_

#include "macros.hpp"
enum MsgType{
    Msg_Show,
    Msg_Show_Reply,
    Msg_Join,
    Msg_Join_Reply,
    Msg_Create,
    Msg_Create_Reply,
    Msg_Talk,
    Msg_Exit
};

struct RoomInfo
{
    unsigned int roomID,onlineNum,totalNum;
};

class MessageHead
{
public:
    MsgType msg_type;//消息类型
    int msg_len;//消息长度
};

// #define MSG_CLASS(type) class Msg##type:public MessageHead     \                                                           
// {                                                              \  
// public:                                                        \      
//     Msg##type()                                                \              
//     {                                                          \      
//         msg_type=Msg_##type;                                   \                          
//         msg_len=sizeof(Msg##type);                             \                                  
//     }                                                          \      
// }

// MSG_CLASS(Show);
// MSG_CLASS(Create);
// MSG_CLASS(Exit);

class MsgShow:public MessageHead { public: MsgShow() { msg_type=Msg_Show; msg_len=sizeof(MsgShow); } };

class MsgCreate:public MessageHead { public: MsgCreate() { msg_type=Msg_Create; msg_len=sizeof(MsgCreate); } };

class MsgExit:public MessageHead 
{ 
public:
    int roomID;
    MsgExit() 
    { 
        msg_type=Msg_Exit; 
        msg_len=sizeof(MsgExit); 
    } 
    MsgExit(int rmID):roomID(rmID) 
    { 
        msg_type=Msg_Exit; 
        msg_len=sizeof(MsgExit); 
    }
};

class MsgJoin:public MessageHead 
{
public: 
    int roomID;
    MsgJoin(int roomid):roomID(roomid) 
    { 
        msg_type=Msg_Join; 
        msg_len=sizeof(MsgJoin); 
    } 
};

class MsgTalk:public MessageHead 
{ 
    char buf[1000];
public: 
    int roomID;
    int speakerID;
    MsgTalk()
    { 
        msg_type=Msg_Talk; 
        msg_len=sizeof(MsgTalk); 
    } 
    MsgTalk(int rmID,int spkID):roomID(rmID),speakerID(spkID)
    { 
        msg_type=Msg_Talk; 
        msg_len=sizeof(MsgTalk); 
    } 
    char* getbuffer()
    {
        return buf;
    }
};

class MsgShowReply:public MessageHead 
{ 
    
public: 
    int roomCount;
    RoomInfo rooms[ROOM_MAX_NUM];
    MsgShowReply()
    { 
        msg_type=Msg_Show_Reply; 
        msg_len=sizeof(MsgShowReply); 
    } 

};

class MsgJoinReply:public MessageHead 
{ 
    
public: 
    bool result;
    int roomID;
    MsgJoinReply():result(true)
    { 
        msg_type=Msg_Join_Reply; 
        msg_len=sizeof(MsgJoinReply); 
    } 

    MsgJoinReply(bool res):result(res)
    { 
        msg_type=Msg_Join_Reply; 
        msg_len=sizeof(MsgJoinReply); 
    } 
};

class MsgCreateReply:public MessageHead 
{ 
    
public: 
    bool result;
    int roomID;
    MsgCreateReply():result(true)
    { 
        msg_type=Msg_Create_Reply; 
        msg_len=sizeof(MsgCreateReply); 
    } 

    MsgCreateReply(bool res):result(res)
    { 
        msg_type=Msg_Create_Reply; 
        msg_len=sizeof(MsgCreateReply); 
    } 
};

#endif