void overlay()
{
  TFile *ftemp = new TFile("ENV.root");
  TGraphErrors *gT3 = (TGraphErrors*)ftemp->Get("DBNS_PTH_T3");
  TFile *frate = new TFile("rate.EH1.root");
  TGraphErrors *g3h = (TGraphErrors*)frate->Get("g3hitrateEH1R3C3");
  TCanvas *c1 = new TCanvas("c1");
  g3h->Draw("APL");
  c1->Update();
  float M3h, m3h, MT3, mT3;
  M3h = gPad->GetUymax();
  m3h = gPad->GetUymin();
  gT3->Draw("APL");
  c1->Update();
  MT3 = gPad->GetUymax();
  mT3 = gPad->GetUymin();
  
  
  double *y3h, *ey3h, *x, *ex;
  x = g3h->GetX();
  ex = g3h->GetEX();
  y3h = g3h->GetY();
  ey3h = g3h->GetEY();
  const int N = g3h->GetN();
  for(int i = 0; i < N; i++)
  {
    y3h[i] = (y3h[i]-m3h)/(M3h-m3h)*(MT3-mT3)+mT3;
    ey3h[i] = ey3h[i]/(M3h-m3h)*(MT3-mT3);
  }
  TGraphErrors *g3hscaled = new TGraphErrors(N, x, y3h, ex, ey3h);
  g3hscaled->SetLineColor(kBlue);
  g3hscaled->SetMarkerColor(kBlue);
  g3hscaled->SetLineWidth(2);
  g3hscaled->SetMarkerStyle(21);
  g3hscaled->Draw("PL");
  c1->Update();
  
  TLegend *l = new TLegend(.1,.1,.3,.2);
  l->AddEntry(gT3, "temperature (^{#circ}C)", "PL");
  l->AddEntry(g3hscaled, "3 hit rate (Hz)", "PL");
  l->SetBorderSize(0);
  l->SetFillColor(kWhite);
  l->Draw();
  c1->Update();
  
  gPad->SetTicky(0);
  TGaxis *axis = new TGaxis(gPad->GetUxmax(),gPad->GetUymin(),
    gPad->GetUxmax(),gPad->GetUymax(),m3h,M3h,510,"+L");
  axis->SetLineColor(kBlue);
  axis->SetTitle("3 hit rate (Hz)");
  axis->Draw();
  c1->SaveAs("Temp3hRate.pdf");
}
