#include "FdEventsDevpoller.h"
#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <unistd.h>
#include <sys/epoll.h>

FdEventsEpoller::FdEventsDevpoller()
{
    devpollfd_ = ::open("/dev/poll", O_RDWR);
    assert(devpollfd_ > 0 && " FdEventsDevpoller create failure!");
    fcntl(devpollfd_, F_SETFD, FD_CLOEXEC)

        events_.resize(64);
}

FdEventsDevpoller::~FdEventsDevpoller()
{
    ::close(devpollfd_);
}

FdEvent* FdEventsDevpoller::addFdEvent(int fd, eventCallback cb)
{
    assert(fd);
    assert(!getChannel(fd));
    //FdEvent *fe = new FdEvent(fd);
    FdEvent *fde = FdEvent::newFdEvent(fd);
    fde->events_ = (FDEVENT_IN | FDEVENT_HUP | FDEVENT_ERR);
    fde->callback_ = cb;
    addFdEvent(fde);
    return fde;
}

int FdEventsDevpoller::addFdEvent(FdEvent *fe)
{
    assert(fe);
    assert(!hasChannel(fe));
    return modFdEvent(fe);
}

int FdEventsDevpoller::modFdEvent(FdEvent *fe)
{
    assert(fe);
    int fd = fe->fd();
    printf("FdEventsDevpoller::updateChannel[%d]\n", fd);
    if (hasChannel(fe))    //exist, update
    {
        assert(getChannel(fd) == fe);
        if (fe->isNoneEvent())
        {
            printf("EpollPoller::updateChannel [%d][%p] NoneEvent\n", fd, fe);
            channelMap_.erase(fd);
            return update(fe, EPOLL_CTL_DEL);
        }
        else
        {
            return update(fe, EPOLL_CTL_MOD);
        }
    }
    else                       //new, add
    {
        assert(getChannel(fd) == NULL);
        channelMap_[fd] = fe;
        return update(fe, EPOLL_CTL_ADD);
    }
}

int FdEventsDevpoller::delFdEvent(FdEvent *fe)
{
    assert(fe);
    if (!hasChannel(fe))   // 注意 updateChannel 函数中也有一处removeChannel的逻辑
        return 0;

    int fd = fe->fd();
    printf("EpollPoller::removeChannel [%d][%p]\n", fd, fe);
    assert(hasChannel(fe) && "the remove socket must be already exist");
    assert(getChannel(fd) == fe && "the remove socket must be already exist");
    assert(fe->isNoneEvent());
    size_t n = channelMap_.erase(fd);
    //ZL_UNUSED(n);
    assert(n == 1);

    return update(fe, POLLREMOVE);
}

bool FdEventsDevpoller::update(FdEvent *fde, int operation)
{
    struct pollfd pfd;
    pfd.fd = fde->fd();
    pfd.revents = 0;
    ev.data.ptr = fe;

    int events = fde->events();
    if (!events) // none event
    {
        assert(operation == POLLREMOVE);
        pfd.events = operation;
    }
    else
    {
        if (events & FDEVENT_IN)  pfd.events |= EPOLLIN;
        if (events & FDEVENT_OUT) pfd.events |= EPOLLOUT;

        events_.push_back(pfd);
    }

    if (write(devpollfd_, pfd, sizeof(pfd) == -1)
    {
        printf("FdEventsDevpoller::update Fail[%d]\n", fde->fd());
        return false;
    }

    return true;
}

int FdEventsDevpoller::poll(std::vector<FdEvent *>& fdevents, int timeoutMs)
{
    struct dvpoll dopoll;
    dopoll.dp_timeout = timeoutMs;
    dopoll.dp_nfds = events_.size();
    dopoll.dp_fds = &*events_.begin();

    int numEvents = ::ioctl(devpollfd_, DP_POLL, &dopoll); // 调用ioctl()阻塞等待

    int savedErrno = errno;
    if (numEvents > 0)
    {
        printf("FdEventsDevpoller::poll: [%d] events happended\n", numEvents);
        fireActiveChannels(numEvents, fdevents);
        if (static_cast<size_t>(numEvents) == events_.size())
        {
            events_.resize(events_.size() * 2);
        }
    }
    else if (numEvents == 0)
    {
        printf("FdEventsDevpoller::poll: nothing happended\n");
        return 0;
    }
    else  // error happens
    {
        // TODO : should return -1 if EINTR, else return 0
        if (savedErrno != EINTR)
        {
            errno = savedErrno;
            printf("FdEventsDevpoller::poll: error [%d]\n", savedErrno);
            return -1;
        }
        return 0;
    }

    return numEvents;
}

void FdEventsDevpoller::fireActiveChannels(int numEvents, ChannelList& activeChannels) const
{
    assert(static_cast<size_t>(numEvents) <= events_.size());

    for (int i = 0; i < numEvents; ++i)
    {
        FdEvent *fde = static_cast<FdEvent*>(events_[i].data.ptr);
        assert(hasChannel(fde) && "the channel must be already exist");

        int revents = FDEVENT_NONE;
        if (events_[i].revents & POLLIN)    revents |= FDEVENT_IN;
        if (events_[i].revents & POLLPRI)   revents |= FDEVENT_PRI;
        if (events_[i].revents & POLLOUT)   revents |= FDEVENT_OUT;
        if (events_[i].revents & POLLERR)   revents |= FDEVENT_ERR;
        if (events_[i].revents & POLLHUP)   revents |= FDEVENT_HUP;
        if (events_[i].revents & POLLNVAL)  revents |= FDEVENT_NVAL;
        fde->set_revents(revents);

        activeChannels.push_back(fde);
    }
}
