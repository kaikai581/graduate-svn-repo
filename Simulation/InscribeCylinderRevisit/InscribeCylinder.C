#include "Math/AxisAngle.h"
#include "Math/Point3D.h"
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

void InscribeCylinder()
{
  XYZVector e1(1,0,0);
  XYZVector e2(0,1,0);
  XYZVector e3(0,0,1);
  
  XYZPoint pXY(-500,0,0);
  XYZVector vDir(1,1,1);
  vDir = vDir.Unit();
  
  AxisAngle rotateIav(e3.Cross(vDir), vDir.Theta());
  
  
  
  
  /* start rendering geometry */
  TGeoManager *geom = new TGeoManager("World", "the universe");
  TGeoMaterial *mat = new TGeoMaterial("Vacuum",0,0,0);
  //mat->SetTransparency(20);
  TGeoMedium *med = new TGeoMedium("Vacuum",1,mat);
  
  /// make top geometry
  TGeoVolume *top = geom->MakeBox("TOP", med, 20000, 20000, 20000);
  geom->SetTopVolume(top);
  
  /// draw track cylinder
  TGeoVolume* trackCyl = geom->MakeTube("TrCyl", med, 0, RINS, 5*RINS);
  trackCyl->SetLineColor(kAzure);
  top->AddNode(trackCyl,1);
  
  /// draw IAV cylinder
  TGeoVolume* iavCyl = geom->MakeTube("IavCyl", med, 0, RIAV, 3*RIAV);
  double angZ = vDir.Phi()*180./TMath::Pi()+90;
  double tilt = vDir.Theta()*180./TMath::Pi();
  cout << "theta and phi: " << vDir.Theta() << ", " << vDir.Phi() << endl;
  TGeoRotation* orientation = new TGeoRotation("IavCyl_orientation", angZ, tilt, 0);
  TGeoCombiTrans* transOrient = new TGeoCombiTrans(pXY.X(),pXY.Y(),pXY.Z(), orientation);
  iavCyl->SetLineColor(kMagenta);
  top->AddNode(iavCyl,1,transOrient);
  
  geom->CloseGeometry();
  /* end rendering geometry */
  
  top->Draw("ogl");
  TGLViewer* glv = (TGLViewer*)gROOT->GetSelectedPad()->GetViewer3D();
  double wc[3] = {0.};
  glv->SetPerspectiveCamera(TGLViewer::kCameraPerspXOY, 30, 3000, wc, 0, 0);
  glv->SetCurrentCamera(TGLViewer::kCameraPerspXOY);
}
