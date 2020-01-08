#ifndef ENHANCEDTREEVARS_HPP
#define ENHANCEDTREEVARS_HPP

#include <vector>
#include "rpcnamespace.hpp"

class TTree;
class CsNVars;
class EnhancedVars;

class EnhancedTreeVars
{
public:

  /// tree variables
  unsigned int eventId;
  unsigned int runNumber;
  unsigned int fileNumber;
  unsigned int site;
  unsigned int detector;
  unsigned int triggerNumber;
  unsigned int triggerType;
  unsigned int triggerTimeSec;
  unsigned int triggerTimeNanoSec;
  unsigned int nHitsRpc;
  bool         allHitsInterior;
  bool         allHitsInterior2;
  unsigned int nModules;
  
  std::vector<unsigned int> row;
  std::vector<unsigned int> col;
  std::vector<unsigned int> nLayers;
  std::vector<bool>         fromRot;
  
  std::vector<unsigned int> nClusters;
  std::vector<unsigned int> nClustersX1;
  std::vector<unsigned int> nClustersY2;
  std::vector<unsigned int> nClustersY3;
  std::vector<unsigned int> nClustersX4;
  
  std::vector<std::vector<unsigned int> > clusterSizeX1;
  std::vector<std::vector<unsigned int> > clusterSizeY2;
  std::vector<std::vector<unsigned int> > clusterSizeY3;
  std::vector<std::vector<unsigned int> > clusterSizeX4;
  std::vector<std::vector<unsigned int> > clusterSizeX;
  std::vector<std::vector<unsigned int> > clusterSizeY;
  std::vector<std::vector<unsigned int> > clusterSizeAll;
  /// It is wierd that the fraction of single cluster per layer is lager
  /// in 3-fold than 4-fold. Thus these 2 debug variables.
  std::vector<std::vector<unsigned int> > clusterSize3Fold;
  std::vector<std::vector<unsigned int> > clusterSize4Fold;
  
  /// number of unfired strips between 2 clusters from different layers
  std::vector<int> xClusterDistance;
  std::vector<int> yClusterDistance;
  
  std::vector<unsigned int> nXTracks;
  std::vector<unsigned int> nYTracks;
  std::vector<unsigned int> nSpuriousX1;
  std::vector<unsigned int> nSpuriousY2;
  std::vector<unsigned int> nSpuriousY3;
  std::vector<unsigned int> nSpuriousX4;
  std::vector<unsigned int> nSpuriousX;
  std::vector<unsigned int> nSpuriousY;
  std::vector<unsigned int> nSpuriousAll;
  
  /// same variables as above but including clusters which are off by 1
  std::vector<unsigned int> nXForcedTracks;
  std::vector<unsigned int> nYForcedTracks;
  //std::vector<unsigned int> nOffSpuriousX1;
  //std::vector<unsigned int> nOffSpuriousY2;
  //std::vector<unsigned int> nOffSpuriousY3;
  //std::vector<unsigned int> nOffSpuriousX4;
  //std::vector<unsigned int> nOffSpuriousX;
  //std::vector<unsigned int> nOffSpuriousY;
  //std::vector<unsigned int> nOffSpuriousAll;
  
  
  /// Finally, reconstruction!
  /// In the first pair of coordinates, clusters off by 1 are not included.
  std::vector<std::vector<double> > xrec;
  std::vector<std::vector<double> > yrec;
  /// In the second pair of coordinates, clusters off by 1 are included.
  std::vector<std::vector<double> > fxrec;
  std::vector<std::vector<double> > fyrec;
  
  EnhancedTreeVars(){};
  EnhancedTreeVars(TTree&);
  ~EnhancedTreeVars(){};
  
  //template<class T> void fillTreeVars(const T&);
  void fillTreeVars(EnhancedVars&);
  void fillTreeVars(CsNVars&);
  
  
  
private:
  void clearContainers();
};



#endif
