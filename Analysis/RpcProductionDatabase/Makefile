CC=g++

DYBGAUDILIB := $(shell echo $(LD_LIBRARY_PATH) | cut -d: -f1)

CFLAGS = `root-config --cflags` -Iinclude -I$(PERCALIBREADOUTEVENTROOT) -I$(PERBASEEVENTROOT) -I$(ROOTIOSVCROOT) -I$(CONTEXTROOT) -I$(CONVENTIONSROOT)
GLIBS = `root-config --glibs` -lTreePlayer -L$(DYBGAUDILIB) -lPerCalibReadoutEvent
EXEC = rpc2db
OBJ_DIR = obj
OBJS = $(OBJ_DIR)/main.o $(OBJ_DIR)/commonfunctions.o $(OBJ_DIR)/loop2ROOT.o
INC_DIR = include
INCS = $(INC_DIR)/commonfunctions.h $(INC_DIR)/loop2ROOT.h $(INC_DIR)/vecdict.h



VPATH = src

$(OBJ_DIR)/%.o: %.cpp $(INCS)
	if [ ! -d $(OBJ_DIR) ]; then mkdir -pv $(OBJ_DIR); fi
	$(CC) -Wall $(CFLAGS) -c -o $@ $<

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) -Wall $(CFLAGS) $(GLIBS) -o $(EXEC) $(OBJS)

$(OBJS): $(INCS)

clean:
	rm -rf *.o $(OBJ_DIR) $(EXEC)
