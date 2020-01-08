CFLAGS = `root-config --cflags`
GLIBS = `root-config --glibs` -lGenVector

all:
	g++ -Wall -o mme meanenergy.cpp $(CFLAGS) $(GLIBS)
