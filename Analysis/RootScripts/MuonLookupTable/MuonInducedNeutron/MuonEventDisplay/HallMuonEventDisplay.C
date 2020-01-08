/*
 * Firstly, source NuWa.
 * Then, before precompiling this script, "LoadDataModel.C" has to be executed beforehand.
 */



#include <iostream>
#include <fstream>
#include <sstream>
#include "Math/Vector3D.h"
#include "PerCalibReadoutEvent/PerCalibReadoutHeader.h"
#include "TCanvas.h"
#include "TEntryList.h"
#include "TFile.h"
#include "TGeoManager.h"
#include "TGeoMaterial.h"
#include "TGeoMatrix.h"
#include "TGeoMedium.h"
#include "TGLCamera.h"
#include "TGLViewer.h"
#include "TLeaf.h"
#include "TMath.h"
#include "TROOT.h"
#include "TTree.h"

/// radius of the imaginary sphere for the extermities of a muon track
#define RUNIVERSE 11000


using namespace ROOT::Math;


/* function prototype declaration */
void prepareAdGeometry();
void prepareWsGeometry();
void prepareRpcGeometry();
void prepareSiteGeometry();
pair<double, double> findTrackExtremities(pair<XYZVector, XYZVector>);


/* global variables used across the script */
/// file names
string rawfn("raw.root");
string procfn("neutron.root");

int site = -1;

/// geometry container
bool geomReady = false;
bool differentSite;
map<int, XYZVector> detOs;
map<int, double> detRotAng;
map<int, map<int, XYZVector> > sensorPtns;
map<int, map<int, XYZVector> > sensorDirs;
map<int, XYZVector> oavCtr;
map<int, XYZVector> iavCtr;

int HallMuonEventDisplay(int entry = 0, bool drawpmt = true, bool drawrpc = true)
{
  map<int, int> triggerNumbers;
  
  /// calibration container
  map<int, map<int, double> > pmtCharge;
  map<int, double> maxCharge;
  vector<int> rpcHitStrip;
  /// reconstruction container
  map<int, XYZVector> pRecs;
  vector<pair<XYZVector, XYZVector> > trComb;
  map<int, pair<XYZVector, XYZVector> > pOavTrackIntersections;
  map<int, pair<XYZVector, XYZVector> > pIavTrackIntersections;
  map<int, pair<XYZVector, XYZVector> > pCylTrackIntersections;
  map<int, pair<double, double> >       tCylTrackIntersections;
  
  TFile* rawf = new TFile(rawfn.c_str());
  TTree* tCalibRoHdr = (TTree*)rawf->Get("/Event/CalibReadout/CalibReadoutHeader");
  TTree* tSpall = (TTree*)rawf->Get("/Event/Data/Physics/Spallation");
  
  TFile* procf = new TFile(procfn.c_str());
  TTree* tRec = (TTree*)procf->Get("RPC-OWS/mun");
  
  /// check if the event entry is valid
  if(entry < 0 || entry >= tRec->GetEntries())
  {
    cerr << "specified muon doesn't exist" << endl;
    return -1;
  }
  
  /// get the muon id, i.e. muon tMu_s & tMu_ns
  /// get the reconstructed points
  int tMu_s, tMu_ns;
  tRec->GetEntry(entry);
  tMu_s = (int)tRec->GetLeaf("tMu_s")->GetValue();
  tMu_ns = (int)tRec->GetLeaf("tMu_ns")->GetValue();
  if((int)tRec->GetLeaf("nRecAd1")->GetValue())
    pRecs[1] = XYZVector(tRec->GetLeaf("xAd1")->GetValue(),tRec->GetLeaf("yAd1")->GetValue(),tRec->GetLeaf("zAd1")->GetValue());
  if((int)tRec->GetLeaf("nRecAd2")->GetValue())
    pRecs[2] = XYZVector(tRec->GetLeaf("xAd2")->GetValue(),tRec->GetLeaf("yAd2")->GetValue(),tRec->GetLeaf("zAd2")->GetValue());
  if((int)tRec->GetLeaf("nRecAd3")->GetValue())
    pRecs[3] = XYZVector(tRec->GetLeaf("xAd3")->GetValue(),tRec->GetLeaf("yAd3")->GetValue(),tRec->GetLeaf("zAd3")->GetValue());
  if((int)tRec->GetLeaf("nRecAd4")->GetValue())
    pRecs[4] = XYZVector(tRec->GetLeaf("xAd4")->GetValue(),tRec->GetLeaf("yAd4")->GetValue(),tRec->GetLeaf("zAd4")->GetValue());
  if((int)tRec->GetLeaf("nRecIws")->GetValue())
    pRecs[5] = XYZVector(tRec->GetLeaf("xIws")->GetValue(),tRec->GetLeaf("yIws")->GetValue(),tRec->GetLeaf("zIws")->GetValue());
  if((int)tRec->GetLeaf("nRecOws")->GetValue())
    pRecs[6] = XYZVector(tRec->GetLeaf("xOws")->GetValue(),tRec->GetLeaf("yOws")->GetValue(),tRec->GetLeaf("zOws")->GetValue());
  if((int)tRec->GetLeaf("nRecRpcA")->GetValue())
    pRecs[8] = XYZVector(tRec->GetLeaf("xRpcA")->GetValue(),tRec->GetLeaf("yRpcA")->GetValue(),tRec->GetLeaf("zRpcA")->GetValue());
  if((int)tRec->GetLeaf("nRecRpcT")->GetValue())
    pRecs[9] = XYZVector(tRec->GetLeaf("xRpcT")->GetValue(),tRec->GetLeaf("yRpcT")->GetValue(),tRec->GetLeaf("zRpcT")->GetValue());
  /// retrieve track
  if((int)tRec->GetLeaf("nTracks")->GetValue())
  {
    double th = tRec->GetLeaf("theta")->GetValue();
    double phi = tRec->GetLeaf("phi")->GetValue();
    double x = sin(th)*cos(phi);
    double y = sin(th)*sin(phi);
    double z = cos(th);
    XYZVector dir(x, y, z);
    
    trComb.push_back(pair<XYZVector, XYZVector>(XYZVector(tRec->GetLeaf("xCt")->GetValue(), tRec->GetLeaf("yCt")->GetValue(), tRec->GetLeaf("zCt")->GetValue()), dir));
  }
  /// get OAV in and out points
  if((int)tRec->GetLeaf("passOav1")->GetValue())
  {
    XYZVector pOavIn(tRec->GetLeaf("xOav1In")->GetValue(),tRec->GetLeaf("yOav1In")->GetValue(),tRec->GetLeaf("zOav1In")->GetValue());
    XYZVector pOavOut(tRec->GetLeaf("xOav1Out")->GetValue(),tRec->GetLeaf("yOav1Out")->GetValue(),tRec->GetLeaf("zOav1Out")->GetValue());
    pOavTrackIntersections[1] = pair<XYZVector, XYZVector>(pOavIn, pOavOut);
  }
  if((int)tRec->GetLeaf("passOav2")->GetValue())
  {
    XYZVector pOavIn(tRec->GetLeaf("xOav2In")->GetValue(),tRec->GetLeaf("yOav2In")->GetValue(),tRec->GetLeaf("zOav2In")->GetValue());
    XYZVector pOavOut(tRec->GetLeaf("xOav2Out")->GetValue(),tRec->GetLeaf("yOav2Out")->GetValue(),tRec->GetLeaf("zOav2Out")->GetValue());
    pOavTrackIntersections[2] = pair<XYZVector, XYZVector>(pOavIn, pOavOut);
  }
  if((int)tRec->GetLeaf("passOav3")->GetValue())
  {
    XYZVector pOavIn(tRec->GetLeaf("xOav3In")->GetValue(),tRec->GetLeaf("yOav3In")->GetValue(),tRec->GetLeaf("zOav3In")->GetValue());
    XYZVector pOavOut(tRec->GetLeaf("xOav3Out")->GetValue(),tRec->GetLeaf("yOav3Out")->GetValue(),tRec->GetLeaf("zOav3Out")->GetValue());
    pOavTrackIntersections[3] = pair<XYZVector, XYZVector>(pOavIn, pOavOut);
  }
  if((int)tRec->GetLeaf("passOav4")->GetValue())
  {
    XYZVector pOavIn(tRec->GetLeaf("xOav4In")->GetValue(),tRec->GetLeaf("yOav4In")->GetValue(),tRec->GetLeaf("zOav4In")->GetValue());
    XYZVector pOavOut(tRec->GetLeaf("xOav4Out")->GetValue(),tRec->GetLeaf("yOav4Out")->GetValue(),tRec->GetLeaf("zOav4Out")->GetValue());
    pOavTrackIntersections[4] = pair<XYZVector, XYZVector>(pOavIn, pOavOut);
  }
  /// get IAV in and out points
  if((int)tRec->GetLeaf("passIav1")->GetValue())
  {
    XYZVector pIavIn(tRec->GetLeaf("xIav1In")->GetValue(),tRec->GetLeaf("yIav1In")->GetValue(),tRec->GetLeaf("zIav1In")->GetValue());
    XYZVector pIavOut(tRec->GetLeaf("xIav1Out")->GetValue(),tRec->GetLeaf("yIav1Out")->GetValue(),tRec->GetLeaf("zIav1Out")->GetValue());
    pIavTrackIntersections[1] = pair<XYZVector, XYZVector>(pIavIn, pIavOut);
  }
  if((int)tRec->GetLeaf("passIav2")->GetValue())
  {
    XYZVector pIavIn(tRec->GetLeaf("xIav2In")->GetValue(),tRec->GetLeaf("yIav2In")->GetValue(),tRec->GetLeaf("zIav2In")->GetValue());
    XYZVector pIavOut(tRec->GetLeaf("xIav2Out")->GetValue(),tRec->GetLeaf("yIav2Out")->GetValue(),tRec->GetLeaf("zIav2Out")->GetValue());
    pIavTrackIntersections[2] = pair<XYZVector, XYZVector>(pIavIn, pIavOut);
  }
  if((int)tRec->GetLeaf("passIav3")->GetValue())
  {
    XYZVector pIavIn(tRec->GetLeaf("xIav3In")->GetValue(),tRec->GetLeaf("yIav3In")->GetValue(),tRec->GetLeaf("zIav3In")->GetValue());
    XYZVector pIavOut(tRec->GetLeaf("xIav3Out")->GetValue(),tRec->GetLeaf("yIav3Out")->GetValue(),tRec->GetLeaf("zIav3Out")->GetValue());
    pIavTrackIntersections[3] = pair<XYZVector, XYZVector>(pIavIn, pIavOut);
  }
  if((int)tRec->GetLeaf("passIav4")->GetValue())
  {
    XYZVector pIavIn(tRec->GetLeaf("xIav4In")->GetValue(),tRec->GetLeaf("yIav4In")->GetValue(),tRec->GetLeaf("zIav4In")->GetValue());
    XYZVector pIavOut(tRec->GetLeaf("xIav4Out")->GetValue(),tRec->GetLeaf("yIav4Out")->GetValue(),tRec->GetLeaf("zIav4Out")->GetValue());
    pIavTrackIntersections[4] = pair<XYZVector, XYZVector>(pIavIn, pIavOut);
  }
  /// get the tangent points of the inscribed cylinders
  for(int i = 1; i <=4; i++)
  {
    stringstream inscribable;
    inscribable << "inscribable" << i;
    if((int)tRec->GetLeaf(inscribable.str().c_str())->GetValue())
    {
      stringstream xup, yup, zup, xbot, ybot, zbot, tup, tbot;
      xup << "xCyl" << i << "Up";
      yup << "yCyl" << i << "Up";
      zup << "zCyl" << i << "Up";
      xbot << "xCyl" << i << "Bot";
      ybot << "yCyl" << i << "Bot";
      zbot << "zCyl" << i << "Bot";
      tup << "tCyl" << i << "Up";
      tbot << "tCyl" << i << "Bot";
      XYZVector pup(tRec->GetLeaf(xup.str().c_str())->GetValue(),tRec->GetLeaf(yup.str().c_str())->GetValue(),tRec->GetLeaf(zup.str().c_str())->GetValue());
      XYZVector pbot(tRec->GetLeaf(xbot.str().c_str())->GetValue(),tRec->GetLeaf(ybot.str().c_str())->GetValue(),tRec->GetLeaf(zbot.str().c_str())->GetValue());
      pCylTrackIntersections[i] = pair<XYZVector, XYZVector>(pup, pbot);
      tCylTrackIntersections[i] = pair<double, double>(tRec->GetLeaf(tup.str().c_str())->GetValue(),tRec->GetLeaf(tbot.str().c_str())->GetValue());
    }
  }
  
  
  /// retrieve constituent event IDs from Spallation tree
  stringstream evtsel;
  evtsel << "tMu_s==" << tMu_s << "&&tMu_ns==" << tMu_ns;
  tSpall->Draw(">>elist", evtsel.str().c_str(), "entrylist");
  TEntryList* elist = (TEntryList*)gDirectory->Get("elist");
  if(!(elist->GetN()))
  {
    cerr << "specified event doesn't exist" << endl;
    return -1;
  }
  tSpall->GetEntry(elist->GetEntry(0));
  int tn;
  tn = (int)tSpall->GetLeaf("triggerNumber_AD1")->GetValue();
  if(tn >= 0) triggerNumbers[1] = tn;
  tn = (int)tSpall->GetLeaf("triggerNumber_AD2")->GetValue();
  if(tn >= 0) triggerNumbers[2] = tn;
  tn = (int)tSpall->GetLeaf("triggerNumber_AD3")->GetValue();
  if(tn >= 0) triggerNumbers[3] = tn;
  tn = (int)tSpall->GetLeaf("triggerNumber_AD4")->GetValue();
  if(tn >= 0) triggerNumbers[4] = tn;
  tn = (int)tSpall->GetLeaf("triggerNumber_IWS")->GetValue();
  if(tn >= 0) triggerNumbers[5] = tn;
  tn = (int)tSpall->GetLeaf("triggerNumber_OWS")->GetValue();
  if(tn >= 0) triggerNumbers[6] = tn;
  tn = (int)tSpall->GetLeaf("triggerNumber_RPC")->GetValue();
  if(tn >= 0) triggerNumbers[7] = tn;
  
  /// retrieve calibration information from CalibReadoutHeader
  PerCalibReadoutHeader* rh = 0;
  tCalibRoHdr->SetBranchAddress("CalibReadout_CalibReadoutHeader", &rh);
  map<int, int>::iterator detit = triggerNumbers.begin();
  for(; detit != triggerNumbers.end(); detit++)
  {
    stringstream tnsel;
    tnsel << "detector==" << detit->first << "&&triggerNumber==" << detit->second;
    tCalibRoHdr->Draw(">>elist", tnsel.str().c_str(), "entrylist");
    TEntryList* elist2 = (TEntryList*)gDirectory->Get("elist");
    tCalibRoHdr->GetEntry(elist2->GetEntry(0));
    
    /// check whether same site
    differentSite = false;
    if((int)rh->site != site) differentSite = true;
    site = rh->site;
    
    /// store sensor info
    if(detit->first <= 4)
    {
      map<int, double> triggerCharge;
      for(unsigned int i = 0; i < rh->nHitsAD; i++)
      {
        int curId = rh->ring[i]*100+rh->column[i];
        if(triggerCharge.find(curId) == triggerCharge.end())
          triggerCharge[curId] = 0;
        triggerCharge[curId] += rh->chargeAD[i];
      }
      pmtCharge[detit->first] = triggerCharge;
    }
    else if(detit->first != 7)
    {
      map<int, double> triggerCharge;
      for(unsigned int i = 0; i < rh->nHitsPool; i++)
      {
        int curId = rh->wallNumber[i]*100+rh->wallSpot[i];
        if(triggerCharge.find(curId) == triggerCharge.end())
          triggerCharge[curId] = 0;
        triggerCharge[curId] += rh->chargePool[i];
      }
      pmtCharge[detit->first] = triggerCharge;
    }
    else
    {
      for(unsigned int i = 0; i < rh->nHitsRpc; i++)
      {
        int curId = rh->rpcRow[i]*1000+rh->rpcColumn[i]*100+rh->rpcLayer[i]*10+rh->rpcStrip[i];
        rpcHitStrip.push_back(curId);
      }
    }
  }
  
  /// prepare geometry
  if(!geomReady || differentSite)
  {
    geomReady = true;
    cout << "first time getting geometry data" << endl;
    prepareSiteGeometry();
    prepareAdGeometry();
    prepareWsGeometry();
    prepareRpcGeometry();
  }
  
  /// find max charge of each PMT based detectors
  for(map<int, map<int, double> >::iterator it1 = pmtCharge.begin();
      it1 != pmtCharge.end(); it1++)
  {
    double maxchg = -1;
    map<int, double>::iterator it2 = it1->second.begin();
    for(; it2 != it1->second.end(); it2++)
      if(it2->second > maxchg) maxchg = it2->second;
    maxCharge[it1->first] = maxchg;
  }
  
  /* start rendering geometry */
  
  TGeoManager *geom = new TGeoManager("HALL", "hall muon event display");
  /// vacuum for RPC strips for its transparency
  TGeoMaterial *mat = new TGeoMaterial("Vacuum",0,0,0);
  mat->SetTransparency(80);
  TGeoMedium *med = new TGeoMedium("Vacuum",1,mat);
  /// copper for PMTs
  TGeoElement *elementCu = geom->GetElementTable()->FindElement("Copper");
  TGeoMaterial* matCu = new TGeoMaterial("medCu",elementCu,8.96);
  TGeoMedium* medCu = new TGeoMedium("Copper",2,matCu);
  /// vacuum for IAV and OAV
  TGeoMaterial *matAv = new TGeoMaterial("Vacuum",0,0,0);
  matAv->SetTransparency(90);
  TGeoMedium *medAv = new TGeoMedium("Vacuum",1,matAv);
  /// vacuum inscribed cylinder
  TGeoMaterial *matInsCyl = new TGeoMaterial("Vacuum",0,0,0);
  matInsCyl->SetTransparency(30);
  TGeoMedium *medInsCyl = new TGeoMedium("Vacuum",1,matInsCyl);
  
  
  TGeoVolume *top = geom->MakeBox("TOP", med, 20000, 20000, 20000);
  geom->SetTopVolume(top);
  
  map<int, map<int, TGeoVolume*> > volsensor;
  
  map<int, map<int, XYZVector> >::iterator detit2 = sensorPtns.begin();
  for(; detit2 != sensorPtns.end(); detit2++)
  {
    int det = detit2->first;
    map<int, XYZVector> senPtns = detit2->second;
    map<int, XYZVector> senDirs = sensorDirs[det];
    
    bool pmtDetFired = (pmtCharge.find(det) != pmtCharge.end());
    bool rpcFired = (rpcHitStrip.size() > 0);
    map<int, XYZVector>::iterator itsen = senPtns.begin();
    for(; itsen != senPtns.end(); itsen++)
    {
      if(det != 7)
      {
        int pmtId = itsen->first;
        int volId = det*1000+pmtId;
        XYZVector pmtPos = itsen->second;
        pmtPos = pmtPos - detOs[6];
        
        /// Get the row number. If it's 0 => 2inch. Otherwise 8inch
        int rownum = (pmtId/100)%10;
        double radius;
        if(rownum == 0) radius = 25;
        else radius = 100;
        
        /// scale the size of the PMT proportionating to the charge received
        if(pmtDetFired)
          radius = 100 + 200/maxCharge[det]*pmtCharge[det][pmtId];
        
        volsensor[det][pmtId] = geom->MakeSphere(Form("pmt%d",volId),medCu,0,radius);
        if(det <= 4)
          volsensor[det][pmtId]->SetLineColor(kGreen);
        else if(det == 5)
          volsensor[det][pmtId]->SetLineColor(kOrange + 1);
        else
          volsensor[det][pmtId]->SetLineColor(kYellow);
        
        if(drawpmt)
          top->AddNode(volsensor[det][pmtId],1,new TGeoTranslation(pmtPos.X(),pmtPos.Y(),pmtPos.Z()));
      }
      else
      {
        int stripId = itsen->first;
        int volId = det*10000+stripId;
        XYZVector stripPos = itsen->second;
        stripPos = stripPos - detOs[6];
        
        int layer = (stripId/10)%10;
        if(layer == 1 || layer == 4)
          volsensor[det][stripId] = geom->MakeBox(Form("strip%d",volId),med,130,1040,1);
        else
          volsensor[det][stripId] = geom->MakeBox(Form("strip%d",volId),med,1040,130,1);
        
        volsensor[det][stripId]->SetLineColor(kCyan);
        if(rpcFired)
          if(find(rpcHitStrip.begin(), rpcHitStrip.end(), stripId) != rpcHitStrip.end())
            volsensor[det][stripId]->SetLineColor(kRed);
        
        /// determine strip orientations
        double stripRotAng = detRotAng[7]*180./TMath::Pi();
        double tilt = acos(senDirs[stripId].Dot(XYZVector(0,0,1)))*180./TMath::Pi();
        TGeoRotation* rotstrip = new TGeoRotation("strip_rotation", stripRotAng, tilt, 0);
        TGeoCombiTrans *combistrip = new TGeoCombiTrans(stripPos.X(),stripPos.Y(),stripPos.Z(), rotstrip);
        //volsensor[det][stripId]->SetTransparency(60);
        if(drawrpc)
          top->AddNode(volsensor[det][stripId],1,combistrip);
      }
    }
  }
  
  /// draw reconstructed points
  map<int, TGeoVolume*> volRecs;
  map<int, XYZVector>::iterator itrec = pRecs.begin();
  for(; itrec != pRecs.end(); itrec++)
  {
    int det = itrec->first;
    XYZVector pRec = itrec->second;
    pRec = pRec - detOs[6];
    
    volRecs[det] = geom->MakeBox(Form("rec%d",det),medCu,225,225,225);
    if(det == 8 || det == 9)
    {
      volRecs[det]->SetLineColor(kBlue);
      cout << "RPC position: " << pRec.X() << ", " << pRec.Y() << ", " << pRec.Z() << endl;
    }
    else if(det <= 4)
      volRecs[det]->SetLineColor(kGreen+3);
    else if(det == 5)
      volRecs[det]->SetLineColor(kRed);
    else if(det == 6)
    {
      volRecs[det]->SetLineColor(kYellow+1);
      cout << "OWS position: " << pRec.X() << ", " << pRec.Y() << ", " << pRec.Z() << endl;
    }
    top->AddNode(volRecs[det],1,new TGeoTranslation(pRec.X(),pRec.Y(),pRec.Z()));
  }
  /// draw tracks
  TGeoVolume* volTrComb;
  for(unsigned int j = 0; j < trComb.size(); j++)
  {
    XYZVector pos = trComb[j].first;
    pos = pos - detOs[6];
    XYZVector dir = trComb[j].second;
    
    double angZ = dir.Phi()*180./TMath::Pi()+90;
    //double tilt = acos(dir.Dot(XYZVector(0,0,1)))*180./TMath::Pi();
    double tilt = dir.Theta()*180./TMath::Pi();
    cout << "track theta and phi: " << dir.Theta() << ", " << dir.Phi() << endl;
    
    pair<double, double> ts = findTrackExtremities(pair<XYZVector, XYZVector>(pos, dir));
    volTrComb = geom->MakeTube("TrComb", medCu, 0, 80, 10000);
    //volTrComb = geom->MakeTube("TrComb", medCu, 0, 80, (ts.first-ts.second)/2);
    
    TGeoRotation* orientation = new TGeoRotation("trComb_orientation", angZ, tilt, 0);
    TGeoCombiTrans* transOrient = new TGeoCombiTrans(pos.X(),pos.Y(),pos.Z(), orientation);
    TGeoTranslation* zTrans = new TGeoTranslation(0,0,(ts.first+ts.second)/2);
    TGeoHMatrix h = (*transOrient) * (*zTrans);
    
    volTrComb->SetLineColor(kMagenta);
    top->AddNode(volTrComb,1,transOrient);
    //top->AddNode(volTrComb,1,new TGeoHMatrix(h));
  }
  
  /// draw outer acrylic vessels
  map<int, TGeoVolume*> volOavs;
  map<int, XYZVector>::iterator itOav = oavCtr.begin();
  for(; itOav != oavCtr.end(); itOav++)
  {
    int det = itOav->first;
    XYZVector pos = itOav->second;
    pos = pos - detOs[6];
    
    volOavs[det] = geom->MakeTube(Form("OAV%d", det), medAv, 0, 2000, 2000);
    volOavs[det]->SetLineColor(kAzure+1);
    top->AddNode(volOavs[det],1,new TGeoTranslation(pos.X(),pos.Y(),pos.Z()));
  }
  /// draw inner acrylic vessels
  map<int, TGeoVolume*> volIavs;
  map<int, XYZVector>::iterator itIav = iavCtr.begin();
  for(; itIav != iavCtr.end(); itIav++)
  {
    int det = itIav->first;
    XYZVector pos = itIav->second;
    pos = pos - detOs[6];
    
    volIavs[det] = geom->MakeTube(Form("IAV%d", det), medAv, 0, 1500, 1500);
    volIavs[det]->SetLineColor(kAzure);
    top->AddNode(volIavs[det],1,new TGeoTranslation(pos.X(),pos.Y(),pos.Z()));
  }
  
  /// draw track and OAV intersections
  map<int, TGeoVolume*> volOavIn;
  map<int, TGeoVolume*> volOavOut;
  map<int, pair<XYZVector, XYZVector> >::iterator itIntO = pOavTrackIntersections.begin();
  for(; itIntO != pOavTrackIntersections.end(); itIntO++)
  {
    int det = itIntO->first;
    XYZVector posIn = itIntO->second.first;
    posIn = posIn - detOs[6];
    XYZVector posOut = itIntO->second.second;
    posOut = posOut - detOs[6];
    
    volOavIn[det] = geom->MakeSphere(Form("OAV%dIn", det), medCu, 0, 100);
    volOavIn[det]->SetLineColor(kBlue);
    top->AddNode(volOavIn[det],1,new TGeoTranslation(posIn.X(),posIn.Y(),posIn.Z()));
    volOavOut[det] = geom->MakeSphere(Form("OAV%dOut", det), medCu, 0, 100);
    volOavOut[det]->SetLineColor(kBlue);
    top->AddNode(volOavOut[det],1,new TGeoTranslation(posOut.X(),posOut.Y(),posOut.Z()));
  }
  /// draw track and IAV intersections
  map<int, TGeoVolume*> volIavIn;
  map<int, TGeoVolume*> volIavOut;
  map<int, pair<XYZVector, XYZVector> >::iterator itIntI = pIavTrackIntersections.begin();
  for(; itIntI != pIavTrackIntersections.end(); itIntI++)
  {
    int det = itIntI->first;
    XYZVector posIn = itIntI->second.first;
    posIn = posIn - detOs[6];
    XYZVector posOut = itIntI->second.second;
    posOut = posOut - detOs[6];
    
    volIavIn[det] = geom->MakeSphere(Form("IAV%dIn", det), medCu, 0, 100);
    volIavIn[det]->SetLineColor(kRed);
    top->AddNode(volIavIn[det],1,new TGeoTranslation(posIn.X(),posIn.Y(),posIn.Z()));
    volIavOut[det] = geom->MakeSphere(Form("IAV%dOut", det), medCu, 0, 100);
    volIavOut[det]->SetLineColor(kRed);
    top->AddNode(volIavOut[det],1,new TGeoTranslation(posOut.X(),posOut.Y(),posOut.Z()));
  }
  /// draw inscribed cylinder
  map<int, TGeoVolume*> volTangUp;
  map<int, TGeoVolume*> volTangBot;
  map<int, pair<XYZVector, XYZVector> >::iterator itTang = pCylTrackIntersections.begin();
  for(; itTang != pCylTrackIntersections.end(); itTang++)
  {
    int det = itTang->first;
    XYZVector posIn = itTang->second.first;
    posIn = posIn - detOs[6];
    XYZVector posOut = itTang->second.second;
    posOut = posOut - detOs[6];
    
    volTangUp[det] = geom->MakeSphere(Form("insc%dup", det), medCu, 0, 100);
    volTangUp[det]->SetLineColor(kRed);
    top->AddNode(volTangUp[det],1,new TGeoTranslation(posIn.X(),posIn.Y(),posIn.Z()));
    volTangBot[det] = geom->MakeSphere(Form("insc%dbot", det), medCu, 0, 100);
    volTangBot[det]->SetLineColor(kRed);
    top->AddNode(volTangBot[det],1,new TGeoTranslation(posOut.X(),posOut.Y(),posOut.Z()));
  }
  map<int, TGeoVolume*> volInsCyl;
  map<int, pair<double, double> >::iterator itInsCyl = tCylTrackIntersections.begin();
  for(; itInsCyl != tCylTrackIntersections.end(); itInsCyl++)
  {
    int det = itInsCyl->first;
    double tup = itInsCyl->second.first;
    double tbot = itInsCyl->second.second;
    
    //volInsCyl[det] = geom->MakeTube(Form("inscyl%d", det), medAv, 0, 1000,(tup-tbot)/2.);
    volInsCyl[det] = geom->MakeTube(Form("inscyl%d", det), medInsCyl, 0, 1000,(tup-tbot)/2.);
    volInsCyl[det]->SetLineColor(kYellow);
    
    XYZVector dir, disp;
    if(trComb.size())
    {
      dir = trComb[0].second;
      disp = (trComb[0].first - detOs[6])+(tup+tbot)/2.*dir;
    }
    
    /// get rotation matrix
    double angZ = dir.Phi()*180./TMath::Pi()+90;
    double tilt = acos(dir.Dot(XYZVector(0,0,1)))*180./TMath::Pi();
    TGeoRotation* orientation = new TGeoRotation("inscyl_orientation", angZ, tilt, 0);
    TGeoCombiTrans* transOrient = new TGeoCombiTrans(disp.X(),disp.Y(),disp.Z(), orientation);
    top->AddNode(volInsCyl[det],1,transOrient);
  }
  
  geom->CloseGeometry();
  
  /* end rendering geometry */
  
  //top->Draw();
  top->Draw("ogl");
  TGLViewer* glv = (TGLViewer*)gROOT->GetSelectedPad()->GetViewer3D();
  double wc[3] = {0.};
  glv->SetPerspectiveCamera(TGLViewer::kCameraPerspXOY, 30, 3000, wc, 0, 0);
  glv->SetCurrentCamera(TGLViewer::kCameraPerspXOY);
  
  return 0;
}


void prepareAdGeometry()
{
  stringstream infn;
  infn << "db/AdPmt" << site;
  ifstream inf(infn.str().c_str());
  string templine;
  int col = 0;
  int detid = 0;
  int sensorid = 0;
  XYZVector pPos;
  XYZVector vDir;
  
  while(inf >> templine)
  {
    col = col % 10;
    
    if(col == 1) detid = atoi(templine.c_str());
    if(col == 2) sensorid += atoi(templine.c_str())*100;
    if(col == 3) sensorid += atoi(templine.c_str());
    if(col == 4) pPos.SetX(atof(templine.c_str()));
    if(col == 5) pPos.SetY(atof(templine.c_str()));
    if(col == 6) pPos.SetZ(atof(templine.c_str()));
    if(col == 7) vDir.SetX(atof(templine.c_str()));
    if(col == 8) vDir.SetY(atof(templine.c_str()));
    if(col == 9)
    {
      vDir.SetZ(atof(templine.c_str()));
      
      sensorPtns[detid][sensorid] = pPos;
      sensorDirs[detid][sensorid] = vDir;
      sensorid = 0;
    }
    
    col++;
  }
  inf.close();
  
  
  /// get IAV centers
  stringstream fnIav;
  fnIav << "db/IAV" << site;
  ifstream fIav(fnIav.str().c_str());
  col = 0;
  detid = 0;
  
  while(fIav >> templine)
  {
    col = col % 8;
    
    if(col == 1) detid = atoi(templine.c_str());
    if(col == 2) pPos.SetX(atof(templine.c_str()));
    if(col == 3) pPos.SetY(atof(templine.c_str()));
    if(col == 4) pPos.SetZ(atof(templine.c_str()));
    if(col == 5) vDir.SetX(atof(templine.c_str()));
    if(col == 6) vDir.SetY(atof(templine.c_str()));
    if(col == 7)
    {
      vDir.SetZ(atof(templine.c_str()));
      
      iavCtr[detid] = pPos;
    }
    
    col++;
  }
  fIav.close();
  
  /// get OAV centers
  stringstream fnOav;
  fnOav << "db/OAV" << site;
  ifstream fOav(fnOav.str().c_str());
  col = 0;
  detid = 0;
  
  while(fOav >> templine)
  {
    col = col % 8;
    
    if(col == 1) detid = atoi(templine.c_str());
    if(col == 2) pPos.SetX(atof(templine.c_str()));
    if(col == 3) pPos.SetY(atof(templine.c_str()));
    if(col == 4) pPos.SetZ(atof(templine.c_str()));
    if(col == 5) vDir.SetX(atof(templine.c_str()));
    if(col == 6) vDir.SetY(atof(templine.c_str()));
    if(col == 7)
    {
      vDir.SetZ(atof(templine.c_str()));
      
      oavCtr[detid] = pPos;
    }
    
    col++;
  }
  fOav.close();
}


pair<double, double> findTrackExtremities(pair<XYZVector, XYZVector> tr)
{
  double tup, tbot;
  
  XYZVector vCentroid = tr.first;
  XYZVector vDir = tr.second;
  
  // length of the centroid vector
  double lenCent = sqrt(vCentroid.Mag2());
  // angle between centroid vector and direction vector
  double angBetw = acos(vCentroid.Dot(vDir)/lenCent);
  // signed distance from the middle point of the cord to the centroid
  double dMidCent = lenCent*cos(angBetw);
  // middle point of the cord
  XYZVector vMid = vCentroid - dMidCent * vDir;
  // distance from the origin to the track
  double dPerp = lenCent*sin(angBetw);
  if(dPerp > RUNIVERSE) return pair<double, double>(0,0);
  // half length of the cord
  double lenHalfCord = sqrt(RUNIVERSE*RUNIVERSE-dPerp*dPerp);
  // vector from the middle point to the centroid
  XYZVector vMidCent = vCentroid - vMid;
  // parameter from the middle point to the centroid
  double tMidCent = dMidCent;
  if(vMidCent.Z() < 0) tMidCent = -dMidCent;
  
  tup = lenHalfCord - tMidCent;
  tbot = -lenHalfCord - tMidCent;
  
  return pair<double, double>(tup, tbot);
}


void prepareWsGeometry()
{
  stringstream infn;
  infn << "db/WsPmt" << site;
  ifstream inf(infn.str().c_str());
  string templine;
  int col = 0;
  int detid = 0;
  int sensorid = 0;
  XYZVector pPos;
  XYZVector vDir;
  
  while(inf >> templine)
  {
    col = col % 11;
    
    if(col == 1) detid = atoi(templine.c_str());
    if(col == 2) sensorid += atoi(templine.c_str())*100;
    if(col == 3) sensorid += atoi(templine.c_str());
    if(col == 5) pPos.SetX(atof(templine.c_str()));
    if(col == 6) pPos.SetY(atof(templine.c_str()));
    if(col == 7) pPos.SetZ(atof(templine.c_str()));
    if(col == 8) vDir.SetX(atof(templine.c_str()));
    if(col == 9) vDir.SetY(atof(templine.c_str()));
    if(col == 10)
    {
      vDir.SetZ(atof(templine.c_str()));
      
      sensorPtns[detid][sensorid] = pPos;
      sensorDirs[detid][sensorid] = vDir;
      sensorid = 0;
    }
    
    col++;
  }
  inf.close();
}


void prepareRpcGeometry()
{
  stringstream infn;
  infn << "db/RpcStrip" << site;
  ifstream inf(infn.str().c_str());
  string templine;
  int col = 0;
  int detid = 0;
  int sensorid = 0;
  XYZVector pPos;
  XYZVector vDir;
  
  while(inf >> templine)
  {
    col = col % 12;
    
    if(col == 1) detid = atoi(templine.c_str());
    if(col == 2) sensorid += atoi(templine.c_str())*1000;
    if(col == 3) sensorid += atoi(templine.c_str())*100;
    if(col == 4) sensorid += atoi(templine.c_str())*10;
    if(col == 5) sensorid += atoi(templine.c_str());
    if(col == 6) pPos.SetX(atof(templine.c_str()));
    if(col == 7) pPos.SetY(atof(templine.c_str()));
    if(col == 8) pPos.SetZ(atof(templine.c_str()));
    if(col == 9) vDir.SetX(atof(templine.c_str()));
    if(col == 10) vDir.SetY(atof(templine.c_str()));
    if(col == 11)
    {
      vDir.SetZ(atof(templine.c_str()));
      
      sensorPtns[detid][sensorid] = pPos;
      sensorDirs[detid][sensorid] = vDir;
      sensorid = 0;
    }
    
    col++;
  }
  inf.close();
}


void prepareSiteGeometry()
{
  stringstream infn;
  infn << "db/DetectorOrigins" << site;
  ifstream inf(infn.str().c_str());
  string templine;
  int col = 0;
  int detid = 0;
  XYZVector pPos;
  XYZVector vDir;
  
  while(inf >> templine)
  {
    col = col % 8;
    
    if(col == 1) detid = atoi(templine.c_str());
    if(col == 2) pPos.SetX(atof(templine.c_str()));
    if(col == 3) pPos.SetY(atof(templine.c_str()));
    if(col == 4) pPos.SetZ(atof(templine.c_str()));
    if(col == 5) vDir.SetX(atof(templine.c_str()));
    if(col == 6) vDir.SetY(atof(templine.c_str()));
    if(col == 7)
    {
      vDir.SetZ(atof(templine.c_str()));
      detOs[detid] = pPos;
      detRotAng[detid] = atan(vDir.Y()/vDir.X());
    }
    
    col++;
  }
  inf.close();
}
