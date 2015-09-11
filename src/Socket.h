#ifndef _SOCKET_H
#define _SOCKET_H

#ifdef WIN32
typedef Socket Socket_Handle;
#else

#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <fcntl.h>

#include <string>
#include <vector>
#include <iostream>
using std::cout;
typedef int Socket_Handle;
#define INVALID_SOCKET -1
#endif

namespace SOCKET_NAMESPACE
{

    class Socket
    {
    public:
        void (*readcb ) (Socket_Handle fd, void *arg);
        void (*writecb ) (Socket_Handle fd, void *arg);
        friend class EVent;
    public:
        Socket_Handle sock;
        bool StartUp();  //initial windows socket env
        bool CleanUp();  //clean socket env
    //constructor
        Socket()
        {
            cout<<"socket default\n";
            readcb = NULL;
            writecb = NULL;
            sock = INVALID_SOCKET;
        }
        Socket(Socket_Handle handle)
        {
            cout<<"socket para\n";
            readcb = NULL;
            writecb = NULL;
            sock = handle;
        }
        Socket(const Socket & socket): readcb(socket.readcb), writecb(socket.writecb),sock(socket.sock){}

    //open close
        Socket_Handle open();
        int close();
        int setnonblocking();
    //bind listen
        bool bind(int port) const;
        bool listen(int backlog) const;

    //callback fucntion set
        void SetReadcb( void(*)(Socket_Handle , void *) );
        void SetWritecb( void(*)(Socket_Handle , void *)  );
    };
}
#endif
