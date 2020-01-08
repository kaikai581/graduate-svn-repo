#include <fstream>
#include <vector>
#include "TROOT.h"
void DrawClusterSize()
{
  gROOT->ProcessLine(".! ls ../EnrichedTrees/2012/p12b/Neutrino/0101/*EH1* >flist");
  //gROOT->ProcessLine(".! ls ../EnrichedTrees/2012/p12b/Neutrino/0101/enrichedrpctree.Neutrino.0021358.Physics.EH1-Merged.P12B-P._0002.root >flist");
  ifstream flist("flist");
  string ifpn;
  vector<string> vifpn;
  while(flist >> ifpn) vifpn.push_back(ifpn);
  flist.close();
  gROOT->ProcessLine(".! rm -f flist");
  TFile *f = new TFile(vifpn[0].c_str());
  TH1I *h = (TH1I*)f->Get("ClusterSize/R3C4");
  if(!h)
  {
    cout << "specified histogram doesn't exist" << endl;
    return;
  }
  for(unsigned int i = 1; i < vifpn.size(); i++)
  {
    TFile f2(vifpn[i].c_str());
    h->Add((TH1I*)f2.Get("ClusterSize/R3C4"));
  }
  h->SetTitle("");
  h->GetXaxis()->SetTitle("cluster size");
  h->GetYaxis()->SetTitle("counts");
  h->SetLineWidth(2);
  TCanvas *c1=new TCanvas("c1","c1",500,500);
  h->Draw();
  h->Draw("TEXT0 SAME");
  gPad->SetLogy(1);
  
  c1->SaveAs("cluster_size.eps");
}
