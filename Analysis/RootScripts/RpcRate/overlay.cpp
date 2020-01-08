void overlay()
{
  TFile *frate = new TFile("rate.EH1.root");
  TGraphErrors *g3h = (TGraphErrors*)frate->Get("g3hitrateEH1R3C3");
  TGraphErrors *g4h = (TGraphErrors*)frate->Get("g4hitrateEH1R3C3");
  TCanvas *c1 = new TCanvas("c1");
  g3h->Draw("APL");
  c1->Update();
  float M3h, m3h, M4h, m4h;
  M3h = gPad->GetUymax();
  m3h = gPad->GetUymin();
  g4h->Draw("APL");
  c1->Update();
  M4h = gPad->GetUymax();
  m4h = gPad->GetUymin();
  g3h->GetXaxis()->SetTitle("3 hit rate");
  g3h->Draw("APL");
  
  double *y4h, *ey4h, *x, *ex;
  x = g4h->GetX();
  ex = g4h->GetEX();
  y4h = g4h->GetY();
  ey4h = g4h->GetEY();
  const int N = g4h->GetN();
  for(int i = 0; i < N; i++)
  {
    y4h[i] = (y4h[i]-m4h)/(M4h-m4h)*(M3h-m3h)+m3h;
    ey4h[i] = ey4h[i]/(M4h-m4h)*(M3h-m3h);
  }
  TGraphErrors *g4hscaled = new TGraphErrors(N, x, y4h, ex, ey4h);
  g4hscaled->SetLineColor(kCyan);
  g4hscaled->SetMarkerColor(kCyan);
  g4hscaled->SetLineWidth(2);
  g4hscaled->SetMarkerStyle(21);
  g4hscaled->Draw("PL");
  c1->Update();
  
  TLegend *l = new TLegend(.1,.1,.3,.2);
  l->AddEntry(g3h, "3 hit rate", "PL");
  l->AddEntry(g4hscaled, "4 hit rate", "PL");
  l->SetBorderSize(0);
  l->SetFillColor(kWhite);
  l->Draw();
  c1->Update();
  
  gPad->SetTicky(0);
  TGaxis *axis = new TGaxis(gPad->GetUxmax(),gPad->GetUymin(),
    gPad->GetUxmax(),gPad->GetUymax(),m4h,M4h,510,"+L");
  axis->SetLineColor(kCyan);
  axis->SetTitle("4 hit rate (Hz)");
  axis->Draw();
  c1->SaveAs("3vs4hRate.pdf");
}
