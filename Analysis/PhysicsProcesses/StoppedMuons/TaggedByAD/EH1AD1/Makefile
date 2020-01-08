DYBGAUDILIB := $(shell echo $(LD_LIBRARY_PATH) | cut -d: -f1)

UDILIB := $(shell echo $(LD_LIBRARY_PATH) | cut -d: -f1)

SQLDIR := $(shell echo $(LD_LIBRARY_PATH) | cut -d: -f1 | rev | cut -d/ -f5- | rev)


# On machines other than pdsf, use the directory setup by NuWa.
# it's weird that pdsf's precompiled NuWa doesn't include sqlite.
HOSTNAME := $(shell hostname)

ifeq ($(HOSTNAME),pdyb-04)
	SQLDIR := /eliza7/dayabay/scratch/sklin/Software/sqlite
	MYSQLCONNDIR := /eliza7/dayabay/scratch/sklin/Software/MySQLConnectorC++
endif


CFLAGS = `root-config --cflags` -Iinclude -I$(PERCALIBREADOUTEVENTROOT) -I$(PERBASEEVENTROOT) -I$(ROOTIOSVCROOT) -I$(CONTEXTROOT) -I$(CONVENTIONSROOT) -I$(SQLDIR)/include -I$(MYSQLCONNDIR)/include


GLIBS = `root-config --glibs` -lTreePlayer -L$(DYBGAUDILIB) -lPerCalibReadoutEvent


all:
	g++ -Wall src/main.cpp src/utilities.cpp src/structopt.cpp src/processFile.cpp src/TreeVariables.cpp src/RecADEvent.cpp src/TaggedMuonEvent.cpp src/CalibEvent.cpp $(CFLAGS) $(GLIBS) -o stopmu
