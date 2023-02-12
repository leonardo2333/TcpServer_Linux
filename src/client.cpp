#include "../include/tcp_client.hpp"

int main()
{
    TcpClient MyTcpClient=TcpClient();
    MyTcpClient.connectserver("172.18.83.136",12345);
    MyTcpClient.registersend();
    MyTcpClient.recvmsg();

    void* res;
    pthread_join(MyTcpClient.getsendhandle(),&res);
    return *(int*)res;
}