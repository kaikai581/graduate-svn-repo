#include <iostream>
#include "RecADEvent.hpp"
#include "TLeaf.h"
#include "TTree.h"



using namespace std;



RecADEvent::RecADEvent(TTree* tr, int entry)
{
  getEntry(tr, entry);
}




RecADEvent::RecADEvent(TTree* tr, TTree* tcs, int entry)
{
  getEntry(tr, entry);
  fillNPESum(tcs);
}




void RecADEvent::fillNPESum(TTree* tcs)
{
  tcs->GetEntryWithIndex(triggerNumber, detector);
  nPESum = (float)tcs->GetLeaf("nPESum")->GetValue();
}




void RecADEvent::fillVariables(TTree* tr)
{
  triggerNumber = (unsigned int)tr->GetLeaf("triggerNumber")->GetValue();
  
  long triggerTimeSec = (long)tr->GetLeaf("triggerTimeSec")->GetValue();
  long triggerTimeNanoSec = (long)tr->GetLeaf("triggerTimeNanoSec")->GetValue();
  triggerTimeInNs = triggerTimeSec*1000000000+triggerTimeNanoSec;
  
  detector = (short)tr->GetLeaf("detector")->GetValue();
  energy = (float)tr->GetLeaf("energy")->GetValue();
  energyStatus = (int)tr->GetLeaf("energyStatus")->GetValue();
  
  x = (float)tr->GetLeaf("x")->GetValue();
  y = (float)tr->GetLeaf("y")->GetValue();
  z = (float)tr->GetLeaf("z")->GetValue();
}




void RecADEvent::getEntry(TTree* tr, int entry)
{
  if(entry >= tr->GetEntries())
  {
    cerr << "RecADEvent::fillVariables entry out of range" << endl;
    return;
  }
  
  tr->GetEntry(entry);
  
  fillVariables(tr);
}




void RecADEvent::getEntryWithIndex(TTree* tr, int tn, int dId)
{
  tr->GetEntryWithIndex(tn, dId);
  
  fillVariables(tr);
}




bool RecADEvent::isFlasher(TTree* tcs)
{
  /// set flasher flag
  tcs->GetEntryWithIndex(triggerNumber, detector);
  float quadrant = tcs->GetLeaf("Quadrant")->GetValue();
  float maxq = tcs->GetLeaf("MaxQ")->GetValue();
  float maxq2in = tcs->GetLeaf("MaxQ_2inchPMT")->GetValue();

  if(quadrant*quadrant+(maxq/.45)*(maxq/.45)<1 && maxq2in<100)
    return false;
  
  return true;
}





bool RecADEvent::isStoppedMuon(TTree* tcs)
{
  if(isFlasher(tcs)) return false;
  
  if(energyStatus==1 && energy>80 && energy<700) return true;
  
  return false;
}





bool RecADEvent::isMichelElectron(TTree* tcs, RecADEvent promptEvent)
{
  if(isFlasher(tcs)) return false;
  
  if(detector != promptEvent.detector) return false;
  
  long dt = triggerTimeInNs-promptEvent.triggerTimeInNs;
  
  /// keep events with all possible energy
  if(dt>3500 && dt<20000)
    //if(energyStatus==1 && energy>20 && energy<60) return true;
    if(energyStatus==1) return true;
  
  return false;
}




bool RecADEvent::isMichel()
{
  return (energy > 20) && (energy < 60);
}





RecADEvent::position RecADEvent::position2Window(long muonTime)
{
  long startTime = muonTime+3500, endTime = muonTime+20000;
  
  if(triggerTimeInNs > endTime) return right;
  else if(triggerTimeInNs > startTime) return in;
  
  return left;
}
