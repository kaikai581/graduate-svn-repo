/*
 * This program is to calculate the mean muon energies for each AD in each hall.
 * First partition the IAV into predefined number of pixels.
 * Second for each pixel take the center of the pixel and draw a line according
 * to each MUSIC muon and extrapolate to the RPC plane. It requires 2
 * conditions to keep the muon:
 *   1) The muon has to go through the RPC coverage
 *   2) The muon has to be able to incribe a cylinder in the IAV
 * Last calculate the mean muon energy of all the kept muons.
 * 
 * Shih-Kai Lin, November 2013.
*/


#include <cmath>
#include <cstdlib>
#include <fstream>
#include <map>
#include <set>
#include <sstream>
#include <vector>
#include "Math/AxisAngle.h"
#include "Math/Point3D.h"
#include "Math/Vector3D.h"
#include "TFile.h"
#include "TMath.h"
#include "TRandom3.h"
#include "TTree.h"
#include "TVector3.h"

#define RIAV 1500.
#define rins 1000.


using namespace ROOT::Math;
using namespace std;



string fnlist[] = {"/disk1/NuWa/external/data/0.0/Muon/mountain_DYB",
                   "/disk1/NuWa/external/data/0.0/Muon/mountain_LA",
                   "/disk1/NuWa/external/data/0.0/Muon/mountain_Far_80m"};



class HallGeoDisc
{
public:
  HallGeoDisc(){};
  HallGeoDisc(int);
  XYZPoint  m_hallOrigin;
  map<int, XYZPoint> m_iavOrigins;
  XYZVector m_localxAxis;
  double    m_localzRpc;
  double    m_localRpcxCoverage;
  double    m_localRpcyCoverage;
};



struct OneMuData {
  double E;
  double th;
  double phi;
  XYZVector pRPC;
};


HallGeoDisc::HallGeoDisc(int hid)
{
  int col, detid;
  string templine;
  
  hid = pow(2,hid-1);
  stringstream fnIav;
  fnIav << "geodb/IAV" << hid;
  ifstream fIav(fnIav.str().c_str());
  
  col = 0;
  while(fIav >> templine)
  {
    col = col % 8;
    
    if(col == 1)
    {
      detid = atoi(templine.c_str());
      m_iavOrigins[detid] = XYZPoint();
    }
    if(col == 2) m_iavOrigins[detid].SetX(atof(templine.c_str()));
    if(col == 3) m_iavOrigins[detid].SetY(atof(templine.c_str()));
    if(col == 4) m_iavOrigins[detid].SetZ(atof(templine.c_str()));
    
    col++;
  }
  fIav.close();
  
  /// get site origins
  stringstream fnDetO;
  fnDetO << "geodb/DetectorOrigins" << hid;
  ifstream fDetO(fnDetO.str().c_str());
  
  col = 0;
  while(fDetO >> templine)
  {
    col = col % 8;
    
    if(col == 1) detid = atoi(templine.c_str());
    if(detid == 6)
    {
      if(col == 2) m_hallOrigin.SetX(atof(templine.c_str()));
      if(col == 3) m_hallOrigin.SetY(atof(templine.c_str()));
      if(col == 4) m_hallOrigin.SetZ(atof(templine.c_str()));
      if(col == 5) m_localxAxis.SetX(atof(templine.c_str()));
      if(col == 6) m_localxAxis.SetY(atof(templine.c_str()));
    }
    
    col++;
  }
  fDetO.close();
  
  /// z of RPC plane to OWS origin distance
  if(hid == 1)
  {
    m_localzRpc = 5783;
    m_localRpcxCoverage = 17200;
    m_localRpcyCoverage = 11500;
  }
  if(hid == 2)
  {
    m_localzRpc = 5779;
    m_localRpcxCoverage = 17200;
    m_localRpcyCoverage = 11500;
  }
  if(hid == 4)
  {
    m_localzRpc = 5840;
    m_localRpcxCoverage = 17200;
    m_localRpcyCoverage = 17200;
  }
}




double perpParameter(XYZPoint p, XYZPoint pCent, XYZVector vDir)
{
  XYZVector vCP = p - pCent;
  double cos = vCP.Unit().Dot(vDir);
  
  return sqrt(vCP.Mag2())*cos;
}





bool inscribable(OneMuData mu_track)
{
  TVector3 tmpvec;
  tmpvec.SetMagThetaPhi(1,mu_track.th,mu_track.phi);
  XYZVector vDir(tmpvec.X(),tmpvec.Y(),tmpvec.Z());
  XYZVector pTrack(mu_track.pRPC);
  
  XYZVector e1(1,0,0);
  XYZVector e2(0,1,0);
  XYZVector e3(0,0,1);
  
  AxisAngle rotateTrack(e3.Cross(vDir), vDir.Theta());
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
    double R = RIAV;
    double r = rins;
    double x1 = R*cos(x)-pTrack.X();
    double x2 = R*sin(x)-pTrack.Y();
    
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
      x31 = roots[0]+pTrack.Z();
      x32 = roots[1]+pTrack.Z();
      x1 += pTrack.X();
      x2 += pTrack.Y();
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
        extT[it->first].first = perpParameter((XYZPoint)it->second[0], (XYZPoint)pTrack, vDir);
        extT[it->first].second = extT[it->first].first;
      }
      else
      {
        double z = perpParameter((XYZPoint)it->second[i], (XYZPoint)pTrack, vDir);
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
  
  
  /// container for the answer
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
    double th = vDir.Theta();
    XYZVector paraup = (-vDir-cos(th)*-e3).Unit();
    XYZVector parabot = (vDir-cos(th)*e3).Unit();
    
    double tIavUp = -(pTrack.Z()-RIAV)/vDir.Z();
    XYZVector pIavUp = pTrack + tIavUp*vDir;
    tIavUp -= rins*tan(th);
    pIavUp = pIavUp + rins/cos(th)*paraup;
    
    double tIavBot = (-RIAV-pTrack.Z())/vDir.Z();
    XYZVector pIavBot = pTrack + tIavBot*vDir;
    tIavBot += rins*tan(th);
    pIavBot = pIavBot + rins/cos(th)*parabot;
    
    XYZPoint pup = (XYZPoint)(extP[reg1].second);
    double tup = extT[reg1].second;
    if(tIavUp < tup)
    {
      tup = tIavUp;
      pup = (XYZPoint)(pIavUp);
    }
    
    XYZPoint pbot = (XYZPoint)(extP[reg2].first);
    double tbot = extT[reg2].first;
    if(tIavBot > tbot)
    {
      tbot = tIavBot;
      pbot = (XYZPoint)(pIavBot);
    }
    if(tup > tbot)
      return true;
  }
  
  
  return false;
}




vector<OneMuData> getMeanMuonEnergyForOneAd(int adId, HallGeoDisc& geom, int hall)
{
  /// result muons
  vector<OneMuData> inscribableMu;


  /// transform everything to AD local coordinate
  ifstream inf;
  inf.open(fnlist[hall-1].c_str());
  
  string tmpline;
  int nline = 0;
  
  vector<OneMuData> vecMu;
  
  while(getline(inf, tmpline))
  {
    /// flush the first 6 lines
    nline++;
    if(nline <= 6) continue;
    
    /// start recording the data
    stringstream ss(tmpline);
    double val;
    vector<double> curline;
    while (ss >> val) curline.push_back(val);
    
    OneMuData curMu;
    curMu.E = curline[0];
    curMu.th = curline[1];
    curMu.phi = curline[2];
    vecMu.push_back(curMu);
  }
  
  inf.close();
  
  
  /// transform to local coordinate
  TVector3 glbx(geom.m_localxAxis.X(), geom.m_localxAxis.Y(), geom.m_localxAxis.Z());
  TVector3 locx(1,0,0);
  double gbl2locAngle = glbx.Angle(locx);
  XYZVector rotAxis = geom.m_localxAxis.Cross(XYZVector(1,0,0));
  AxisAngle gbl2locRot(rotAxis, gbl2locAngle);
  
  XYZVector iav2ctr = (XYZVector)geom.m_hallOrigin - (XYZVector)geom.m_iavOrigins[adId];
  iav2ctr = gbl2locRot*iav2ctr;
  
  /// define RPC coverage
  double xmax = geom.m_localRpcxCoverage/2 + iav2ctr.X();
  double xmin = -geom.m_localRpcxCoverage/2 + iav2ctr.X();
  double ymax = geom.m_localRpcyCoverage/2 + iav2ctr.Y();
  double ymin = -geom.m_localRpcyCoverage/2 + iav2ctr.Y();

  
  /// transform muon angles to local coordinate
  /// and pick a point in the RPC plane
  //double rpcx, rpcy, rpcz;
  //TFile tfout("result.root", "recreate");
  //TTree tr("tr","simulation results");
  //tr.Branch("rpcx", &rpcx, "rpcx/D");
  //tr.Branch("rpcy", &rpcy, "rpcy/D");
  //tr.Branch("rpcz", &rpcz, "rpcz/D");
  
  for(unsigned int j = 0; j < vecMu.size(); j++)
  {
    double th = vecMu[j].th*M_PI/180.;
    double phi = vecMu[j].phi*M_PI/180.;

    TVector3 curAngle;
    curAngle.SetMagThetaPhi(1, th, phi);
    XYZVector curVec(curAngle.X(), curAngle.Y(), curAngle.Z());
    curVec = gbl2locRot*curVec;
    
    vecMu[j].th = curVec.Theta();
    vecMu[j].phi = curVec.Phi();
    
    TRandom3 ranX(0);
    TRandom3 ranY(0);
    
    double muX = ranX.Uniform(xmin, xmax);
    double muY = ranX.Uniform(ymin, ymax);
    vecMu[j].pRPC = XYZVector(muX,muY,geom.m_localzRpc-(geom.m_iavOrigins[adId]-geom.m_hallOrigin).Z());
    
    if(inscribable(vecMu[j])) inscribableMu.push_back(vecMu[j]);
    
    //rpcx = muX;
    //rpcy = muY;
    //rpcz = vecMu[j].pRPC.Z();
    //tr.Fill();
    
    //cout << endl;
    //cout << iav2ctr.X() << " " << xmin << " " << xmax << endl;
    //cout << iav2ctr.Y() << " " << ymin << " " << ymax << endl;
    //cout << muX << " " << muY << endl;
  }
  
  //tr.Write();
  //tfout.Close();
  
  
  return inscribableMu;
}




/***********************************************************************
main
***********************************************************************/
int main(int argc, char** argv)
{
  /// container for hall geometry. key is hall id [1..3]
  map<int, HallGeoDisc> hallGeom;
  
  /// output container: mean muon energy for each AD
  vector<double> vecAdMeanMuonEnergy;
  
  
  /// initialize hall geometry
  for(int i = 1; i <= 3; i++) hallGeom[i] = HallGeoDisc(i);
  
  /// print the geometry information
  for(int i = 1; i <= 3; i++)
  {
    cout << "EH" << i << ":" << endl;
    for(map<int, XYZPoint>::iterator it = hallGeom[i].m_iavOrigins.begin(); it != hallGeom[i].m_iavOrigins.end(); it++)
    {
      cout << "IAV" << it->first << " center: ";
      cout << it->second.X() << " " << it->second.Y() << " " << it->second.Z() << endl;
    }
    cout << "hall origin: ";
    cout << hallGeom[i].m_hallOrigin.X() << " " << hallGeom[i].m_hallOrigin.Y() << " " << hallGeom[i].m_hallOrigin.Z() << endl;
    cout << "local x: " << hallGeom[i].m_localxAxis.X() << " " << hallGeom[i].m_localxAxis.Y() << " " << hallGeom[i].m_localxAxis.Z() << endl;
    cout << "RPC local z: " << hallGeom[i].m_localzRpc << endl;
    cout << "RPC x and y extension: " << hallGeom[i].m_localRpcxCoverage << " " << hallGeom[i].m_localRpcyCoverage << endl;
  }
  
  
  
  vector<vector<OneMuData> > resMu;
  /// process halls
  map<int, HallGeoDisc>::iterator hallIt = hallGeom.begin();
  for(; hallIt != hallGeom.end(); hallIt++)
  {
    int hall = hallIt->first;
    map<int, XYZPoint>::iterator itAd = hallGeom[hall].m_iavOrigins.begin();
    for(; itAd != hallGeom[hall].m_iavOrigins.end(); itAd++)
    {
      int adId = itAd->first;
      resMu.push_back(getMeanMuonEnergyForOneAd(adId, hallGeom[hall], hall));
    }
  }
  
  
  /// output result
  ofstream resfile("result");
  for(unsigned int i = 0; i < resMu.size(); i++)
  {
    vector<OneMuData> curAdMu = resMu[i];
    vector<double> en;
    for(unsigned int j = 0; j < curAdMu.size(); j++) en.push_back(curAdMu[j].E);
    resfile << TMath::Mean(en.begin(), en.end()) << " ";
    resfile << TMath::RMS(en.begin(), en.end()) << endl;
  }
  resfile.close();
  
  
  return 0;
}
