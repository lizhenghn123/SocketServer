#include "FdEventsSelecter.h"
#include <stdio.h>
#include <assert.h>
#include <errno.h>

// 由于select能够管理的socket是有限且有最大上限的
// 这里可以直接在构造时初始化FD_SETSIZE个FdEvent的数组，每次加入或删除fd时更新数组对应位置的FdEvent
// 这样的话，就可以进一步优化fdlist_的实现，只需要一个mix_fd和一个max_fd即可
FdEventsSelecter::FdEventsSelecter()
{
    FD_ZERO(&readfds_);
    FD_ZERO(&writefds_);
    FD_ZERO(&exceptfds_);
    FD_ZERO(&select_readfds_);
    FD_ZERO(&select_writefds_);
    FD_ZERO(&select_exceptfds_);
}

FdEventsSelecter::~FdEventsSelecter()
{

}

FdEvent* FdEventsSelecter::addFdEvent(int fd, eventCallback cb)
{
    assert(fd);
    assert(!getChannel(fd));
    FdEvent *fde = FdEvent::newFdEvent(fd);
    fde->events_ = (FDEVENT_IN | FDEVENT_HUP | FDEVENT_ERR);
    fde->callback_ = cb;
    addFdEvent(fde);
    return fde;
}

int FdEventsSelecter::addFdEvent(FdEvent *fe)
{
    assert(fe);
    assert(!hasChannel(fe));
    return modFdEvent(fe);
}

int FdEventsSelecter::modFdEvent(FdEvent *fe)
{
    int fd = fe->fd();
    if (fd >= FD_SETSIZE)  // linux, iff windows, it should be fdlist_.size() < FD_SETSIZE (windos下socket未必连续,比较总数)
    {
        printf("FdEventsSelecter: socket fd[%d] >= FD_SETSIZE[%d]", fd, FD_SETSIZE);
        return -1;
    }
    printf("SelectPoller::updateChannel[%d]\n", fd);

    FD_SET(fd, &exceptfds_);

    int events = fe->events();
    if (hasChannel(fe))    //exist, update
    {
        assert(getChannel(fd) == fe);

        if (events & FDEVENT_IN)    FD_SET(fd, &select_readfds_);
        else                        FD_CLR(fd, &select_readfds_);

        if (events & FDEVENT_OUT)   FD_SET(fd, &select_writefds_);
        else                        FD_CLR(fd, &select_writefds_);

        if (fe->isNoneEvent())
        {
            printf("SelectPoller::updateChannel [%d][%p] NoneEvent\n", fd, fe);
            fdlist_.erase(fd);
        }
    }
    else                       //new, add
    {
        assert(getChannel(fd) == NULL);

        if (events & FDEVENT_IN)   FD_SET(fd, &select_readfds_);
        if (events & FDEVENT_OUT)  FD_SET(fd, &select_writefds_);

        channelMap_[fd] = fe;
        fdlist_.insert(fd);
    }
    return 0;
}

int FdEventsSelecter::delFdEvent(FdEvent *fe)
{
    if (!hasChannel(fe))
        return -1;

    int fd = fe->fd();
    printf("SelectPoller::delFdEvent [%d][%p]\n", fd, fe);
    assert(hasChannel(fe) && "the remove socket must be already exist");
    assert(getChannel(fd) == fe && "the remove socket must be already exist");
    assert(fe->isNoneEvent());
    size_t n = channelMap_.erase(fd);
    (void)(n);
    assert(n == 1);

    fdlist_.erase(fd);

    FD_CLR(fd, &select_readfds_);
    FD_CLR(fd, &select_writefds_);
    FD_CLR(fd, &select_exceptfds_);

    return 0;
}

int FdEventsSelecter::poll(std::vector<FdEvent *>& fdevents, int timeoutMs)
{
    readfds_ = select_readfds_;
    writefds_ = select_writefds_;
    exceptfds_ = select_exceptfds_;

    int numEvents = 0;
    if (timeoutMs < 0)
    {
        numEvents = ::select(*fdlist_.begin() + 1, &readfds_, &writefds_, &exceptfds_, NULL);
    }
    else
    {
        struct timeval tv = { 0, 0 };
        tv.tv_sec = timeoutMs / 1000;
        tv.tv_usec = (timeoutMs % 1000) * 1000;

        numEvents = ::select(*fdlist_.begin() + 1, &readfds_, &writefds_, &exceptfds_, &tv);
    }

    int savedErrno = errno;
    if (numEvents > 0)
    {
        printf("SelectPoller::poll_once: events happended[%d]\n", numEvents);
        fireActiveChannels(numEvents, fdevents);
    }
    else if (numEvents == 0)
    {
        printf("SelectPoller::poll_once: nothing happended\n");
    }
    else
    {
        if (savedErrno != EINTR)
        {
            errno = savedErrno;
            printf("SelectPoller::poll_once: error [%d]\n", errno);
        }
    }
    return numEvents;
}

void FdEventsSelecter::fireActiveChannels(int numEvents, ChannelList& fdevents) const
{
    for (FdList::iterator it = fdlist_.begin(); numEvents > 0 && it != fdlist_.end(); ++it)
    {
        int fd = *it;

        int revents = FDEVENT_NONE;
        if (FD_ISSET(fd, &readfds_))	 revents |= FDEVENT_IN;
        if (FD_ISSET(fd, &writefds_))    revents |= FDEVENT_OUT;
        if (FD_ISSET(fd, &exceptfds_))   revents |= FDEVENT_ERR;

        if (revents != FDEVENT_NONE)
        {
            --numEvents;
            FdEvent *fde = getChannel(fd);
            assert(fde && "the channel must be already exist");

            fde->set_revents(revents);
            fdevents.push_back(fde);
        }
    }
}
