#include "Utils.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/utsname.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <errno.h>
#include <poll.h>
#include <sys/uio.h>
namespace zl
{
namespace net
{

SocketFd  createSocket()
{
#if 1
  SocketFd fd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
#else
  SocketFd fd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
#endif
  if(fd <= 0)
  {
      perror("createSocket failure:");
  }
  return fd;
}

int       bind(SocketFd fd, const struct sockaddr_in& addr)
{
    int ret = ::bind(fd, (struct sockaddr *)&addr, static_cast<socklen_t>(sizeof(addr)));
    if(ret < 0)
    {
        perror("socket bind:");
    }
    return ret;
}

int       bind(SocketFd fd, const char *ip, short port)
{
    struct sockaddr_in  sockaddr;
    ::memset(&sockaddr, 0, sizeof(sockaddr));
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(port);
    int nIP = 0;
    if(!ip || '\0' == *ip || 0 == strcmp(ip, "0")
        || 0 == strcmp(ip, "0.0.0.0") || 0 == strcmp(ip, "*"))
    {
        nIP = htonl(INADDR_ANY);
    }
    else
    {
        nIP = inet_addr(ip);
    }
    sockaddr.sin_addr.s_addr = nIP;

    return bind(fd, sockaddr);
}

int       listen(SocketFd fd, int backlog/* = 31*/)
{
    int ret = ::listen(fd, backlog);        // SOMAXCONN
    if (ret < 0)
    {
        perror("socket listen:");
    }
    return ret;
}

SocketFd  createSocketAndListen(const char *ip, short port, int backlog/* = 31*/)
{
    SocketFd sockfd = createSocket();
    if(sockfd < 0) return sockfd;

    setNonBlocking(sockfd, true);

    bind(sockfd, ip, port);

    listen(sockfd, backlog);

    return sockfd;
}

int       accept(SocketFd fd, struct sockaddr_in *addr)
{
    int addrlen = sizeof(*addr);
    int connfd = ::accept(fd, (struct sockaddr *)addr, (socklen_t *)&addrlen);
    return connfd;
}

int       connect(SocketFd fd, const struct sockaddr_in& addr)
{
    return ::connect(fd, (struct sockaddr *)&addr, static_cast<socklen_t>(sizeof(addr)));
}

int       connect(SocketFd fd, const char *ip, short port)
{
    struct sockaddr_in addr;
    ::memset(&addr, 0, sizeof(addr));
    getSockAddr(ip, port, &addr);

    return connect(fd, addr);
}

ssize_t   read(SocketFd fd, void *buf, size_t count)
{
    return ::read(fd, buf, count);
}

ssize_t   readv(SocketFd fd, const struct iovec *iov, int iovcount)
{
    return ::readv(fd, iov, iovcount);
}

ssize_t   write(SocketFd fd, const void *buf, size_t count)
{
    return ::write(fd, buf, count);
}

void      closeSocket(SocketFd fd)
{
    if (::close(fd) < 0)
    {
        perror("closeSocket:");
    }
}

void      shutdownWrite(SocketFd fd)
{
    if (::shutdown(fd, SHUT_WR) < 0)
    {
        perror("shutdownWrite:");
    }
}

int       setNonBlocking(SocketFd fd, bool nonBlocking/* = true*/)
{
    int flags = ::fcntl(fd, F_GETFL);
    if(flags < 0)
        return flags;

    nonBlocking ? flags |= O_NONBLOCK : flags &= (~O_NONBLOCK);
    return ::fcntl(fd, F_SETFL, flags);
}

int       setNoDelay(SocketFd fd, bool noDelay/* = true*/)
{
    int optval = noDelay ? 1 : 0;
    return ::setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char *)&optval, sizeof(optval));
}

int       setReuseAddr(SocketFd fd, bool reuse/* = true*/)
{
    int optval = reuse ? 1 : 0;
    return ::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *)&optval, sizeof(optval));
}

void      getSockAddr(const char* ip, uint16_t port, struct sockaddr_in* addr)
{
    addr->sin_family = AF_INET;
    addr->sin_port = htons(port);
    if (::inet_pton(AF_INET, ip, &addr->sin_addr) <= 0)
    {
        perror("sockets getSockAddr: ");
    }
}

struct sockaddr_in getLocalAddr(SocketFd fd)
{
    struct sockaddr_in localaddr;
    ::memset(&localaddr, 0, sizeof(localaddr));
    socklen_t addrlen = static_cast<socklen_t>(sizeof(localaddr));
    //if (::getsockname(fd, static_cast<struct sockaddr *>(&localaddr), &addrlen) < 0)
    if (::getsockname(fd, (struct sockaddr *)(&localaddr), &addrlen) < 0)
    {
        perror("sockets::getLocalAddr: ");
    }
    return localaddr;
}

struct sockaddr_in getPeerAddr(SocketFd fd)
{
    struct sockaddr_in peeraddr;
    ::memset(&peeraddr, 0, sizeof(peeraddr));
    socklen_t addrlen = static_cast<socklen_t>(sizeof peeraddr);
    if (::getpeername(fd, (struct sockaddr *)(&peeraddr), &addrlen) < 0)
    {
        perror("sockets::getPeerAddr: ");
    }
    return peeraddr;
}

int       getSocketError(SocketFd fd)
{
    int optval;
    socklen_t optlen = static_cast<socklen_t>(sizeof(optval));
    if (::getsockopt(fd, SOL_SOCKET, SO_ERROR, (char*)&optval, &optlen) < 0)
    {
        return errno;
    }
    else
    {
        return optval;
    }
}

}
}