#ifndef CS2VARS_HPP
#define CS2VARS_HPP


#include "EnhancedVars.hpp"
#include "rpcnamespace.hpp"

//class EnhancedVars;

class CsNVars : public EnhancedVars
{
public:
  CsNVars(){};
  CsNVars(EnhancedVars&);
  CsNVars(const EnhancedVars&, unsigned int cutSize);
  
  
  /*
   * Loop over all clusters in all modules in a readout and discard any cluster
   * with size <= a specified size
   */
  void filterClusters(EnhancedVars, unsigned int);
  unsigned int nModules;
  
  
  /// mRpcClusterFiltered is used for storing filtered data.
  /// If a module still has >=3 layers after filter, it will be in turn put in
  /// mRpcCluster
  rpc::mReadoutClusters   mRpcClusterFiltered;
  
  /// store if the layer has a true hardware hit
  rpc::roHitLayerHardware roHitLayerRaw;



private:
  unsigned int       getNLayers(const rpc::mModuleClusters&);
  void               findTracksSpurious();
  rpc::mTrackPairIdx pairForcedClusters(rpc::mLayerClusters, rpc::mLayerClusters, bool, bool);
};


#endif
