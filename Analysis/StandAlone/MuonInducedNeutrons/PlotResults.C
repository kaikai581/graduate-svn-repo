#include <fstream>
#include <iostream>
#include "TCanvas.h"
#include "TF1.h"
#include "TFile.h"
#include "TH1F.h"
#include "TParameter.h"
#include "TROOT.h"


using namespace std;

void PlotResults()
{
  gROOT->ProcessLine(".! ls -d output/* > refinedlist.txt");
  
  vector<string> EH1filelist, EH2filelist, EH3filelist;
  TCanvas* c[6];
  TH1F* radDist[6], *radDistEast[6], *radDistWest[6], *radDistHighLoss[6], *radDistLowLoss[6], *radDistEastThCut[6], *radDistWestThCut[6];
  TH1F* dt[6];
  TH1F* dEdx[6];
  TF1* fitrad[6], *fitdt[6], *fitradEast[6], *fitradWest[6];
  double tottrlen[6] = {0}, tottrlenEast[6] = {0}, tottrlenWest[6] = {0};
  int nNeu[6] = {0}, nNeuEast[6] = {0}, nNeuWest[6] = {0};
  
  ifstream inflist("refinedlist.txt");
  string tmpfn;
  while(inflist >> tmpfn)
  {
    if(tmpfn.find("EH1") != string::npos)
      EH1filelist.push_back(tmpfn);
    if(tmpfn.find("EH2") != string::npos)
      EH2filelist.push_back(tmpfn);
    if(tmpfn.find("EH3") != string::npos)
      EH3filelist.push_back(tmpfn);
  }
  inflist.close();
  
  //for(unsigned int i = 0; i < EH1filelist.size(); i++) cout << EH2filelist[i] << endl;
  /// 6 AD results. Numbering from AD1 to AD6.
  dt[0] = new TH1F("dt0","EH1 AD1 neutron capture time",500,0,1000);
  dt[1] = new TH1F("dt1","EH1 AD2 neutron capture time",500,0,1000);
  dt[2] = new TH1F("dt2","EH2 AD1 neutron capture time",500,0,1000);
  dt[3] = new TH1F("dt3","EH3 AD1 neutron capture time",500,0,1000);
  dt[4] = new TH1F("dt4","EH3 AD2 neutron capture time",500,0,1000);
  dt[5] = new TH1F("dt5","EH3 AD3 neutron capture time",500,0,1000);
  dEdx[0] = new TH1F("dEdx0","EH1 AD1 muon energy loss",500,0,20);
  dEdx[1] = new TH1F("dEdx1","EH1 AD2 muon energy loss",500,0,20);
  dEdx[2] = new TH1F("dEdx2","EH2 AD1 muon energy loss",500,0,20);
  dEdx[3] = new TH1F("dEdx3","EH3 AD1 muon energy loss",500,0,20);
  dEdx[4] = new TH1F("dEdx4","EH3 AD2 muon energy loss",500,0,20);
  dEdx[5] = new TH1F("dEdx5","EH3 AD3 muon energy loss",500,0,20);
  radDist[0] = new TH1F("radDist0","EH1 AD1 neutron lateral distance",200,0,1000);
  radDist[1] = new TH1F("radDist1","EH1 AD2 neutron lateral distance",200,0,1000);
  radDist[2] = new TH1F("radDist2","EH2 AD1 neutron lateral distance",200,0,1000);
  radDist[3] = new TH1F("radDist3","EH3 AD1 neutron lateral distance",200,0,1000);
  radDist[4] = new TH1F("radDist4","EH3 AD2 neutron lateral distance",200,0,1000);
  radDist[5] = new TH1F("radDist5","EH3 AD3 neutron lateral distance",200,0,1000);
  radDistEast[0] = new TH1F("radDist0East","EH1 AD1 neutron lateral distance from east muons",200,0,1000);
  radDistEast[1] = new TH1F("radDist1East","EH1 AD2 neutron lateral distance from east muons",200,0,1000);
  radDistEast[2] = new TH1F("radDist2East","EH2 AD1 neutron lateral distance from east muons",200,0,1000);
  radDistEast[3] = new TH1F("radDist3East","EH3 AD1 neutron lateral distance from east muons",200,0,1000);
  radDistEast[4] = new TH1F("radDist4East","EH3 AD2 neutron lateral distance from east muons",200,0,1000);
  radDistEast[5] = new TH1F("radDist5East","EH3 AD3 neutron lateral distance from east muons",200,0,1000);
  radDistWest[0] = new TH1F("radDist0West","EH1 AD1 neutron lateral distance from west muons",200,0,1000);
  radDistWest[1] = new TH1F("radDist1West","EH1 AD2 neutron lateral distance from west muons",200,0,1000);
  radDistWest[2] = new TH1F("radDist2West","EH2 AD1 neutron lateral distance from west muons",200,0,1000);
  radDistWest[3] = new TH1F("radDist3West","EH3 AD1 neutron lateral distance from west muons",200,0,1000);
  radDistWest[4] = new TH1F("radDist4West","EH3 AD2 neutron lateral distance from west muons",200,0,1000);
  radDistWest[5] = new TH1F("radDist5West","EH3 AD3 neutron lateral distance from west muons",200,0,1000);
  radDistEastThCut[0] = new TH1F("radDist0EastThCut","EH1 AD1 neutron lateral distance from east muons with #theta < 45",200,0,1000);
  radDistEastThCut[1] = new TH1F("radDist1EastThCut","EH1 AD2 neutron lateral distance from east muons with #theta < 45",200,0,1000);
  radDistEastThCut[2] = new TH1F("radDist2EastThCut","EH2 AD1 neutron lateral distance from east muons with #theta < 45",200,0,1000);
  radDistEastThCut[3] = new TH1F("radDist3EastThCut","EH3 AD1 neutron lateral distance from east muons with #theta < 45",200,0,1000);
  radDistEastThCut[4] = new TH1F("radDist4EastThCut","EH3 AD2 neutron lateral distance from east muons with #theta < 45",200,0,1000);
  radDistEastThCut[5] = new TH1F("radDist5EastThCut","EH3 AD3 neutron lateral distance from east muons with #theta < 45",200,0,1000);
  radDistWestThCut[0] = new TH1F("radDist0WestThCut","EH1 AD1 neutron lateral distance from west muons with #theta < 45",200,0,1000);
  radDistWestThCut[1] = new TH1F("radDist1WestThCut","EH1 AD2 neutron lateral distance from west muons with #theta < 45",200,0,1000);
  radDistWestThCut[2] = new TH1F("radDist2WestThCut","EH2 AD1 neutron lateral distance from west muons with #theta < 45",200,0,1000);
  radDistWestThCut[3] = new TH1F("radDist3WestThCut","EH3 AD1 neutron lateral distance from west muons with #theta < 45",200,0,1000);
  radDistWestThCut[4] = new TH1F("radDist4WestThCut","EH3 AD2 neutron lateral distance from west muons with #theta < 45",200,0,1000);
  radDistWestThCut[5] = new TH1F("radDist5WestThCut","EH3 AD3 neutron lateral distance from west muons with #theta < 45",200,0,1000);
  radDistHighLoss[0] = new TH1F("radDist0HighLoss","EH1 AD1 neutron lateral distance",200,0,1000);
  radDistHighLoss[1] = new TH1F("radDist1HighLoss","EH1 AD2 neutron lateral distance",200,0,1000);
  radDistHighLoss[2] = new TH1F("radDist2HighLoss","EH2 AD1 neutron lateral distance",200,0,1000);
  radDistHighLoss[3] = new TH1F("radDist3HighLoss","EH3 AD1 neutron lateral distance",200,0,1000);
  radDistHighLoss[4] = new TH1F("radDist4HighLoss","EH3 AD2 neutron lateral distance",200,0,1000);
  radDistHighLoss[5] = new TH1F("radDist5HighLoss","EH3 AD3 neutron lateral distance",200,0,1000);
  radDistLowLoss[0] = new TH1F("radDist0LowLoss","EH1 AD1 neutron lateral distance",200,0,1000);
  radDistLowLoss[1] = new TH1F("radDist1LowLoss","EH1 AD2 neutron lateral distance",200,0,1000);
  radDistLowLoss[2] = new TH1F("radDist2LowLoss","EH2 AD1 neutron lateral distance",200,0,1000);
  radDistLowLoss[3] = new TH1F("radDist3LowLoss","EH3 AD1 neutron lateral distance",200,0,1000);
  radDistLowLoss[4] = new TH1F("radDist4LowLoss","EH3 AD2 neutron lateral distance",200,0,1000);
  radDistLowLoss[5] = new TH1F("radDist5LowLoss","EH3 AD3 neutron lateral distance",200,0,1000);
  for(int i = 0; i < 6; i++)
  {
    dt[i]->GetXaxis()->SetTitle("capture time (#mus)");
    dEdx[i]->GetXaxis()->SetTitle("energy loss (MeV/cm)");
    radDist[i]->GetXaxis()->SetTitle("lateral distance (mm)");
    radDistEast[i]->GetXaxis()->SetTitle("lateral distance (mm)");
    radDistEast[i]->SetLineColor(kGreen);
    radDistWest[i]->GetXaxis()->SetTitle("lateral distance (mm)");
    radDistWest[i]->SetLineColor(kMagenta);
    radDistEastThCut[i]->GetXaxis()->SetTitle("lateral distance (mm)");
    radDistEastThCut[i]->SetLineColor(kGreen);
    radDistWestThCut[i]->GetXaxis()->SetTitle("lateral distance (mm)");
    radDistWestThCut[i]->SetLineColor(kMagenta);
    radDistHighLoss[i]->GetXaxis()->SetTitle("lateral distance (mm)");
    radDistHighLoss[i]->SetLineColor(kGreen);
    radDistLowLoss[i]->GetXaxis()->SetTitle("lateral distance (mm)");
    radDistLowLoss[i]->SetLineColor(kMagenta);
  }
  
  /// process EH1
  for(unsigned int i = 0; i < EH1filelist.size(); i++)
  {
    TFile f(EH1filelist[i].c_str());
    TParameter<double>* totTrLen1 = (TParameter<double>*)f.Get("totTrLen1");
    TParameter<double>* totTrLen2 = (TParameter<double>*)f.Get("totTrLen2");
    TParameter<double>* totTrLen1East = (TParameter<double>*)f.Get("totTrLen1East");
    TParameter<double>* totTrLen2East = (TParameter<double>*)f.Get("totTrLen2East");
    TParameter<double>* totTrLen1West = (TParameter<double>*)f.Get("totTrLen1West");
    TParameter<double>* totTrLen2West = (TParameter<double>*)f.Get("totTrLen2West");
    TParameter<int>* nNeu1 = (TParameter<int>*)f.Get("nNeu1");
    TParameter<int>* nNeu2 = (TParameter<int>*)f.Get("nNeu2");
    TParameter<int>* nNeu1East = (TParameter<int>*)f.Get("nNeu1East");
    TParameter<int>* nNeu2East = (TParameter<int>*)f.Get("nNeu2East");
    TParameter<int>* nNeu1West = (TParameter<int>*)f.Get("nNeu1West");
    TParameter<int>* nNeu2West = (TParameter<int>*)f.Get("nNeu2West");
    TH1F* radDist1 = (TH1F*)f.Get("radDist1");
    TH1F* radDist2 = (TH1F*)f.Get("radDist2");
    TH1F* radDist1East = (TH1F*)f.Get("radDist1East");
    TH1F* radDist2East = (TH1F*)f.Get("radDist2East");
    TH1F* radDist1West = (TH1F*)f.Get("radDist1West");
    TH1F* radDist2West = (TH1F*)f.Get("radDist2West");
    TH1F* radDist1EastThCut = (TH1F*)f.Get("radDist1EastThCut");
    TH1F* radDist2EastThCut = (TH1F*)f.Get("radDist2EastThCut");
    TH1F* radDist1WestThCut = (TH1F*)f.Get("radDist1WestThCut");
    TH1F* radDist2WestThCut = (TH1F*)f.Get("radDist2WestThCut");
    TH1F* radDist1HighLoss = (TH1F*)f.Get("radDist1HighLoss");
    TH1F* radDist2HighLoss = (TH1F*)f.Get("radDist2HighLoss");
    TH1F* radDist1LowLoss = (TH1F*)f.Get("radDist1LowLoss");
    TH1F* radDist2LowLoss = (TH1F*)f.Get("radDist2LowLoss");
    TH1F* dt1 = (TH1F*)f.Get("dt1");
    TH1F* dt2 = (TH1F*)f.Get("dt2");
    TH1F* dEdx1 = (TH1F*)f.Get("dEdx1");
    TH1F* dEdx2 = (TH1F*)f.Get("dEdx2");
    radDist[0]->Add(radDist1);
    radDist[1]->Add(radDist2);
    radDistEast[0]->Add(radDist1East);
    radDistEast[1]->Add(radDist2East);
    radDistWest[0]->Add(radDist1West);
    radDistWest[1]->Add(radDist2West);
    radDistEastThCut[0]->Add(radDist1EastThCut);
    radDistEastThCut[1]->Add(radDist2EastThCut);
    radDistWestThCut[0]->Add(radDist1WestThCut);
    radDistWestThCut[1]->Add(radDist2WestThCut);
    radDistHighLoss[0]->Add(radDist1HighLoss);
    radDistHighLoss[1]->Add(radDist2HighLoss);
    radDistLowLoss[0]->Add(radDist1LowLoss);
    radDistLowLoss[1]->Add(radDist2LowLoss);
    dt[0]->Add(dt1);
    dt[1]->Add(dt2);
    dEdx[0]->Add(dEdx1);
    dEdx[1]->Add(dEdx2);
    tottrlen[0] += totTrLen1->GetVal();
    tottrlen[1] += totTrLen2->GetVal();
    tottrlenEast[0] += totTrLen1East->GetVal();
    tottrlenEast[1] += totTrLen2East->GetVal();
    tottrlenWest[0] += totTrLen1West->GetVal();
    tottrlenWest[1] += totTrLen2West->GetVal();
    nNeu[0] += nNeu1->GetVal();
    nNeu[1] += nNeu2->GetVal();
    nNeuEast[0] += nNeu1East->GetVal();
    nNeuEast[1] += nNeu2East->GetVal();
    nNeuWest[0] += nNeu1West->GetVal();
    nNeuWest[1] += nNeu2West->GetVal();
  }
  
  /// process EH2
  for(unsigned int i = 0; i < EH2filelist.size(); i++)
  {
    TFile f(EH2filelist[i].c_str());
    TParameter<double>* totTrLen1 = (TParameter<double>*)f.Get("totTrLen1");
    TParameter<double>* totTrLen1East = (TParameter<double>*)f.Get("totTrLen1East");
    TParameter<double>* totTrLen1West = (TParameter<double>*)f.Get("totTrLen1West");
    TParameter<int>* nNeu1 = (TParameter<int>*)f.Get("nNeu1");
    TParameter<int>* nNeu1East = (TParameter<int>*)f.Get("nNeu1East");
    TParameter<int>* nNeu1West = (TParameter<int>*)f.Get("nNeu1West");
    TH1F* radDist1 = (TH1F*)f.Get("radDist1");
    TH1F* radDist1East = (TH1F*)f.Get("radDist1East");
    TH1F* radDist1West = (TH1F*)f.Get("radDist1West");
    TH1F* radDist1EastThCut = (TH1F*)f.Get("radDist1EastThCut");
    TH1F* radDist1WestThCut = (TH1F*)f.Get("radDist1WestThCut");
    TH1F* radDist1HighLoss = (TH1F*)f.Get("radDist1HighLoss");
    TH1F* radDist1LowLoss = (TH1F*)f.Get("radDist1LowLoss");
    TH1F* dt1 = (TH1F*)f.Get("dt1");
    TH1F* dEdx1 = (TH1F*)f.Get("dEdx1");
    radDist[2]->Add(radDist1);
    radDistEast[2]->Add(radDist1East);
    radDistWest[2]->Add(radDist1West);
    radDistEastThCut[2]->Add(radDist1EastThCut);
    radDistWestThCut[2]->Add(radDist1WestThCut);
    radDistHighLoss[2]->Add(radDist1HighLoss);
    radDistLowLoss[2]->Add(radDist1LowLoss);
    dt[2]->Add(dt1);
    dEdx[2]->Add(dEdx1);
    tottrlen[2] += totTrLen1->GetVal();
    tottrlenEast[2] += totTrLen1East->GetVal();
    tottrlenWest[2] += totTrLen1West->GetVal();
    nNeu[2] += nNeu1->GetVal();
    nNeuEast[2] += nNeu1East->GetVal();
    nNeuWest[2] += nNeu1West->GetVal();
  }
  
  /// process EH3
  for(unsigned int i = 0; i < EH3filelist.size(); i++)
  {
    TFile f(EH3filelist[i].c_str());
    TParameter<double>* totTrLen1 = (TParameter<double>*)f.Get("totTrLen1");
    TParameter<double>* totTrLen2 = (TParameter<double>*)f.Get("totTrLen2");
    TParameter<double>* totTrLen3 = (TParameter<double>*)f.Get("totTrLen3");
    TParameter<double>* totTrLen1East = (TParameter<double>*)f.Get("totTrLen1East");
    TParameter<double>* totTrLen2East = (TParameter<double>*)f.Get("totTrLen2East");
    TParameter<double>* totTrLen3East = (TParameter<double>*)f.Get("totTrLen3East");
    TParameter<double>* totTrLen1West = (TParameter<double>*)f.Get("totTrLen1West");
    TParameter<double>* totTrLen2West = (TParameter<double>*)f.Get("totTrLen2West");
    TParameter<double>* totTrLen3West = (TParameter<double>*)f.Get("totTrLen3West");
    TParameter<int>* nNeu1 = (TParameter<int>*)f.Get("nNeu1");
    TParameter<int>* nNeu2 = (TParameter<int>*)f.Get("nNeu2");
    TParameter<int>* nNeu3 = (TParameter<int>*)f.Get("nNeu3");
    TParameter<int>* nNeu1East = (TParameter<int>*)f.Get("nNeu1East");
    TParameter<int>* nNeu2East = (TParameter<int>*)f.Get("nNeu2East");
    TParameter<int>* nNeu3East = (TParameter<int>*)f.Get("nNeu3East");
    TParameter<int>* nNeu1West = (TParameter<int>*)f.Get("nNeu1West");
    TParameter<int>* nNeu2West = (TParameter<int>*)f.Get("nNeu2West");
    TParameter<int>* nNeu3West = (TParameter<int>*)f.Get("nNeu3West");
    TH1F* radDist1 = (TH1F*)f.Get("radDist1");
    TH1F* radDist2 = (TH1F*)f.Get("radDist2");
    TH1F* radDist3 = (TH1F*)f.Get("radDist3");
    TH1F* radDist1East = (TH1F*)f.Get("radDist1East");
    TH1F* radDist2East = (TH1F*)f.Get("radDist2East");
    TH1F* radDist3East = (TH1F*)f.Get("radDist3East");
    TH1F* radDist1West = (TH1F*)f.Get("radDist1West");
    TH1F* radDist2West = (TH1F*)f.Get("radDist2West");
    TH1F* radDist3West = (TH1F*)f.Get("radDist3West");
    TH1F* radDist1EastThCut = (TH1F*)f.Get("radDist1EastThCut");
    TH1F* radDist2EastThCut = (TH1F*)f.Get("radDist2EastThCut");
    TH1F* radDist3EastThCut = (TH1F*)f.Get("radDist3EastThCut");
    TH1F* radDist1WestThCut = (TH1F*)f.Get("radDist1WestThCut");
    TH1F* radDist2WestThCut = (TH1F*)f.Get("radDist2WestThCut");
    TH1F* radDist3WestThCut = (TH1F*)f.Get("radDist3WestThCut");
    TH1F* radDist1HighLoss = (TH1F*)f.Get("radDist1HighLoss");
    TH1F* radDist2HighLoss = (TH1F*)f.Get("radDist2HighLoss");
    TH1F* radDist3HighLoss = (TH1F*)f.Get("radDist3HighLoss");
    TH1F* radDist1LowLoss = (TH1F*)f.Get("radDist1LowLoss");
    TH1F* radDist2LowLoss = (TH1F*)f.Get("radDist2LowLoss");
    TH1F* radDist3LowLoss = (TH1F*)f.Get("radDist3LowLoss");
    TH1F* dt1 = (TH1F*)f.Get("dt1");
    TH1F* dt2 = (TH1F*)f.Get("dt2");
    TH1F* dt3 = (TH1F*)f.Get("dt3");
    TH1F* dEdx1 = (TH1F*)f.Get("dEdx1");
    TH1F* dEdx2 = (TH1F*)f.Get("dEdx2");
    TH1F* dEdx3 = (TH1F*)f.Get("dEdx3");
    radDist[3]->Add(radDist1);
    radDist[4]->Add(radDist2);
    radDist[5]->Add(radDist3);
    radDistEast[3]->Add(radDist1East);
    radDistEast[4]->Add(radDist2East);
    radDistEast[5]->Add(radDist3East);
    radDistWest[3]->Add(radDist1West);
    radDistWest[4]->Add(radDist2West);
    radDistWest[5]->Add(radDist3West);
    radDistEastThCut[3]->Add(radDist1EastThCut);
    radDistEastThCut[4]->Add(radDist2EastThCut);
    radDistEastThCut[5]->Add(radDist3EastThCut);
    radDistWestThCut[3]->Add(radDist1WestThCut);
    radDistWestThCut[4]->Add(radDist2WestThCut);
    radDistWestThCut[5]->Add(radDist3WestThCut);
    radDistHighLoss[3]->Add(radDist1HighLoss);
    radDistHighLoss[4]->Add(radDist2HighLoss);
    radDistHighLoss[5]->Add(radDist3HighLoss);
    radDistLowLoss[3]->Add(radDist1LowLoss);
    radDistLowLoss[4]->Add(radDist2LowLoss);
    radDistLowLoss[5]->Add(radDist3LowLoss);
    dt[3]->Add(dt1);
    dt[4]->Add(dt2);
    dt[5]->Add(dt3);
    dEdx[3]->Add(dEdx1);
    dEdx[4]->Add(dEdx2);
    dEdx[5]->Add(dEdx3);
    tottrlen[3] += totTrLen1->GetVal();
    tottrlen[4] += totTrLen2->GetVal();
    tottrlen[5] += totTrLen3->GetVal();
    tottrlenEast[3] += totTrLen1East->GetVal();
    tottrlenEast[4] += totTrLen2East->GetVal();
    tottrlenEast[5] += totTrLen3East->GetVal();
    tottrlenWest[3] += totTrLen1West->GetVal();
    tottrlenWest[4] += totTrLen2West->GetVal();
    tottrlenWest[5] += totTrLen3West->GetVal();
    nNeu[3] += nNeu1->GetVal();
    nNeu[4] += nNeu2->GetVal();
    nNeu[5] += nNeu3->GetVal();
    nNeuEast[3] += nNeu1East->GetVal();
    nNeuEast[4] += nNeu2East->GetVal();
    nNeuEast[5] += nNeu3East->GetVal();
    nNeuWest[3] += nNeu1West->GetVal();
    nNeuWest[4] += nNeu2West->GetVal();
    nNeuWest[5] += nNeu3West->GetVal();
  }
  for(int i = 0; i < 6; i++)
  {
    cout << "all angle: " << nNeu[i] << " " << tottrlen[i] << " " << nNeu[i]/tottrlen[i]*10 << endl;
    cout << "east muon: " << nNeuEast[i] << " " << tottrlenEast[i] << " " << nNeuEast[i]/tottrlenEast[i]*10 << endl;
    cout << "west muon: " << nNeuWest[i] << " " << tottrlenWest[i] << " " << nNeuWest[i]/tottrlenWest[i]*10 << endl;
  }
  
  for(int i = 0; i < 6; i++)
  {
    fitrad[i] = new TF1(Form("fitrad%d",i+1),"[0]*x*exp(-x*x/2/[1]/[1])",0,1000);
    fitrad[i]->SetLineColor(kRed);
    fitrad[i]->SetParameters(1000,600);
    radDist[i]->Fit(fitrad[i]);
    fitradEast[i] = new TF1(Form("fitrad%dEast",i+1),"[0]*x*exp(-x*x/2/[1]/[1])",0,1000);
    fitradEast[i]->SetLineColor(kRed);
    fitradEast[i]->SetParameters(100,600);
    radDistEast[i]->Fit(fitradEast[i]);
    fitradWest[i] = new TF1(Form("fitrad%dWest",i+1),"[0]*x*exp(-x*x/2/[1]/[1])",0,1000);
    fitradWest[i]->SetLineColor(kRed);
    fitradWest[i]->SetParameters(100,600);
    radDistWest[i]->Fit(fitradWest[i]);
    fitdt[i] = new TF1(Form("fitdt%d",i+1),"[0]*exp(-x/[1])",0,1000);
    fitdt[i]->SetLineWidth(2);
    fitdt[i]->SetLineColor(kGreen);
    fitdt[i]->SetParameters(1000,30);
    dt[i]->Fit(fitdt[i]);
  }
  c[0] = new TCanvas("c0","c0",900,600);
  c[0]->Divide(3,2);
  c[1] = new TCanvas("c1","c1",900,600);
  c[1]->Divide(3,2);
  c[2] = new TCanvas("c2","c2",900,600);
  c[2]->Divide(3,2);
  c[3] = new TCanvas("c3","c3",900,600);
  c[3]->Divide(3,2);
  c[4] = new TCanvas("c4","c4",900,600);
  c[4]->Divide(3,2);
  c[5] = new TCanvas("c5","c5",900,600);
  c[5]->Divide(3,2);
  
  for(int i = 1; i <= 6; i++)
  {
    c[0]->cd(i);
    radDist[i-1]->Draw();
  }
  for(int i = 1; i <= 6; i++)
  {
    c[1]->cd(i);
    gPad->SetLogy();
    dt[i-1]->Draw();
  }
  for(int i = 1; i <= 6; i++)
  {
    c[2]->cd(i);
    radDistEast[i-1]->Draw();
    radDistWest[i-1]->Draw("same");
  }
  for(int i = 1; i <= 6; i++)
  {
    c[3]->cd(i);
    radDistHighLoss[i-1]->Draw();
    radDistLowLoss[i-1]->Draw("same");
  }
  for(int i = 1; i <= 6; i++)
  {
    c[4]->cd(i);
    radDistEastThCut[i-1]->Draw();
    radDistWestThCut[i-1]->Draw("same");
  }
  for(int i = 1; i <= 6; i++)
  {
    c[5]->cd(i);
    dEdx[i-1]->Draw();
  }
  
  c[0]->SaveAs("plots/rad_dist.eps");
  c[1]->SaveAs("plots/dt_dist.eps");
  c[2]->SaveAs("plots/rad_dist_east_west.eps");
  c[3]->SaveAs("plots/rad_dist_Low_high_loss.eps");
  c[4]->SaveAs("plots/rad_dist_east_west_th_cut.eps");
  c[5]->SaveAs("plots/dEdx.eps");
  
  for(int i = 0; i < 6; i++)
  {
    double cor = .8383*.9224*.5972*.861;
    double eastAcc = 1-exp(-1000*1000/2/fitradEast[i]->GetParameter(1)/fitradEast[i]->GetParameter(1));
    double westAcc = 1-exp(-1000*1000/2/fitradWest[i]->GetParameter(1)/fitradWest[i]->GetParameter(1));
    cout << "AD" << i+1 << " east acc: " << eastAcc << " east width: " << fitradEast[i]->GetParameter(1) << endl;
    cout << "AD" << i+1 << " wast acc: " << westAcc << " west width: " << fitradWest[i]->GetParameter(1) <<  endl;
    cout << "AD" << i+1 << " east yield: " << nNeuEast[i]/tottrlenEast[i]*10/cor/eastAcc << endl;
    cout << "AD" << i+1 << " west yield: " << nNeuEast[i]/tottrlenEast[i]*10/cor/westAcc << endl;
  }
}
