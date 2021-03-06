﻿// ***********************************************************************
// Filename         : FdEventsPoller.h
// Author           : lizhenghn@gmail.com
// Created          : 2015-05-18
// Description      : I/O MultiPlexing 的 poll 实现
//
// Last Modified By : LIZHENG
// Last Modified On : 2015-05-19
//
// Copyright (c) lizhenghn@gmail.com. All rights reserved.
// ***********************************************************************
#ifndef ZL_FDEVENTS_POLLER_H
#define ZL_FDEVENTS_POLLER_H
#include "FdEvents.h"
#include <vector>
//#include <unordered_map>
#ifdef __DEPRECATED
#include <tr1/unordered_map>
#define hash_map std::tr1::unordered_map
#else
#include <ext/hash_map>
#define hash_map __gnu_cxx::hash_map
#endif
#include <sys/poll.h>

class FdEventsPoller : public FdEvents
{
public:
    FdEventsPoller();
    ~FdEventsPoller();

public:
    virtual FdEvent* addFdEvent(int fd, eventCallback cb);
    virtual int addFdEvent(FdEvent *fe);
    virtual int modFdEvent(FdEvent *fe);
    virtual int delFdEvent(FdEvent *fe);

    virtual int poll(std::vector<FdEvent *>& fdevents, int timeoutMs);

    virtual const char* getName() const { return "linux_poll"; }

private:
    void fireActiveChannels(int numEvents, ChannelList& fdevents) const;

private:
    typedef std::vector<struct pollfd>         PollFdList;
    typedef hash_map<FdEvent*, int>            ChannelIter;

    PollFdList    pollfds_;
    ChannelIter   channelIter_;
};

#endif /* ZL_FDEVENTS_POLLER_H */
