#include <algorithm>
#include <iostream>
#include <string>
#include "RawVars.hpp"
#include "rpcnamespace.hpp"


using namespace std;


//unsigned int RawVars::site;
//unsigned int RawVars::detector;
//unsigned int RawVars::triggerNumber;
//unsigned int RawVars::triggerType;
//unsigned int RawVars::triggerTimeSec;
//unsigned int RawVars::triggerTimeNanoSec;
//unsigned int RawVars::nHitsRpc;
//rpc::mEvent  RawVars::mRpcReadout;
//rpc::mFromRot RawVars::mRpcFromRot;
bool RawVars::allHitsInterior(true);
bool RawVars::allHitsInterior2(true);



//RawVars::RawVars(PerCalibReadoutHeader* rh) : rearranged(false)
RawVars::RawVars(PerCalibReadoutHeader* rh) : varTitle("raw")
{
  /// a direct copy of variables from persistent calibration readout header
  site = rh->site;
  detector = rh->detector;
  triggerNumber = rh->triggerNumber;
  triggerType = rh->triggerType;
  triggerTimeSec = rh->triggerTimeSec;
  triggerTimeNanoSec = rh->triggerTimeNanoSec;
  nHitsRpc = rh->nHitsRpc;
  rpcRow = rh->rpcRow;
  rpcColumn = rh->rpcColumn;
  rpcLayer = rh->rpcLayer;
  rpcStrip = rh->rpcStrip;
  rpcFromRot = rh->rpcFromRot;
  
  //mRpcReadout.clear();
  
  rearrangeVars();
}



void RawVars::rearrangeVars()
{
  
  allHitsInterior = true;
  allHitsInterior2 = true;
  
  for(unsigned int mIdx = 0; mIdx < nHitsRpc; mIdx++)
  {
    rpc::mId modId(rpcRow[mIdx], rpcColumn[mIdx]);
    rpc::mEvent::iterator it = mRpcReadout.find(modId);
    
    /// if this module is new then allocate storage
    if(it == mRpcReadout.end())
      mRpcReadout[modId] = std::vector<std::vector<unsigned int> >(4, std::vector<unsigned int>());
    mRpcReadout[modId][rpcLayer[mIdx]-1].push_back(rpcStrip[mIdx]);
    mRpcFromRot[modId] = rpcFromRot[mIdx];
    
    /// check if this is an interior readout
    if(rpcStrip[mIdx] == 1 || rpcStrip[mIdx] == 8) allHitsInterior = false;
    if(rpcStrip[mIdx] == 1 || rpcStrip[mIdx] == 2 ||
       rpcStrip[mIdx] == 7 || rpcStrip[mIdx] == 8) allHitsInterior2 = false;
  }
  
  /// it's possible the strips are unsorted
  /// sort them!
  for(rpc::mEvent::iterator it = mRpcReadout.begin(); it != mRpcReadout.end(); it++)
    for(int lIdx = 0; lIdx < 4; lIdx++)
      sort(it->second[lIdx].begin(), it->second[lIdx].end());

  ///// After executing this method, set rearranged flag to true.
  //rearranged = true;
}



void RawVars::printRpcRow()
{
  cout << "   raw row: ";
  for(unsigned int i = 0; i < rpcRow.size(); i++)
    cout << rpcRow[i] << " ";
  cout << endl;
}



void RawVars::printRpcColumn()
{
  cout << "raw column: ";
  for(unsigned int i = 0; i < rpcColumn.size(); i++)
    cout << rpcColumn[i] << " ";
  cout << endl;
}



void RawVars::printRpcLayer()
{
  cout << " raw layer: ";
  for(unsigned int i = 0; i < rpcLayer.size(); i++)
    cout << rpcLayer[i] << " ";
  cout << endl;
}



void RawVars::printRpcStrip()
{
  cout << " raw strip: ";
  for(unsigned int i = 0; i < rpcStrip.size(); i++)
    cout << rpcStrip[i] << " ";
  cout << endl;
}



void RawVars::printRpcFromRot()
{
  cout << "  from ROT: ";
  for(unsigned int i = 0; i < rpcFromRot.size(); i++)
    cout << rpcFromRot[i] << " ";
  cout << endl;
}



void RawVars::printRearranged()
{
  for(rpc::mEvent::iterator it = mRpcReadout.begin(); it != mRpcReadout.end(); it++)
  {
    
    /// print fired modules
    cout << " (row,col): ";
    cout << "(" << it->first.first << "," << it->first.second << ")" << endl;
    
    /// print fired layers
    cout << "    layers: ";
    for(unsigned int l = 0; l < 4; l++)
      if(it->second[l].size())
        cout << l+1 << " ";
    cout << endl;
    
    /// print fired strips
    cout << "    strips: ";
    for(unsigned int l = 0; l < 4; l++)
    {
      cout << "(";
      for(unsigned int s = 0; s < it->second[l].size(); s++)
      {
        cout << it->second[l][s];
        cout << ((s == (it->second[l].size()-1)) ? "":",");
      }
      cout << ")";
    }
    cout << endl;
  }
}
