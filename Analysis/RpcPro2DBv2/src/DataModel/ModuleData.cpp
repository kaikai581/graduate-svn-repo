#include <algorithm>
#include <iostream>
#include "DataModel/ModuleData.hpp"


using namespace std;


ModuleData::ModuleData()
{
  layers = vector<LayerData>(4, LayerData());
}






void ModuleData::fillLooseTrackAndSpurious(LayerData l1, LayerData l2,
     rpc::mTrackPairIndices& pairIdx, vector<unsigned int>& spuIdx1,
     vector<unsigned int>& spuIdx2)
{
  vector<unsigned int> paired1, paired2;
  
  for(unsigned int i1 = 0; i1 < l1.clusters.size(); i1++)
    for(unsigned int i2 = 0; i2 < l2.clusters.size(); i2++)
    {
      if(getClusterDistance(l1.clusters[i1], l2.clusters[i2]) <= 0)
      {
        if(find(paired1.begin(), paired1.end(), i1) == paired1.end() &&
           find(paired2.begin(), paired2.end(), i2) == paired2.end())
        {
          pairIdx.push_back(pair<unsigned int, unsigned int>(i1, i2));
          paired1.push_back(i1);
          paired2.push_back(i2);
        }
      }
    }

  for(unsigned int i1 = 0; i1 < l1.clusters.size(); i1++)
    if(find(paired1.begin(), paired1.end(), i1) == paired1.end())
      spuIdx1.push_back(i1);

  for(unsigned int i2 = 0; i2 < l2.clusters.size(); i2++)
    if(find(paired2.begin(), paired2.end(), i2) == paired2.end())
      spuIdx2.push_back(i2);
}






void ModuleData::fillReconstructedPositions()
{
  //if(getNLayersOffline() < 3) return;
  
  vector<double> xs, ys;
  
  if(layers[0].clusters.size() && layers[3].clusters.size()) // paired layers
  {
    for(unsigned int ix = 0; ix < xLooseTrackPairIndices.size(); ix++)
    {
      double cmb = layers[0].clusters[xLooseTrackPairIndices[ix].first].getClusterCM();
      double cmt = layers[3].clusters[xLooseTrackPairIndices[ix].second].getClusterCM();
      xs.push_back((cmb+cmt)/2.);
    }
  }
  else // unpaired layers
  {
    LayerData ld;
    if(layers[0].clusters.size() == 0) ld = layers[3];
    else                               ld = layers[0];
    for(unsigned int c = 0; c < ld.clusters.size(); c++)
      xs.push_back(ld.clusters[c].getClusterCM());
  }



  if(layers[1].clusters.size() && layers[2].clusters.size())
  {
    for(unsigned int iy = 0; iy < yLooseTrackPairIndices.size(); iy++)
    {
      double cmb = layers[1].clusters[yLooseTrackPairIndices[iy].first].getClusterCM();
      double cmt = layers[2].clusters[yLooseTrackPairIndices[iy].second].getClusterCM();
      ys.push_back((cmb+cmt)/2.);
    }
  }
  else
  {
    LayerData ld;
    if(layers[1].clusters.size() == 0) ld = layers[2];
    else                               ld = layers[1];
    for(unsigned int c = 0; c < ld.clusters.size(); c++)
      ys.push_back(ld.clusters[c].getClusterCM());
  }
  
  
  for(unsigned int i = 0; i < xs.size(); i++)
    for(unsigned int j = 0; j < ys.size(); j++)
      muonPositions.push_back(pair<double, double>(xs[i], ys[j]));
}






int ModuleData::getClusterDistance(ClusterData& cb, ClusterData& ct)
{  
  int ncommon = getNStripsInCommon(cb, ct);
  if(ncommon) return -ncommon;
  
  vector<unsigned int> c1 = cb.strips;
  vector<unsigned int> c2 = ct.strips;

  int c1begin = c1[0];
  int c1end   = c1[c1.size()-1];
  int c2begin = c2[0];
  int c2end   = c2[c2.size()-1];
  
  return ((c1begin>c2end)? (c1begin-c2end-1) : (c2begin-c1end-1));
}







unsigned int ModuleData::getNLayersHardware()
{
  unsigned int nLayers = 0;
  for(unsigned int l = 0; l < 4; l++)
    if(layers[l].getNStrips()) nLayers++;

  return nLayers;
}







unsigned int ModuleData::getNLayersOffline()
{
  unsigned int nLayers = 0;
  for(unsigned int l = 0; l < 4; l++)
    if(layers[l].getNClusters()) nLayers++;

  return nLayers;
}






unsigned int ModuleData::getNStripsInCommon(ClusterData& cb, ClusterData& ct)
{
  vector<unsigned int> c1 = cb.strips;
  vector<unsigned int> c2 = ct.strips;
  
  vector<unsigned int> v(8);
  vector<unsigned int>::iterator it;
  
  it = set_intersection(c1.begin(), c1.end(), c2.begin(), c2.end(), v.begin());
  
  return (unsigned int)(it-v.begin());
}







vector<unsigned int> ModuleData::getUnfiredLayersOffline()
{
  vector<unsigned int> unfiredLayers;
  
  for(unsigned int l = 0; l < 4; l++)
    if(!layers[l].getNClusters()) unfiredLayers.push_back(l+1);
  
  return unfiredLayers;
}








void ModuleData::printLayers()
{
  /// print strip information
  cout << "in strip:" << endl << "\t";
  for(unsigned int lIdx = 0; lIdx < 4; lIdx++)
    layers[lIdx].printStrips();
  cout << endl;
  
  /// print cluster information
  cout << "in cluster:" << endl << "\t";
  for(unsigned int lIdx = 0; lIdx < 4; lIdx++)
    layers[lIdx].printClusters();
  cout << endl;
  
  /// print cluster center of gravity
  cout << "cluster cm:" << endl << "\t";
  for(unsigned int l = 0; l < 4; l++)
    layers[l].printClustersCM();
  cout << endl;
}
