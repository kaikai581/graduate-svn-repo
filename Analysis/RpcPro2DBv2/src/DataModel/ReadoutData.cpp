#include <iostream>
#include "DataModel/ReadoutData.hpp"
#include "PerCalibReadoutEvent/PerCalibReadoutHeader.h"



using namespace std;




ReadoutData::ReadoutData(PerCalibReadoutHeader* rh) : allHitsInterior1(true),
allHitsInterior2(true)
{
  clCutSize = 8;
  fillVars(rh, 8);
}





ReadoutData::ReadoutData(PerCalibReadoutHeader* rh, unsigned int clsize) : 
allHitsInterior1(true), allHitsInterior2(true)
{
  clCutSize = clsize;
  fillVars(rh, clsize);
}






void ReadoutData::fillVars(PerCalibReadoutHeader* rh, unsigned int clsize)
{
  /// a direct copy of variables from persistent calibration readout header
  site = rh->site;
  detector = rh->detector;
  triggerNumber = rh->triggerNumber;
  triggerType = rh->triggerType;
  triggerTimeSec = rh->triggerTimeSec;
  triggerTimeNanoSec = rh->triggerTimeNanoSec;
  
  
  for(unsigned int mIdx = 0; mIdx < rh->nHitsRpc; mIdx++)
  {
    rpc::moduleId mId(rh->rpcRow[mIdx], rh->rpcColumn[mIdx]);
    rpc::readout::iterator it = modules.find(mId);
    
    /// if this module is new then allocate storage
    if(it == modules.end()) modules[mId] = ModuleData();

    modules[mId].layers[rh->rpcLayer[mIdx]-1].strips.push_back(rh->rpcStrip[mIdx]);
    modules[mId].fromRot = rh->rpcFromRot[mIdx];
    
    /// check if this is an interior readout
    if(rh->rpcStrip[mIdx] == 1 || rh->rpcStrip[mIdx] == 8) allHitsInterior1 = false;
    if(rh->rpcStrip[mIdx] == 1 || rh->rpcStrip[mIdx] == 2 ||
       rh->rpcStrip[mIdx] == 7 || rh->rpcStrip[mIdx] == 8) allHitsInterior2 = false;
  }


  /// It's possible the strips are unsorted.
  /// Sort them!
  /// After sorting, clusterize strips!
  for(rpc::readout::iterator it = modules.begin(); it != modules.end(); it++)
  {
    for(int lIdx = 0; lIdx < 4; lIdx++)
    {
      it->second.layers[lIdx].sortStrips();
      it->second.layers[lIdx].clusterize(clsize);
    }
    
    /// start filling special variables
    it->second.fillLooseTrackAndSpurious(it->second.layers[0],
    it->second.layers[3], it->second.xLooseTrackPairIndices,
    it->second.layers[0].spuriousIndices, it->second.layers[3].spuriousIndices);
    
    it->second.fillLooseTrackAndSpurious(it->second.layers[1],
    it->second.layers[2], it->second.yLooseTrackPairIndices,
    it->second.layers[1].spuriousIndices, it->second.layers[2].spuriousIndices);
    
    /// muon position reconstruction
    it->second.fillReconstructedPositions();
    
    /// start making number of tracks cut
    /// for 3-fold, requre number of tracks in one direction to be 1
    /// and theother direction to be 0.
    /// equivalently, sum is 1.
    if(it->second.getNLayersOffline() == 3)
      if(it->second.xLooseTrackPairIndices.size() +
         it->second.yLooseTrackPairIndices.size() == 1)
        modulesNT[it->first] = it->second;
    ///// for 4-fold, require both direction to have 1 track
    //if(it->second.getNLayersOffline() == 4)
      //if(it->second.xLooseTrackPairIndices.size() == 1 &&
         //it->second.yLooseTrackPairIndices.size() == 1)
        //modulesNT[it->first] = it->second;
    /// if both directions have 1 track => include trivially
    if(it->second.getNLayersOffline() == 4)
    {
      if(it->second.xLooseTrackPairIndices.size() == 1)
      {
        if(it->second.yLooseTrackPairIndices.size() == 1)
          modulesNT[it->first] = it->second;
        
        /// if a direction has 1 track but the other direction doesn't
        /// in the layers without a track, treat one layer as spurious cluster
        /// layer and the other as a track and an inefficient layer
        else
        {
          ModuleData md;
          md.layers[0] = it->second.layers[0];
          md.layers[3] = it->second.layers[3];
          md.layers[1] = it->second.layers[1];
          modulesNT[it->first] = md;
        }
      }
      else
      {
        if(it->second.yLooseTrackPairIndices.size() == 1)
        {
          ModuleData md;
          md.layers[1] = it->second.layers[1];
          md.layers[2] = it->second.layers[2];
          md.layers[0] = it->second.layers[0];
          modulesNT[it->first] = md;
        }
      }
    }
  }

}






void ReadoutData::printModules()
{
  cout << "number of modules in this readout: ";
  cout << modules.size() << endl;
  
  for(rpc::readout::iterator it = modules.begin(); it != modules.end(); it++)
  {
    cout << "module ID (row,col):" << endl;
    cout << "\t(" << it->first.first << "," << it->first.second << ")" << endl;
    it->second.printLayers();
    
    /// print N-fold before cluster size cut
    cout << "N-fold before cluster size cut: ";
    cout << it->second.getNLayersHardware() << endl;
    
    /// print N-fold after cluster size cut
    cout << "N-fold  after cluster size cut: ";
    cout << it->second.getNLayersOffline() << endl;
    
    /// print forced trigger information
    cout << "from ROT: " << boolalpha << it->second.fromRot << endl;
    
    /// print number of tracks
    cout << "# x tracks: " << it->second.xLooseTrackPairIndices.size() << endl;
    cout << "# y tracks: " << it->second.yLooseTrackPairIndices.size() << endl;
    
    /// print muon reconstructed positions
    cout << "reconstructed coordinates: ";
    for(unsigned int r = 0; r < it->second.muonPositions.size(); r++)
    {
      cout << "(";
      cout << it->second.muonPositions[r].first << ",";
      cout << it->second.muonPositions[r].second;
      cout << ") ";
    }
    cout << endl;
  }
}
