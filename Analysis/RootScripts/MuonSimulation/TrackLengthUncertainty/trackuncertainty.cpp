/*
 * 
 * Given the reconstructed and true tracks, find the uncertainty of the track
 * length due to track reconstruction.
 * 
 * 
 * July 2014 by Shih-Kai.
 * 
*/



#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include "InscribeCylinder.hpp"
#include "Math/Point3D.h"
#include "Math/Vector3D.h"
#include "TFile.h"
#include "TParameter.h"
#include "TTree.h"


using namespace ROOT::Math;
using namespace std;


int main()
{
  XYZVector iavctr[2];
  iavctr[0] = XYZVector(2871,0,-26.5);
  iavctr[1] = XYZVector(-2871,0,-26.5);
  
  string tmpline;
  ifstream iftru("truetrackinfo.txt");
  ifstream ifrec("rectrackinfo.txt");
  
  /// output containers
  double cylLenTrue, cylLenRec;
  double totLenTrue = 0, totLenRec = 0;
  TParameter<double> pTotLenTrue("totLenTrue", totLenTrue);
  TParameter<double> pTotLenRec("totLenRec", totLenRec);
  
  TFile tfout("trlen.root","recreate");
  TTree t("len", "true and reconstructed cylinder lengths");
  t.Branch("lenTrue", &cylLenTrue, "lenTrue/D");
  t.Branch("lenRec", &cylLenRec, "lenRec/D");
  
  
  for(; getline(iftru, tmpline); )
  {
    double x, y, z, vx, vy, vz;
    
    stringstream sstru(tmpline);
    sstru >> x >> y >> z >> vx >> vy >> vz;
    XYZPoint pTru(x, y, z);
    XYZVector vTru(vx, vy, vz);
    //cout << x << " " << y << " " << z << endl;
    
    getline(ifrec, tmpline);
    stringstream ssrec(tmpline);
    ssrec >> x >> y >> z >> vx >> vy >> vz;
    XYZPoint pRec(x, y, z);
    if(pRec.Mag2() < 1) continue;
    XYZVector vRec(vx, vy, vz);
    //cout << vRec.x() << " " << vRec.y() << " " << vRec.z() << endl;
    
    cylLenTrue = GetCylLen(pTru, vTru, iavctr[0]);
    cylLenRec = GetCylLen(pRec, vRec, iavctr[0]);
    totLenTrue += cylLenTrue;
    totLenRec += cylLenRec;
    
    //if(cylLenTrue>0 && cylLenRec>0)
      //cout << cylLenTrue << " " << cylLenRec << endl;
    
    t.Fill();
  }
  
  
  iftru.close();
  ifrec.close();
  
  cout << totLenTrue << " " << totLenRec << endl;
  t.Write();
  pTotLenTrue.SetVal(totLenTrue);
  pTotLenTrue.Write();
  pTotLenRec.SetVal(totLenRec);
  pTotLenRec.Write();
  tfout.Close();
  
  return 0;
}
