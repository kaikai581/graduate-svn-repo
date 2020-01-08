#include <iostream>
#include "CsNVars.hpp"


using namespace std;



CsNVars::CsNVars(EnhancedVars& ev)
{
  site = ev.getSite();
  detector = ev.getDetector();
  triggerNumber = ev.getTriggerNumber();
  triggerTimeSec = ev.getTriggerTimeSec();
  triggerTimeNanoSec = ev.getTriggerTimeNanoSec();
}




CsNVars::CsNVars(const EnhancedVars& ev, unsigned int cutSize)
{
  filterClusters(ev, cutSize);
}



void CsNVars::filterClusters(EnhancedVars ev, unsigned int cutSize)
{
  /// set title
  varTitle = Form("cs%d", cutSize);
  
  /// assign basic variables
  site = ev.getSite();
  detector = ev.getDetector();
  triggerNumber = ev.getTriggerNumber();
  triggerType = ev.getTriggerType();
  triggerTimeSec = ev.getTriggerTimeSec();
  triggerTimeNanoSec = ev.getTriggerTimeNanoSec();
  nHitsRpc = ev.getNHitsRpc();
  
  /// number of modules with hit layers >= 3 after cluster purge
  nModules = 0;

  rpc::mReadoutClusters curReadout = ev.getRoInCluster();
  
  for(rpc::mReadoutClusters::iterator it = curReadout.begin(); it != curReadout.end(); it++)
  {
    mRpcClusterFiltered[it->first] = rpc::mModuleClusters(4, rpc::mLayerClusters(0, rpc::clusterUnit()));
    
    for(unsigned int l = 0; l < 4; l++)
      for(unsigned int c = 0; c < it->second[l].size(); c++)
        if(it->second[l][c].size() <= cutSize)
          mRpcClusterFiltered[it->first][l].push_back(it->second[l][c]);
    
    
    /// offline trigger discrimination
    mRpcCluster[it->first] = rpc::mModuleClusters(4, rpc::mLayerClusters(0, rpc::clusterUnit()));
    if(getNLayers(mRpcClusterFiltered[it->first]) >= 3)
    {
      mRpcCluster[it->first] = mRpcClusterFiltered[it->first];
      nModules++;
    }
    
    
    /// check if the layer has true hardware hit
    roHitLayerRaw[it->first] = rpc::mHitLayerHardware(4, false);
    for(unsigned int l = 0; l < 4; l++)
      if(it->second[l].size()) roHitLayerRaw[it->first][l] = true;
  }
  
  clusterizeModule();
  findTracksSpurious();
  findClusterDistance();
}




unsigned int CsNVars::getNLayers(const rpc::mModuleClusters& curMod)
{
  unsigned int nl = 0;
  
  for(unsigned int l = 0; l < 4; l++)
    if(curMod[l].size()) nl++;
  
  return nl;
}






void CsNVars::findTracksSpurious()
{
  for(rpc::mReadoutClusters::iterator it = mRpcCluster.begin(); it != mRpcCluster.end(); it++)
  {
    
    /// allocate storage for spurious hits
    /// copy the layer clusters for getting the spurious hits
    rpc::mLayerClusters copylc[4];
    rpc::mSpuriousIndex spuIdx;
    for(int i = 0; i < 4; i++)
    {
      copylc[i] = it->second[i];
      spuIdx[i] = vector<unsigned int>();
    }

    roXTrackPairIdx[it->first] = findPairTracksSpurious(it->second[0], it->second[3], copylc[0], copylc[3]);
    
    roYTrackPairIdx[it->first] = findPairTracksSpurious(it->second[1], it->second[2], copylc[1], copylc[2]);
    
    /// store spurious hits
    for(int i = 0; i < 4; i++)
      for(unsigned int j = 0; j < copylc[i].size(); j++)
      {
        unsigned int pos = find(it->second[i].begin(), it->second[i].end(), copylc[i][j]) - it->second[i].begin();
        spuIdx[i].push_back(pos);
      }
    
    roSpuIdx[it->first] = spuIdx;
    
    /// find force pair tracks
    /// the idea is if it's a 3-fold, form a track in the unpaired layers mandatorily
    roXForcedTrackPairIdx[it->first] = pairForcedClusters(it->second[0], it->second[3], roHitLayerRaw[it->first][0], roHitLayerRaw[it->first][3]);
    roYForcedTrackPairIdx[it->first] = pairForcedClusters(it->second[1], it->second[2], roHitLayerRaw[it->first][1], roHitLayerRaw[it->first][2]);
  }
}










rpc::mTrackPairIdx CsNVars::pairForcedClusters(rpc::mLayerClusters lc1, rpc::mLayerClusters lc2, bool hwHit1, bool hwHit2)
{
  /// return value
  rpc::mTrackPairIdx pairIdx(0, pair<unsigned int, unsigned int>());
  
  
  /// If one layer has no hits, copy the hits from the paired counterpart to it.
  if(!hwHit1) lc1 = lc2;
  if(!hwHit2) lc2 = lc1;
  
  
  /// record the cluster ID after it is paired
  vector<unsigned int> pairedc1;
  vector<unsigned int> pairedc2;
  
  
  for(unsigned int lc1Idx = 0; lc1Idx < lc1.size(); lc1Idx++)
    for(unsigned int lc2Idx = 0; lc2Idx < lc2.size(); lc2Idx++)
    {
      rpc::clusterUnit c1 = lc1[lc1Idx];
      rpc::clusterUnit c2 = lc2[lc2Idx];
      
      
      /// By requiring distance = 0, clusters off by 1 strip are included.
      if(findPairClusterDistance(c1, c2) <= 0)
      {
        /// check if the cluster was paired with someone
        /// if not, pair them and tag them as paired
        vector<unsigned int>::iterator itpair1 = find(pairedc1.begin(), pairedc1.end(), lc1Idx);
        vector<unsigned int>::iterator itpair2 = find(pairedc2.begin(), pairedc2.end(), lc2Idx);
        if(itpair1 == pairedc1.end() && itpair2 == pairedc2.end())
        {
          pairIdx.push_back(pair<unsigned int, unsigned int>(lc1Idx, lc2Idx));
          pairedc1.push_back(lc1Idx);
          pairedc2.push_back(lc2Idx);
        }
      }
    }

  return pairIdx;
}




