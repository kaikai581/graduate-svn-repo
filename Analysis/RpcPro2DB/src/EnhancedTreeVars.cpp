#include <iostream>
#include <string>
#include "CsNVars.hpp"
#include "EnhancedTreeVars.hpp"
#include "EnhancedVars.hpp"
#include "TTree.h"


using namespace std;

EnhancedTreeVars::EnhancedTreeVars(TTree& t)
{
  
  t.Branch("eventId", &eventId, "eventId/i");
  t.Branch("runNumber", &runNumber, "runNumber/i");
  t.Branch("fileNumber", &fileNumber, "fileNumber/i");
  t.Branch("site", &site, "site/i");
  t.Branch("detector", &detector, "detector/i");
  t.Branch("triggerNumber", &triggerNumber, "triggerNumber/i");
  t.Branch("triggerType", &triggerType, "triggerType/i");
  t.Branch("triggerTimeSec", &triggerTimeSec, "triggerTimeSec/i");
  t.Branch("triggerTimeNanoSec", &triggerTimeNanoSec, "triggerTimeNanoSec/i");
  t.Branch("nHitsRpc", &nHitsRpc, "nHitsRpc/i");
  t.Branch("allHitsInterior", &allHitsInterior, "allHitsInterior/O");
  t.Branch("allHitsInterior2", &allHitsInterior2, "allHitsInterior2/O");
  t.Branch("nModules", &nModules, "nModules/i");
  t.Branch("row", &row);
  t.Branch("col", &col);
  t.Branch("nLayers", &nLayers);
  t.Branch("fromRot", &fromRot);
  t.Branch("nClusters", &nClusters);
  t.Branch("nClustersX1", &nClustersX1);
  t.Branch("nClustersY2", &nClustersY2);
  t.Branch("nClustersY3", &nClustersY3);
  t.Branch("nClustersX4", &nClustersX4);
  t.Branch("clusterSizeX1", &clusterSizeX1);
  t.Branch("clusterSizeY2", &clusterSizeY2);
  t.Branch("clusterSizeY3", &clusterSizeY3);
  t.Branch("clusterSizeX4", &clusterSizeX4);
  t.Branch("clusterSizeX", &clusterSizeX);
  t.Branch("clusterSizeY", &clusterSizeY);
  t.Branch("clusterSizeAll", &clusterSizeAll);
  t.Branch("clusterSize3Fold", &clusterSize3Fold);
  t.Branch("clusterSize4Fold", &clusterSize4Fold);
  
  t.Branch("xClusterDistance", &xClusterDistance);
  t.Branch("yClusterDistance", &yClusterDistance);
  
  t.Branch("nXTracks", &nXTracks);
  t.Branch("nYTracks", &nYTracks);
  t.Branch("nSpuriousX1", &nSpuriousX1);
  t.Branch("nSpuriousY2", &nSpuriousY2);
  t.Branch("nSpuriousY3", &nSpuriousY3);
  t.Branch("nSpuriousX4", &nSpuriousX4);
  t.Branch("nSpuriousX", &nSpuriousX);
  t.Branch("nSpuriousY", &nSpuriousY);
  t.Branch("nSpuriousAll", &nSpuriousAll);
  
  t.Branch("nXForcedTracks", &nXForcedTracks);
  t.Branch("nYForcedTracks", &nYForcedTracks);
  
  t.Branch("xrec", &xrec);
  t.Branch("yrec", &yrec);
  t.Branch("fxrec", &fxrec);
  t.Branch("fyrec", &fyrec);
}




//template<class T> void EnhancedTreeVars::fillTreeVars(const T& somevars)
//{
  //clearContainers();
  
  //eventId = somevars.eventId;
  //runNumber = somevars.runNumber;
  //fileNumber = somevars.fileNumber;
  //site = somevars.getSite();
  //detector = somevars.getDetector();
  //triggerNumber = somevars.getTriggerNumber();
  //triggerType = somevars.getTriggerType();
  //triggerTimeSec = somevars.getTriggerTimeSec();
  //triggerTimeNanoSec = somevars.getTriggerTimeNanoSec();
  //nHitsRpc = somevars.getNHitsRpc();
  
  
  
  //for(rpc::mFromRot::iterator it = somevars.mRpcFromRot.begin(); it != somevars.mRpcFromRot.end(); it++)
  //{
    ///// fill if the trigger is from ROT
    //fromRot.push_back(it->second);
  //}
  
  
  //for(rpc::mReadoutClusters::iterator it = somevars.mRpcCluster.begin(); it != somevars.mRpcCluster.end(); it++)
  //{
    //row.push_back(it->first.first);
    //col.push_back(it->first.second);
    
    ///// fill number of fired layers in an event
    //unsigned int nlyr = 0;
    ///// get cluster size of each layer
    //std::vector<unsigned int> csInLayer[4];
    //std::vector<unsigned int> csOrientation[2];
    //std::vector<unsigned int> csAll;
    //for(unsigned int l = 0; l < 4; l++)
    //{
      //if(it->second[l].size()) nlyr++;
      //for(unsigned int c = 0; c < it->second[l].size(); c++)
      //{
        //csInLayer[l].push_back(it->second[l][c].size());
        
        //csAll.push_back(it->second[l][c].size());
        
        //if((l%3)==0) // x layers
          //csOrientation[0].push_back(it->second[l][c].size());
        //else         // y layers
          //csOrientation[1].push_back(it->second[l][c].size());
      //}
    //}
    //nLayers.push_back(nlyr);
    
    ///// fill number of clusters for each layer
    //nClustersX1.push_back(it->second[0].size());
    //nClustersY2.push_back(it->second[1].size());
    //nClustersY3.push_back(it->second[2].size());
    //nClustersX4.push_back(it->second[3].size());
    
    ///// fill cluster size of each layer
    //clusterSizeX1.push_back(csInLayer[0]);
    //clusterSizeY2.push_back(csInLayer[1]);
    //clusterSizeY3.push_back(csInLayer[2]);
    //clusterSizeX4.push_back(csInLayer[3]);
    //clusterSizeX.push_back(csOrientation[0]);
    //clusterSizeY.push_back(csOrientation[1]);
    //clusterSizeAll.push_back(csAll);
  //}
  
  
  ///// fill number of unfired strips between 2 cluster in different layers
  //for(std::map<rpc::mId, int>::iterator it = somevars.roXClusterDistance.begin(); it != somevars.roXClusterDistance.end(); it++)
    //xClusterDistance.push_back(it->second);
  //for(std::map<rpc::mId, int>::iterator it = somevars.roYClusterDistance.begin(); it != somevars.roYClusterDistance.end(); it++)
    //yClusterDistance.push_back(it->second);

  
  ///// fill track information
  //for(rpc::roTrackPairIdx::iterator it = somevars.roXTrackPairIdx.begin(); it != somevars.roXTrackPairIdx.end(); it++)
    //nXTracks.push_back(it->second.size());
  //for(rpc::roTrackPairIdx::iterator it = somevars.roYTrackPairIdx.begin(); it != somevars.roYTrackPairIdx.end(); it++)
    //nYTracks.push_back(it->second.size());
  //for(rpc::roSpuriousIndex::iterator it = somevars.roSpuIdx.begin(); it != somevars.roSpuIdx.end(); it++)
  //{
    //nSpuriousX1.push_back(it->second[0].size());
    //nSpuriousY2.push_back(it->second[1].size());
    //nSpuriousY3.push_back(it->second[2].size());
    //nSpuriousX4.push_back(it->second[3].size());
    //nSpuriousX.push_back(it->second[0].size()+it->second[3].size());
    //nSpuriousY.push_back(it->second[1].size()+it->second[2].size());
    //nSpuriousAll.push_back(it->second[0].size()+it->second[1].size()+it->second[2].size()+it->second[3].size());
  //}
  
  
  //for(rpc::roTrackPairIdx::iterator it = somevars.roXForcedTrackPairIdx.begin(); it != somevars.roXForcedTrackPairIdx.end(); it++)
    //nXForcedTracks.push_back(it->second.size());
  //for(rpc::roTrackPairIdx::iterator it = somevars.roYForcedTrackPairIdx.begin(); it != somevars.roYForcedTrackPairIdx.end(); it++)
    //nYForcedTracks.push_back(it->second.size());
//}



void EnhancedTreeVars::fillTreeVars(EnhancedVars& somevars)
{
  clearContainers();
  
  eventId = somevars.eventId;
  runNumber = somevars.runNumber;
  fileNumber = somevars.fileNumber;
  site = somevars.getSite();
  detector = somevars.getDetector();
  triggerNumber = somevars.getTriggerNumber();
  triggerType = somevars.getTriggerType();
  triggerTimeSec = somevars.getTriggerTimeSec();
  triggerTimeNanoSec = somevars.getTriggerTimeNanoSec();
  nHitsRpc = somevars.getNHitsRpc();
  allHitsInterior = somevars.allHitsInterior;
  allHitsInterior2 = somevars.allHitsInterior2;
  nModules = somevars.nModules;
  
  
  
  for(rpc::mFromRot::iterator it = somevars.mRpcFromRot.begin(); it != somevars.mRpcFromRot.end(); it++)
  {
    /// fill if the trigger is from ROT
    fromRot.push_back(it->second);
  }
  
  
  for(rpc::mReadoutClusters::iterator it = somevars.mRpcCluster.begin(); it != somevars.mRpcCluster.end(); it++)
  {
    row.push_back(it->first.first);
    col.push_back(it->first.second);
    
    /// fill number of fired layers in an event
    unsigned int nlyr = 0;
    /// get cluster size of each layer
    std::vector<unsigned int> csInLayer[4];
    std::vector<unsigned int> csOrientation[2];
    std::vector<unsigned int> csAll;
    for(unsigned int l = 0; l < 4; l++)
    {
      if(it->second[l].size()) nlyr++;
      for(unsigned int c = 0; c < it->second[l].size(); c++)
      {
        csInLayer[l].push_back(it->second[l][c].size());
        
        csAll.push_back(it->second[l][c].size());
        
        if((l%3)==0) // x layers
          csOrientation[0].push_back(it->second[l][c].size());
        else         // y layers
          csOrientation[1].push_back(it->second[l][c].size());
      }
    }
    nLayers.push_back(nlyr);
    
    /// fill number of clusters for each layer and whole module
    nClustersX1.push_back(it->second[0].size());
    nClustersY2.push_back(it->second[1].size());
    nClustersY3.push_back(it->second[2].size());
    nClustersX4.push_back(it->second[3].size());
    nClusters.push_back(it->second[0].size()+it->second[1].size()+it->second[2].size()+it->second[3].size());
    
    /// fill cluster size of each layer
    clusterSizeX1.push_back(csInLayer[0]);
    clusterSizeY2.push_back(csInLayer[1]);
    clusterSizeY3.push_back(csInLayer[2]);
    clusterSizeX4.push_back(csInLayer[3]);
    clusterSizeX.push_back(csOrientation[0]);
    clusterSizeY.push_back(csOrientation[1]);
    clusterSizeAll.push_back(csAll);
    /// It is wierd that the fraction of single cluster per layer is lager
    /// in 3-fold than 4-fold. Thus these 2 debug variables.
    if(nlyr == 3) clusterSize3Fold.push_back(csAll);
    if(nlyr == 4) clusterSize4Fold.push_back(csAll);
  }
  
  
  /// fill number of unfired strips between 2 cluster in different layers
  for(std::map<rpc::mId, int>::iterator it = somevars.roXClusterDistance.begin(); it != somevars.roXClusterDistance.end(); it++)
    xClusterDistance.push_back(it->second);
  for(std::map<rpc::mId, int>::iterator it = somevars.roYClusterDistance.begin(); it != somevars.roYClusterDistance.end(); it++)
    yClusterDistance.push_back(it->second);

  
  /// fill track information
  for(rpc::roTrackPairIdx::iterator it = somevars.roXTrackPairIdx.begin(); it != somevars.roXTrackPairIdx.end(); it++)
    nXTracks.push_back(it->second.size());
  for(rpc::roTrackPairIdx::iterator it = somevars.roYTrackPairIdx.begin(); it != somevars.roYTrackPairIdx.end(); it++)
    nYTracks.push_back(it->second.size());
  for(rpc::roSpuriousIndex::iterator it = somevars.roSpuIdx.begin(); it != somevars.roSpuIdx.end(); it++)
  {
    nSpuriousX1.push_back(it->second[0].size());
    nSpuriousY2.push_back(it->second[1].size());
    nSpuriousY3.push_back(it->second[2].size());
    nSpuriousX4.push_back(it->second[3].size());
    nSpuriousX.push_back(it->second[0].size()+it->second[3].size());
    nSpuriousY.push_back(it->second[1].size()+it->second[2].size());
    nSpuriousAll.push_back(it->second[0].size()+it->second[1].size()+it->second[2].size()+it->second[3].size());
  }
  
  
  for(rpc::roTrackPairIdx::iterator it = somevars.roXForcedTrackPairIdx.begin(); it != somevars.roXForcedTrackPairIdx.end(); it++)
    nXForcedTracks.push_back(it->second.size());
  for(rpc::roTrackPairIdx::iterator it = somevars.roYForcedTrackPairIdx.begin(); it != somevars.roYForcedTrackPairIdx.end(); it++)
    nYForcedTracks.push_back(it->second.size());


  /// reconstructed point
  for(rpc::mReadoutClusters::iterator it = somevars.mRpcCluster.begin(); it != somevars.mRpcCluster.end(); it++)
  {
    rpc::mTrackCMS vecx = somevars.roXTrackCMS[it->first];
    rpc::mTrackCMS vecy = somevars.roYTrackCMS[it->first];
    rpc::mTrackCMS vecfx = somevars.roXForcedTrackCMS[it->first];
    rpc::mTrackCMS vecfy = somevars.roYForcedTrackCMS[it->first];
    
    vector<double> mxrec;
    vector<double> myrec;
    vector<double> mfxrec;
    vector<double> mfyrec;
    
    for(unsigned int c1 = 0; c1 < vecx.size(); c1++)
      for(unsigned int c2 = 0; c2 < vecy.size(); c2++)
      {
        mxrec.push_back(vecx[c1]);
        myrec.push_back(vecy[c2]);
      }
    xrec.push_back(mxrec);
    yrec.push_back(myrec);
    
    for(unsigned int c1 = 0; c1 < vecfx.size(); c1++)
      for(unsigned int c2 = 0; c2 < vecfy.size(); c2++)
      {
        mfxrec.push_back(vecfx[c1]);
        mfyrec.push_back(vecfy[c2]);
      }
    fxrec.push_back(mfxrec);
    fyrec.push_back(mfyrec);
  }
}








void EnhancedTreeVars::fillTreeVars(CsNVars& somevars)
{
  clearContainers();
  
  eventId = somevars.eventId;
  runNumber = somevars.runNumber;
  fileNumber = somevars.fileNumber;
  site = somevars.getSite();
  detector = somevars.getDetector();
  triggerNumber = somevars.getTriggerNumber();
  triggerType = somevars.getTriggerType();
  triggerTimeSec = somevars.getTriggerTimeSec();
  triggerTimeNanoSec = somevars.getTriggerTimeNanoSec();
  nHitsRpc = somevars.getNHitsRpc();
  allHitsInterior = somevars.allHitsInterior;
  allHitsInterior2 = somevars.allHitsInterior2;
  nModules = somevars.nModules;
  
  
  
  for(rpc::mFromRot::iterator it = somevars.mRpcFromRot.begin(); it != somevars.mRpcFromRot.end(); it++)
  {
    /// fill if the trigger is from ROT
    fromRot.push_back(it->second);
  }
  
  
  for(rpc::mReadoutClusters::iterator it = somevars.mRpcCluster.begin(); it != somevars.mRpcCluster.end(); it++)
  {
    row.push_back(it->first.first);
    col.push_back(it->first.second);
    
    /// fill number of fired layers in an event
    unsigned int nlyr = 0;
    /// get cluster size of each layer
    std::vector<unsigned int> csInLayer[4];
    std::vector<unsigned int> csOrientation[2];
    std::vector<unsigned int> csAll;
    for(unsigned int l = 0; l < 4; l++)
    {
      if(it->second[l].size()) nlyr++;
      for(unsigned int c = 0; c < it->second[l].size(); c++)
      {
        csInLayer[l].push_back(it->second[l][c].size());
        
        csAll.push_back(it->second[l][c].size());
        
        if((l%3)==0) // x layers
          csOrientation[0].push_back(it->second[l][c].size());
        else         // y layers
          csOrientation[1].push_back(it->second[l][c].size());
      }
    }
    nLayers.push_back(nlyr);
    
    /// fill number of clusters for each layer and whole module
    nClustersX1.push_back(it->second[0].size());
    nClustersY2.push_back(it->second[1].size());
    nClustersY3.push_back(it->second[2].size());
    nClustersX4.push_back(it->second[3].size());
    nClusters.push_back(it->second[0].size()+it->second[1].size()+it->second[2].size()+it->second[3].size());
    
    /// fill cluster size of each layer
    clusterSizeX1.push_back(csInLayer[0]);
    clusterSizeY2.push_back(csInLayer[1]);
    clusterSizeY3.push_back(csInLayer[2]);
    clusterSizeX4.push_back(csInLayer[3]);
    clusterSizeX.push_back(csOrientation[0]);
    clusterSizeY.push_back(csOrientation[1]);
    clusterSizeAll.push_back(csAll);
    /// It is wierd that the fraction of single cluster per layer is lager
    /// in 3-fold than 4-fold. Thus these 2 debug variables.
    if(nlyr == 3) clusterSize3Fold.push_back(csAll);
    if(nlyr == 4) clusterSize4Fold.push_back(csAll);
  }
  
  
  /// fill number of unfired strips between 2 cluster in different layers
  for(std::map<rpc::mId, int>::iterator it = somevars.roXClusterDistance.begin(); it != somevars.roXClusterDistance.end(); it++)
    xClusterDistance.push_back(it->second);
  for(std::map<rpc::mId, int>::iterator it = somevars.roYClusterDistance.begin(); it != somevars.roYClusterDistance.end(); it++)
    yClusterDistance.push_back(it->second);

  
  /// fill track information
  for(rpc::roTrackPairIdx::iterator it = somevars.roXTrackPairIdx.begin(); it != somevars.roXTrackPairIdx.end(); it++)
    nXTracks.push_back(it->second.size());
  for(rpc::roTrackPairIdx::iterator it = somevars.roYTrackPairIdx.begin(); it != somevars.roYTrackPairIdx.end(); it++)
    nYTracks.push_back(it->second.size());
  for(rpc::roSpuriousIndex::iterator it = somevars.roSpuIdx.begin(); it != somevars.roSpuIdx.end(); it++)
  {
    nSpuriousX1.push_back(it->second[0].size());
    nSpuriousY2.push_back(it->second[1].size());
    nSpuriousY3.push_back(it->second[2].size());
    nSpuriousX4.push_back(it->second[3].size());
    nSpuriousX.push_back(it->second[0].size()+it->second[3].size());
    nSpuriousY.push_back(it->second[1].size()+it->second[2].size());
    nSpuriousAll.push_back(it->second[0].size()+it->second[1].size()+it->second[2].size()+it->second[3].size());
  }
  
  
  for(rpc::roTrackPairIdx::iterator it = somevars.roXForcedTrackPairIdx.begin(); it != somevars.roXForcedTrackPairIdx.end(); it++)
    nXForcedTracks.push_back(it->second.size());
  for(rpc::roTrackPairIdx::iterator it = somevars.roYForcedTrackPairIdx.begin(); it != somevars.roYForcedTrackPairIdx.end(); it++)
    nYForcedTracks.push_back(it->second.size());
}








void EnhancedTreeVars::clearContainers()
{
  /// empty the containers
  row.clear();
  col.clear();
  nLayers.clear();
  fromRot.clear();
  nClustersX1.clear();
  nClustersY2.clear();
  nClustersY3.clear();
  nClustersX4.clear();
  nClusters.clear();
  clusterSizeX1.clear();
  clusterSizeY2.clear();
  clusterSizeY3.clear();
  clusterSizeX4.clear();
  clusterSizeX.clear();
  clusterSizeY.clear();
  clusterSizeAll.clear();
  clusterSize3Fold.clear();
  clusterSize4Fold.clear();
  
  xClusterDistance.clear();
  yClusterDistance.clear();
  
  nXTracks.clear();
  nYTracks.clear();
  nSpuriousX1.clear();
  nSpuriousY2.clear();
  nSpuriousY3.clear();
  nSpuriousX4.clear();
  nSpuriousX.clear();
  nSpuriousY.clear();
  nSpuriousAll.clear();
  
  nXForcedTracks.clear();
  nYForcedTracks.clear();
  
  xrec.clear();
  yrec.clear();
  fxrec.clear();
  fyrec.clear();
}
