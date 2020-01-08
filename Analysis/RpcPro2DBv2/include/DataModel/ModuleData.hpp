#ifndef MODULEDATA_HPP
#define MODULEDATA_HPP


#include <vector>
#include "DataModel/LayerData.hpp"
#include "DataModel/rpcnamespace.hpp"


class ModuleData
{
public:
  /*
   * member variables
   */
  std::vector<LayerData> layers;
  bool                   fromRot;
  
  /// A track is a pair of clusters from same oriented layers which have strips
  /// lining up.
  /// Loose definition of tracks allow off by 1 clusters to form tracks.
  rpc::mTrackPairIndices xLooseTrackPairIndices;
  rpc::mTrackPairIndices yLooseTrackPairIndices;
  
  
  /// container for reconstructed muon positions
  rpc::mReconPoints      muonPositions;
  
  
  
  /*
   * member functions
   */
  ModuleData();
  void printLayers();
  
  /// pairs clusters into tracks and idetifies spurious clusters
  /// LayerData are passed by values on purpose
  void fillLooseTrackAndSpurious(LayerData, LayerData, rpc::mTrackPairIndices&,
       std::vector<unsigned int>&, std::vector<unsigned int>&);
  
  void fillReconstructedPositions();
  
  /// Get the number of unfired strips between two clusters.
  /// Returns 0-6 if they are apart.
  /// Returns -1 to -8 if they overlap.
  int getClusterDistance(ClusterData&, ClusterData&);
  
  /// returns N-fold at hardware level, i.e. without cluster size cut
  unsigned int getNLayersHardware();
  /// returns N-fold after cluster size cut
  unsigned int getNLayersOffline();
  /// returns the number of strips in common between 2 clusters
  unsigned int getNStripsInCommon(ClusterData&, ClusterData&);
  /// get a vector of unfired layers AFTER cluster size cut
  std::vector<unsigned int> getUnfiredLayersOffline();
};




#endif
