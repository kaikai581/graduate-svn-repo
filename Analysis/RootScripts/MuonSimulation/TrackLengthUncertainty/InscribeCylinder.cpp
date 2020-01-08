#include "InscribeCylinder.hpp"
#include "Math/AxisAngle.h"
#include "TMath.h"
#include <map>
#include <set>
#include <vector>


using namespace ROOT::Math;
using namespace std;




double PerpParameter(XYZPoint p, XYZPoint p0, XYZVector v)
{
  XYZVector vCP = p - p0;
  double cos = vCP.Unit().Dot(v);
  
  return sqrt(vCP.Mag2())*cos;
}




double GetCylLen(XYZPoint p, XYZVector v, XYZVector c)
{
  double reslen = 0.;
  
  /// translate coordinate to the IAV center of the current AD
  p = p-c;
  
  XYZVector e1(1,0,0);
  XYZVector e2(0,1,0);
  XYZVector e3(0,0,1);
  
  /// reverse the directional vector
  v = -v;
  
  AxisAngle rotateTrack(e3.Cross(v), v.Theta());
  AxisAngle rotateIav = rotateTrack.Inverse();
  
  /// matrix elements of the inverse rotation matrix
  double a11 = e1.Dot(rotateIav*e1);
  double a12 = e1.Dot(rotateIav*e2);
  double a13 = e1.Dot(rotateIav*e3);
  double a21 = e2.Dot(rotateIav*e1);
  double a22 = e2.Dot(rotateIav*e2);
  double a23 = e2.Dot(rotateIav*e3);
  
  /// containers
  vector<pair<XYZVector, XYZVector> > positionCurvePoints;
  vector<pair<int, int> > regionAndNRoots;
  set<int> regionName;
  
  int nsteps = 360;
  
  /// partition phi into regions: closed loop regions and empty regions
  for(int i = 0; i < nsteps; i++)
  {
    double x = -TMath::Pi()+TMath::TwoPi()/nsteps*i;
    double R = 1500.;
    double r = 1000.;
    double x1 = R*cos(x)-p.X();
    double x2 = R*sin(x)-p.Y();
    
    double a2 = a13*a13+a23*a23;
    double b1 = a11*x1+a12*x2;
    double b2 = a21*x1+a22*x2;
    double a1 = 2*(a13*b1+a23*b2);
    double a0 = b1*b1+b2*b2-r*r;
    
    vector<double> roots;
    double D = a1*a1-4*a2*a0;
    if(D > 0)
    {
      roots.push_back((-a1+sqrt(D))/2./a2);
      roots.push_back((-a1-sqrt(D))/2./a2);
    }
    
    double x31, x32;
    if(roots.size() == 2)
    {
      x31 = roots[0]+p.Z();
      x32 = roots[1]+p.Z();
      x1 += p.X();
      x2 += p.Y();
      XYZVector v1(x1,x2,x31);
      XYZVector v2(x1,x2,x32);
      positionCurvePoints.push_back(pair<XYZVector, XYZVector>(v1, v2));
    }
    else
    {
      XYZVector vz(0,0,0);
      positionCurvePoints.push_back(pair<XYZVector, XYZVector>(vz, vz));
    }
    
    if( i == 0 )
    {
      regionAndNRoots.push_back(pair<int, int>(0,roots.size()));
    }
    else
    {
      int lastRegion = regionAndNRoots[i-1].first;
      int lastNRoots = regionAndNRoots[i-1].second;
      int curRegion = lastRegion;
      int curNRoots = roots.size();
      if(((curNRoots > 0) && (lastNRoots == 0)) ||
         ((curNRoots == 0) && (lastNRoots > 0))) curRegion++;
      regionAndNRoots.push_back(pair<int, int>(curRegion,curNRoots));
    }
    regionName.insert(regionAndNRoots[i].first);
  }
  
  
  /// wrap the last region to the first region due to phi periodicity
  map<int, vector<XYZVector> > posPartCurve;
  if(regionName.size() > 1)
  {
    for(unsigned int i = 0; i < regionAndNRoots.size(); i++)
    {
      regionAndNRoots[i].first = regionAndNRoots[i].first%(regionName.size()-1);
    }
    
    /// map from region to the group of curve points
    for(unsigned int i = 0; i < regionAndNRoots.size(); i++)
    {
      int curRegion = regionAndNRoots[i].first;
      int curNRoots = regionAndNRoots[i].second;
      if(curNRoots == 2)
      {
        posPartCurve[curRegion].push_back(positionCurvePoints[i].first);
        posPartCurve[curRegion].push_back(positionCurvePoints[i].second);
      }
    }
  }
  
  
  /// container map from region to the indices of the extremities
  map<int, pair<XYZVector, XYZVector> > extP;
  map<int, pair<unsigned int,unsigned int> > extInd;
  map<int, pair<double, double> > extT;
  for(map<int, vector<XYZVector> >::iterator it = posPartCurve.begin(); it != posPartCurve.end(); it++)
  {
    for(unsigned int i = 0; i < it->second.size(); i++)
    {
      if(i == 0)
      {
        extInd[it->first] = pair<int,int>(0,0);
        extP[it->first] = pair<XYZVector, XYZVector>(it->second[0], it->second[0]);
        extT[it->first].first = PerpParameter((XYZPoint)(it->second[0]+c), p+c, v);
        extT[it->first].second = extT[it->first].first;
      }
      else
      {
        double z = PerpParameter((XYZPoint)(it->second[i]+c), p+c, v);
        if(z > extT[it->first].first)
        {
          extT[it->first].first = z;
          extInd[it->first].first = i;
          extP[it->first].first = it->second[i];
        }
        if(z < extT[it->first].second)
        {
          extT[it->first].second = z;
          extInd[it->first].second = i;
          extP[it->first].second = it->second[i];
        }
      }
    }
  }
  
  
  if(extT.size() == 2)
  {
    map<int, pair<double, double> >::iterator itup = extT.begin();
    int reg1 = itup->first;
    itup++;
    int reg2 = itup->first;
    if(extT[reg1].first < extT[reg2].first)
    {
      int regtemp = reg2;
      reg2 = reg1;
      reg1 = regtemp;
    }
    
    /// unit vectors parallel to the up and bottom plane of the IAV
    double th = v.Theta();
    XYZVector paraup = (-v-cos(th)*-e3).Unit();
    XYZVector parabot = (v-cos(th)*e3).Unit();
    
    double tIavUp = -(p.Z()-1500.)/v.Z();
    XYZPoint pIavUp = p + tIavUp*v;
    tIavUp -= 1000.*tan(th);
    pIavUp = pIavUp + 1000./cos(th)*paraup;
    
    double tIavBot = (-1500.-p.Z())/v.Z();
    XYZPoint pIavBot = p + tIavBot*v;
    tIavBot += 1000.*tan(th);
    pIavBot = pIavBot + 1000./cos(th)*parabot;
    
    XYZPoint pup = (XYZPoint)(extP[reg1].second+c);
    double tup = extT[reg1].second;
    if(tIavUp < tup)
    {
      tup = tIavUp;
      pup = (XYZPoint)(pIavUp+c);
    }
    
    XYZPoint pbot = (XYZPoint)(extP[reg2].first+c);
    double tbot = extT[reg2].first;
    if(tIavBot > tbot)
    {
      tbot = tIavBot;
      pbot = (XYZPoint)(pIavBot+c);
    }
    if(tup > tbot)
    {
      reslen = tup - tbot;
    }
  }
  
  
  return reslen;
}


