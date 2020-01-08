void getMaxMin(float& max, float& min, TGraphErrors* g)
{
  int N = g->GetN();
  float tempmax = 0., tempmin = 1.;
  double *y = g->GetY(), *ey = g->GetEY();
  for(int i = 0; i < N; i++)
  {
    if(y[i]+ey[i]>tempmax)
      tempmax = y[i]+ey[i];
    if(y[i]+ey[i]<tempmin)
      tempmin = y[i]+ey[i];
  }
  max = tempmax;
  min = tempmin;
}





void overlay()
{
  TFile *feff = new TFile("EffPlot.root");
  TGraphErrors *geff = (TGraphErrors*)feff->Get("Graph;1");
  TFile *frate = new TFile("rate.EH1.root");
  TGraphErrors *g3h = (TGraphErrors*)frate->Get("g3hitrateEH1R3C3;1");
  TGraphErrors *g4h = (TGraphErrors*)frate->Get("g4hitrateEH1R3C3;1");
  TCanvas *c1 = new TCanvas("c1");
  geff->SetLineWidth(2);
  geff->Draw("APL");
  c1->Update();
  float pMax = gPad->GetUymax(), pMin = gPad->GetUymin();
  
  TCanvas *c2 = new TCanvas("c2");
  float M1, M2, m1, m2, M, m;
  g3h->Draw("APL");
  c2->Update();
  M1 = gPad->GetUymax();
  m1 = gPad->GetUymin();
  g4h->Draw("APL");
  c2->Update();
  M2 = gPad->GetUymax();
  m2 = gPad->GetUymin();
  if(M1>M2)
    M = M1;
  else
    M = M2;
  if(m1<m2)
    m = m1;
  else
    m = m2;
  cout << M << " " << m << endl;
  double *y3h, *y4h, *ey3h, *ey4h, *x, *ex;
  x = g3h->GetX();
  ex = g3h->GetEX();
  y3h = g3h->GetY();
  ey3h = g3h->GetEY();
  y4h = g4h->GetY();
  ey4h = g4h->GetEY();
  const int N = g3h->GetN();
  for(int i = 0; i < N; i++)
  {
    y3h[i] = (y3h[i]-m)/(M-m)*(pMax-pMin)+pMin;
    y4h[i] = (y4h[i]-m)/(M-m)*(pMax-pMin)+pMin;
    ey3h[i] = ey3h[i]/(M-m)*(pMax-pMin);
    ey4h[i] = ey4h[i]/(M-m)*(pMax-pMin);
  }
  TGraphErrors *g3hscaled = new TGraphErrors(N, x, y3h, ex, ey3h);
  TGraphErrors *g4hscaled = new TGraphErrors(N, x, y4h, ex, ey4h);
  c1->cd();
  g3hscaled->Draw("PL");
  g4hscaled->Draw("PL");
}

void overlay2(int nhit = 3)
{
  TFile *feff = new TFile("EffPlot.root");
  TGraphErrors *geff = (TGraphErrors*)feff->Get("Graph;1");
  TFile *frate = new TFile("rate.EH1.root");
  const char* gName = Form("g%dhitrateEH1R3C3;1", nhit);
  TGraphErrors *g3h = (TGraphErrors*)frate->Get(gName);
  TCanvas *c1 = new TCanvas("c1");
  geff->Draw("APL");
  c1->Update();
  float pMax = gPad->GetUymax(), pMin = gPad->GetUymin();
  
  TCanvas *c2 = new TCanvas("c2");
  float M, m;
  g3h->Draw("APL");
  c2->Update();
  M = gPad->GetUymax();
  m = gPad->GetUymin();
  double *y3h, *ey3h, *x, *ex;
  x = g3h->GetX();
  ex = g3h->GetEX();
  y3h = g3h->GetY();
  ey3h = g3h->GetEY();
  const int N = g3h->GetN();
  for(int i = 0; i < N; i++)
  {
    y3h[i] = (y3h[i]-m)/(M-m)*(pMax-pMin)+pMin;
    ey3h[i] = ey3h[i]/(M-m)*(pMax-pMin);
  }
  TGraphErrors *g3hscaled = new TGraphErrors(N, x, y3h, ex, ey3h);
  c1->cd();
  g3hscaled->SetLineColor(kMagenta);
  g3hscaled->SetMarkerColor(kMagenta);
  g3hscaled->SetLineWidth(2);
  g3hscaled->SetMarkerStyle(21);
  g3hscaled->Draw("PL");
  
  TLegend *l = new TLegend(.1,.1,.3,.2);
  l->AddEntry(geff, "efficiency", "PL");
  l->AddEntry(g3hscaled, Form("%d hit rate (Hz)", nhit), "PL");
  l->SetBorderSize(0);
  l->SetFillColor(kWhite);
  l->Draw();
  
  gPad->SetTicky(0);
  TGaxis *axis = new TGaxis(gPad->GetUxmax(),gPad->GetUymin(),
    gPad->GetUxmax(),gPad->GetUymax(),m,M,510,"+L");
  axis->SetLineColor(kMagenta);
  axis->SetTitle(Form("%d hit rate (Hz)", nhit));
  axis->Draw();
  c1->SaveAs(Form("Eff%dhRate.pdf", nhit));
}
