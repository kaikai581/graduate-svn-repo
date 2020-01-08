#include <string>
#include <vector>
#include "TCanvas.h"
#include "TChain.h"
#include "TDirectory.h"
#include "TLegend.h"
#include "TH1F.h"

using namespace std;

void NeutronDistance2Track()
{
  string basepath = "/global/project/projectdirs/dayabay/scratch/sklin/RootFile/StoppedMuonsNuWa/";
  vector<string> datepath;
  datepath.push_back(string("2012/p12e/Neutrino/0104/*EH1*.root"));
  datepath.push_back(string("2012/p12e/Neutrino/0105/*EH1*.root"));
  datepath.push_back(string("2012/p12e/Neutrino/0106/*EH1*.root"));
  datepath.push_back(string("2012/p12e/Neutrino/0107/*EH1*.root"));
  datepath.push_back(string("2012/p12e/Neutrino/0108/*EH1*.root"));
  datepath.push_back(string("2012/p12e/Neutrino/0109/*EH1*.root"));
  datepath.push_back(string("2012/p12e/Neutrino/0110/*EH1*.root"));
  datepath.push_back(string("2012/p12e/Neutrino/0111/*EH1*.root"));
  datepath.push_back(string("2012/p12e/Neutrino/0112/*EH1*.root"));
  datepath.push_back(string("2012/p12e/Neutrino/0113/*EH1*.root"));
  
  TChain* ch = new TChain("dmucoin");
  for(unsigned int i = 0; i < datepath.size(); i++)
    ch->Add((basepath+datepath[i]).c_str());
  
  //TCanvas* c1 = new TCanvas("c1");
  //ch->Draw("rhoTrack","inCyl==1");
  ////TH1F* htemp = (TH1F*)gDirectory->Get("htemp");
  ////htemp->SetTitle("");
  ////htemp->GetXaxis()->SetTitle("closest approach (mm)");
  
  //c1->SaveAs("n_distance_2_track.eps");
  
  //TCanvas* c2 = new TCanvas("c2");
  //ch->Draw("eMu/dlIav:rhoTrack","dlIav>0&&inCyl==1");
  
  TCanvas* c3 = new TCanvas("c3");
  TLegend* l = new TLegend(.7,.5,.9,.9);
  TH1F* hEMuDist[9];
  for(int i = 0; i < 9; i++)
  {
    hEMuDist[i] = new TH1F(Form("hEMuDist%d",i), "", 100,0,1000);
    ch->Project(Form("hEMuDist%d",i), "rhoTrack", Form("dlIav>0&&inCyl==1&&eMu/(dlRimMu+dlMuRim)>%f&&eMu/(dlRimMu+dlMuRim)<%f&&eMi>6&&eMi<12",0.2*i,0.2*i+0.2));
    hEMuDist[i]->SetLineWidth(2);
    hEMuDist[i]->SetLineColor(i+1);
    if(i == 0)
    {
      hEMuDist[0]->GetXaxis()->SetTitle("closest approach (mm)");
      hEMuDist[0]->Draw();
    }
    else hEMuDist[i]->Draw("same");
    l->AddEntry(hEMuDist[i], Form("%f<dE/dx<%f",0.2*i,0.2*i+0.2), "L");
  }
  l->Draw();
}
