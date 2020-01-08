CC = g++

#DYBGAUDILIB := $(shell arrIN=(${LD_LIBRARY_PATH//:/ })&&echo ${arrIN[0]})
DYBGAUDILIB := $(shell echo $(LD_LIBRARY_PATH) | cut -d: -f1)

CFLAGS = `root-config --cflags --glibs` -lTreePlayer -I$(PERCALIBREADOUTEVENTROOT) -I$(PERBASEEVENTROOT) -I$(ROOTIOSVCROOT) -I$(CONTEXTROOT) -I$(CONVENTIONSROOT) -I./include -L$(DYBGAUDILIB) -lPerCalibReadoutEvent


EXEC = calcrate

all:
	$(CC) -Wall $(CFLAGS) src/calibHdrDict.cpp eventSelection.cpp -o $(EXEC)

clean:
	rm -rfv $(EXEC)
