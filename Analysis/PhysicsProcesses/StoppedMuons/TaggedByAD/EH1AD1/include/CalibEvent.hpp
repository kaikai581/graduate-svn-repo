#ifndef CALIBEVENT_HPP
#define CALIBEVENT_HPP

#include <map>
#include <vector>


class TTree;
class PerCalibReadoutHeader;


class CalibEvent
{
public:
  typedef std::pair<unsigned int, unsigned int> moduleId;
  typedef std::vector<std::vector<unsigned int> > mLayers;
  typedef std::map<moduleId, mLayers> mEvent;
  typedef std::map<moduleId, bool> mFromRot;
  
  mEvent   mRpcReadout; // variable used for storing rearranged information
  mFromRot mRpcFromRot;

  long                   triggerTimeInNs;
  int                    byte;
  
  CalibEvent() {};
  CalibEvent(PerCalibReadoutHeader*, TTree*, int, int);
  CalibEvent(PerCalibReadoutHeader*, int);
  
  void fillVariables();
  void getEntryWithIndex(PerCalibReadoutHeader*, TTree*, int, int);
  void rearrangeVars();
  
  PerCalibReadoutHeader* rh;
};


#endif
