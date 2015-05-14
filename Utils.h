#ifndef ZL_NET_UTILS_H
#define ZL_NET_UTILS_H
#include <inttypes.h>
#include <stdint.h>
#include <arpa/inet.h>

typedef int SocketFd;
typedef int TimerFd;
typedef int EventFd;
typedef int SignalFd;

namespace zl
{
namespace net
{

// socket util
SocketFd  createSocket();
int       bind(SocketFd fd, const struct sockaddr_in& addr);
int       bind(SocketFd fd, const char *ip, short port);
int       listen(SocketFd fd, int backlog = 31);
SocketFd  createSocketAndListen(const char *ip, short port, int backlog = 31);
int       accept(SocketFd fd, struct sockaddr_in *addr);
int       connect(SocketFd fd, const struct sockaddr_in& addr);
int       connect(SocketFd fd, const char *ip, short port);

ssize_t   read(SocketFd fd, void *buf, size_t count);
ssize_t   readv(SocketFd fd, const struct iovec *iov, int iovcount);
ssize_t   write(SocketFd fd, const void *buf, size_t count);

void      closeSocket(SocketFd fd);
void      shutdownWrite(SocketFd fd);

int       setNonBlocking(SocketFd fd, bool nonBlocking = true);
int       setNoDelay(SocketFd fd, bool noDelay = true);
int       setReuseAddr(SocketFd fd, bool flag = true);

void      getSockAddr(const char* ip, uint16_t port, struct sockaddr_in* addr);
struct sockaddr_in getLocalAddr(SocketFd sockfd);
struct sockaddr_in getPeerAddr(SocketFd sockfd);
int       getSocketError(SocketFd sockfd);


// timer fd 

// signal fd

// event fd

}
}

#endif  /* ZL_NET_UTILS_H */
