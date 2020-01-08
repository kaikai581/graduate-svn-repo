#include <iostream>
#include "TCanvas.h"
#include "TFile.h"
#include "TGraphErrors.h"
#include "TLegend.h"
#include "TLegendEntry.h"
#include "TROOT.h"
#include "TGaxis.h"
#include "TStyle.h"

void overlay()
{
  TFile feff("EffPlot.root");
  TGraphErrors *geff = (TGraphErrors*)feff.Get("Graph;1");
  TLegend *leff = (TLegend*)feff.Get("TPave;1");
  TCanvas *c1 = new TCanvas("c1");
  geff->SetMarkerColor(kBlack);
  geff->SetLineColor(kBlack);
  geff->Draw("APL");
  c1->Update();

  const char *label = geff->GetTitle();
  leff->Clear();
  
  double *y = geff->GetY();
  double *ey = geff->GetEY();
  const int npoints = geff->GetN();
  double ymax = 0., ymin = 1.;
  for(int i = 0; i < npoints; i++)
  {
    if(y[i]+ey[i]>ymax)
      ymax = y[i]+ey[i];
    if(y[i]+ey[i]<ymin)
      ymin = y[i]+ey[i];
  }
  cout << ymax << " " << ymin << endl;
  ymax = gPad->GetUymax();
  ymin = gPad->GetUymin();
  
  gROOT->ProcessLine(".L DrawRPCHV.cpp+");
  gROOT->ProcessLine(Form("DrawRPCHVZoom(\"EH1RPCHVLoose.txt\",%f,%f)",ymax,ymin));
  
  TFile fvol("RPCVol.root");
  TGraph *gvol[8];
  TLegend *lvol = (TLegend*)fvol.Get("TPave;1");
  lvol->AddEntry(geff, label, "LP");
  lvol->Draw();

  for(int i = 0; i < 8; i++)
  {
    gvol[i] = (TGraph*)fvol.Get(Form("Graph;%d", i+1));
    gvol[i]->Draw("PL");
  }
  geff->SetLineWidth(2);
  geff->Draw("PL");
  
  gPad->SetTicky(0);
  TGaxis *axis = new TGaxis(gPad->GetUxmax(),gPad->GetUymin(),
  gPad->GetUxmax(),gPad->GetUymax(),3699.,3703.,510,"+L");

  gStyle->SetOptTitle(0);
  
  axis->SetLineColor(kRed);
  axis->SetTitle("voltage (V)");
  axis->Draw();

}
