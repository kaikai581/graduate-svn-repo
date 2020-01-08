void DrawStripMultiplicity()
{
  TChain* chSM = new TChain("rpcTree");
  chSM->Add("../EnrichedTrees/2012/p12b/Neutrino/0101/*EH1*");
  chSM->Draw("mNStrips>>ho","mRpcFromRot==1&&mRpcRow==3&&mRpcColumn==4");
  chSM->Draw("mNStrips>>hc","mRpcFromRot==1&&mRpcRow==3&&mRpcColumn==4&&(mNSpuriousY+mNSpuriousX<=1)");
  TH1F* ho = (TH1F*)gDirectory->Get("ho");
  TH1F* hc = (TH1F*)gDirectory->Get("hc");
  hc->SetLineColor(kRed);
  hc->SetLineWidth(2);
  hc->SetTitle("strip multiplicity with number of spurious tracks cut");
  hc->GetXaxis()->SetTitle("module strip multiplicity");
  hc->GetYaxis()->SetTitle("counts");
  TCanvas *c1=new TCanvas("c1","c1",500,500);
  hc->Draw();
  ho->SetLineWidth(2);
  ho->SetTitle("strip multiplicity with number of spurious tracks cut");
  ho->Draw("same");
  gPad->SetLogy(1);
  c1->SaveAs("strip_multi_spurious_cut.eps");
}
