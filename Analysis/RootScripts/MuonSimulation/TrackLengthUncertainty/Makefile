CFLAGS = `root-config --cflags` -Iinclude


GLIBS = `root-config --glibs` -lTreePlayer -lGenVector


all:
	g++ -Wall $(CFLAGS) -o trackuncertainty trackuncertainty.cpp InscribeCylinder.cpp $(GLIBS)
