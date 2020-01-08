#include "TCanvas.h"
#include "TF1.h"
#include "TFile.h"
#include "TH1F.h"
#include "TTree.h"
#include "TStyle.h"

void NeutronRadial()
{
  // Set stat options
  gStyle->SetStatY(0.9);
  // Set y-position (fraction of pad size)
  gStyle->SetStatX(0.47);
  // Set x-position (fraction of pad size)
  gStyle->SetStatW(0.2);
  // Set width of stat-box (fraction of pad size)
  gStyle->SetStatH(0.2);
  // Set height of stat-box (fraction of pad size)
  gStyle->SetOptFit(1);
  
  TFile* inf = new TFile("../../output/simneutron.root");
  TTree* t = (TTree*)inf->Get("t");
  
  TCanvas* c1 = new TCanvas("c1","c1",500,500);
  TH1F* hrad = new TH1F("hrad","neutron closest approach",50,0,1000);
  t->Project("hrad","rhoTrack","nNeuInCyl==1");
  
  hrad->GetXaxis()->SetTitle("distance to the track (mm)");
  hrad->Draw();
  
  TF1* ffit1 = new TF1("ffit1", "[0]*x*exp(-x*x/2/[1]/[1])", 0, 1000);
  ffit1->SetParameter(0,.3);
  ffit1->SetParameter(1,1500);
  hrad->Fit("ffit1");
  
  c1->SaveAs("radial.gif");
}
