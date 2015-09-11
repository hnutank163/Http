#include "Socket.h"

namespace SOCKET_NAMESPACE
{

    bool Socket::StartUp()
    {
        bool ret = true;
#ifdef WIN32
        //add
#endif

        return ret;
    }

    bool Socket::CleanUp()
    {
        bool ret = true;
#ifdef WIN32
#endif

        return ret;
    }

    Socket_Handle Socket::open()
    {
        sock = socket(AF_INET, SOCK_STREAM, 0);
        return sock;
    }

    int Socket::close()
    {
        int ret = 0;
         if( sock != INVALID_SOCKET )
         {
#ifdef WIN32
             ret = closesocket(sock);
#else
             ret = ::close(sock);
#endif
             if( ret == 0 )
                 sock = INVALID_SOCKET;
         }
         return ret;
    }

    int Socket::setnonblocking()
    {
    int ret = 0;
#ifdef WIN32
#else
    ret = fcntl(sock, F_GETFD);
    if( ret == -1 )
        return ret;
    ret = fcntl(sock, F_SETFD, ret|O_NONBLOCK);
#endif
    return ret;
    }

    bool Socket::bind(int port) const
    {
         struct sockaddr_in addr;
         addr.sin_family = AF_INET;
         addr.sin_port = htons(port);
         addr.sin_addr.s_addr = htonl(INADDR_ANY);

         return ::bind(sock, (struct sockaddr *)&addr, sizeof(addr)) == 0;
    }

    bool Socket::listen(int backlog) const
    {
        return ::listen(sock, backlog) == 0;
    }

    void Socket::SetReadcb( void (*_readcb) (Socket_Handle , void *) )
    {
         this->readcb = _readcb;
    }

    void Socket::SetWritecb( void(*writecb) (Socket_Handle , void *))
    {
        this->writecb = writecb;
    }

}
