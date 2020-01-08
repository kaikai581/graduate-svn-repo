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
#include "TChain.h"
#include "TEntryList.h"
#include "TFile.h"
#include "TGeoManager.h"
#include "TGeoMaterial.h"
#include "TGeoMedium.h"
#include "TGLViewer.h"
#include "TMath.h"
#include "TPad.h"
#include "TROOT.h"

#define MAXNEUTRONNUM 100


using namespace ROOT::Math;
using namespace std;


void EventDisplayByTriggerNumber(int trigNum, int detId = 1, string filename = "/disk1/data/2012/p12e/Neutrino/0726/*EH1*.root")
{
  
  map<int, XYZPoint> detectorCenter;
  detectorCenter[1] = XYZPoint(-18079.45, -799699.4, -7141.5);
  detectorCenter[2] = XYZPoint(-14960.54, -804520.5, -7141.5);
  
  /// extract information of the cylinder from root file
  TChain* ch = new TChain("dmucoin");
  
  ch->Add(filename.c_str());
  ch->Draw(">>elist", Form("triggerNumberMu==%d&&detector==%d",trigNum,detId), "entrylist");
  
  TEntryList* elist = (TEntryList*)gDirectory->Get("elist");
  
  int nentry = elist->GetN();
  if(nentry == 0)
  {
    cout << "specified event not found" << endl;
    return;
  }
  if(nentry > 1)
  {
    cout << "multiple events found" << endl;
    return;
  }
  
  int nNeu, detector;
  double xNeu[MAXNEUTRONNUM], yNeu[MAXNEUTRONNUM], zNeu[MAXNEUTRONNUM];
  double xIavIn, yIavIn, zIavIn;
  double xIavOut, yIavOut, zIavOut;
  double zInsCylTopTrackCoord, zInsCylBotTrackCoord;
  
  double xInsCylTopIavCoord, xInsCylBotIavCoord;
  double yInsCylTopIavCoord, yInsCylBotIavCoord;
  double zInsCylTopIavCoord, zInsCylBotIavCoord;
  
  ch->SetBranchAddress("detector", &detector);
  ch->SetBranchAddress("nMi", &nNeu);
  ch->SetBranchAddress("xMi", &xNeu);
  ch->SetBranchAddress("yMi", &yNeu);
  ch->SetBranchAddress("zMi", &zNeu);
  ch->SetBranchAddress("xIavIn", &xIavIn);
  ch->SetBranchAddress("yIavIn", &yIavIn);
  ch->SetBranchAddress("zIavIn", &zIavIn);
  ch->SetBranchAddress("xIavOut", &xIavOut);
  ch->SetBranchAddress("yIavOut", &yIavOut);
  ch->SetBranchAddress("zIavOut", &zIavOut);
  ch->SetBranchAddress("zInsCylTopTrackCoord", &zInsCylTopTrackCoord);
  ch->SetBranchAddress("zInsCylBotTrackCoord", &zInsCylBotTrackCoord);
  ch->SetBranchAddress("xInsCylTopIavCoord", &xInsCylTopIavCoord);
  ch->SetBranchAddress("yInsCylTopIavCoord", &yInsCylTopIavCoord);
  ch->SetBranchAddress("zInsCylTopIavCoord", &zInsCylTopIavCoord);
  ch->SetBranchAddress("xInsCylBotIavCoord", &xInsCylBotIavCoord);
  ch->SetBranchAddress("yInsCylBotIavCoord", &yInsCylBotIavCoord);
  ch->SetBranchAddress("zInsCylBotIavCoord", &zInsCylBotIavCoord);
  
  /// retrieve data of this event
  Int_t treeNum = 0;
  Long64_t treeEntry = elist->GetEntryAndTree(0, treeNum);
  Long64_t chainEntry = treeEntry + ch->GetTreeOffset()[treeNum];
  ch->GetEntry(chainEntry);
  
  /// set rotation and translation of geometry elements
  XYZPoint pIavIn(xIavIn-detectorCenter[detector].X(),yIavIn-detectorCenter[detector].Y(),zIavIn-detectorCenter[detector].Z());
  XYZPoint pIavOut(xIavOut-detectorCenter[detector].X(),yIavOut-detectorCenter[detector].Y(),zIavOut-detectorCenter[detector].Z());
  XYZVector trDirVec = pIavOut - pIavIn;
  trDirVec = -trDirVec/sqrt(trDirVec.Mag2());
  
  /// transformation for muon track line
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
  
  TGeoMedium *med = new TGeoMedium("Vacuum",1,mat);
  
  TGeoVolume *top = geom->MakeTube("IAV", med, 0., 1500., 1500.);
  top->SetTransparency(50);
  geom->SetTopVolume(top);
  
  //TGeoVolume *pGeoNeu = geom->MakeSphere("Neu", med,0,30);
  TGeoVolume *pGeoIavIn = geom->MakeSphere("IavIn", med,0,30);
  TGeoVolume *pGeoIavOut = geom->MakeSphere("IavOut", med,0,30);
  TGeoVolume *trackCyl = geom->MakeTube("TRACK_CYLINDER", med,0,1000,dzTrack);
  TGeoVolume *muTrack = geom->MakeTube("muon_track", med,0,20,trLen/2);
  
  //top->AddNode(pGeoNeu, 1, new TGeoTranslation(xNeu[0]-detectorCenter[detector].X(), yNeu[0]-detectorCenter[detector].Y(), zNeu[0]-detectorCenter[detector].Z()));
  top->AddNode(pGeoIavIn, 1, new TGeoTranslation(xIavIn-detectorCenter[detector].X(), yIavIn-detectorCenter[detector].Y(), zIavIn-detectorCenter[detector].Z()));
  top->AddNode(pGeoIavOut, 1, new TGeoTranslation(xIavOut-detectorCenter[detector].X(), yIavOut-detectorCenter[detector].Y(), zIavOut-detectorCenter[detector].Z()));
  top->AddNode(trackCyl, 1, combiTrCyl);
  top->AddNode(muTrack, 1, combiMuTr);
  
  geom->CloseGeometry();
  
  //pGeoNeu->SetLineColor(kRed);
  pGeoIavIn->SetLineColor(kGreen);
  pGeoIavOut->SetLineColor(kGreen);
  trackCyl->SetLineColor(kBlue);
  muTrack->SetLineColor(kGreen);
  top->SetLineColor(kMagenta);
  
  geom->SetTopVisible();
  
  top->Draw("ogl");
  
  /// set up the camera
  TGLViewer* glv = (TGLViewer*)gROOT->GetSelectedPad()->GetViewer3D();
  double wc[3] = {0.};
  glv->SetPerspectiveCamera(TGLViewer::kCameraPerspXOY, 30, 3000, wc, 0, 0);
  glv->SetCurrentCamera(TGLViewer::kCameraPerspXOY);
  glv->SavePictureUsingBB(Form("graph/tn%d_ad%d.gif", trigNum, detId));
}
