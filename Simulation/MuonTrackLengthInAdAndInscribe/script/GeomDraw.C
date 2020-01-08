#include <iostream>
#include "Math/AxisAngle.h"
#include "Math/EulerAngles.h"
#include "Math/Point3D.h"
#include "Math/Vector3D.h"
#include "TEntryList.h"
#include "TFile.h"
#include "TGeoManager.h"
#include "TGeoMaterial.h"
#include "TGeoMatrix.h"
#include "TGeoMedium.h"
#include "TGeoVolume.h"
#include "TMath.h"
#include "TSystem.h"
#include "TTree.h"

using namespace std;
using namespace ROOT::Math;

void GeomDraw(int evtnum = 0)
{
  /// extract information of the cylinder from root file
  TFile* f = new TFile("../output/simtrack.root");
  
  TTree* t = (TTree*)f->Get("t");
  
  t->Draw(">>elist", "inscribable==1", "entrylist");
  
  TEntryList* elist = (TEntryList*)gDirectory->Get("elist");
  
  double theta, phi;
  double zInsCylTopTrackCoord, zInsCylBotTrackCoord;
  double xOSepaIavCoord, yOSepaIavCoord;
  
  double xIavIn, yIavIn, zIavIn;
  double xIavOut, yIavOut, zIavOut;
  
  double xInsCylTopIavCoord, xInsCylBotIavCoord;
  double yInsCylTopIavCoord, yInsCylBotIavCoord;
  double zInsCylTopIavCoord, zInsCylBotIavCoord;
  
  t->SetBranchAddress("theta", &theta);
  t->SetBranchAddress("phi", &phi);
  t->SetBranchAddress("zInsCylTopTrackCoord", &zInsCylTopTrackCoord);
  t->SetBranchAddress("zInsCylBotTrackCoord", &zInsCylBotTrackCoord);
  t->SetBranchAddress("xOSepaIavCoord", &xOSepaIavCoord);
  t->SetBranchAddress("yOSepaIavCoord", &yOSepaIavCoord);
  t->SetBranchAddress("xIavIn", &xIavIn);
  t->SetBranchAddress("yIavIn", &yIavIn);
  t->SetBranchAddress("zIavIn", &zIavIn);
  t->SetBranchAddress("xIavOut", &xIavOut);
  t->SetBranchAddress("yIavOut", &yIavOut);
  t->SetBranchAddress("zIavOut", &zIavOut);
  t->SetBranchAddress("xInsCylTopIavCoord", &xInsCylTopIavCoord);
  t->SetBranchAddress("yInsCylTopIavCoord", &yInsCylTopIavCoord);
  t->SetBranchAddress("zInsCylTopIavCoord", &zInsCylTopIavCoord);
  t->SetBranchAddress("xInsCylBotIavCoord", &xInsCylBotIavCoord);
  t->SetBranchAddress("yInsCylBotIavCoord", &yInsCylBotIavCoord);
  t->SetBranchAddress("zInsCylBotIavCoord", &zInsCylBotIavCoord);
  
  if(evtnum >= elist->GetN()) {
    cerr << "event number exceeds number of events" << endl;
    return;
  }
  
  t->GetEntry(elist->GetEntry(evtnum));
  
  XYZVector trDirVec(sin(theta)*cos(phi),sin(theta)*sin(phi),cos(theta));
  trDirVec = -trDirVec;
  
  double dzTrack = (zInsCylTopTrackCoord-zInsCylBotTrackCoord)/2;
  
  //cout << trDirVec.X() << " " << trDirVec.Y() << " " << trDirVec.Z() << endl;
  
  /// get rotation and translation of geometric elements
  AxisAngle rot1(XYZVector(0,0,1).Cross(trDirVec), trDirVec.Theta());
  
  EulerAngles rotEuler(AxisAngle(XYZVector(0,0,1).Cross(trDirVec), -trDirVec.Theta()));
  
  cout << (rotEuler*XYZVector(0,0,1)-trDirVec).X() << " " <<(rotEuler*XYZVector(0,0,1)-trDirVec).Y() << " " << (rotEuler*XYZVector(0,0,1)-trDirVec).Z() << endl;
  
  cout << rotEuler.Phi() << " " << rotEuler.Theta() << " " << rotEuler.Psi() << endl;
  
  TGeoRotation* rotTrack = new TGeoRotation("track_rotation", rotEuler.Phi()*180/TMath::Pi(), rotEuler.Theta()*180/TMath::Pi(), rotEuler.Psi()*180/TMath::Pi());
  
  gSystem->Load("libGeom");
  
  TGeoManager *geom = new TGeoManager("inscribe_cylinder",
  "inscribe a small cylinder in a big cylinder");
  
  TGeoMaterial *mat = new TGeoMaterial("Vacuum",0,0,0);
  
  TGeoMedium *med = new TGeoMedium("Vacuum",1,mat);
  
  TGeoVolume *top = geom->MakeTube("IAV", med, 0., 1500., 1500.);
  geom->SetTopVolume(top);
  
  TGeoVolume *trackCyl = geom->MakeTube("TRACK_CYLINDER", med,0,1000,dzTrack);
  
  TGeoVolume *muTrack = geom->MakeTube("muon_track", med,0,20,1500);
  
  TGeoVolume *tpTop = geom->MakeSphere("top", med,0,30);
  TGeoVolume *tpBot = geom->MakeSphere("bot", med,0,30);
  
  TGeoVolume *pGeoIavIn = geom->MakeSphere("IavIn", med,0,30);
  TGeoVolume *pGeoIavOut = geom->MakeSphere("IavOut", med,0,30);
  
  XYZVector transTrCyl = XYZVector(xOSepaIavCoord,yOSepaIavCoord,0)+(zInsCylTopTrackCoord-dzTrack)*trDirVec;
  TGeoCombiTrans *combiTrack = new TGeoCombiTrans(transTrCyl.X(), transTrCyl.Y(), transTrCyl.Z(), rotTrack);
  
  XYZVector transMuTrack = XYZVector(xOSepaIavCoord,yOSepaIavCoord,0);
  TGeoCombiTrans *combiMuTrack = new TGeoCombiTrans(transMuTrack.X(), transMuTrack.Y(), transMuTrack.Z(), rotTrack);
  
  top->AddNode(trackCyl, 1, combiTrack);
  //top->AddNode(muTrack, 1, combiMuTrack);
  top->AddNode(tpTop, 1, new TGeoTranslation(xInsCylTopIavCoord, yInsCylTopIavCoord, zInsCylTopIavCoord));
  top->AddNode(tpBot, 1, new TGeoTranslation(xInsCylBotIavCoord, yInsCylBotIavCoord, zInsCylBotIavCoord));
  top->AddNode(pGeoIavIn, 1, new TGeoTranslation(xIavIn, yIavIn, zIavIn));
  top->AddNode(pGeoIavOut, 1, new TGeoTranslation(xIavOut, yIavOut, zIavOut));
  
  geom->CloseGeometry();
  
  tpTop->SetLineColor(kRed);
  tpBot->SetLineColor(kRed);
  pGeoIavIn->SetLineColor(kGreen);
  pGeoIavOut->SetLineColor(kGreen);
  muTrack->SetLineColor(kYellow);
  trackCyl->SetLineColor(kBlue);
  top->SetLineColor(kMagenta);
  
  geom->SetTopVisible();
  
  top->Draw();
  
}
