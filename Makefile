CFLAGS = -g -Wall
all: runserver runclient 

Utils.o:
	g++ ${CFLAGS} -c -o Utils.o Utils.cpp
FdEvents.o:
	g++ ${CFLAGS} -DFDEVENT_USE_SELECT -c -o FdEvents.o FdEvents.cpp
FdEventsEpoller.o:
	g++ ${CFLAGS} -c -o FdEventsEpoller.o FdEventsEpoller.cpp


runserver: FdEvents.o Utils.o FdEventsEpoller.o main.cpp
	g++ ${CFLAGS} -o runserver Utils.o FdEvents.o FdEventsEpoller.o main.cpp

runclient: TestClient.cpp
	g++ -o runclient TestClient.cpp

clean:
	rm -f *o runserver runclient
