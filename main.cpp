#include <iostream>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include "FdEvents.h"
#include "Utils.h"
using namespace std;
using namespace zl;
using namespace zl::net;

// 参考lighttpd fdevent 

void print_events_str(const FdEvent *fe)
{
    int revents = fe->revents();
    if (revents & FDEVENT_IN)
        printf("FDEVENT_IN | ");
    if (revents & FDEVENT_PRI)
        printf("FDEVENT_PRI | ");
    if (revents & FDEVENT_OUT)
        printf("FDEVENT_OUT | ");
    if (revents & FDEVENT_HUP)
        printf("FDEVENT_HUP | ");
    if (revents & FDEVENT_ERR)
        printf("FDEVENT_ERR | ");
    printf("\n");
}

int socket_read(FdEvents *poller, FdEvent *fe)
{
    char buf[1024] = { 0 };
    int fd = fe->fd();
    print_events_str(fe);

    // fix : loop read ( while() )
    ssize_t size = zl::net::read(fd, buf, sizeof(buf));
    if (size == 0)   // the peer already close this connection
    {
        fe->disableAll();   // must disable all
        poller->delFdEvent(fe);
        zl::net::closeSocket(fd);
    }
    else
    {
        printf("server recv[%d] msg[%s]\n", fd, buf);

        zl::net::write(fd, buf, size);
    }
    return 0;
}

int socket_write(FdEvents *poller, FdEvent *fe)
{
    return 0;
}

int socket_accept(FdEvents *poller, FdEvent *fe)
{
    int srvfd = fe->fd();
    int max_accept = 0;
    while (max_accept < 100)
    {
        struct sockaddr_in addr;
        int clientfd = zl::net::accept(srvfd, &addr);
        if (clientfd > 0)
        {
            printf("get one client [%d]\n", clientfd);
            zl::net::setNonBlocking(clientfd);

            poller->addFdEvent(clientfd, socket_read);

            const char *welcome = "hello world, welcome to connecting server\n";
            zl::net::write(clientfd, welcome, strlen(welcome));
        }
        else
        {
            switch (errno)
            {
            case EAGAIN:
            if EWOULDBLOCK != EAGAIN
            case EWOULDBLOCK:
            #endif
            case EINTR:
                break;
            case EMFILE:  // out of fds
                break;
            default:
                perror("socket_accept : ");
                break;
            }
            break;
        }
    }

    return 0;
}

void test_server()
{
    int srvsock = createSocketAndListen("127.0.0.1", 8888, 1023);
    assert(srvsock > 0);

    FdEvents *poller = createFdEvents();
    assert(poller);
    cout << poller->getName() << "\n";

    zl::net::setNonBlocking(srvsock);
    poller->addFdEvent(srvsock, socket_accept);

    std::vector<FdEvent *> fdevents;
    while (1)
    {
        fdevents.clear();
        int nfds = poller->poll(fdevents, 10000); // 10000 ms
        if (nfds == 0)
        {
            continue;
        }
        else if (nfds == -1)
        {
            break;
        }

        assert(nfds == static_cast<int>(fdevents.size()));
        for (int i = 0; i < nfds; i++)
        {
            FdEvent *fde = fdevents[i];
            fde->callback_(poller, fde);
        }
    }

    delete poller;
}

int main()
{
    test_server();

    return 0;
}
