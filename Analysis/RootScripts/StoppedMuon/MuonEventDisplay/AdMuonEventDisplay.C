/*
 * Firstly, source NuWa.
 * Then, before compiling this script, "LoadDataModel.C" has to be executed beforehand.
 */
/*
root [2] XYZVector og(-18079.5,-799699,-6605);
root [3] XYZVector ol(5742./2,0,0);
root [4] XYZVector pg(-18537.7,-799547,-7159.77);
root [5] XYZVector pl(2494.1,-301.899,-18.2696);
root [6] XYZVector vg=pg-og;
root [7] XYZVector vl=pl-ol;
*/


#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include "Math/AxisAngle.h"
#include "Math/Vector3D.h"
#include "PerCalibReadoutEvent/PerCalibReadoutHeader.h"
#include "TEntryList.h"
#include "TFile.h"
#include "TGeoManager.h"
#include "TGeoMaterial.h"
#include "TGeoMatrix.h"
#include "TGeoMedium.h"
#include "TLeaf.h"
#include "TMath.h"
#include "TTree.h"
#include "TROOT.h"
#include "TSystem.h"


using namespace ROOT::Math;
using namespace std;


/// pmt id = hall*10000+detector*1000+ring*100+column
typedef map<int, double> TriggerPmtCharge;

void fillGeometry(int,int,int,map<int,XYZVector>&);
void PrintVector(XYZVector&);

string infn("raw.root");
string infn2("processed.root");
XYZVector pAdCenter;

double dPhi = 2.82128320916106379e+00 - (-2.46623885854318692e+00);

/// coordtype: 0: local, 1: global
int AdMuonEventDisplay(int coordtype = 0, int detector = 1, int trNum = 1441)
{
  if(detector > 4)
  {
    cerr << "specified detector is not an AD" << endl;
    return -1;
  }
  
  /// can be put in the LoadDataModel.C file to avoid multiple loadings.
  //gROOT->ProcessLine(".x $ROOTIOTESTROOT/share/load.C");
  
  
  TFile* inf = new TFile(infn.c_str());
  TTree* tCalibRoHdr = (TTree*)inf->Get("/Event/CalibReadout/CalibReadoutHeader");
  stringstream evtsel;
  evtsel << "detector==" << detector << "&&triggerNumber==" << trNum;
  tCalibRoHdr->Draw(">>elist", evtsel.str().c_str(), "entrylist");
  TEntryList* elist = (TEntryList*)gDirectory->Get("elist");
  if(!(elist->GetN()))
  {
    cerr << "specified event doesn't exist" << endl;
    return -1;
  }
  
  PerCalibReadoutHeader* rh = 0;
  tCalibRoHdr->SetBranchAddress("CalibReadout_CalibReadoutHeader", &rh);
  /// retrieve selected trigger
  tCalibRoHdr->GetEntry(elist->GetEntry(0));
  
  map<int, XYZVector> pmtTranslation;
  fillGeometry(coordtype, rh->site, detector, pmtTranslation);
  
  TriggerPmtCharge triggerCharge;
  for(unsigned int i = 0; i < rh->nHitsAD; i++)
  {
    int curId = rh->site*10000+rh->detector*1000+rh->ring[i]*100+rh->column[i];
    if(triggerCharge.find(curId) == triggerCharge.end())
      triggerCharge[curId] = 0;
    triggerCharge[curId] += rh->chargeAD[i];
  }
  /// find the max charge among PMTs to scale the PMT size
  double maxcharge = 0;
  TriggerPmtCharge::iterator itch = triggerCharge.begin();
  for(; itch != triggerCharge.end(); itch++)
    if(itch->second > maxcharge) maxcharge = itch->second;
  
  /* start drawing geometry */
  
  TGeoManager *geom = new TGeoManager("AD", "AD muon event display");
  
  TGeoMaterial *mat = new TGeoMaterial("Vacuum",0,0,0);
  
  TGeoMedium *med = new TGeoMedium("Vacuum",1,mat);
  
  TGeoVolume *top = geom->MakeBox("TOP", med, 6000, 6000, 6000);
  geom->SetTopVolume(top);
  
  map<int, TGeoVolume*> volPmt;
  
  map<int, XYZVector>::iterator ittrans = pmtTranslation.begin();
  for(; ittrans != pmtTranslation.end(); ittrans++)
  {
    int pmtId = ittrans->first;
    /// Get the row number. If it's 0 => 2inch. Otherwise 8inch
    int rownum = (pmtId/100)%10;
    double radius;
    if(rownum == 0) radius = 25;
    else radius = 100;
    
    /// if this PMT is fired, scale the size to the charge
    bool fired = false;
    if(triggerCharge.find(pmtId) != triggerCharge.end())
    {
      fired = true;
      radius = 100 + triggerCharge[pmtId]/maxcharge*100;
    }
    
    volPmt[pmtId] = geom->MakeSphere(Form("pmt%d",pmtId),med,0,radius);
    if(!fired) volPmt[pmtId]->SetLineColor(kYellow);
    else
    {
      int colorBdy[] = {(int)kViolet, (int)kAzure, (int)kTeal,
                        (int)kSpring, (int)kOrange, (int)kPink};
      int group = (triggerCharge[pmtId]/maxcharge)*6;
      if(group == 6) group--;
      double chgoffset = triggerCharge[pmtId] - (int)(group*(maxcharge/6));
      int coloroffset = (int)(chgoffset/(maxcharge/6./20.)) - 9;
      if(coloroffset > 10) coloroffset = 10;
      volPmt[pmtId]->SetLineColor(colorBdy[group]+coloroffset);
      //volPmt[pmtId]->SetLineColor(kRed);
    }
    
    top->AddNode(volPmt[pmtId],1,new TGeoTranslation(ittrans->second.X(),ittrans->second.Y(),ittrans->second.Z()));
  }
  
  /// draw the reconstructed point
  if(coordtype == 0)
  {
    TTree* tAdSimple = (TTree*)inf->Get("/Event/Rec/AdSimple");
    tAdSimple->Draw(">>elist", evtsel.str().c_str(), "entrylist");
    elist = (TEntryList*)gDirectory->Get("elist");
    tAdSimple->GetEntry(elist->GetEntry(0));
    XYZVector pRec(tAdSimple->GetLeaf("x")->GetValue(),tAdSimple->GetLeaf("y")->GetValue(),tAdSimple->GetLeaf("z")->GetValue());
    TGeoVolume* volRec = geom->MakeSphere("pRec",med,0,100);
    volRec->SetLineColor(kBlack);
    top->AddNode(volRec,1,new TGeoTranslation(pRec.X(),pRec.Y(),pRec.Z()));
  }
  if(coordtype == 1)
  {
    TFile* inf2 = new TFile("processed.root");
    TTree* tdmucoin = (TTree*)inf2->Get("dmucoin");
    stringstream curcut;
    curcut << "detector==" << detector << "&&triggerNumberMu==" << trNum;
    tdmucoin->Draw(">>elist", curcut.str().c_str(), "entrylist");
    elist = (TEntryList*)gDirectory->Get("elist");
    if(elist->GetN())
    {
      tdmucoin->GetEntry(elist->GetEntry(0));
      XYZVector pRec(tdmucoin->GetLeaf("xMu")->GetValue(),tdmucoin->GetLeaf("yMu")->GetValue(),tdmucoin->GetLeaf("zMu")->GetValue());
      PrintVector(pRec);
      pRec = pRec - pAdCenter;
      TGeoVolume* volRec = geom->MakeSphere("pRec",med,0,100);
      volRec->SetLineColor(kBlack);
      top->AddNode(volRec,1,new TGeoTranslation(pRec.X(),pRec.Y(),pRec.Z()));
    }
  }
  
  geom->CloseGeometry();
  
  top->Draw();
  
  /* end drawing geometry */
  
  
  /// debug output: total charge of a trigger
  double triggerTotCge = 0;
  TriggerPmtCharge::iterator it = triggerCharge.begin();
  for(; it != triggerCharge.end(); it++)
    triggerTotCge += it->second;
  cout << "total charge: " << triggerTotCge << endl;
  cout << "npmt: " << triggerCharge.size() << endl;
  //PrintVector(pmtTranslation[12101]);
  
  
  return 0;
}


void fillGeometry(int coordtype, int site, int detector, map<int,XYZVector>& trans)
{
  ifstream detfile("db/detector.txt");
  ifstream pmtfile("db/adPmt.txt");
  
  /// global to local rotation
  /// either use -phi or 2*pi-phi
  AxisAngle gl2lo(XYZVector(0,0,1), -dPhi);
  
  string readfield;
  int col = 0;
  int readsite = 0, readdet = 0;
  /// pAdCenter is a global variable
  while(detfile >> readfield)
  {
    col = col % 5;
    if(col == 0)
      readsite = atoi(readfield.c_str());
    if(col == 1)
      readdet = atoi(readfield.c_str());
    if(col == 2 && readsite == site && readdet == detector)
      pAdCenter.SetX(atof(readfield.c_str()));
    if(col == 3 && readsite == site && readdet == detector)
      pAdCenter.SetY(atof(readfield.c_str()));
    if(col == 4 && readsite == site && readdet == detector)
    {
      pAdCenter.SetZ(atof(readfield.c_str()));
      break;
    }
    if(col == 4)
    {
      readsite = 0;
      readdet = 0;
    }
    col++;
  }
  
  col = 0;
  readsite = 0;
  readdet = 0;
  int readring = -1, readcolumn = -1;
  XYZVector pPmtTrans;
  bool targetDetectorStarted = false;
  bool targetDetectorFinished = false;
  while(pmtfile >> readfield)
  {
    col = col % 11;
    if(col == 1)
      readsite = atoi(readfield.c_str());
    if(col == 2)
      readdet = atoi(readfield.c_str());
    if(col == 3 && readsite == site && readdet == detector)
      readring = atoi(readfield.c_str());
    if(col == 4 && readsite == site && readdet == detector)
      readcolumn = atoi(readfield.c_str());
    if(col == 5 && readsite == site && readdet == detector)
      pPmtTrans.SetX(atof(readfield.c_str()));
    if(col == 6 && readsite == site && readdet == detector)
      pPmtTrans.SetY(atof(readfield.c_str()));
    if(col == 7 && readsite == site && readdet == detector)
      pPmtTrans.SetZ(atof(readfield.c_str()));
    if(col == 10 && readsite == site && readdet == detector)
    {
      int curId = readsite*10000+readdet*1000+readring*100+readcolumn;
      if(coordtype == 1) /// global coordinate, no rotation
        trans[curId] = pPmtTrans - pAdCenter;
      else               /// local coordinate, with rotation
                         /// There is a constant z shift here.
        trans[curId] = gl2lo * (pPmtTrans - pAdCenter) + XYZVector(0,0,487.5);
      targetDetectorStarted = true;
    }
    if(col == 10 && !(readsite == site && readdet == detector))
    {
      readsite = 0;
      readdet = 0;
      readring = -1;
      readcolumn = -1;
      if(targetDetectorStarted) targetDetectorFinished = true;
      if(targetDetectorFinished) break;
    }
    col++;
  }
  
  detfile.close();
  pmtfile.close();
}


void PrintVector(XYZVector& v)
{
  cout << v.X() << " " << v.Y() << " " << v.Z() << endl;
}
