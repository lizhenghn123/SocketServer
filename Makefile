CFLAGS = -g -Wall
all: runserver runclient 

Utils.o:
	g++ ${CFLAGS} -c -o Utils.o Utils.cpp
FdEvents.o:
	g++ ${CFLAGS} -c -o FdEvents.o FdEvents.cpp
FdEventsEpoller.o:
	g++ ${CFLAGS} -c -o FdEventsEpoller.o FdEventsEpoller.cpp
FdEventsPoller.o:
	g++ ${CFLAGS} -c -o FdEventsPoller.o FdEventsPoller.cpp
FdEventsSelecter.o:
	g++ ${CFLAGS} -c -o FdEventsSelecter.o FdEventsSelecter.cpp

runserver: FdEvents.o Utils.o FdEventsEpoller.o FdEventsPoller.o FdEventsSelecter.o main.cpp
	g++ ${CFLAGS} -o runserver Utils.o FdEvents.o FdEventsEpoller.o FdEventsPoller.o FdEventsSelecter.o main.cpp

runclient: TestClient.cpp
	g++ -o runclient TestClient.cpp

clean:
	rm -f *o runserver runclient
