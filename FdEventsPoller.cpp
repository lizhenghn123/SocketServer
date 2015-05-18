#include "FdEventsPoller.h"
#include <stdio.h>
#include <assert.h>
#include <errno.h>

FdEventsPoller::FdEventsPoller()
{

}

FdEventsPoller::~FdEventsPoller()
{

}

FdEvent* FdEventsPoller::addFdEvent(int fd, eventCallback cb)
{
	assert(fd);
	assert(!getChannel(fd));
	FdEvent *fde = FdEvent::newFdEvent(fd);
	fde->events_ = (FDEVENT_IN | FDEVENT_HUP | FDEVENT_ERR);
	fde->callback_ = cb;
    addFdEvent(fde);
	return fde;
}

int FdEventsPoller::addFdEvent(FdEvent *fe)
{
	assert(fe);
	assert(!hasChannel(fe));
	return modFdEvent(fe);
}

int FdEventsPoller::modFdEvent(FdEvent *fde)
{
	int fd = fde->fd();
	printf("SelectPoller::updateChannel[%d]\n", fd);

	int events = fde->events();
	int pevents = FDEVENT_NONE;
	if (events & FDEVENT_IN)  pevents |= POLLIN;
	if (events & FDEVENT_OUT) pevents |= POLLOUT;
	pevents |= POLLERR | POLLHUP;

	if (channelIter_.find(fde) != channelIter_.end())    //exist, update
	{
		assert(getChannel(fd) == fde);
		int idx = channelIter_[fde];
		printf("PollPoller::updateChannel  fd2 [%d][%d]\n", idx, channelIter_[fde]);
		assert(0 <= idx && idx < static_cast<int>(pollfds_.size()));

		struct pollfd& pfd = pollfds_[idx];
		assert(pfd.fd == fde->fd() || pfd.fd == -fde->fd() - 1);
		pfd.events = static_cast<short>(pevents);
		pfd.revents = 0;
		if (fde->isNoneEvent())
		{
			printf("PollPoller::updateChannel [%d][%p][%d] NoneEvent\n", fd, fde, pfd.events);
			pfd.fd = -fde->fd() - 1;
		}
	}
	else                       //new, add
	{
		assert(getChannel(fd) == NULL);
		struct pollfd pfd;
		pfd.fd = fd;
		pfd.events = static_cast<short>(pevents);
		pfd.revents = 0;
		pollfds_.push_back(pfd);

		channelMap_[fd] = fde;
        channelIter_.insert(std::make_pair(fde, pollfds_.size() - 1));
	}
	return 0;
}

int FdEventsPoller::delFdEvent(FdEvent *fde)
{
	if (!hasChannel(fde))
		return -1;

	int fd = fde->fd();
	int idx = channelIter_[fde];
	printf("PollPoller::removeChannel [%d][%d][%p]\n", fd, idx, fde);
	assert(getChannel(fd) == fde && "the remove socket must be already exist");
	assert(fde->isNoneEvent());
	assert(0 <= idx && idx < static_cast<int>(pollfds_.size()));

	const struct pollfd& pfd = pollfds_[idx];
	(void)(pfd);
	assert(pfd.fd == -fde->fd() - 1 && pfd.events == fde->events());

	size_t n = channelMap_.erase(fd);
	(void)(n);
	assert(n == 1);
	if ((idx) == static_cast<int>(pollfds_.size()) - 1) // last one
	{

	}
	else
	{
		int lastfd = pollfds_.back().fd;
		iter_swap(pollfds_.begin() + idx, pollfds_.end() - 1);
		if (lastfd < 0)
		{
			lastfd = -lastfd - 1;
		}
		channelIter_[getChannel(lastfd)] = idx;
	}

	pollfds_.pop_back();
	channelIter_.erase(fde);
	return 0;
}

int FdEventsPoller::poll(std::vector<FdEvent *>& fdevents, int timeoutMs)
{
	int numEvents = ::poll(&*pollfds_.begin(), pollfds_.size(), timeoutMs);
	int savedErrno = errno;
	if (numEvents > 0)
	{
		printf("FdEventsPoller::poll_once: events happended[%d]\n", numEvents);
		fireActiveChannels(numEvents, fdevents);
	}
	else if (numEvents == 0)
	{
		printf("FdEventsPoller::poll_once: nothing happended\n");
	}
	else
	{
		if (savedErrno != EINTR)
		{
			errno = savedErrno;
			printf("FdEventsPoller::poll_once: error [%d]\n", errno);
		}
	}

    return numEvents;
}

void FdEventsPoller::fireActiveChannels(int numEvents, ChannelList& fdevents) const
{
	for (PollFdList::const_iterator it = pollfds_.begin(); numEvents > 0 && it != pollfds_.end(); ++it)
	{
		if (it->revents > 0)
		{
			--numEvents;
			FdEvent *fde = getChannel(it->fd);
			assert(fde && "the channel must be already exist");
			//channel->set_revents(it->revents);
			int revents = FDEVENT_NONE;
			if (it->revents & POLLIN)    revents |= FDEVENT_IN;
			if (it->revents & POLLPRI)   revents |= FDEVENT_PRI;
			if (it->revents & POLLOUT)   revents |= FDEVENT_OUT;
			if (it->revents & POLLERR)   revents |= FDEVENT_ERR;
			if (it->revents & POLLHUP)   revents |= FDEVENT_HUP;
			if (it->revents & POLLNVAL)  revents |= FDEVENT_NVAL;  // never happen
			fde->set_revents(revents);

			fdevents.push_back(fde);
			//LOG_INFO("PollPoller::fireActiveChannels [%d][%d]", it->fd, it->revents);
		}
	}
}
