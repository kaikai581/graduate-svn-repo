#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include "TAxis.h"
#include "TCanvas.h"
#include "TDatime.h"
#include "TFile.h"
#include "TGraphErrors.h"
#include "TLegend.h"
#include "TROOT.h"

using namespace std;

void getData(ifstream& fp, vector<int>& vdate, vector<double>& ve,
             vector<double>& vee, vector<double>& vr, vector<double>& ver)
{
  int fIdx = 0;
  string tmpstr;
  while(fp >> tmpstr)
  {
    switch (fIdx%13)
    {
      case 0:
        vdate.push_back(atoi(tmpstr.c_str()));
        break;
      case 1:
        ve.push_back(atof(tmpstr.c_str()));
        break;
      case 2:
        vee.push_back(atof(tmpstr.c_str()));
        break;
      case 3:
        vr.push_back(atof(tmpstr.c_str()));
        break;
      case 4:
        ver.push_back(atof(tmpstr.c_str()));
        break;
    }
    
    fIdx++;
  }
}


void write2ROOT(string gName, vector<int>& vdate, vector<double>& vy,
                vector<double>& vey)
{
  TFile fof("effrate.root", "update");
  TObject *obj = (TObject*)fof.Get(gName.c_str());
  if(obj) fof.Delete(Form("%s;*", gName.c_str()));
  
  const unsigned int ndays = vdate.size();
  double x[ndays], ex[ndays], y[ndays], ey[ndays];
  
  for(unsigned int i = 0; i < ndays; i++)
  {
    int year = vdate[i]/10000;
    int month = (vdate[i]%10000)/100;
    int date = (vdate[i]%10000)%100;
    TDatime curDate(year, month, date, 0, 0, 0);
    x[i] = curDate.Convert();
    ex[i] = 0.;
    y[i] = vy[i];
    ey[i] = vey[i];
  }
  
  TGraphErrors ge(ndays, x, y, ex, ey);
  ge.SetName(gName.c_str());
  ge.SetTitle(gName.c_str());
  ge.GetXaxis()->SetTimeDisplay(1);
  ge.GetXaxis()->SetTimeFormat("%m/%d/%Y");
  ge.GetXaxis()->SetNdivisions(505);
  ge.GetXaxis()->SetTitle("date (mm/dd/yyyy)");
  ge.SetLineColor(kBlue);
  ge.SetLineWidth(kBlue);
  ge.SetMarkerColor(kBlue);
  ge.SetMarkerStyle(7);
  ge.Write();
  
  fof.Close();
  
}

void ploteffvstime(double eM=0., double em=0., double rM=0., double rm=0.,
                   int hall=1, int row=3, int col=4)
{
  ifstream ffit(Form("../effByFittingEH%dR%dC%d.txt", hall, row, col));
  ifstream fquo(Form("../effByQuotientEH%dR%dC%d.txt", hall, row, col));
  
  vector<int> xfit, xquo;
  vector<double> efit, equo;
  vector<double> eefit, eequo;
  vector<double> rfit, rquo;
  vector<double> erfit, erquo;
  getData(ffit, xfit, efit, eefit, rfit, erfit);
  getData(fquo, xquo, equo, eequo, rquo, erquo);

  string gNameEffFit(Form("effFittingEH%dR%dC%d", hall, row, col));
  string gNameEffQuo(Form("effQuotientEH%dR%dC%d", hall, row, col));
  string gNameRateFit(Form("rateFittingEH%dR%dC%d", hall, row, col));
  string gNameRateQuo(Form("rateQuotientEH%dR%dC%d", hall, row, col));
  write2ROOT(gNameEffFit, xfit, efit, eefit);
  write2ROOT(gNameEffQuo, xquo, equo, eequo);
  write2ROOT(gNameRateFit, xfit, rfit, erfit);
  write2ROOT(gNameRateQuo, xquo, rquo, erquo);
  
  
  TFile *fif = new TFile("effrate.root");
  TGraphErrors* gEffFit = (TGraphErrors*)fif->Get(gNameEffFit.c_str());
  TGraphErrors* gEffQuo = (TGraphErrors*)fif->Get(gNameEffQuo.c_str());
  TGraphErrors* gRateFit = (TGraphErrors*)fif->Get(gNameRateFit.c_str());
  TGraphErrors* gRateQuo = (TGraphErrors*)fif->Get(gNameRateQuo.c_str());
  
  if(eM!=0.) gEffQuo->SetMaximum(eM);
  if(em!=0.) gEffQuo->SetMinimum(em);
  if(rM!=0.) gRateQuo->SetMaximum(rM);
  if(rm!=0.) gRateQuo->SetMinimum(rm);
  
  TCanvas *c1 = new TCanvas("c1", "c1", 900, 600);
  c1->Divide(1,2);
  c1->cd(1)->SetGridy(1);
  c1->cd(1);
  gEffQuo->GetYaxis()->SetTitle("efficiency");
  gEffQuo->SetTitle("module efficiency");
  //gEffQuo->SetLineWidth(2);
  gEffQuo->Draw("APZ");
  gEffFit->SetLineColor(kRed);
  gEffFit->SetMarkerColor(kRed);
  //gEffFit->SetLineWidth(2);
  gEffFit->Draw("PZ");
  TLegend *l1 = new TLegend(.15,.75,.25,.85);
  l1->AddEntry(gEffQuo, "quotient", "PL");
  l1->AddEntry(gEffFit, "fitting", "PL");
  l1->SetBorderSize(0);
  l1->SetFillColor(kWhite);
  l1->Draw();
  c1->cd(2)->SetGridy(1);
  c1->cd(2);
  gRateQuo->GetYaxis()->SetTitle("rate (Hz)");
  gRateQuo->SetTitle("muon rate");
  //gRateQuo->SetLineWidth(2);
  gRateQuo->Draw("APZ");
  gRateFit->SetLineColor(kRed);
  gRateFit->SetMarkerColor(kRed);
  //gRateFit->SetLineWidth(2);
  gRateFit->Draw("PZ");
  TLegend *l2 = new TLegend(.15,.75,.25,.85);
  l2->AddEntry(gRateQuo, "quotient", "PL");
  l2->AddEntry(gRateFit, "fitting", "PL");
  l2->SetBorderSize(0);
  l2->SetFillColor(kWhite);
  l2->Draw();
}
