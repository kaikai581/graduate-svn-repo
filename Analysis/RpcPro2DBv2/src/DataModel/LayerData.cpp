#include <algorithm>
#include <iostream>
#include "DataModel/LayerData.hpp"



using namespace std;






void LayerData::clusterize(unsigned int clsize)
{
  ClusterData currentCluster;
  for(unsigned int s = 0; s < strips.size(); s++)
  {
    currentCluster.strips.push_back(strips[s]);

    if((strips[s+1]-strips[s]!=1) || // noncontiguous strips
       (s == strips.size()-1))       // last strip
    {
      /// Cluster size cut is achieved here.
      if(currentCluster.strips.size() <= clsize)
        clusters.push_back(currentCluster);
      currentCluster.strips.clear();
    }
    
  }
}






unsigned int LayerData::getNClusters()
{
  unsigned int nClusters = 0;
  for(unsigned int c = 0; c < clusters.size(); c++) nClusters++;
  
  return nClusters;
}






unsigned int LayerData::getNStrips()
{
  unsigned int nStrips = 0;
  for(unsigned int s = 0; s < strips.size(); s++) nStrips++;
  
  return nStrips;
}






void LayerData::printClusters()
{
  cout << "(";
  for(unsigned int c = 0; c < clusters.size(); c++)
    clusters[c].printStrips();
  cout << ")";
}







void LayerData::printClustersCM()
{
  cout << "(";
  for(unsigned int c = 0; c < clusters.size(); c++)
    clusters[c].printClusterCM();
  cout << ")";
}








void LayerData::printStrips()
{
  cout << "(";
  if(!strips.size()) cout << ")";
  for(unsigned int s = 0; s < strips.size(); s++)
  {
    cout << strips[s];
    cout << ((s==strips.size()-1)?")":",");
  }
}




void LayerData::sortStrips()
{
  sort(strips.begin(), strips.end());
}
