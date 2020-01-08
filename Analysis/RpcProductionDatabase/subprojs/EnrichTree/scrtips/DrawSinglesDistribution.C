void DrawSinglesDistribution()
{
  TChain* chSingles = new TChain("rpcTree");
  chSingles->Add("../EnrichedTrees/2012/p12b/Neutrino/0101/*EH1*");
  chSingles->Draw("singleStripId32>>h1","mRpcRow==3&&mRpcColumn==4&&singleStripId32!=-1");
  TH1F* h1=(TH1F*)gDirectory->Get("h1");
  h1->SetTitle("singles distribution");
  h1->SetLineWidth(2);
  h1->GetXaxis()->SetTitle("8*(layer-1)+(strip-1)");
  h1->GetYaxis()->SetTitle("counts");
  TCanvas *c1=new TCanvas("c1","c1",800,500);
  h1->Draw();
  c1->Update();
  double framemax = gPad->GetUymax();
  TLine* l[4];
  for(int i = 0; i < 4; i++)
  {
    double x0 = 8*(i+1);
    l[i] = new TLine(x0,0,x0,framemax);
    l[i]->SetLineColor(kRed);
    l[i]->SetLineStyle(7);
    l[i]->Draw();
  }
}
