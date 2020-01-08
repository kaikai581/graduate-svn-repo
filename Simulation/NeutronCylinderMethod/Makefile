CFLAGS = `root-config --cflags` -Iinclude


GLIBS = `root-config --glibs` -lTreePlayer -lGenVector


all:
	g++ -Wall $(CFLAGS)  -o bin/neutronsim src/main.cpp $(GLIBS)
