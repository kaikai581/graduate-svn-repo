#ifndef ENHANCEDVARS_HPP
#define ENHANCEDVARS_HPP


#include "RawVars.hpp"

class EnhancedTreeVars;

class EnhancedVars : public RawVars
{
public:


  static unsigned int eventId;
  static unsigned int runNumber;
  static unsigned int fileNumber;
  unsigned int        nModules;
  
  
  rpc::mReadoutClusters mRpcCluster;
  
  /// gather strips to form clusters in a whole readout
  rpc::roSpuriousIndex  roSpuIdx;
  rpc::roTrackPairIdx   roXTrackPairIdx;
  rpc::roTrackPairIdx   roYTrackPairIdx;
  
  /// These variables are the same as previous 2 for 4-fold events.
  /// For 3-fold, strips with IDs the same as those in the unpaired layer
  /// are fired artificially
  rpc::roTrackPairIdx   roXForcedTrackPairIdx;
  rpc::roTrackPairIdx   roYForcedTrackPairIdx;
  
  /*
   * number of unfired strips between 2 clusters of different layers
   * only applicable to events with exactly 1 cluster per layer
   * return value:
   * 0-6: number of unfired strips between the 2 clusters
   * -1 : the two clusters overlap
   * -2 : non-applicable events
   */
  std::map<rpc::mId, int> roXClusterDistance;
  std::map<rpc::mId, int> roYClusterDistance;
  
  /// cluster center of mass variables for reconstruction
  rpc::roClusterCMS       roClCMS;
  
  /// track center of mass
  rpc::roTrackCMS         roXTrackCMS;
  rpc::roTrackCMS         roYTrackCMS;
  rpc::roTrackCMS         roXForcedTrackCMS;
  rpc::roTrackCMS         roYForcedTrackCMS;
  

  EnhancedVars(){};
  EnhancedVars(PerCalibReadoutHeader*);
  virtual ~EnhancedVars(){};
  

  void printClusterCMS();
  void printClusters();
  void printClusterDistance();
  void printModTrackCMS(rpc::mId);
  void printTracks();
  void printForcedTracks();
  void printSpurios();
  void fillTreeVars(EnhancedTreeVars&);
  
  rpc::mReadoutClusters getRoInCluster() {return mRpcCluster;};


protected:

  
  void                        clusterizeModule();
  rpc::mTrackCMS              findPairedLayerTrackCMS(const rpc::mTrackPairIdx&, rpc::lClusterCMS, rpc::lClusterCMS);
  /// methonds of finding tracks and spurious hits
  virtual void                findTracksSpurious();
  rpc::mTrackPairIdx          findPairTracksSpurious(const rpc::mLayerClusters&, const rpc::mLayerClusters&, rpc::mLayerClusters&, rpc::mLayerClusters&);
  virtual rpc::mTrackPairIdx  pairForcedClusters(rpc::mLayerClusters, rpc::mLayerClusters);
  void                        findClusterDistance();
  void                        findRoClusterCMS();
  void                        findRoTrackCMS();
  double                      findClusterUnitCMS(const rpc::clusterUnit&);


  rpc::mLayerClusters         clusterizeLayer(const rpc::mStripsInLayer&);
  
  int                         findPairClusterDistance(const rpc::clusterUnit&, const rpc::clusterUnit&);
  unsigned int                nStripsInCommon(const rpc::clusterUnit&, const rpc::clusterUnit&);
};



#endif
