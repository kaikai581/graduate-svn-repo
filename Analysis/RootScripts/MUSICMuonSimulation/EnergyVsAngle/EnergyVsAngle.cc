/*
 * 
 * Muon data by MUSIC simulation can be found in the following files
 * /disk1/NuWa/external/data/0.0/Muon/mountain_DYB
 * /disk1/NuWa/external/data/0.0/Muon/mountain_LA
 * /disk1/NuWa/external/data/0.0/Muon/mountain_Far_80m
 * 
 */

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

void EnergyVsAngle(int fnId = 0)
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
  
  /*for(unsigned int i = 0; i < muDataList.size(); i++)
  {
    cout << muDataList[i].E << " ";
    cout << cos(muDataList[i].th*TMath::Pi()/180.) << " ";
    cout << muDataList[i].phi << endl;
  }*/
  
  map<int, map<int, vector<double> > > binnedE;
  map<int, map<int, EStats> > binnedEStats;
  
  for(unsigned int i = 0; i < muDataList.size(); i++)
  {
    int ith = (int)(cos(muDataList[i].th*TMath::Pi()/180.)/.2);
    int jphi = (int)(muDataList[i].phi/120.);
    
    binnedE[ith][jphi].push_back(muDataList[i].E);
  }
  
  /// fill "binnedEStats"
  ofstream outf("hall_en_binned_by_angle.txt", std::fstream::in | std::fstream::out | std::fstream::app);
  map<int, map<int, vector<double> > >::iterator itth = binnedE.begin();
  for(; itth != binnedE.end(); itth++)
  {
    map<int, vector<double> >::iterator itphi = itth->second.begin();
    for(; itphi != itth->second.end(); itphi++)
    {
      binnedEStats[itth->first][itphi->first].Eavg = TMath::Mean(itphi->second.begin(), itphi->second.end());
      binnedEStats[itth->first][itphi->first].Erms = TMath::RMS(itphi->second.begin(), itphi->second.end());
      binnedEStats[itth->first][itphi->first].nEvt = itphi->second.size();
      outf << fnId + 1 << " " << TMath::Mean(itphi->second.begin(), itphi->second.end()) << " " << TMath::RMS(itphi->second.begin(), itphi->second.end()) << endl;
    }
  }
  outf.close();
  
  const int nbin = 200;
  const double maprange = TMath::PiOver2();
  
  TH2F* havg = new TH2F("havg","average muon energy (GeV)",nbin,-maprange,maprange,nbin,-maprange,maprange);
  havg->SetStats(0);
  havg->GetXaxis()->SetTitle("E");
  havg->GetYaxis()->SetTitle("N");
  TH2F* hrms = new TH2F("hrms","RMS of muon energy (GeV)",nbin,-maprange,maprange,nbin,-maprange,maprange);
  hrms->SetStats(0);
  hrms->GetXaxis()->SetTitle("E");
  hrms->GetYaxis()->SetTitle("N");
  TH2F* hcnt = new TH2F("hcnt","counts of each bin",nbin,-maprange,maprange,nbin,-maprange,maprange);
  hcnt->SetStats(0);
  hcnt->GetXaxis()->SetTitle("E");
  hcnt->GetYaxis()->SetTitle("N");
  
  for(int i = 1; i <= nbin; i++)
    for(int j = 1; j <= nbin; j++)
    {
      double x = -maprange+2*maprange/(double)nbin*i;
      double y = -maprange+2*maprange/(double)nbin*j;
      XYPoint cell(x,y);
      if(cell.R() > maprange)
      {
        havg->SetBinContent(i,j,0);
        hrms->SetBinContent(i,j,0);
        hcnt->SetBinContent(i,j,0);
      }
      else
      {
        int ith = (int)(cos(cell.R())/.2);
        int jphi;
        if(cell.phi() > 0)
          jphi = (int)(cell.phi()*180./TMath::Pi()/120.);
        else
          jphi = (int)((TMath::TwoPi()+cell.phi())*180./TMath::Pi()/120.);
        havg->SetBinContent(i,j,binnedEStats[ith][jphi].Eavg);
        hrms->SetBinContent(i,j,binnedEStats[ith][jphi].Erms);
        hcnt->SetBinContent(i,j,binnedEStats[ith][jphi].nEvt);
      }
    }
  
  //TCanvas* c1 = new TCanvas("c1","c1",500,500);
  //havg->Draw("colz");
  
  //TCanvas* c2 = new TCanvas("c2","c2",500,500);
  //hrms->Draw("colz");
  
  //TCanvas* c3 = new TCanvas("c3","c3",500,500);
  //hcnt->Draw("colz");
  
  TCanvas* c1 = new TCanvas("c1","c1",600,600);
  c1->Divide(2,2);
  c1->cd(1);
  havg->Draw("colz");
  c1->Modified();
  c1->cd(2);
  hrms->Draw("colz");
  c1->cd(3);
  hcnt->Draw("colz");
}
