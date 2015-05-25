#include "FdEventsKqueuer.h"
#include <sys/event.h>

FdEventsKqueuer::FdEventsKqueuer()
{
    kqueuefd_ = ::kqueue();
    assert(kqueuefd_ > 0 && " kqueue create failure!");

    events_.resize(64);
}

FdEventsKqueuer::~FdEventsKqueuer()
{
    ::close(kqueuefd_);
}

FdEvent* FdEventsKqueuer::addFdEvent(int fd, eventCallback cb)
{
    assert(fd);
    assert(!getChannel(fd));
    FdEvent *fde = FdEvent::newFdEvent(fd);
    fde->events_ = (FDEVENT_IN | FDEVENT_HUP | FDEVENT_ERR);
    fde->callback_ = cb;
    addFdEvent(fde);
    return fde;
}

int FdEventsKqueuer::addFdEvent(FdEvent *fe)
{
    assert(fe);
    assert(!hasChannel(fe));
    return modFdEvent(fe);
}

int FdEventsKqueuer::modFdEvent(FdEvent *fe)
{
    assert(fe);
    int fd = fe->fd();
    printf("FdEventsKqueuer::updateChannel[%d]\n", fd);
    if (hasChannel(fe))    //exist, update
    {
        assert(getChannel(fd) == fe);
        if (fe->isNoneEvent())
        {
            printf("FdEventsKqueuer::updateChannel [%d][%p] NoneEvent\n", fd, fe);
            channelMap_.erase(fd);
            return update(fe, EV_DELETE);
        }
        else
        {
            return update(fe, EV_ADD | EV_ENABLE);
        }
    }
    else                       //new, add
    {
        assert(getChannel(fd) == NULL);
        channelMap_[fd] = fe;
        return update(fe, EV_ADD);
    }
}

int FdEventsKqueuer::delFdEvent(FdEvent *fe)
{
    assert(fe);
    if (!hasChannel(fe))   // 注意 updateChannel 函数中也有一处removeChannel的逻辑
        return 0;

    int fd = fe->fd();
    printf("FdEventsKqueuer::removeChannel [%d][%p]\n", fd, fe);
    assert(hasChannel(fe) && "the remove socket must be already exist");
    assert(getChannel(fd) == fe && "the remove socket must be already exist");
    assert(fe->isNoneEvent());
    size_t n = channelMap_.erase(fd);
    //ZL_UNUSED(n);
    assert(n == 1);

    update(fe, EV_DELETE);
        
    FdEvent::deleteFdEvent(fe); 
    return 0;
}

bool FdEventsKqueuer::update(FdEvent *fe, int operation)
{
    struct kevent changes[1];
    EV_SET(&changes[0], fe->fd(), EVFILT_READ | EVFILT_WRITE, operation, 0, 0, fe);

    int ret = ::kevent(kq, changes, 1, NULL, 0, NULL);
    if (ret == -1)
    {
        return false;
    }

    return true;
}

int FdEventsKqueuer::poll(std::vector<FdEvent *>& fdevents, int timeoutMs)
{
    struct timespec stimespec;
    if (timeoutMs > 0)
    {
        stimespec.tv_sec = timeval / 1000;
        stimespec.tv_nsec = (timeval % 1000) * 1000000;
    }

    int numEvents = ::kevent(kqueuefd_, NULL, 0, &*events_.begin(), static_cast<int>(events_.size()),
        timeoutMs > 0 ? &stimespec : 0);
    int savedErrno = errno;
    if (numEvents > 0)
    {
        printf("FdEventsKqueuer::poll: [%d] events happended\n", numEvents);
        fireActiveChannels(numEvents, fdevents);
        if (static_cast<size_t>(numEvents) == events_.size())
        {
            events_.resize(events_.size() * 2);
        }
    }
    else if (numEvents == 0)
    {
        printf("FdEventsKqueuer::poll: nothing happended\n");
        return 0;
    }
    else  // error happens
    {
        // TODO : should return -1 if EINTR, else return 0
        if (savedErrno != EINTR)
        {
            errno = savedErrno;
            printf("FdEventsKqueuer::poll: error [%d]\n", savedErrno);
        }
        return -1;
    }

    return numEvents;
}

void FdEventsKqueuer::fireActiveChannels(int numEvents, ChannelList& fdevents) const
{
    assert(static_cast<size_t>(numEvents) <= events_.size());
    for (int i = 0; i < numEvents; ++i)
    {
        //int fd = events_[i].ident;
        FdEvent *fde = static_cast<FdEvent*>(events_[i].udata);
        assert(hasChannel(fde) && "the channel must be already exist");

        int revents = FDEVENT_NONE;
        if (events_[i].events & EVFILT_READ)     revents |= FDEVENT_IN;
        if (events_[i].events & EVFILT_WRITE)    revents |= FDEVENT_OUT;
        if (events_[i].flags & EV_EOF)           revents |= FDEVENT_HUP;
        if (events_[i].flags & EV_ERROR)         revents |= FDEVENT_ERR;

        fde->set_revents(revents);

        fdevents.push_back(fde);
    }
}
