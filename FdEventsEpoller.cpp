#include "FdEventsEpoller.h"
#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <sys/epoll.h>


FdEventsEpoller::FdEventsEpoller()
{
	epollfd_ = epoll_create(1024);
	assert(epollfd_ > 0 && " epoll create failure!");

	events_.resize(64);
}

FdEventsEpoller::~FdEventsEpoller()
{
	::close(epollfd_);
}

FdEvent* FdEventsEpoller::addFdEvent(int fd, eventCallback cb)
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

int FdEventsEpoller::addFdEvent(FdEvent *fe)
{
	assert(fe);
	assert(!hasChannel(fe));
	return modFdEvent(fe);
}

int FdEventsEpoller::modFdEvent(FdEvent *fe)
{
	assert(fe);
	int fd = fe->fd();
	printf("FdEventsEpoller::updateChannel[%d]\n", fd);
	if (hasChannel(fe))    //exist, update
	{
		assert(getChannel(fd) == fe);
		if (fe->isNoneEvent())
		{
			printf("EpollPoller::updateChannel [%d][%0x] NoneEvent\n", fd, fe);
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

int FdEventsEpoller::delFdEvent(FdEvent *fe)
{
	assert(fe);
	if (!hasChannel(fe))   // 注意 updateChannel 函数中也有一处removeChannel的逻辑
		return 0;

	int fd = fe->fd();
	printf("EpollPoller::removeChannel [%d][%0x]\n", fd, fe);
	assert(hasChannel(fe) && "the remove socket must be already exist");
	assert(getChannel(fd) == fe && "the remove socket must be already exist");
	assert(fe->isNoneEvent());
	size_t n = channelMap_.erase(fd);
	//ZL_UNUSED(n);
	assert(n == 1);

	return update(fe, EPOLL_CTL_DEL);
}

bool FdEventsEpoller::update(FdEvent *fe, int operation)
{
	struct epoll_event ev = { 0, { 0 } };

	int events = fe->events();
	if (events & FDEVENT_IN)  ev.events |= EPOLLIN;
	if (events & FDEVENT_OUT) ev.events |= EPOLLOUT;
	ev.events |= EPOLLERR | EPOLLHUP;

	ev.data.ptr = fe;

	if (::epoll_ctl(epollfd_, operation, fe->fd(), &ev) < 0)
	{
		printf("EpollPoller::update error, [socket %d][op %d]\n", fe->fd(), operation);
		return false;
	}
	return true;
}

int FdEventsEpoller::poll(std::vector<FdEvent *>& fdevents, int timeoutMs)
{
	int numEvents = ::epoll_wait(epollfd_, &*events_.begin(), static_cast<int>(events_.size()), timeoutMs);
	int savedErrno = errno;
	if (numEvents > 0)
	{
		printf("FdEventsEpoller::poll: [%d] events happended\n", numEvents);
		fireActiveChannels(numEvents, fdevents);
		if (static_cast<size_t>(numEvents) == events_.size())
		{
			events_.resize(events_.size() * 2);
		}
	}
	else if (numEvents == 0)
	{
		printf("FdEventsEpoller::poll: nothing happended\n");
		return 0;
	}
	else  // error happens
	{
		// TODO : should return -1 if EINTR, else return 0
		if (savedErrno != EINTR)
		{
			errno = savedErrno;
			perror("fff :");
			printf("FdEventsEpoller::poll: error [%d]\n", savedErrno);
		}
		return -1;
	}

	return fdevents.size();
}

void FdEventsEpoller::fireActiveChannels(int numEvents, ChannelList& activeChannels) const
{
	assert(static_cast<size_t>(numEvents) <= events_.size());
	for (int i = 0; i < numEvents; ++i)
	{
		FdEvent *fde = static_cast<FdEvent*>(events_[i].data.ptr);
		assert(hasChannel(fde) && "the channel must be already exist");

		int revents = FDEVENT_NONE;
		if (events_[i].events & EPOLLIN)    revents |= FDEVENT_IN;
		if (events_[i].events & EPOLLPRI)   revents |= FDEVENT_PRI;
		if (events_[i].events & EPOLLOUT)   revents |= FDEVENT_OUT;
		if (events_[i].events & EPOLLERR)   revents |= FDEVENT_ERR;
		if (events_[i].events & EPOLLHUP)   revents |= FDEVENT_HUP;
		channel->set_revents(revents);

		activeChannels.push_back(fde);
	}
}
