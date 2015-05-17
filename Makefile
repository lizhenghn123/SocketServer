CFLAGS = -g -Wall
all: runserver runclient 

Utils.o:
	g++ ${CFLAGS} -c -o Utils.o Utils.cpp
FdEvents.o:
	g++ ${CFLAGS} -DFDEVENT_USE_SELECT -c -o FdEvents.o FdEvents.cpp


runserver: FdEvents.o Utils.o main.cpp
	g++ ${CFLAGS} -o runserver Utils.o FdEvents.o main.cpp

runclient: TestClient.cpp
	g++ -o runclient TestClient.cpp

clean:
	rm -f *o runserver runclient