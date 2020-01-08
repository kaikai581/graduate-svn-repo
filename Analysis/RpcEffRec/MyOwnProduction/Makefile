CC = g++

CFLAGS = `root-config --cflags --glibs` -lTreePlayer

EXEC = evtsel

all:
	$(CC) $(CFLAGS) src/eventSelection.cpp -o $(EXEC)
