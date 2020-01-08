#include <algorithm>
#include <cmath>
#include <iostream>
#include "DataModel/DatasetModuleData.hpp"
#include "DataModel/ReadoutData.hpp"



using namespace std;



DatasetModuleData::DatasetModuleData()
{
  nFoldCountsCS = vector<unsigned int>(5);
  nFoldCountsCSInt1 = vector<unsigned int>(5);
  nFoldCountsCSInt2 = vector<unsigned int>(5);
  effCS = vector<rpc::efficiency>(5, pair<double, double>());
  effCSInt1 = vector<rpc::efficiency>(5, pair<double, double>());
  effCSInt2 = vector<rpc::efficiency>(5, pair<double, double>());
}





void DatasetModuleData::fillLayerEfficiencies()
{
  
  for(unsigned int l = 1; l < 5; l++)
  {
    if(!(nFoldCountsCS[0] + nFoldCountsCS[l]))
    {
      effCS[l].first = 0.;
      effCS[l].second = 0.;
    }
    else
    {
      double n = nFoldCountsCS[0] + nFoldCountsCS[l];
      double k = nFoldCountsCS[0];
      
      effCS[l].first = k/n;
      effCS[l].second = sqrt((k+1)/(n+2)*(k+2)/(n+3)-(k+1)*(k+1)/(n+2)/(n+2));
    }
    
    /// fill interior counts
    if(!(nFoldCountsCSInt1[0] + nFoldCountsCSInt1[l]))
    {
      effCSInt1[l].first = 0.;
      effCSInt1[l].second = 0.;
    }
    else
    {
      double n = nFoldCountsCSInt1[0] + nFoldCountsCSInt1[l];
      double k = nFoldCountsCSInt1[0];
      
      effCSInt1[l].first = k/n;
      effCSInt1[l].second = sqrt((k+1)/(n+2)*(k+2)/(n+3)-(k+1)*(k+1)/(n+2)/(n+2));
    }
    
    if(!(nFoldCountsCSInt2[0] + nFoldCountsCSInt2[l]))
    {
      effCSInt2[l].first = 0.;
      effCSInt2[l].second = 0.;
    }
    else
    {
      double n = nFoldCountsCSInt2[0] + nFoldCountsCSInt2[l];
      double k = nFoldCountsCSInt2[0];
      
      effCSInt2[l].first = k/n;
      effCSInt2[l].second = sqrt((k+1)/(n+2)*(k+2)/(n+3)-(k+1)*(k+1)/(n+2)/(n+2));
    }
  }
}






void DatasetModuleData::fillModuleEfficiency()
{
  fillLayerEfficiencies();
  
  double fourProduct = 1., fourProductInt1 = 1., fourProductInt2 = 1.;
  double lEff[4], lEffInt1[4], lEffInt2[4];
  
  for(unsigned int l = 0; l < 4; l++)
  {
    lEff[l] = effCS[l+1].first;
    lEffInt1[l] = effCSInt1[l+1].first;
    lEffInt2[l] = effCSInt2[l+1].first;
  }
  
  for(int l = 0; l < 4; l++)
  {
    double threeProduct = 1-lEff[l];
    double threeProductInt1 = 1-lEffInt1[l];
    double threeProductInt2 = 1-lEffInt2[l];
    for(int p = 1; p < 4; p++)
    {
      threeProduct *= lEff[(l+p)%4];
      threeProductInt1 *= lEffInt1[(l+p)%4];
      threeProductInt2 *= lEffInt2[(l+p)%4];
    }
    effCS[0].first += threeProduct;
    effCSInt1[0].first += threeProductInt1;
    effCSInt2[0].first += threeProductInt2;
    fourProduct *= lEff[l];
    fourProductInt1 *= lEffInt1[l];
    fourProductInt2 *= lEffInt2[l];
  }
  effCS[0].first += fourProduct;
  effCSInt1[0].first += fourProductInt1;
  effCSInt2[0].first += fourProductInt2;


  /// calculate error according to layer errors
  double D1f, D2f, D3f, D4f;
  double x1, x2, x3, x4;
  double dx1, dx2, dx3, dx4;
  x1 = effCS[1].first;
  dx1 = effCS[1].second;
  x2 = effCS[2].first;;
  dx2 = effCS[2].second;
  x3 = effCS[3].first;;
  dx3 = effCS[3].second;
  x4 = effCS[4].first;;
  dx4 = effCS[4].second;
  D1f = x3*x4+x2*x4+x2*x3-3*x2*x3*x4;
  D2f = x1*x3+x3*x4+x1*x4-3*x1*x3*x4;
  D3f = x1*x2+x2*x4+x1*x4-3*x1*x2*x4;
  D4f = x1*x2+x2*x3+x1*x3-3*x1*x2*x3;
  D1f *= dx1;
  D2f *= dx2;
  D3f *= dx3;
  D4f *= dx4;
  effCS[0].second = sqrt(D1f*D1f+D2f*D2f+D3f*D3f+D4f*D4f);

  x1 = effCSInt1[1].first;
  dx1 = effCSInt1[1].second;
  x2 = effCSInt1[2].first;;
  dx2 = effCSInt1[2].second;
  x3 = effCSInt1[3].first;;
  dx3 = effCSInt1[3].second;
  x4 = effCSInt1[4].first;;
  dx4 = effCSInt1[4].second;
  D1f = x3*x4+x2*x4+x2*x3-3*x2*x3*x4;
  D2f = x1*x3+x3*x4+x1*x4-3*x1*x3*x4;
  D3f = x1*x2+x2*x4+x1*x4-3*x1*x2*x4;
  D4f = x1*x2+x2*x3+x1*x3-3*x1*x2*x3;
  D1f *= dx1;
  D2f *= dx2;
  D3f *= dx3;
  D4f *= dx4;
  effCSInt1[0].second = sqrt(D1f*D1f+D2f*D2f+D3f*D3f+D4f*D4f);

  x1 = effCSInt2[1].first;
  dx1 = effCSInt2[1].second;
  x2 = effCSInt2[2].first;;
  dx2 = effCSInt2[2].second;
  x3 = effCSInt2[3].first;;
  dx3 = effCSInt2[3].second;
  x4 = effCSInt2[4].first;;
  dx4 = effCSInt2[4].second;
  D1f = x3*x4+x2*x4+x2*x3-3*x2*x3*x4;
  D2f = x1*x3+x3*x4+x1*x4-3*x1*x3*x4;
  D3f = x1*x2+x2*x4+x1*x4-3*x1*x2*x4;
  D4f = x1*x2+x2*x3+x1*x3-3*x1*x2*x3;
  D1f *= dx1;
  D2f *= dx2;
  D3f *= dx3;
  D4f *= dx4;
  effCSInt2[0].second = sqrt(D1f*D1f+D2f*D2f+D3f*D3f+D4f*D4f);
  /// end of error calculation
}






void DatasetModuleData::fillTriggerRate()
{  
  if(!triggerTimeCS.size()) return;
  
  sort(triggerTimeCS.begin(), triggerTimeCS.end());
  
  double dt = triggerTimeCS[triggerTimeCS.size()-1] - triggerTimeCS[0];
  if(dt == 0.) return;
  
  double totN = 0.;
  for(unsigned int i = 0; i < 5; i++) totN += nFoldCountsCS[i];
  
  triggerRateCS.first = totN/dt;
  triggerRateCS.second = sqrt(totN)/dt;
}






//void DatasetModuleData::incrementTriggerTime(double tt)
//{
  //triggerTimeCS.push_back(tt);
//}
void DatasetModuleData::incrementTriggerTime(unsigned int s, unsigned int ns)
{
  double triggerTime = s + 1e-9*ns;
  triggerTimeSec.push_back(s);
  triggerTimeNanoSec.push_back(ns);
  triggerTimeCS.push_back(triggerTime);
}






void DatasetModuleData::printDatasetModuleData()
{

  cout << "\t(N4 ,N31,N32,N33,N34): (";
  for(unsigned int l = 0; l < 5; l++)
  {
    cout << nFoldCountsCS[l];
    cout << ((l==4)? ")":",");
  }
  cout << endl;

  cout << "\t(EM ,E1 ,E2 ,E3 ,E4 ): (";
  for(unsigned int l = 0; l < 5; l++)
  {
    cout << effCS[l].first;
    cout << ((l==4)? ")":",");
  }
  cout << endl;
  
  cout << "efficiency with interior hits only" << endl;
  cout << "\t(EM ,E1 ,E2 ,E3 ,E4 ): (";
  for(unsigned int l = 0; l < 5; l++)
  {
    cout << effCSInt2[l].first;
    cout << ((l==4)? ")":",");
  }
  cout << endl;
  
  cout << "\t trigger rate: " << triggerRateCS.first << endl;
  
  cout << "\t muon rate: ";
  cout << ((effCS[0].first==0.)? 0.:triggerRateCS.first/effCS[0].first) << endl;
}
