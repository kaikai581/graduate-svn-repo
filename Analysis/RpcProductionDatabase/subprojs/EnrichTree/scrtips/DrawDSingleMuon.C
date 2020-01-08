#include <fstream>
#include <vector>
#include "TROOT.h"
void DrawDSingleMuon()
{
  gROOT->ProcessLine(".! ls ../EnrichedTrees/2012/p12b/Neutrino/0101/*EH1* >flist");
  ifstream flist("flist");
  string ifpn;
  vector<string> vifpn;
  while(flist >> ifpn) vifpn.push_back(ifpn);
  flist.close();
  gROOT->ProcessLine(".! rm -f flist");
  TFile *f = new TFile(vifpn[0].c_str());
  TH1I *h = (TH1I*)f->Get("Singles2MuonDist/R3C4");
  if(!h)
  {
    cout << "specified histogram doesn't exist" << endl;
    return;
  }
  for(unsigned int i = 1; i < vifpn.size(); i++)
  {
    TFile f2(vifpn[i].c_str());
    h->Add((TH1I*)f2.Get("Singles2MuonDist/R3C4"));
  }
  h->SetTitle("");
  h->GetXaxis()->SetTitle("distance of the singles strip to the expected muon track");
  h->GetYaxis()->SetTitle("counts");
  h->GetXaxis()->CenterTitle();
  h->GetYaxis()->CenterTitle();
  h->SetLineWidth(2);
  TCanvas *c1=new TCanvas("c1","c1",500,500);
  h->Draw();
  h->Draw("TEXT0 SAME");
}
