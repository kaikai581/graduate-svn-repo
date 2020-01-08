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
#include "Rtypes.h"
#include "TGraphErrors.h"
#include "TH2F.h"
#include "TLegend.h"
#include "TMath.h"

struct OneMuData {
  double E;
  double th;
  double phi;
  int hall;
};

struct EStats {
  double Eavg;
  double Erms;
  double nEvt;
  int hall;
};

using namespace ROOT::Math;

typedef std::pair<double, unsigned int> mypair;
bool comparator ( const mypair& l, const mypair& r)
{ return l.first < r.first; }

char *fnlist[] = {"/disk1/NuWa/external/data/0.0/Muon/mountain_DYB",
                  "/disk1/NuWa/external/data/0.0/Muon/mountain_LA",
                  "/disk1/NuWa/external/data/0.0/Muon/mountain_Far_80m"};

void EnergyVsAngleCompDavid()
{
  
  vector<OneMuData> muDataList;
  
  for(int fidx = 0; fidx < 3; fidx++)
  {
    
    ifstream inf(fnlist[fidx]);
    
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
          curMu.hall = fidx+1;
          muDataList.push_back(curMu);
          break;
      }
      
      nRead++;
    }
  }

  map<int, map<int, map<int, vector<double> > > > binnedE;
  map<int, map<int, EStats> > binnedEStats;
  vector<EStats> vecEStats;
  
  for(unsigned int i = 0; i < muDataList.size(); i++)
  {
    int ith = (int)(cos(muDataList[i].th*TMath::Pi()/180.)/.2);
    int jphi = (int)(muDataList[i].phi/120.);
    
    // For each hall there are several muons with polar angle > 90 degree!
    if(ith < 0)
    {
      cout << "A muon with > 90 degree polar angle found in EH" << muDataList[i].hall << endl;
      continue;
    }
    binnedE[muDataList[i].hall][ith][jphi].push_back(muDataList[i].E);
  }
  
  /// fill "binnedEStats"
  for(int h = 1; h <= 3; h++)
  {
    map<int, map<int, vector<double> > >::iterator itth = binnedE[h].begin();
    for(; itth != binnedE[h].end(); itth++)
    {
      map<int, vector<double> >::iterator itphi = itth->second.begin();
      for(; itphi != itth->second.end(); itphi++)
      {
        binnedEStats[itth->first][itphi->first].Eavg = TMath::Mean(itphi->second.begin(), itphi->second.end());
        binnedEStats[itth->first][itphi->first].Erms = TMath::RMS(itphi->second.begin(), itphi->second.end());
        binnedEStats[itth->first][itphi->first].nEvt = itphi->second.size();
        binnedEStats[itth->first][itphi->first].hall = h;
        EStats es;
        es.Eavg = TMath::Mean(itphi->second.begin(), itphi->second.end());
        es.Erms = TMath::RMS(itphi->second.begin(), itphi->second.end());
        es.nEvt = itphi->second.size();
        es.hall = h;
        vecEStats.push_back(es);
      }
    }
  }
  
  vector<mypair> vEnIdx;
  for(unsigned int i = 0; i < vecEStats.size(); i++)
  {
    vEnIdx.push_back(mypair(vecEStats[i].Eavg, i));
  }
  sort(vEnIdx.begin(), vEnIdx.end(), comparator);
  
  
  /// prepare for graph
  double x[3][15];
  double y[3][15];
  double ex[3][15];
  double ey[3][15];
  int hallIdx[3] = {0};
  
  for(unsigned int i = 0; i < vEnIdx.size(); i++)
  {
    unsigned int idx = vEnIdx[i].second;
    int hall = vecEStats[idx].hall-1;
    x[hall][hallIdx[hall]] = vecEStats[idx].Eavg;
    y[hall][hallIdx[hall]] = i+1;
    ex[hall][hallIdx[hall]] = vecEStats[idx].Erms;
    ey[hall][hallIdx[hall]] = 0;
    hallIdx[hall]++;
  }
  
  TGraphErrors* g[3];
  EColor gcol[3] = {kBlack, kRed, kBlue};
  TCanvas* c1 = new TCanvas("c1","c1",600,600);
  c1->SetGridx(1);
  TH2F* h = new TH2F("h", "hall data", 100,-200,700,100,0,46);
  h->SetStats(0);
  h->GetXaxis()->CenterTitle();
  h->GetXaxis()->SetTitle("mean muon energy (GeV)");
  h->Draw();
  for(int h = 0; h < 3; h++)
  {
    g[h] = new TGraphErrors(15, x[h], y[h], ex[h], ey[h]);
    g[h]->SetMarkerStyle(20);
    g[h]->SetMarkerColor(gcol[h]);
    g[h]->SetLineColor(gcol[h]);
    g[h]->SetLineWidth(2);
    g[h]->SetTitle("hall data");
    g[h]->Draw("PZ");
  }
  
  TLegend* l = new TLegend(.7,.15,.9,.35);
  l->AddEntry(g[0],"Daya Bay","PL");
  l->AddEntry(g[1],"Ling Ao","PL");
  l->AddEntry(g[2],"Far","PL");
  l->SetBorderSize(0);
  l->SetFillColor(kWhite);
  l->Draw();
  
  c1->SaveAs("figures/CompDavid_hall.eps");
}
