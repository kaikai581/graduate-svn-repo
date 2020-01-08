#include <algorithm>
#include <iostream>
#include "CalibEvent.hpp"
#include "TTree.h"
#include "PerCalibReadoutEvent/PerCalibReadoutHeader.h"


using namespace std;



CalibEvent::CalibEvent(PerCalibReadoutHeader* rh, TTree* tc, int triggerNum, int det)
{
  tc->SetBranchAddress("CalibReadout_CalibReadoutHeader", &rh);

  getEntryWithIndex(rh, tc, triggerNum, det);
}




CalibEvent::CalibEvent(PerCalibReadoutHeader* rhin, int bytein)
{
  rh = rhin;
  byte = bytein;
  
  fillVariables();
}



void CalibEvent::fillVariables()
{
  if(byte > 0)
  {
    triggerTimeInNs = (long)rh->triggerTimeSec*1000000000+(long)rh->triggerTimeNanoSec;
    rearrangeVars();
  }
  else
  {
    triggerTimeInNs = -1;
    mRpcReadout.clear();
  }
}




void CalibEvent::getEntryWithIndex(PerCalibReadoutHeader* rh, TTree* tc, int triggerNum, int det)
{
  byte = tc->GetEntryWithIndex(triggerNum, det);
  
  fillVariables();
}





void CalibEvent::rearrangeVars()
{
  
  for(unsigned int mIdx = 0; mIdx < rh->nHitsRpc; mIdx++)
  {
    moduleId modId(rh->rpcRow[mIdx], rh->rpcColumn[mIdx]);
    mEvent::iterator it = mRpcReadout.find(modId);
    
    /// if this module is new then allocate storage
    if(it == mRpcReadout.end())
      mRpcReadout[modId] = std::vector<std::vector<unsigned int> >(4, std::vector<unsigned int>());
    mRpcReadout[modId][rh->rpcLayer[mIdx]-1].push_back(rh->rpcStrip[mIdx]);
    mRpcFromRot[modId] = rh->rpcFromRot[mIdx];
  }
  
  /// it's possible the strips are unsorted
  /// sort them!
  for(mEvent::iterator it = mRpcReadout.begin(); it != mRpcReadout.end(); it++)
    for(int lIdx = 0; lIdx < 4; lIdx++)
      sort(it->second[lIdx].begin(), it->second[lIdx].end());
}
