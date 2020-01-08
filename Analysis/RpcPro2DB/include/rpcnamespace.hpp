#ifndef RPCNAMESPACE_HPP
#define RPCNAMESPACE_HPP


#include <map>
#include <vector>

namespace rpc
{
  /// prefix "m" means "module"
  typedef std::pair<unsigned int, unsigned int> mId;
  typedef std::vector<std::vector<unsigned int> > mLayers;
  typedef std::vector<unsigned int> mStripsInLayer;
  typedef std::map<mId, mLayers> mEvent;
  typedef std::map<mId, bool> mFromRot;
  
  /// cluster containers
  typedef std::vector<std::vector<std::vector<unsigned int> > > mModuleClusters;
  typedef std::vector<std::vector<unsigned int> >               mLayerClusters;
  typedef std::vector<unsigned int>                             clusterUnit;
  typedef std::map<mId, mModuleClusters>                        mReadoutClusters;
  /// cluster center of mass containers
  typedef std::vector<double>                                   lClusterCMS;
  typedef std::vector<lClusterCMS>                              mClusterCMS;
  typedef std::map<mId, mClusterCMS>                            roClusterCMS;
  
  
  /// tracks and spurious hits container
  /// below is a map from layer to spurious hit indices
  typedef std::vector<std::pair<unsigned int, unsigned int> > mTrackPairIdx;
  typedef std::map<mId, mTrackPairIdx> roTrackPairIdx;
  typedef std::map<unsigned int, std::vector<unsigned int> > mSpuriousIndex;
  typedef std::map<mId, mSpuriousIndex> roSpuriousIndex;
  
  
  /// data type for recording if the layer has true hardware level hit
  /// this is used when inspecting data after cluster size cut
  typedef std::vector<bool> mHitLayerHardware;
  typedef std::map<mId, mHitLayerHardware> roHitLayerHardware;
  
  
  /// recnstruction containers
  typedef std::vector<double>      mTrackCMS;
  typedef std::map<mId, mTrackCMS> roTrackCMS;
}


#endif
