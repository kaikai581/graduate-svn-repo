#include <algorithm>
#include <iostream>
#include "EnhancedTreeVars.hpp"
#include "EnhancedVars.hpp"


using namespace std;

unsigned int EnhancedVars::eventId(0);
unsigned int EnhancedVars::runNumber(0);
unsigned int EnhancedVars::fileNumber(0);

EnhancedVars::EnhancedVars(PerCalibReadoutHeader* rh) : RawVars(rh)
{
  /// set title
  varTitle = "enhanced";
  
  clusterizeModule();
  findRoClusterCMS();
  findTracksSpurious();
  findClusterDistance();
  findRoTrackCMS();
}



void EnhancedVars::clusterizeModule()
{
  nModules = 0;
  
  for(rpc::mEvent::iterator it = mRpcReadout.begin(); it != mRpcReadout.end(); it++)
  {
    rpc::mModuleClusters modClusters(4, std::vector<std::vector<unsigned int> >(0, std::vector<unsigned int>()));
    for(unsigned int l = 0; l < 4; l++)
      modClusters[l] = clusterizeLayer(it->second[l]);

    /// it->first is module ID
    mRpcCluster[it->first] = modClusters;
    
    /// fill number of modules fired in a readout
    nModules++;
  }
}



rpc::mLayerClusters EnhancedVars::clusterizeLayer(const rpc::mStripsInLayer& stripInLayer)
{
  rpc::mLayerClusters layerClusters(0, std::vector<unsigned int>());
  
  
  std::vector<unsigned int> singleCluster;
  for(unsigned int s = 0; s < stripInLayer.size(); s++)
  {
    singleCluster.push_back(stripInLayer[s]);

    if((stripInLayer[s+1]-stripInLayer[s]!=1) || // noncontiguous strips
       (s == stripInLayer.size()-1)) { // last strip
      layerClusters.push_back(singleCluster);
      singleCluster.clear();
    }
    
  }
  
  return layerClusters;
}



void EnhancedVars::printClusters()
{
  for(rpc::mReadoutClusters::iterator it = mRpcCluster.begin(); it != mRpcCluster.end(); it++)
  {
    /// print clusters
    cout << "   clusters: ";
    for(unsigned int l = 0; l < 4; l++)
    {
      cout << "(";
      for(unsigned int c = 0; c < it->second[l].size(); c++)
      {
        cout << "(";
        for(unsigned int s = 0; s < it->second[l][c].size(); s++)
        {
          cout << it->second[l][c][s];
          cout << ((s == (it->second[l][c].size()-1)) ? "":",");
        }
        cout << ")";
      }
      cout << ")";
    }
    cout << endl;
    
    /// print track CMS
    printModTrackCMS(it->first);
  }
}




void EnhancedVars::printClusterCMS()
{
  for(rpc::roClusterCMS::iterator it = roClCMS.begin(); it != roClCMS.end(); it++)
  {
    /// print clusters CMS
    cout << "cluster CMS: ";
    for(unsigned int l = 0; l < 4; l++)
    {
      cout << "(";
      for(unsigned int c = 0; c < it->second[l].size(); c++)
      {
        cout << "(";
        cout << it->second[l][c];
        cout << ")";
      }
      cout << ")";
    }
    cout << endl;
  }
}




void EnhancedVars::printClusterDistance()
{
  cout << "x cluster distance: ";
  for(std::map<rpc::mId, int>::iterator it = roXClusterDistance.begin(); it != roXClusterDistance.end(); it++)
    cout << it->second << " ";
  cout << endl;
  cout << "y cluster distance: ";
  for(std::map<rpc::mId, int>::iterator it = roYClusterDistance.begin(); it != roYClusterDistance.end(); it++)
    cout << it->second << " ";
  cout << endl;
}




void EnhancedVars::findTracksSpurious()
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
    roXForcedTrackPairIdx[it->first] = pairForcedClusters(it->second[0], it->second[3]);
    roYForcedTrackPairIdx[it->first] = pairForcedClusters(it->second[1], it->second[2]);
  }
}



rpc::mTrackPairIdx EnhancedVars::findPairTracksSpurious(const rpc::mLayerClusters& lc1, const rpc::mLayerClusters& lc2, rpc::mLayerClusters& copylc1, rpc::mLayerClusters& copylc2)
//rpc::mTrackPairIdx EnhancedVars::findPairTracksSpurious(const rpc::mLayerClusters& lc1, const rpc::mLayerClusters& lc2)
{
  rpc::mTrackPairIdx pairIdx(0, pair<unsigned int, unsigned int>());
  
  //rpc::mLayerClusters copylc1 = lc1;
  //rpc::mLayerClusters copylc2 = lc2;
  
  
  /// record the cluster ID after it is paired
  vector<unsigned int> pairedc1;
  vector<unsigned int> pairedc2;
  
  for(unsigned int lc1Idx = 0; lc1Idx < lc1.size(); lc1Idx++)
    for(unsigned int lc2Idx = 0; lc2Idx < lc2.size(); lc2Idx++)
    {
      vector<unsigned int> c1 = lc1[lc1Idx];
      vector<unsigned int> c2 = lc2[lc2Idx];
      
      /// container for stroing common strips
      //vector<unsigned int> v(8);
      //vector<unsigned int>::iterator it;
      
      //it = set_intersection(c1.begin(), c1.end(), c2.begin(), c2.end(), v.begin());
      
      //if(int(it-v.begin()))
      if(nStripsInCommon(c1, c2))
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
        
        /// erase paired clusters
        /// only if they are in the paired lists
        itpair1 = find(pairedc1.begin(), pairedc1.end(), lc1Idx);
        rpc::mLayerClusters::iterator it = find(copylc1.begin(), copylc1.end(), c1);
        if(itpair1 != pairedc1.end() && it != copylc1.end()) copylc1.erase(it);
        
        itpair2 = find(pairedc2.begin(), pairedc2.end(), lc2Idx);
        it = find(copylc2.begin(), copylc2.end(), c2);
        if(itpair2 != pairedc2.end() && it != copylc2.end()) copylc2.erase(it);
      }
    }
  //cout << "bottom number of spurios hits: " << copylc1.size() << endl;
  //cout << "   top number of spurios hits: " << copylc2.size() << endl;

  return pairIdx;
}








void EnhancedVars::findRoClusterCMS()
{
  for(rpc::mReadoutClusters::iterator it = mRpcCluster.begin(); it != mRpcCluster.end(); it++)
  {
    roClCMS[it->first] = rpc::mClusterCMS(4, rpc::lClusterCMS());
    
    for(unsigned int l = 0; l < 4; l++)
      for(unsigned int c = 0; c < it->second[l].size(); c++)
        roClCMS[it->first][l].push_back(findClusterUnitCMS(it->second[l][c]));
  }
}








void EnhancedVars::findRoTrackCMS()
{
  for(rpc::roTrackPairIdx::iterator it = roXTrackPairIdx.begin(); it != roXTrackPairIdx.end(); it++)
  {
    /// track definition: line up only
    roXTrackCMS[it->first] = findPairedLayerTrackCMS(roXTrackPairIdx[it->first], roClCMS[it->first][0], roClCMS[it->first][3]);
    roYTrackCMS[it->first] = findPairedLayerTrackCMS(roYTrackPairIdx[it->first], roClCMS[it->first][1], roClCMS[it->first][2]);
    /// track definition: line up + off by one strip
    roXForcedTrackCMS[it->first] = findPairedLayerTrackCMS(roXForcedTrackPairIdx[it->first], roClCMS[it->first][0], roClCMS[it->first][3]);
    roYForcedTrackCMS[it->first] = findPairedLayerTrackCMS(roYForcedTrackPairIdx[it->first], roClCMS[it->first][1], roClCMS[it->first][2]);
  }
}










rpc::mTrackPairIdx EnhancedVars::pairForcedClusters(rpc::mLayerClusters lc1,
                   rpc::mLayerClusters lc2)
{
  /// return value
  rpc::mTrackPairIdx pairIdx(0, pair<unsigned int, unsigned int>());
  
  
  /// If one layer has no hits, copy the hits from the paired counterpart to it.
  if(!lc1.size()) lc1 = lc2;
  if(!lc2.size()) lc2 = lc1;
  
  
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







void EnhancedVars::findClusterDistance()
{
  for(rpc::mReadoutClusters::iterator it = mRpcCluster.begin(); it != mRpcCluster.end(); it++)
  {
    /// x strips
    /// applicable only if the 2 paired layers have exactly 1 cluster each
    if(it->second[0].size() != 1 || it->second[3].size() != 1)
      roXClusterDistance[it->first] = -2;
    else
      roXClusterDistance[it->first] = findPairClusterDistance(it->second[0][0], it->second[3][0]);

    /// y strips
    if(it->second[1].size() != 1 || it->second[2].size() != 1)
      roYClusterDistance[it->first] = -2;
    else
      roYClusterDistance[it->first] = findPairClusterDistance(it->second[1][0], it->second[2][0]);
  }
}






double EnhancedVars::findClusterUnitCMS(const rpc::clusterUnit& cu)
{
  double clusterSize = cu.size();
  double cms = 0.;
  
  for(unsigned int s = 0; s < cu.size(); s++) cms += cu[s];
  
  return cms/clusterSize;
}









int EnhancedVars::findPairClusterDistance(const rpc::clusterUnit& c1, const rpc::clusterUnit& c2)
{
  if(nStripsInCommon(c1, c2)) return -1;
  
  int c1begin = c1[0];
  int c1end   = c1[c1.size()-1];
  int c2begin = c2[0];
  int c2end   = c2[c2.size()-1];
  
  return ((c1begin>c2end)? (c1begin-c2end-1) : (c2begin-c1end-1));
}




rpc::mTrackCMS EnhancedVars::findPairedLayerTrackCMS(const rpc::mTrackPairIdx& pairIdx, rpc::lClusterCMS ccms1, rpc::lClusterCMS ccms2)
{
  /// return value
  rpc::mTrackCMS vecTrackCMS;
  
  /// If one of the layers has no hits, assign the other layer's hits to
  /// the unfired layer to accommodate to 3-fold reconstruction.
  if(!ccms1.size()) return ccms2;
  if(!ccms2.size()) return ccms1;
  
  for(unsigned int p = 0; p < pairIdx.size(); p++)
  {
    vecTrackCMS.push_back((ccms1[pairIdx[p].first]+ccms2[pairIdx[p].second])/2);
  }
  
  return vecTrackCMS;
}



unsigned int EnhancedVars::nStripsInCommon(const rpc::clusterUnit& c1, const rpc::clusterUnit& c2)
{
  rpc::clusterUnit v(8);
  rpc::clusterUnit::iterator it;
  
  it = set_intersection(c1.begin(), c1.end(), c2.begin(), c2.end(), v.begin());
  
  return (unsigned int)(it-v.begin());
}




void EnhancedVars::printTracks()
{
  for(rpc::roTrackPairIdx::iterator it = roXTrackPairIdx.begin(); it != roXTrackPairIdx.end(); it++)
  {
    /// print number of x tracks
    cout << "# x tracks: ";
    cout << it->second.size() << endl;
  }
  for(rpc::roTrackPairIdx::iterator it = roYTrackPairIdx.begin(); it != roYTrackPairIdx.end(); it++)
  {
    /// print number of y tracks
    cout << "# y tracks: ";
    cout << it->second.size() << endl;
  }
}






void EnhancedVars::printForcedTracks()
{
  for(rpc::roTrackPairIdx::iterator it = roXForcedTrackPairIdx.begin(); it != roXForcedTrackPairIdx.end(); it++)
  {
    /// print number of x tracks
    cout << "# forced x tracks: ";
    cout << it->second.size() << endl;
  }
  for(rpc::roTrackPairIdx::iterator it = roYForcedTrackPairIdx.begin(); it != roYForcedTrackPairIdx.end(); it++)
  {
    /// print number of y tracks
    cout << "# forced y tracks: ";
    cout << it->second.size() << endl;
  }
}




void EnhancedVars::printModTrackCMS(rpc::mId id)
{
  rpc::mTrackCMS x = roXTrackCMS[id];
  rpc::mTrackCMS y = roYTrackCMS[id];
  rpc::mTrackCMS fx = roXForcedTrackCMS[id];
  rpc::mTrackCMS fy = roYForcedTrackCMS[id];
  
  cout << "       x track CMS: ";
  for(unsigned int t = 0; t < x.size(); t++) cout << x[t] << " ";
  cout << endl;
  
  cout << "       y track CMS: ";
  for(unsigned int t = 0; t < y.size(); t++) cout << y[t] << " ";
  cout << endl;
  
  cout << "forced x track CMS: ";
  for(unsigned int t = 0; t < fx.size(); t++) cout << fx[t] << " ";
  cout << endl;
  
  cout << "forced y track CMS: ";
  for(unsigned int t = 0; t < fy.size(); t++) cout << fy[t] << " ";
  cout << endl;
}





void EnhancedVars::printSpurios()
{
  cout << "spurious hits" << endl;
  for(rpc::roSpuriousIndex::iterator it = roSpuIdx.begin(); it != roSpuIdx.end(); it++)
  {
    for(unsigned int i = 0; i < 4; i++)
    {
      if(it->second[i].size())
      {
        cout << "layer " << i+1 << " index: ";
        for(unsigned int j = 0; j < it->second[i].size(); j++)
          cout << it->second[i][j] << " ";
        cout << endl;
      }
    }
  }
}



void EnhancedVars::fillTreeVars(EnhancedTreeVars& t)
{
  t.eventId = eventId;
  t.runNumber = runNumber;
  t.fileNumber = fileNumber;
  t.site = getSite();
  t.detector = getDetector();
  t.triggerNumber = getTriggerNumber();
  t.triggerType = getTriggerType();
  t.triggerTimeSec = getTriggerTimeSec();
  t.triggerTimeNanoSec = getTriggerTimeNanoSec();
  t.nHitsRpc = getNHitsRpc();
  
  /// empty the containers
  t.row.clear();
  t.col.clear();
  t.nLayers.clear();
  t.fromRot.clear();
  //t.nClusters.clear();
  t.nClustersX1.clear();
  t.nClustersY2.clear();
  t.nClustersY3.clear();
  t.nClustersX4.clear();
  t.clusterSizeX1.clear();
  t.clusterSizeY2.clear();
  t.clusterSizeY3.clear();
  t.clusterSizeX4.clear();
  t.clusterSizeX.clear();
  t.clusterSizeY.clear();
  t.clusterSizeAll.clear();
  
  t.xClusterDistance.clear();
  t.yClusterDistance.clear();
  
  t.nXTracks.clear();
  t.nYTracks.clear();
  t.nSpuriousX1.clear();
  t.nSpuriousY2.clear();
  t.nSpuriousY3.clear();
  t.nSpuriousX4.clear();
  t.nSpuriousX.clear();
  t.nSpuriousY.clear();
  t.nSpuriousAll.clear();
  
  
  for(rpc::mFromRot::iterator it = mRpcFromRot.begin(); it != mRpcFromRot.end(); it++)
  {
    /// fill if the trigger is from ROT
    t.fromRot.push_back(it->second);
  }
  
  
  for(rpc::mReadoutClusters::iterator it = mRpcCluster.begin(); it != mRpcCluster.end(); it++)
  {
    t.row.push_back(it->first.first);
    t.col.push_back(it->first.second);
    
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
    t.nLayers.push_back(nlyr);
    
    /// fill number of clusters for each layer
    t.nClustersX1.push_back(it->second[0].size());
    t.nClustersY2.push_back(it->second[1].size());
    t.nClustersY3.push_back(it->second[2].size());
    t.nClustersX4.push_back(it->second[3].size());
    
    /// fill cluster size of each layer
    t.clusterSizeX1.push_back(csInLayer[0]);
    t.clusterSizeY2.push_back(csInLayer[1]);
    t.clusterSizeY3.push_back(csInLayer[2]);
    t.clusterSizeX4.push_back(csInLayer[3]);
    t.clusterSizeX.push_back(csOrientation[0]);
    t.clusterSizeY.push_back(csOrientation[1]);
    t.clusterSizeAll.push_back(csAll);
  }
  
  
  /// fill number of unfired strips between 2 cluster in different layers
  for(std::map<rpc::mId, int>::iterator it = roXClusterDistance.begin(); it != roXClusterDistance.end(); it++)
    t.xClusterDistance.push_back(it->second);
  for(std::map<rpc::mId, int>::iterator it = roYClusterDistance.begin(); it != roYClusterDistance.end(); it++)
    t.yClusterDistance.push_back(it->second);

  
  /// fill track information
  for(rpc::roTrackPairIdx::iterator it = roXTrackPairIdx.begin(); it != roXTrackPairIdx.end(); it++)
    t.nXTracks.push_back(it->second.size());
  for(rpc::roTrackPairIdx::iterator it = roYTrackPairIdx.begin(); it != roYTrackPairIdx.end(); it++)
    t.nYTracks.push_back(it->second.size());
  for(rpc::roSpuriousIndex::iterator it = roSpuIdx.begin(); it != roSpuIdx.end(); it++)
  {
    t.nSpuriousX1.push_back(it->second[0].size());
    t.nSpuriousY2.push_back(it->second[1].size());
    t.nSpuriousY3.push_back(it->second[2].size());
    t.nSpuriousX4.push_back(it->second[3].size());
    t.nSpuriousX.push_back(it->second[0].size()+it->second[3].size());
    t.nSpuriousY.push_back(it->second[1].size()+it->second[2].size());
    t.nSpuriousAll.push_back(it->second[0].size()+it->second[1].size()+it->second[2].size()+it->second[3].size());
  }
  
}
