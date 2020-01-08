CFLAGS = `root-config --cflags` -I$(PERCALIBREADOUTEVENTROOT) -I$(PERBASEEVENTROOT) -I$(ROOTIOSVCROOT) -I$(CONTEXTROOT) -I$(CONVENTIONSROOT)
GLIBS = `root-config --glibs` -lTreePlayer -lPerJobInfoDict -lPerCalibReadoutEvent -lPerCalibReadoutEventDict -lPerCalibReadoutEventLib -lCore -lCint

all:
	g++ -Wall src/neutron.cpp $(CFLAGS) -L/common/dayabay/releases/NuWa/NuWa-23045/NuWa-trunk/dybgaudi/InstallArea/x86_64-slc5-gcc41-dbg/lib $(GLIBS) -o bin/neutron
