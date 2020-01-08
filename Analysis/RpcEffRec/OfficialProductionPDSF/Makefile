CC = g++

#DYBGAUDILIB := $(shell arrIN=(${LD_LIBRARY_PATH//:/ })&&echo ${arrIN[0]})
DYBGAUDILIB := $(shell echo $(LD_LIBRARY_PATH) | cut -d: -f1)
TCLAPINC=/eliza7/dayabay/scratch/sklin/Software/tclap-1.2.1/include

CFLAGS = `root-config --cflags --glibs` -lTreePlayer -I$(PERCALIBREADOUTEVENTROOT) -I$(PERBASEEVENTROOT) -I$(ROOTIOSVCROOT) -I$(CONTEXTROOT) -I$(CONVENTIONSROOT) -I$(TCLAPINC) -L$(DYBGAUDILIB) -lPerCalibReadoutEvent


EXEC = evtsel

all:
	$(CC) -Wall $(CFLAGS) src/calibHdrDict.cpp src/eventSelection.cpp -o $(EXEC)
