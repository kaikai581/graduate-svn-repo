#ifndef CLUSTERDATA_HPP
#define CLUSTERDATA_HPP


#include <vector>


class ClusterData
{
public:
  /*
   * member variables
   */
  std::vector<unsigned int> strips;


  /*
   * member functions
   */
  /// returns center of grivity of the cluster
  double getClusterCM();
  void printClusterCM();
  void printStrips();
  
};




#endif
