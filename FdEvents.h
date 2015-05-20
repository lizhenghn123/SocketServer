// ***********************************************************************
// Filename         : FdEvents.h
// Author           : lizhenghn@gmail.com
// Created          : 2015-05-17
// Description      : 模仿lighttpd中的fdevents设计，实现FdEvent及FdEvents(I/O复用接口类)
//
// Last Modified By : LIZHENG
// Last Modified On : 2015-05-19
//
// Copyright (c) lizhenghn@gmail.com. All rights reserved.
// ***********************************************************************
#ifndef ZL_FDEVENTS_H
#define ZL_FDEVENTS_H
#include <vector>
#include <map>

//#define FDEVENTS_TYPE_EPOLL
#define FDEVENTS_TYPE_SELECT
//#define FDEVENTS_TYPE_POLL
//#define FDEVENTS_TYPE_KQUEUE
//#define FDEVENTS_TYPE_POLL

#define FDEVENT_NONE    (0)
#define FDEVENT_IN      (1<<0)
#define FDEVENT_PRI     (1<<1)
#define FDEVENT_OUT     (1<<2)
#define FDEVENT_HUP     (1<<3)
#define FDEVENT_ERR     (1<<4)
#define FDEVENT_NVAL	(1<<5)

class  FdEvent;
class  FdEvents;

typedef int(*eventCallback)(FdEvents *poller, FdEvent *fe);
typedef int(*readCallback)(FdEvents *poller, FdEvent *fe);
typedef int(*writeCallback)(FdEvents *poller, FdEvent *fe);

class FdEvent
{
    friend class FdEvents;
public:
    static FdEvent*  newFdEvent(int fd);

public:
    int fd() const                 { return fd_; }
    void set_fd(int fd)            { fd_ = fd; }
    int events() const             { return events_; }
    //void set_events(int events)    { revents_ = events; }
    void enableReading()           { events_ |= (FDEVENT_IN | FDEVENT_PRI); }
    void enableWriting()           { events_ |= FDEVENT_OUT; }
    void enableAll()               { events_ |= (FDEVENT_IN | FDEVENT_PRI | FDEVENT_OUT); }
    void disableReading()          { events_ &= ~(FDEVENT_IN | FDEVENT_PRI); }
    void disableWriting()          { events_ &= ~FDEVENT_OUT; }
    void disableAll()              { events_ = FDEVENT_NONE; }
    bool isNoneEvent() const       { return events_ == FDEVENT_NONE; }
    void set_revents(int revents)  { revents_ = revents; }
    int  revents() const           { return revents_; }

public://private:
    FdEvent(int fd);
    ~FdEvent();

public:
    int fd_;
    int events_;
    int revents_;
    eventCallback callback_;
};


FdEvents* createFdEvents();

class FdEvents
{
public:
    typedef std::vector<FdEvent *>          ChannelList;
    typedef std::map<int, FdEvent *>        ChannelMap;

public:
    FdEvents();
    virtual ~FdEvents();

public:
    bool hasChannel(const FdEvent *channel) const;
    FdEvent* getChannel(int sock) const;

public:
    virtual FdEvent* addFdEvent(int fd, eventCallback cb) = 0;

    virtual int addFdEvent(FdEvent *fe) = 0;
    virtual int modFdEvent(FdEvent *fe) = 0;
    virtual int delFdEvent(FdEvent *fe) = 0;

    virtual int poll(std::vector<FdEvent *>& fdevents, int timeoutMs) = 0;

    virtual const char* getName() const = 0;

protected:
    ChannelMap  channelMap_;
};


#endif  /* ZL_FDEVENTS_H */
