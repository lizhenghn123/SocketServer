// ***********************************************************************
// Filename         : FdEventsDevpoller.h
// Author           : lizhenghn@gmail.com
// Created          : 2015-05-22
// Description      : I/O MultiPlexing 的 devpoll 实现(solaris), 未验证
//
// Last Modified By : LIZHENG
// Last Modified On : 2015-05-22
//
// Copyright (c) lizhenghn@gmail.com. All rights reserved.
// ***********************************************************************
#ifndef ZL_FDEVENTS_DEVPOLLER_H
#define ZL_FDEVENTS_DEVPOLLER_H
#include "FdEvents.h"
#include <vector>
#include <sys/devpoll.h>

class FdEventsDevpoller : public FdEvents
{
public:
    FdEventsDevpoller();
    ~FdEventsDevpoller();

public:
    virtual FdEvent* addFdEvent(int fd, eventCallback cb);

    virtual int addFdEvent(FdEvent *fe);
    virtual int modFdEvent(FdEvent *fe);
    virtual int delFdEvent(FdEvent *fe);

    virtual int poll(std::vector<FdEvent *>& fdevents, int timeoutMs);

    virtual const char* getName() const { return "linux_epoll"; }

private:
    bool update(FdEvent *fe, int operation);
    void fireActiveChannels(int numEvents, ChannelList& fdevents) const;

private:
    typedef std::vector<struct pollfd> DevPollEventList;

    int                 devpollfd_;
    DevPollEventList    events_;

}

#endif /* ZL_FDEVENTS_DEVPOLLER_H */