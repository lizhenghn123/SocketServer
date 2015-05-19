// ***********************************************************************
// Filename         : FdEventsSelecter.h
// Author           : lizhenghn@gmail.com
// Created          : 2015-05-19
// Description      : I/O MultiPlexing 的 select 实现
//
// Last Modified By : LIZHENG
// Last Modified On : 2015-05-20
//
// Copyright (c) lizhenghn@gmail.com. All rights reserved.
// ***********************************************************************
#ifndef ZL_FDEVENTS_SELECTER_H
#define ZL_FDEVENTS_SELECTER_H
#include "FdEvents.h"
#include <set>
#include <sys/select.h>

class FdEventsSelecter : public FdEvents
{
public:
	FdEventsSelecter();
	~FdEventsSelecter();

public:
	virtual FdEvent* addFdEvent(int fd, eventCallback cb);
	virtual int addFdEvent(FdEvent *fe);
	virtual int modFdEvent(FdEvent *fe);
	virtual int delFdEvent(FdEvent *fe);

	virtual int poll(std::vector<FdEvent *>& fdevents, int timeoutMs);

	virtual const char* getName() const { return "linux_select"; }

private:
	void fireActiveChannels(int numEvents, ChannelList& fdevents) const;

private:
    typedef std::set< int, std::greater<int> >  FdList;
	FdList  fdlist_;

	fd_set readfds_;           /// select返回的所有可读事件
	fd_set writefds_;          /// select返回的所有可写事件
	fd_set exceptfds_;         /// select返回的所有错误事件

	fd_set select_readfds_;    /// 加入到select中的感兴趣的所有可读事件
	fd_set select_writefds_;   /// 加入到select中的感兴趣的所有可写事件
	fd_set select_exceptfds_;  /// 加入到select中的感兴趣的所有错误事件
};

#endif /* ZL_FDEVENTS_SELECTER_H */
