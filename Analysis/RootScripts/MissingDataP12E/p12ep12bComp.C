#include "TCanvas.h"
#include "TChain.h"
#include "TH1F.h"


p12ep12bComp(string leafname="nHitsPool")
{
  TChain* ch1 = new TChain("/Event/CalibReadout/CalibReadoutHeader");
  TChain* ch2 = new TChain("/Event/CalibReadout/CalibReadoutHeader");
  
  ch1->Add("/eliza16/dayabay/data/exp/dayabay/2011/p12e/Neutrino/1224/*21223*.root");
  ch1->Add("/eliza16/dayabay/data/exp/dayabay/2011/p12e/Neutrino/1225/*21223*.root");
  ch1->Add("/eliza16/dayabay/data/exp/dayabay/2011/p12e/Neutrino/1226/*21223*.root");
  
  ch2->Add("/eliza16/dayabay/data/exp/dayabay/2011/p12b/Neutrino/1224/*21223*.root");
  ch2->Add("/eliza16/dayabay/data/exp/dayabay/2011/p12b/Neutrino/1225/*21223*.root");
  ch2->Add("/eliza16/dayabay/data/exp/dayabay/2011/p12b/Neutrino/1226/*21223*.root");
  
  TCanvas* c1 = new TCanvas("c1","c1",1000,500);
  c1->Divide(2,1);
  
  c1->cd(2);
  ch1->Draw(Form("%s>>h1",leafname.c_str()),"detector==5");
  h1->SetTitle("p12e run21223");
  h1->GetXaxis()->SetTitle(leafname.c_str());
  
  c1->cd(1);
  ch2->Draw(Form("%s>>h2",leafname.c_str()),"detector==5");
  h2->SetTitle("p12b run21223");
  h2->GetXaxis()->SetTitle(leafname.c_str());
  
}
