#ifndef MACROS_H_
#define MACROS_H_

#ifdef _WIN32


#elif __linux__

#define SOCKET int
#define INVALID_SOCKET (-1)
#define HANDLE int
#define DWORD unsigned long
#define SOCKET_ERROR (-1)

#endif

#define MAX_MEMBER_NUM 50
#define PKG_MAX_SIZE 1024
#define ROOM_MAX_NUM 20

#endif