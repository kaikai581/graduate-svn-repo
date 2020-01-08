void DrawXYNSpurious()
{
  TChain* chNSpu = new TChain("rpcTree");
  chNSpu->Add("../EnrichedTrees/2012/p12b/Neutrino/0101/*EH1*");
  chNSpu->Draw("mNSpuriousY:mNSpuriousX>>hSpu","mRpcFromRot==1&&mRpcRow==3&&mRpcColumn==4");
  TH2F* hSpu = (TH2F*)gDirectory->Get("hSpu");
  hSpu->SetTitle("number of spurious y tracks vs. number of spurious x tracks");
  hSpu->GetXaxis()->SetTitle("number of spurious x tracks");
  hSpu->GetYaxis()->SetTitle("number of spurious y tracks");
  TCanvas *c1=new TCanvas("c1","c1",500,500);
  hSpu->Draw("coltext");
  gPad->SetLogz(1);
  c1->SaveAs("xy_nspurious.eps");
}
