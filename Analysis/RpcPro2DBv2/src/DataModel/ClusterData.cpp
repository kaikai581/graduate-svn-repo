#include <iostream>
#include "DataModel/ClusterData.hpp"



using namespace std;




double ClusterData::getClusterCM()
{
  if(!strips.size()) return -1.;
  
  double result = 0.;
  
  for(unsigned int s = 0; s < strips.size(); s++) result += strips[s];
  
  result /= strips.size();
  
  return result;
}




void ClusterData::printClusterCM()
{
  cout << "(";
  cout << getClusterCM();
  cout << ")";
}




void ClusterData::printStrips()
{
  cout << "(";
  if(!strips.size()) cout << ")";

  for(unsigned int s = 0; s < strips.size(); s++)
  {
    cout << strips[s];
    cout << ((s==strips.size()-1)?")":",");
  }
}
