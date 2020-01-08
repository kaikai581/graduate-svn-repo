DYBGAUDILIB := $(shell echo $(LD_LIBRARY_PATH) | cut -d: -f1)

SQLDIR := $(shell echo $(LD_LIBRARY_PATH) | cut -d: -f1 | rev | cut -d/ -f5- | rev)


# On machines other than pdsf, use the directory setup by NuWa.
# it's weird that pdsf's precompiled NuWa doesn't include sqlite.
HOSTNAME := $(shell hostname)

ifeq ($(HOSTNAME),pdyb-04)
	SQLDIR := /eliza7/dayabay/scratch/sklin/Software/sqlite
	MYSQLCONNDIR := /eliza7/dayabay/scratch/sklin/Software/MySQLConnectorC++
endif


CFLAGS = `root-config --cflags` -Iinclude -I$(PERCALIBREADOUTEVENTROOT) -I$(PERBASEEVENTROOT) -I$(ROOTIOSVCROOT) -I$(CONTEXTROOT) -I$(CONVENTIONSROOT) -I$(SQLDIR)/include -I$(MYSQLCONNDIR)/include


GLIBS = `root-config --glibs` -lTreePlayer -L$(DYBGAUDILIB) -lPerCalibReadoutEvent -L$(SQLDIR)/lib -lsqlite3 -L$(MYSQLCONNDIR)/lib -lmysqlcppconn


all:
	g++ -Wall src/main.cpp src/DataModel/LayerData.cpp src/DataModel/ModuleData.cpp src/DataModel/ReadoutData.cpp src/DataModel/ClusterData.cpp src/DataModel/DatasetHallData.cpp src/DataModel/DatasetModuleData.cpp src/Database/Database.cpp src/Database/RpcCalibDB.cpp src/Database/MySQLDB.cpp src/utilities.cpp src/structopt.cpp src/processFile.cpp $(CFLAGS) $(GLIBS) -o pro2db
