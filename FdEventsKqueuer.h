// ***********************************************************************
// Filename         : FdEventsKqueuer.h
// Author           : lizhenghn@gmail.com
// Created          : 2015-05-20
// Description      : I/O MultiPlexing 的 kqueue 实现(freebsd), 未验证
//
// Last Modified By : LIZHENG
// Last Modified On : 2015-05-20
//
// Copyright (c) lizhenghn@gmail.com. All rights reserved.
// ***********************************************************************
#ifndef ZL_FDEVENTS_KQUEUER_H
#define ZL_FDEVENTS_KQUEUER_H
#include "FdEvents.h"
#include <vector>
#include <sys/event.h>

class FdEventsKqueuer : public FdEvents
{
public:
    FdEventsKqueuer();
    ~FdEventsKqueuer();

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
    typedef std::vector<struct kevent> KqueuelEventList;

    int                 kqueuefd_;
    KqueuelEventList    events_;

}

#endif /* ZL_FDEVENTS_KQUEUER_H */