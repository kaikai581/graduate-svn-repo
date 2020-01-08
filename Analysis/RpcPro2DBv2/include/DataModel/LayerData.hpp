#ifndef LAYERDATA_HPP
#define LAYERDATA_HPP



#include <vector>
#include "DataModel/ClusterData.hpp"


class LayerData
{
public:
  /*
   * member variables
   */
  /// used for storing fired strips
  std::vector<unsigned int> strips;
  std::vector<ClusterData>  clusters;
  /// container for spurious clusters
  /// A spurious cluster is a cluster that is not paired with any other
  /// clusters to form a track.
  std::vector<unsigned int> spuriousIndices;
  
  
  /*
   * member functions
   */
  /// Sometimes the strip IDs in the readout are not sorted. Sort them!
  void sortStrips();
  /// Contiguous strips form clusters.
  void clusterize(unsigned int);
  
  unsigned int getNClusters();
  unsigned int getNStrips();
  
  void printStrips();
  void printClusters();
  void printClustersCM();
};



#endif
