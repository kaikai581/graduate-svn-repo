#include <cmath>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include "Math/Vector3D.h"
#include "TCanvas.h"
#include "TGeoManager.h"
#include "TGeoMaterial.h"
#include "TGeoMatrix.h"
#include "TGeoMedium.h"
#include "TGLCamera.h"
#include "TGLViewer.h"
#include "TMath.h"
#include "TROOT.h"


using namespace ROOT::Math;
using namespace std;

double locgblAngle = 2.14500911480304701e+00*180./TMath::Pi();
double stripTiltAng = 4.90151094498128964e-02*180./TMath::Pi();

void SensorArrangement(int hall=1)
{
  if(hall > 3 || hall < 1)
  {
    cerr << "Hall number can only be 1, 2, or 3." << endl;
    return;
  }
  
  stringstream detectorOrigins;
  stringstream ifnAdPmt, ifnWsPmt, ifnRpcStrip;
  
  int hallsuf = pow(2.,hall-1.);
  
  detectorOrigins << "db/DetectorOrigins" << hallsuf;
  ifnAdPmt << "db/AdPmt" << hallsuf;
  ifnWsPmt << "db/WsPmt" << hallsuf;
  ifnRpcStrip << "db/RpcStrip" << hallsuf;
  
  /// global variables for geometry
  map<int, XYZVector> detOs;
  map<int, map<int, XYZVector> > sensorPtns;

  /* start retrieving data */
  ifstream inf(detectorOrigins.str().c_str());
  string templine;
  int col = 0;
  int detid = 0;
  int sensorid = 0;
  XYZVector pPos;
  while(inf >> templine)
  {
    col = col % 8;
    
    if(col == 1) detid = atoi(templine.c_str());
    if(col == 2) pPos.SetX(atof(templine.c_str()));
    if(col == 3) pPos.SetY(atof(templine.c_str()));
    if(col == 4) pPos.SetZ(atof(templine.c_str()));
    if(col == 7) detOs[detid] = pPos;
    
    col++;
  }
  inf.close();
  
  //map<int, XYZVector>::iterator detit = detOs.begin();
  //for(; detit != detOs.end(); detit++)
    //cout << detit->second.X() << " " << detit->second.Y() << " " << detit->second.Z() << endl;

  inf.open(ifnAdPmt.str().c_str());
  col = 0;
  detid = 0;
  sensorid = 0;
  while(inf >> templine)
  {
    col = col % 10;
    
    if(col == 1) detid = atoi(templine.c_str());
    if(col == 2) sensorid += atoi(templine.c_str())*100;
    if(col == 3) sensorid += atoi(templine.c_str());
    if(col == 4) pPos.SetX(atof(templine.c_str()));
    if(col == 5) pPos.SetY(atof(templine.c_str()));
    if(col == 6) pPos.SetZ(atof(templine.c_str()));
    if(col == 9)
    {
      if(sensorPtns.find(detid) == sensorPtns.end())
        sensorPtns[detid] = map<int, XYZVector>();
      sensorPtns[detid][sensorid] = pPos;
      detid = 0;
      sensorid = 0;
    }
    
    col++;
  }
  inf.close();
  
  inf.open(ifnWsPmt.str().c_str());
  col = 0;
  detid = 0;
  sensorid = 0;
  while(inf >> templine)
  {
    col = col % 11;
    
    if(col == 1) detid += atoi(templine.c_str());
    if(col == 2) sensorid += atoi(templine.c_str())*100;
    if(col == 3) sensorid += atoi(templine.c_str());
    if(col == 5) pPos.SetX(atof(templine.c_str()));
    if(col == 6) pPos.SetY(atof(templine.c_str()));
    if(col == 7) pPos.SetZ(atof(templine.c_str()));
    if(col == 10)
    {
      if(sensorPtns.find(detid) == sensorPtns.end())
        sensorPtns[detid] = map<int, XYZVector>();
      sensorPtns[detid][sensorid] = pPos;
      detid = 0;
      sensorid = 0;
    }
    
    col++;
  }
  inf.close();


  inf.open(ifnRpcStrip.str().c_str());
  col = 0;
  detid = 0;
  sensorid = 0;
  while(inf >> templine)
  {
    col = col % 12;
    
    if(col == 1) detid += atoi(templine.c_str());
    if(col == 2) sensorid += atoi(templine.c_str())*1000;
    if(col == 3) sensorid += atoi(templine.c_str())*100;
    if(col == 4) sensorid += atoi(templine.c_str())*10;
    if(col == 5) sensorid += atoi(templine.c_str());
    if(col == 6) pPos.SetX(atof(templine.c_str()));
    if(col == 7) pPos.SetY(atof(templine.c_str()));
    if(col == 8) pPos.SetZ(atof(templine.c_str()));
    if(col == 11)
    {
      if(sensorPtns.find(detid) == sensorPtns.end())
        sensorPtns[detid] = map<int, XYZVector>();
      sensorPtns[detid][sensorid] = pPos;
      detid = 0;
      sensorid = 0;
    }
    
    col++;
  }
  inf.close();
  
  /* end retrieving data */
  
  
  /* start drawing geometry */
  
  TGeoManager *geom = new TGeoManager("hall", "site event display");
  
  TGeoMaterial *mat = new TGeoMaterial("Vacuum",0,0,0);
  
  TGeoMedium *med = new TGeoMedium("Vacuum",1,mat);
  
  TGeoVolume *top = geom->MakeBox("TOP", med, 20000, 20000, 20000);
  geom->SetTopVolume(top);
  
  map<int, map<int, TGeoVolume*> > volsensor;
  
  map<int, map<int, XYZVector> >::iterator totsenit = sensorPtns.begin();
  for(; totsenit != sensorPtns.end(); totsenit++)
  {
    int curdet = totsenit->first;
    map<int, XYZVector> senOneDet = totsenit->second;
    map<int, XYZVector>::iterator senit = senOneDet.begin();
    for(; senit != senOneDet.end(); senit++)
    {
      int sensorId = senit->first;
      XYZVector curtrans = senit->second;
      curtrans = curtrans - detOs[5];
      
      if(curdet != 7)
      {
        /// Get the row number. If it's 0 => 2inch. Otherwise 8inch
        int rownum = sensorId/100.;
        double radius;
        if(rownum == 0) radius = 25;
        else radius = 100;
        
        if(volsensor.find(curdet) == volsensor.end())
          volsensor[curdet] = map<int, TGeoVolume*>();
        volsensor[curdet][sensorId] = geom->MakeSphere(Form("pmt%d",curdet*1000+sensorId),med,0,radius);
        if(curdet <= 4) volsensor[curdet][sensorId]->SetLineColor(kGreen);
        if(curdet >= 5) volsensor[curdet][sensorId]->SetLineColor(kYellow);
        
        top->AddNode(volsensor[curdet][sensorId],1,new TGeoTranslation(curtrans.X(),curtrans.Y(),curtrans.Z()));
      }
      else
      {
        /// for RPC strips deployment
        double tilt = -stripTiltAng;
        int rpcrow = (sensorId/1000)%10;
        int maxrow = 10;
        if(hall != 3) maxrow = 7;
        if(rpcrow == 0 || rpcrow == maxrow) tilt = 0;
        TGeoRotation* rotstrip = new TGeoRotation("track_rotation", locgblAngle, tilt, 0);
        TGeoCombiTrans *combistrip = new TGeoCombiTrans(curtrans.X(),curtrans.Y(),curtrans.Z(), rotstrip);
        
        if(volsensor.find(curdet) == volsensor.end())
          volsensor[curdet] = map<int, TGeoVolume*>();
        
        int layer = (sensorId%100)/10;
        if(layer == 1 || layer == 4)
          volsensor[curdet][sensorId] = geom->MakeBox(Form("strip%d",curdet*10000+sensorId),med,130,1040,1);
        else
          volsensor[curdet][sensorId] = geom->MakeBox(Form("strip%d",curdet*10000+sensorId),med,1040,130,1);
        
        volsensor[curdet][sensorId]->SetLineColor(kBlue);
        top->AddNode(volsensor[curdet][sensorId],1,combistrip);
      }
    }
  }
  
  geom->CloseGeometry();
  TCanvas* c1=new TCanvas("c1","c1",1000,1000);
  //top->Draw();
  
  /* end drawing geometry */
  
  top->Draw("ogl");
  TGLViewer* glv = (TGLViewer*)gROOT->GetSelectedPad()->GetViewer3D();
  double wc[3] = {0.};
  glv->SetPerspectiveCamera(TGLViewer::kCameraPerspXOY, 30, 3000, wc, 0, 0);
  glv->SetCurrentCamera(TGLViewer::kCameraPerspXOY);

}
