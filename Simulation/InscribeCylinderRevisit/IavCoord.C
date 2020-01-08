#include <set>
#include "Math/AxisAngle.h"
#include "Math/Point3D.h"
#include "Math/Polynomial.h"
#include "Math/Vector3D.h"
#include "TGeoManager.h"
#include "TGeoMaterial.h"
#include "TGeoMatrix.h"
#include "TGeoMedium.h"
#include "TGLCamera.h"
#include "TGLViewer.h"
#include "TMath.h"
#include "TROOT.h"
#include "TVirtualPad.h"

#define RIAV 1500.
#define RINS 1000.

using namespace ROOT::Math;


double getZTrack(XYZVector p, XYZVector p0, XYZVector v)
{
  v = v.Unit();
  
  XYZVector vp0p = p - p0;
  double cos = vp0p.Unit().Dot(v);
  
  return sqrt(vp0p.Mag2())*cos;
}


void IavCoord(double xdisp = 500)
{
  /// track info
  XYZVector pXY(xdisp,100,300);
  XYZVector vDir(1,1,1);
  vDir = vDir.Unit();
  
  XYZVector e1(1,0,0);
  XYZVector e2(0,1,0);
  XYZVector e3(0,0,1);
  
  AxisAngle rotateTrack(e3.Cross(vDir), vDir.Theta());
  AxisAngle rotateIav = rotateTrack.Inverse();
  
  double a11 = e1.Dot(rotateIav*e1);
  double a12 = e1.Dot(rotateIav*e2);
  double a13 = e1.Dot(rotateIav*e3);
  double a21 = e2.Dot(rotateIav*e1);
  double a22 = e2.Dot(rotateIav*e2);
  double a23 = e2.Dot(rotateIav*e3);
  
  
  vector<pair<XYZVector, XYZVector> > positionCurvePoints;
  vector<pair<int, int> > regionAndNRoots;
  set<int> regionName;
  
  int nsteps = 360;
  
  for(int i = 0; i < nsteps; i++)
  {
    double x = -TMath::Pi()+TMath::TwoPi()/nsteps*i;
    double R = RIAV;
    double r = RINS;
    double x1 = R*cos(x)-pXY.X();
    double x2 = R*sin(x)-pXY.Y();
    
    double a2 = a13*a13+a23*a23;
    double b1 = a11*x1+a12*x2;
    double b2 = a21*x1+a22*x2;
    double a1 = 2*(a13*b1+a23*b2);
    double a0 = b1*b1+b2*b2-r*r;
    
    Polynomial eq(a2,a1,a0);
    vector<double> roots = eq.FindRealRoots();
    double x31, x32;
    if(roots.size() == 2)
    {
      x31 = roots[0]+pXY.Z();
      x32 = roots[1]+pXY.Z();
      x1 += pXY.X();
      x2 += pXY.Y();
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
  /// -3048.62 -1340.11 300.882 2009.39
  map<int, pair<unsigned int,unsigned int> > extInd;
  for(map<int, vector<XYZVector> >::iterator it = posPartCurve.begin(); it != posPartCurve.end(); it++)
  {
    double max, min;
    for(unsigned int i = 0; i < it->second.size(); i++)
    {
      if(i == 0)
      {
        extInd[it->first] = pair<int,int>(0,0);
        max = getZTrack(it->second[0], pXY, vDir);
        min = max;
      }
      else
      {
        double z = getZTrack(it->second[i], pXY, vDir);
        if(z > max)
        {
          max = z;
          extInd[it->first].first = i;
        }
        if(z < min)
        {
          min = z;
          extInd[it->first].second = i;
        }
      }
    }
  }
  
  
  
  
  /* start rendering geometry */
  TGeoManager *geom = new TGeoManager("World", "the universe");
  TGeoMaterial *mat = new TGeoMaterial("Vacuum",0,0,0);
  TGeoMedium *med = new TGeoMedium("Vacuum",1,mat);
  
  /// make top geometry
  TGeoVolume *top = geom->MakeBox("TOP", med, 20000, 20000, 20000);
  geom->SetTopVolume(top);
  
  /// draw track cylinder
  TGeoVolume* trackCyl = geom->MakeTube("TrCyl", med, 0, RINS, 5*RINS);
  double angZ = vDir.Phi()*180./TMath::Pi()+90;
  double tilt = vDir.Theta()*180./TMath::Pi();
  TGeoRotation* orientation = new TGeoRotation("TrCyl_orientation", angZ, tilt, 0);
  TGeoCombiTrans* transOrient = new TGeoCombiTrans(pXY.X(),pXY.Y(),pXY.Z(), orientation);
  trackCyl->SetLineColor(kMagenta);
  top->AddNode(trackCyl,1,transOrient);
  
  /// draw IAV cylinder
  TGeoVolume* iavCyl = geom->MakeTube("IavCyl", med, 0, RIAV, 3*RIAV);
  iavCyl->SetLineColor(kAzure);
  top->AddNode(iavCyl,1);
  
  /// draw points on the solution curve
  map<int, vector<XYZVector> >::iterator itcur = posPartCurve.begin();
  for(; itcur != posPartCurve.end(); itcur++)
  {
    for(unsigned int i = 0; i < itcur->second.size(); i++)
    {
      TGeoVolume* curPos = geom->MakeSphere(Form("curvep%d%d",itcur->first,i), med, 0, 100);
      curPos->SetLineColor(kYellow);
      if((i == extInd[itcur->first].first) || (i == extInd[itcur->first].second))
        curPos->SetLineColor(kRed);
      top->AddNode(curPos,1,new TGeoTranslation(itcur->second[i].X(),itcur->second[i].Y(),itcur->second[i].Z()));
    }
  }
  
  geom->CloseGeometry();
  /* end rendering geometry */
  
  top->Draw("ogl");
  TGLViewer* glv = (TGLViewer*)gROOT->GetSelectedPad()->GetViewer3D();
  double wc[3] = {0.};
  glv->SetPerspectiveCamera(TGLViewer::kCameraPerspXOY, 30, 3000, wc, 0, 0);
  glv->SetCurrentCamera(TGLViewer::kCameraPerspXOY);
}
