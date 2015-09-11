#ifndef _EVENT_H
#define _EVENT_H

#include <vector>
#include "Socket.h"
namespace EVENT
{
#define IO_READ 1
#define IO_WRITE 2
    using SOCKET_NAMESPACE::Socket;
    using std::vector;
    class EventOp
    {
    public:
        virtual int EventRegister(Socket_Handle fd, int types)
        {
            return 0;
        }

        virtual int EventLook(struct timeval , vector<Socket_Handle> &rsets, vector<Socket_Handle> &wser){return 0;}
        virtual ~EventOp(){}
    };

    class SelectOp:public EventOp
    {
    private:
        int nfds;
        fd_set rfds;
        fd_set wfds;
        fd_set efds;
        struct timeval timeout;
    public:
        SelectOp()
        {
            FD_ZERO(&rfds);
            FD_ZERO(&wfds);
        }
        ~SelectOp(){}
        virtual int EventRegister(Socket_Handle fd, int type);
        virtual int EventLook(struct timeval , vector<Socket_Handle> &rsets,
                vector<Socket_Handle> &wset);
    };

    class EpollOp:public EventOp
    {
    private:
        int epfd;
        struct epoll_event events[1024];
    public:
        EpollOp();
        ~EpollOp();
        virtual int EventRegister(Socket_Handle fd, int types);
        virtual int EventLook(struct timeval timeouts, vector<Socket_Handle> &rset,
                vector<Socket_Handle> &wset);
    };

    class Event
    {
    private:
        vector<Socket> lists;
        vector<int> rset;
        vector<int> wset;
        EventOp *evop;
    public:
        Event();
        ~Event();
        int EventAdd(Socket &sock, int types);
        int EventDispatch();
    };
}

#endif
