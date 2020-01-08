#include "TCanvas.h"
#include "TF1.h"
#include "TFile.h"
#include "TH1F.h"
#include "TTree.h"
#include "TStyle.h"

void NeutronZ()
{
  //// Set stat options
  //gStyle->SetStatY(0.9);
  //// Set y-position (fraction of pad size)
  //gStyle->SetStatX(0.47);
  //// Set x-position (fraction of pad size)
  gStyle->SetStatW(0.2);
  //// Set width of stat-box (fraction of pad size)
  gStyle->SetStatH(0.1);
  //// Set height of stat-box (fraction of pad size)
  gStyle->SetOptFit(1);
  
  TFile* inf = new TFile("../../output/simneutron.root");
  TTree* t = (TTree*)inf->Get("t");
  
  TCanvas* c1 = new TCanvas("c1","c1",500,500);
  TH1F* hrad = new TH1F("hrad","neutron z' / (cylinder length)",50,-.5,.5);
  t->Project("hrad","zTrack/cylLen","nNeuInCyl==1");
  
  hrad->GetXaxis()->SetTitle("scaled z");
  hrad->SetMinimum(0);
  hrad->SetMaximum(300);
  hrad->Draw();
  
  TF1* ffit1 = new TF1("pol0", "pol0", -.5, .5);
  hrad->Fit("pol0");
  
  c1->SaveAs("z.gif");
}
