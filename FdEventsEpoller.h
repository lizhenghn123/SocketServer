// ***********************************************************************
// Filename         : FdEventsEpoller.h
// Author           : lizhenghn@gmail.com
// Created          : 2015-05-17
// Description      : I/O MultiPlexing 的 epoll 实现
//
// Last Modified By : LIZHENG
// Last Modified On : 2015-05-17
//
// Copyright (c) lizhenghn@gmail.com. All rights reserved.
// ***********************************************************************
#ifndef ZL_FDEVENTS_EPOLLER_H
#define ZL_FDEVENTS_EPOLLER_H
#include "FdEvents.h"
#include <set>
#include <sys/epoll.h>

class FdEventsEpoller : public FdEvents
{
public:
	FdEventsEpoller();
	~FdEventsEpoller();

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
	typedef std::vector<struct epoll_event> EpollEventList;

	int  epollfd_;
	EpollEventList events_;
};

#endif /* ZL_FDEVENTS_EPOLLER_H */
