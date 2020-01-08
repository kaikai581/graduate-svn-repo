void trackLength()
{
  
  TFile* f = new TFile("simtrack.root");
  
  TTree* t = (TTree*)f->Get("t");
  
  TH1F* fall = new TH1F("fall", "", 100,0,4500);
  
  t->Project("fall","trackLength");
  
  fall->GetXaxis()->SetTitle("track length (mm)");
  
  fall->SetLineWidth(2);
  
  fall->SetLineColor(1);
  
  fall->SetStats(0);
  
  fall->Draw();
  
  TH1F* ftb = new TH1F("ftb", "", 100,0,4500);
  
  t->Project("ftb","trackLength","trackType==1");
  
  ftb->SetLineWidth(2);
  
  ftb->SetLineColor(2);
  
  ftb->Draw("same");
  
  
  TH1F* fts = new TH1F("fts", "", 100,0,4500);
  
  t->Project("fts","trackLength","trackType==2");
  
  fts->SetLineWidth(2);
  
  fts->SetLineColor(3);
  
  fts->Draw("same");
  
  
  TH1F* fsb = new TH1F("fsb", "", 100,0,4500);
  
  t->Project("fsb","trackLength","trackType==3");
  
  fsb->SetLineWidth(2);
  
  fsb->SetLineColor(4);
  
  fsb->Draw("same");
  
  
  TH1F* fss = new TH1F("fss", "", 100,0,4500);
  
  t->Project("fss","trackLength","trackType==4");
  
  fss->SetLineWidth(2);
  
  fss->SetLineColor(5);
  
  fss->Draw("same");
  
  
  TLegend* l = new TLegend(.7,.7,.9,.9);
  
  l->AddEntry(fall, "all tracks", "L");
  
  l->AddEntry(ftb, "top-bottom", "L");
  
  l->AddEntry(fts, "top-side", "L");
  
  l->AddEntry(fsb, "side-bottom", "L");
  
  l->AddEntry(fss, "side-side", "L");
  
  l->SetFillColor(kWhite);
  
  l->Draw();
}
