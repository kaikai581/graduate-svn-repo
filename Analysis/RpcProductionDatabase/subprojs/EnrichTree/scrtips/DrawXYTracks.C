void DrawXYTracks()
{
  TChain* chTracks = new TChain("rpcTree");
  //chTracks->Add("../EnrichedTrees/2012/p12b/Neutrino/0101/enrichedrpctree.Neutrino.0021358.Physics.EH1-Merged.P12B-P._0002.root");
  chTracks->Add("../EnrichedTrees/2012/p12b/Neutrino/0101/*EH1*");
  TCanvas *c1=new TCanvas("c1","c1",1000,500);
  c1->Divide(2,1);
  c1->cd(1);
  gPad->SetLogz(1);
  chTracks->Draw("mNTracksY:mNTracksX>>h1","mRpcRow==3&&mRpcColumn==4&&mNLayers==4","coltext");
  TH2F* h1=(TH2F*)gDirectory->Get("h1");
  h1->SetTitle("4-fold");
  h1->GetXaxis()->SetTitle("number of x tracks");
  h1->GetYaxis()->SetTitle("number of y tracks");
  h1->Draw("coltext");
  
  c1->cd(2);
  gPad->SetLogz(1);
  chTracks->Draw("mNTracksY:mNTracksX>>h2","mRpcRow==3&&mRpcColumn==4&&mNLayers==3","coltext");
  TH2F* h2=(TH2F*)gDirectory->Get("h2");
  h2->SetTitle("3-fold");
  h2->GetXaxis()->SetTitle("number of x tracks");
  h2->GetYaxis()->SetTitle("number of y tracks");
  h2->Draw("coltext");
}
