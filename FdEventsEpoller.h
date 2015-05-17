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
	virtual FdEvent* addFdEvent(int fd, eventCallback cb) = 0;

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
