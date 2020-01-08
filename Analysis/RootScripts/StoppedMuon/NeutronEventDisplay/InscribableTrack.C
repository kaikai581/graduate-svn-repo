/*
 * 
 * For each muon track, inscribe a cylinder coaxial with the track with a fixed
 * radius(first value is 1000mm). This script is used for the sanity check of
 * my algorithm. This script plots the reconstructed neutron position and
 * the incoming and outgoing points on the IAV.
 * 
 */


#include <iostream>
#include "Math/AxisAngle.h"
#include "Math/EulerAngles.h"
#include "Math/Point3D.h"
#include "Math/Vector3D.h"
#include "TEntryList.h"
#include "TFile.h"
#include "TGeoManager.h"
#include "TGeoMaterial.h"
#include "TGeoMedium.h"
#include "TGLCamera.h"
#include "TGLViewer.h"
#include "TMath.h"
#include "TPad.h"
#include "TROOT.h"
#include "TTree.h"

#define MAXNEUTRONNUM 100


using namespace ROOT::Math;
using namespace std;


void InscribableTrack(string filename = "output.root", int evtnum = 0)
{
  
  map<int, XYZPoint> detectorCenter;
  detectorCenter[1] = XYZPoint(-18079.45, -799699.4, -7141.5);
  detectorCenter[2] = XYZPoint(-14960.54, -804520.5, -7141.5);
  
  /// extract information of the cylinder from root file
  TFile* f = new TFile(filename.c_str());
  
  TTree* t = (TTree*)f->Get("dmucoin");
  t->Draw(">>elist", "inscribable==1", "entrylist");
  
  TEntryList* elist = (TEntryList*)gDirectory->Get("elist");
  
  int detector, trigNumMu;
  
  double xIavIn, yIavIn, zIavIn;
  double xIavOut, yIavOut, zIavOut;
  double zInsCylTopTrackCoord, zInsCylBotTrackCoord;
  
  double xInsCylTopIavCoord, xInsCylBotIavCoord;
  double yInsCylTopIavCoord, yInsCylBotIavCoord;
  double zInsCylTopIavCoord, zInsCylBotIavCoord;
  
  t->SetBranchAddress("triggerNumberMu", &trigNumMu);
  t->SetBranchAddress("detector", &detector);
  t->SetBranchAddress("xIavIn", &xIavIn);
  t->SetBranchAddress("yIavIn", &yIavIn);
  t->SetBranchAddress("zIavIn", &zIavIn);
  t->SetBranchAddress("xIavOut", &xIavOut);
  t->SetBranchAddress("yIavOut", &yIavOut);
  t->SetBranchAddress("zIavOut", &zIavOut);
  t->SetBranchAddress("zInsCylTopTrackCoord", &zInsCylTopTrackCoord);
  t->SetBranchAddress("zInsCylBotTrackCoord", &zInsCylBotTrackCoord);
  t->SetBranchAddress("xInsCylTopIavCoord", &xInsCylTopIavCoord);
  t->SetBranchAddress("yInsCylTopIavCoord", &yInsCylTopIavCoord);
  t->SetBranchAddress("zInsCylTopIavCoord", &zInsCylTopIavCoord);
  t->SetBranchAddress("xInsCylBotIavCoord", &xInsCylBotIavCoord);
  t->SetBranchAddress("yInsCylBotIavCoord", &yInsCylBotIavCoord);
  t->SetBranchAddress("zInsCylBotIavCoord", &zInsCylBotIavCoord);
  
  if(evtnum >= elist->GetN())
  {
    cout << "event number larger than number of events" << endl;
    return;
  }
  
  /// retrieve data of this event
  t->GetEntry(elist->GetEntry(evtnum));
  cout << "(AD muon trigger number, detector ID): (" << trigNumMu;
  cout << ", " << detector << ")" << endl;
  
  /// set rotation and translation of geometry elements
  XYZPoint pIavIn(xIavIn-detectorCenter[detector].X(),yIavIn-detectorCenter[detector].Y(),zIavIn-detectorCenter[detector].Z());
  XYZPoint pIavOut(xIavOut-detectorCenter[detector].X(),yIavOut-detectorCenter[detector].Y(),zIavOut-detectorCenter[detector].Z());
  XYZVector trDirVec = pIavOut - pIavIn;
  trDirVec = -trDirVec/sqrt(trDirVec.Mag2());
  
  double dzTrack = (zInsCylTopTrackCoord-zInsCylBotTrackCoord)/2;
  double trLen = sqrt((pIavOut - pIavIn).Mag2());
  XYZVector trCenter = ((XYZVector)pIavOut + (XYZVector)pIavIn)/2.;
  
  AxisAngle rot1(XYZVector(0,0,1).Cross(trDirVec), trDirVec.Theta());
  EulerAngles rotEuler(AxisAngle(XYZVector(0,0,1).Cross(trDirVec), -trDirVec.Theta()));
  
  TGeoRotation* rotTrack = new TGeoRotation("track_rotation", rotEuler.Phi()*180/TMath::Pi(), rotEuler.Theta()*180/TMath::Pi(), rotEuler.Psi()*180/TMath::Pi());
  
  /// the intersection of the track and the xy plane
  XYZPoint trXYPlane = pIavIn - (pIavIn.Z()/trDirVec.Z())*trDirVec;
  
  XYZVector transTrCyl = (XYZVector)trXYPlane+(zInsCylTopTrackCoord-dzTrack)*trDirVec;
  TGeoCombiTrans *combiTrCyl = new TGeoCombiTrans(transTrCyl.X(), transTrCyl.Y(), transTrCyl.Z(), rotTrack);
  TGeoCombiTrans *combiMuTr = new TGeoCombiTrans(trCenter.X(), trCenter.Y(), trCenter.Z(), rotTrack);
  
  /// make geometry
  TGeoManager *geom = new TGeoManager("inscribe_cylinder",
  "inscribe a small cylinder in a big cylinder");
  
  TGeoMaterial *mat = new TGeoMaterial("Vacuum",0,0,0);
  TGeoMaterial *Fe = new TGeoMaterial("Fe",55.845,26,7.87);
  
  TGeoMedium *med = new TGeoMedium("Vacuum",1,mat);
  TGeoMedium *Iron = new TGeoMedium("Iron",0,Fe);
  
  TGeoVolume *top = geom->MakeTube("IAV", med, 0., 1500., 1500.);
  top->SetTransparency(50);
  geom->SetTopVolume(top);
  
  TGeoVolume *tpTop = geom->MakeSphere("top", med,0,30);
  TGeoVolume *tpBot = geom->MakeSphere("bot", med,0,30);
  
  TGeoVolume *pGeoIavIn = geom->MakeSphere("IavIn", med,0,30);
  TGeoVolume *pGeoIavOut = geom->MakeSphere("IavOut", med,0,30);
  TGeoVolume *trackCyl = geom->MakeTube("TRACK_CYLINDER", Iron,0,1000,dzTrack);
  TGeoVolume *muTrack = geom->MakeTube("muon_track", med,0,20,trLen/2);
  
  top->AddNode(pGeoIavIn, 1, new TGeoTranslation(pIavIn.X(), pIavIn.Y(), pIavIn.Z()));
  top->AddNode(pGeoIavOut, 1, new TGeoTranslation(pIavOut.X(), pIavOut.Y(), pIavOut.Z()));
  top->AddNode(tpTop, 1, new TGeoTranslation(xInsCylTopIavCoord, yInsCylTopIavCoord, zInsCylTopIavCoord));
  top->AddNode(tpBot, 1, new TGeoTranslation(xInsCylBotIavCoord, yInsCylBotIavCoord, zInsCylBotIavCoord));
  top->AddNode(trackCyl, 1, combiTrCyl);
  top->AddNode(muTrack, 1, combiMuTr);
  
  geom->CloseGeometry();
  
  tpTop->SetLineColor(kRed);
  tpBot->SetLineColor(kRed);
  pGeoIavIn->SetLineColor(kGreen);
  pGeoIavOut->SetLineColor(kGreen);
  trackCyl->SetLineColor(kBlue);
  muTrack->SetLineColor(kGreen);
  top->SetLineColor(kMagenta);
  
  geom->SetTopVisible();
  
  top->Draw("ogl");
  
  TGLViewer* glv = (TGLViewer*)gROOT->GetSelectedPad()->GetViewer3D();
  double wc[3] = {0.};
  glv->SetPerspectiveCamera(TGLViewer::kCameraPerspXOY, 30, 3000, wc, 0, 0);
  glv->SetCurrentCamera(TGLViewer::kCameraPerspXOY);
}
