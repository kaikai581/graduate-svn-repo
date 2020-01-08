DYBGAUDILIB := $(shell echo $(LD_LIBRARY_PATH) | cut -d: -f1)

CFLAGS = `root-config --cflags` -Iinclude -I$(PERCALIBREADOUTEVENTROOT) -I$(PERBASEEVENTROOT) -I$(ROOTIOSVCROOT) -I$(CONTEXTROOT) -I$(CONVENTIONSROOT)

GLIBS = `root-config --glibs` -lTreePlayer -L$(DYBGAUDILIB) -lPerCalibReadoutEvent


all:
	g++ -Wall src/main.cpp src/processFile.cpp src/utilities.cpp src/structopt.cpp src/processEvent.cpp src/RawVars.cpp src/EnhancedVars.cpp src/CsNVars.cpp src/EnhancedTreeVars.cpp $(CFLAGS) $(GLIBS) -o pro2db
