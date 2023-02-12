#include "../include/tcp_server.hpp"


int main()
{
    MyTcpServer TS=MyTcpServer();
    TS.bindandlisten(12345);
    TS.startthread(1);
    while (true)
    {
        TS.onlisten();
    }
    
    return 0;

}