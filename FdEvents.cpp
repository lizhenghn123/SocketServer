#include "FdEvents.h"
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include "FdEventsEpoller.h"
#include "FdEventsPoller.h"

/*static*/ FdEvent*  FdEvent::newFdEvent(int fd)
{
    return new FdEvent(fd);
}

FdEvent::FdEvent(int fd)
{
    fd_ = fd;
    events_ = FDEVENT_NONE;
    revents_ = FDEVENT_NONE;
    callback_ = NULL;
}

FdEvent::~FdEvent()
{

}

FdEvents::FdEvents()
{

}

FdEvents::~FdEvents()
{

}

bool FdEvents::hasChannel(const FdEvent* channel) const
{
    ChannelMap::const_iterator itr = channelMap_.find(channel->fd());
    return itr != channelMap_.end() && itr->second == channel;
}

FdEvent* FdEvents::getChannel(int sock) const
{
    ChannelMap::const_iterator itr = channelMap_.find(sock);
    if(itr == channelMap_.end())
        return NULL;
    return itr->second;
}

FdEvents* createFdEvents()
{
#ifdef FDEVENTS_TYPE_EPOLL
    return new FdEventsEpoller();
#elif defined(FDEVENTS_TYPE_POLL)
    return new FdEventsPoller();
#elif defined(FDEVENTS_TYPE_SELECT)
    //return new FdEventsSelecter();
#endif
	return NULL;
}
