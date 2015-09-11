#include "Event.h"
#include <iostream>
using namespace EVENT;
#include <algorithm>
using std::max;
int SelectOp::EventRegister(Socket_Handle fd, int types)
{
    if( types & IO_READ )
    {
        FD_SET(fd, &this->rfds);
        nfds = max(nfds, fd);
    }
    if( types & IO_WRITE )
    {
        FD_SET(fd, &this->wfds);
        nfds = max(nfds, fd);
    }
    return 0;
}

EpollOp::EpollOp()
{
    this->epfd = epoll_create1(EPOLL_CLOEXEC);
}

EpollOp::~EpollOp()
{

}

int EpollOp::EventRegister(Socket_Handle fd, int types)
{
    int ret;
    struct epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLET;
    if( types & IO_READ )
        event.events = EPOLLIN | event.events;
    if( types & IO_WRITE )
        event.events = EPOLLOUT | event.events;

    ret = epoll_ctl(this->epfd, EPOLL_CTL_ADD, fd, &event);
    if( -1 == ret )
        std::cout<<"epoll_ctl error\n";
    return ret;
}

Event::Event()
{
     this->evop = new SelectOp();
}

Event::~Event()
{
    delete this->evop;
}

int Event::EventAdd(Socket &sock, int types)
{
     lists.push_back(sock);
     this->evop->EventRegister(sock.sock, types);

     return 0;
}

int Event::EventDispatch()
{
    std::cout<<"enter EventDispatch\n";
    for(;;)
    {
        struct timeval timeout={1,0};
        this->evop->EventLook(timeout, this->rset, this->wset);
        std::cout<<"end look\n";
        vector<int>::iterator it = rset.begin();
        for(; it!=rset.end(); it++)
        {
            if( *it == -8 )
                break;
            for(vector<Socket>::iterator iter=lists.begin(); iter!=lists.end(); ++iter)
            {
                if( (*iter).sock == *it )
                {
                    std::cout<<"read "<<std::endl;
                    (*iter).readcb(*it, NULL);
                }
            }
        }
    }
}

int SelectOp::EventLook(struct timeval timeouts, vector<Socket_Handle>&rset,
        vector<Socket_Handle >&wset)
{
    fd_set _rfds = this->rfds;
    fd_set _wfds = this->wfds;
    fd_set _efds = this->efds;
    int ret = select(nfds+1,&_rfds, &_wfds, &_efds,&timeouts );
    vector<int>::iterator it_r = rset.begin();
    vector<int>::iterator it_w = wset.begin();
    if( ret > 0 )
    {
        for(int i=3; i<nfds+1; i++)
        {
            if( FD_ISSET(i, &_rfds) )
            {
                std::cout<<"read fd "<<i<<std::endl;
                rset.insert(it_r++, i);
            }
            if( FD_ISSET(i, &_wfds) )
            {
                wset.insert(it_w++, i);
            }
        }

    }
    rset.insert(it_r, -8);
    wset.insert(it_w, -8);

    return 0;
}

int EpollOp::EventLook(struct timeval timeouts, vector<Socket_Handle> &rset,
        vector<Socket_Handle> &wset)
{
    int ms = timeouts.tv_sec*1000 + timeouts.tv_usec*1000;
    int ri=0, wi=0;
    int n = epoll_wait(epfd, events,1024,  ms);
    if(n > 0)
    {
        for(int i=0; i<n; i++)
        {
            if(events[i].events & EPOLLIN)
            {
                ri++;
                rset.push_back(events[i].data.fd);
            }
            if(events[i].events & EPOLLOUT)
            {
                wi++;
                wset.push_back(events[i].data.fd);
            }
        }
    }

    vector<int>::iterator it = rset.begin();
    rset.insert(it+ri, -8);
    it = wset.begin();
    wset.insert(it+wi, -8);

    return 0;
}
