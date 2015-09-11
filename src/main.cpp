#include <iostream>
#include "Socket.h"
#include "Event.h"
using namespace SOCKET_NAMESPACE;
using namespace EVENT;
using std::cout;
using std::cerr;

void readcb(Socket_Handle fd, void *arg);

void readcb(Socket_Handle fd, void *arg)
{
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    int connfd  = accept(fd, (struct sockaddr *)&addr, &len);
    if(connfd > 0)
    {
        cout<<"recv connection\n";
        close(connfd);
    }
}

int
main()
{
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9999);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    int op = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const char *)&op, sizeof(int));
    if( 0 != bind(fd, (struct sockaddr *)&addr, sizeof(addr)) )
    {
        cerr<<"bind error\n";
        return 0;
    }

    if( 0 != listen(fd, 5) )
    {
        cerr<<"listen error\n";
        return 0;
    }

    Socket sock(fd);
    sock.SetReadcb(&readcb);
    Event event;
    event.EventAdd(sock, IO_READ);
    event.EventDispatch();

}
