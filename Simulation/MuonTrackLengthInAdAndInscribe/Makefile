CFLAGS = `root-config --cflags` -Iinclude


GLIBS = `root-config --glibs` -lTreePlayer -lGenVector


all:
	g++ -Wall $(CFLAGS)  -o bin/tracksim src/main.cpp $(GLIBS)
