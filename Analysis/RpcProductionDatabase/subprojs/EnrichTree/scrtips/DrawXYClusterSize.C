void DrawXYClusterSize()
{
  TChain* chTracks = new TChain("rpcTree");
  //chTracks->Add("../EnrichedTrees/2012/p12b/Neutrino/0101/enrichedrpctree.Neutrino.0021358.Physics.EH1-Merged.P12B-P._0002.root");
  chTracks->Add("../EnrichedTrees/2012/p12b/Neutrino/0101/*EH1*");
  TCanvas *c1=new TCanvas("c1","c1",500,500);
  gPad->SetLogz(1);
  chTracks->Draw("mMaxClusterSizeY:mMaxClusterSizeX>>h1","mRpcRow==3&&mRpcColumn==4&&mRpcFromRot==1");
  TH2F* h1=(TH2F*)gDirectory->Get("h1");
  h1->SetTitle("");
  h1->GetXaxis()->SetTitle("maximum x cluster size in an event");
  h1->GetYaxis()->SetTitle("maximum y cluster size in an event");
  gStyle->SetOptStat(11);
  gStyle->SetStatY(.95);                
  // Set y-position (fraction of pad size)
  gStyle->SetStatX(.9);                
  // Set x-position (fraction of pad size)
  gStyle->SetStatW(0.2);                
  // Set width of stat-box (fraction of pad size)
  gStyle->SetStatH(0.1);                
  // Set height of stat-box (fraction of pad size)
  h1->Draw("coltext");
  
  TLine *xleft, *xright;
  TLine *ybot, *ytop;
  xleft = new TLine(1,1,1,3);
  xleft->SetLineWidth(2);
  xleft->SetLineColor(kOrange);
  xleft->Draw();
  xright = new TLine(3,3,1,3);
  xright->SetLineWidth(2);
  xright->SetLineColor(kOrange);
  xright->Draw();
  ybot = new TLine(1,1,3,1);
  ybot->SetLineWidth(2);
  ybot->SetLineColor(kOrange);
  ybot->Draw();
  ytop = new TLine(3,3,3,1);
  ytop->SetLineWidth(2);
  ytop->SetLineColor(kOrange);
  ytop->Draw();
  
  c1->SaveAs("XYClusterSize.eps");
}
