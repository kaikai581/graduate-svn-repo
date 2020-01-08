#include <fstream>
#include <iostream>
#include <sstream>
#include "TChain.h"
#include "TFile.h"
#include "TH1F.h"


void RhoDistSpecifyInput(int det = 1, string infname = "EH3DataPath.txt")
{
  vector<string> folders;
  ifstream infpath(infname.c_str());
  string tmpline;
  while(infpath >> tmpline) folders.push_back(tmpline.c_str());
  
  TChain* ch = new TChain("RPC-OWS/mun");
  for(unsigned int i = 0; i < folders.size(); i++)
  {
    string files = folders[i];
    ch->Add(files.c_str());
  }
  
  
  stringstream outfn;
  outfn << infname << ".root";
  TFile* f1 = new TFile(outfn.str().c_str(), "UPDATE");
  TH1F* h1 = new TH1F(Form("ad%d",det),Form("AD%d neutron radial distribution", det), 50, 0, 1000);
  //TH1F* h1shower = new TH1F(Form("ad%d_shower",det),Form("AD%d neutron radial distribution of shower muons", det), 50, 0, 1000);
  //TH1F* h1nonshower = new TH1F(Form("ad%d_nonshower",det),Form("AD%d neutron radial distribution of nonshower muons", det), 50, 0, 1000);
  h1->GetXaxis()->SetTitle("closest approach (mm)");
  //h1shower->GetXaxis()->SetTitle("closest approach (mm)");
  //h1nonshower->GetXaxis()->SetTitle("closest approach (mm)");

  ch->Project(Form("ad%d",det),Form("dlIntEvt%d",det),Form("inCyl%d&&eIntEvtAd%d>6&&eIntEvtAd%d<12&&dtIntEvtAd%d>20e-6&&dtIntEvtAd%d<1000e-6",det,det,det,det,det));
  ch->Project(Form("ad%d_shower",det),Form("dlIntEvt%d",det),Form("inCyl%d&&eIntEvtAd%d>6&&eIntEvtAd%d<12&&dtIntEvtAd%d>20e-6&&dtIntEvtAd%d<1000e-6&&eAd%d>1500",det,det,det,det,det,det));
  ch->Project(Form("ad%d_nonshower",det),Form("dlIntEvt%d",det),Form("inCyl%d&&eIntEvtAd%d>6&&eIntEvtAd%d<12&&dtIntEvtAd%d>20e-6&&dtIntEvtAd%d<1000e-6&&eAd%d<1500",det,det,det,det,det,det));
  h1->Write(Form("ad%d",det), TObject::kOverwrite);
  //h1shower->Write(Form("ad%d_shower",det), TObject::kOverwrite);
  //h1nonshower->Write(Form("ad%d_nonshower",det), TObject::kOverwrite);
  f1->Close();
}
