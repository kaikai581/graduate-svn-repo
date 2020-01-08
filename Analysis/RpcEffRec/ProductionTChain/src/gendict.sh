# This script is used for generating cint dictionary for root TTree to
# understand user defined data type.

rootcint -f calibHdrDict.cpp -c -I$PERBASEEVENTROOT -I$ROOTIOSVCROOT -I$CONTEXTROOT -I$CONVENTIONSROOT $PERCALIBREADOUTEVENTROOT/PerCalibReadoutEvent/PerCalibReadoutHeader.h LinkDef.h
