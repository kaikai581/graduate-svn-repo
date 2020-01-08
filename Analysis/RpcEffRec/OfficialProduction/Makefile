CC = g++

CFLAGS = `root-config --cflags --glibs` -lTreePlayer -I/home/sklin/Data/sklin/Data/Software/NuWa-trunk/NuWa-trunk/dybgaudi/RootIO/PerCalibReadoutEvent -I/home/sklin/Data/sklin/Data/Software/NuWa-trunk/NuWa-trunk/dybgaudi/RootIO/PerBaseEvent -I/home/sklin/Data/sklin/Data/Software/NuWa-trunk/NuWa-trunk/dybgaudi/RootIO/RootIOSvc -I/home/sklin/Data/sklin/Data/Software/NuWa-trunk/NuWa-trunk/dybgaudi/DataModel/Context -I/home/sklin/Data/sklin/Data/Software/NuWa-trunk/NuWa-trunk/dybgaudi/DataModel/Conventions -L/home/sklin/Data/sklin/Data/Software/NuWa-trunk/NuWa-trunk/dybgaudi/RootIO/PerCalibReadoutEvent/x86_64-debian-gcc43-dbg -lPerCalibReadoutEvent
# -L/home/sklin/Data/sklin/Data/Software/NuWa-trunk/NuWa-trunk/dybgaudi/RootIO/PerCalibReadoutEvent/x86_64-debian-gcc43-dbg/libPerCalibReadoutEventDict.so

EXEC = evtsel

all:
	$(CC) -Wall $(CFLAGS) src/calibHdrDict.cpp src/eventSelection.cpp -o $(EXEC)
