/*
 * 
 * Muon data by MUSIC simulation can be found in the following files
 * /disk1/NuWa/external/data/0.0/Muon/mountain_DYB
 * /disk1/NuWa/external/data/0.0/Muon/mountain_LA
 * /disk1/NuWa/external/data/0.0/Muon/mountain_Far_80m
 * 
 */

#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include "Math/Point2D.h"
#include "TCanvas.h"
#include "TH2F.h"
#include "TMath.h"

struct OneMuData {
  double E;
  double th;
  double phi;
};

struct EStats {
  double Eavg;
  double Erms;
  double nEvt;
};

using namespace ROOT::Math;

char *fnlist[] = {"/disk1/NuWa/external/data/0.0/Muon/mountain_DYB",
                  "/disk1/NuWa/external/data/0.0/Muon/mountain_LA",
                  "/disk1/NuWa/external/data/0.0/Muon/mountain_Far",
                  "/disk1/NuWa/external/data/0.0/Muon/mountain_Far_80m"};

void AsymmetricRMS(int fnId = 0)
{
  if(fnId >= 4 || fnId < 0)
  {
    cerr << "file index out of range" << endl;
    return;
  }
  ifstream inf(fnlist[fnId]);
  
  vector<OneMuData> muDataList;
  
  string infline;
  /// flush the first 6 lines
  for(int i = 0; i < 6; i++) getline(inf, infline);
  
  int nTest = 0;
  int nRead = 0;
  while(inf >> infline)
  {
    if(!(nRead%6)) nTest++;
    //if(nTest > 100) break;
    
    int nCol = nRead%6;
    OneMuData curMu;
    
    switch (nCol) {
      case 0:
        curMu.E = atof(infline.c_str());
        break;
      case 1:
        curMu.th = atof(infline.c_str());
        break;
      case 2:
        curMu.phi = atof(infline.c_str());
        break;
      case 3:
        muDataList.push_back(curMu);
        break;
    }
    
    nRead++;
  }
  
  vector<double> highE;
  vector<double> lowE;
  vector<double> EList;
  
  for(unsigned int i = 0; i < muDataList.size(); i++)
  {
    if(cos(muDataList[i].th*TMath::Pi()/180.)>0.5&&muDataList[i].phi>240&&muDataList[i].phi<360)
      lowE.push_back(muDataList[i].E);
    if(cos(muDataList[i].th*TMath::Pi()/180.)<0.5&&muDataList[i].phi>0&&muDataList[i].phi<120)
      highE.push_back(muDataList[i].E);
    EList.push_back(muDataList[i].E);
  }
  
  
  double meanEHigh = TMath::Mean(highE.begin(), highE.end());
  double meanELow = TMath::Mean(lowE.begin(), lowE.end());
  double rmsHigh = 0, rmsLow = 0;
  double meanE = TMath::Mean(EList.begin(), EList.end());
  double rms = TMath::RMS(EList.begin(), EList.end());
  
  
  
  for(unsigned int i = 0; i < highE.size(); i++)
  {
    rmsHigh += (highE[i]-meanEHigh)*(highE[i]-meanEHigh);
  }
  rmsHigh = sqrt(rmsHigh/highE.size());
  
  for(unsigned int i = 0; i < lowE.size(); i++)
  {
    rmsLow += (lowE[i]-meanELow)*(lowE[i]-meanELow);
  }
  rmsLow = sqrt(rmsLow/lowE.size());
  
  //cout << "high energy mean and sigma: " << meanEHigh << " " << rmsHigh << endl;
  //cout << "low energy mean and sigma: " << meanELow << " " << rmsLow << endl;
  cout << "energy mean and sigma: " << meanE << " " << rms << endl;
  
  
  /* start forming asymmetric RMS */
  // defined as 34% area to the left and right of the mean value
  int indexoffset = .34*muDataList.size();
  
  std::sort(EList.begin(), EList.end());
  
  unsigned int meanidx;
  for(meanidx = 1; meanidx < EList.size(); meanidx++)
  {
    double ldiff, rdiff;
    ldiff = EList[meanidx-1] - meanE;
    rdiff = EList[meanidx] - meanE;
    if(ldiff*rdiff <= 0) break;
  }
  
  int lowIdx = meanidx - indexoffset;
  lowIdx = ((lowIdx > 0) ? lowIdx : 0);
  int upIdx = meanidx + indexoffset;
  upIdx = ((upIdx < EList.size()-1) ? upIdx : EList.size()-1);
  cout << "low and up indeces: " << lowIdx << " " << upIdx << endl;
  cout << "low and up energies: " << EList[lowIdx] << " " << EList[upIdx] << endl;
  cout << "left and right RMS: " << meanE - EList[lowIdx] << " " << EList[upIdx] - meanE << endl;
}
