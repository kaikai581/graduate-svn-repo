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
#include "TPaveLabel.h"
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

void getRawRate(ifstream& fp, vector<int>& vdate, vector<double>& vr,
                vector<double>& ver)
{
  int fIdx = 0;
  string tmpstr;
  while(fp >> tmpstr)
  {
    switch(fIdx%3)
    {
      case 0:
        vdate.push_back(atoi(tmpstr.c_str()));
        break;
      case 1:
        vr.push_back(atof(tmpstr.c_str()));
        break;
      case 2:
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
  ge.SetFillStyle(0);
  ge.SetFillColor(kWhite);
  ge.Write();
  
  fof.Close();
  
}

void ploteffvstimecut(double eM=0., double em=0., double rM=0., double rm=0.,
                   int hall=1, int row=3, int col=4)
{
  ifstream fif[3];
  fif[0].open((Form("effByQuotientEH%dR%dC%d_no_cut.txt", hall, row, col)));
  fif[1].open((Form("effByQuotientEH%dR%dC%d_ntracks_cut.txt", hall, row, col)));
  fif[2].open((Form("effByQuotientEH%dR%dC%d_ntracks_cluster_size_cut.txt", hall, row, col)));

  ifstream fRate[3];
  fRate[0].open((Form("RawRateEH%dR%dC%d_no_cut.txt", hall, row, col)));
  fRate[1].open((Form("RawRateEH%dR%dC%d_ntracks_cut.txt", hall, row, col)));
  fRate[2].open((Form("RawRateEH%dR%dC%d_ntracks_cluster_size_cut.txt", hall, row, col)));
  
  vector<int> dates[3], datesRawRate[3];
  vector<double> eff[3];
  vector<double> eeff[3];
  vector<double> rate[3], rawRate[3];
  vector<double> erate[3], eRawRate[3];
//  getData(ffit, xfit, efit, eefit, rfit, erfit);
//  getData(fquo, xquo, equo, eequo, rquo, erquo);
  for(int i = 0; i < 3; i++)
  {
    getData(fif[i], dates[i], eff[i], eeff[i], rate[i], erate[i]);
    getRawRate(fRate[i], datesRawRate[i], rawRate[i], eRawRate[i]);
  }


  string strModId = Form("EH%dR%dC%d", hall, row, col);
  string gEffName[3];
  gEffName[0] = Form("eff_no_cut_%s", strModId.c_str());
  gEffName[1] = Form("eff_nt_cut_%s", strModId.c_str());
  gEffName[2] = Form("eff_nt_cs_cut_%s", strModId.c_str());
  string gRateName[3];
  gRateName[0] = Form("rate_no_cut_%s", strModId.c_str());
  gRateName[1] = Form("rate_nt_cut_%s", strModId.c_str());
  gRateName[2] = Form("rate_nt_cs_cut_%s", strModId.c_str());


  for(int i = 0; i < 3; i++)
    write2ROOT(gEffName[i], dates[i], eff[i], eeff[i]);
  for(int i = 0; i < 3; i++)
    write2ROOT(gRateName[i], dates[i], rate[i], erate[i]);
  
  
  TFile *tfif = new TFile("effrate.root");
//  TGraphErrors* gEffFit = (TGraphErrors*)fif->Get(gNameEffFit.c_str());
//  TGraphErrors* gEffQuo = (TGraphErrors*)fif->Get(gNameEffQuo.c_str());
//  TGraphErrors* gRateFit = (TGraphErrors*)fif->Get(gNameRateFit.c_str());
//  TGraphErrors* gRateQuo = (TGraphErrors*)fif->Get(gNameRateQuo.c_str());
  TGraphErrors* gEff[3];
  TGraphErrors* gRate[3];
  for(int i = 0; i < 3; i++)
  {
    gEff[i] = (TGraphErrors*)tfif->Get(gEffName[i].c_str());
    gRate[i] = (TGraphErrors*)tfif->Get(gRateName[i].c_str());
    if(eM!=0.) gEff[i]->SetMaximum(eM);
    if(em!=0.) gEff[i]->SetMinimum(em);
    if(rM!=0.) gRate[i]->SetMaximum(rM);
    if(rm!=0.) gRate[i]->SetMinimum(rm);
  }
  
  
  TCanvas *c1 = new TCanvas("c1", "c1", 900, 600);
  c1->Divide(1,2);
  c1->cd(1)->SetGridy(1);
  c1->cd(2)->SetGridy(1);

  string strEff[3], strRate[3];
  strEff[0] = "no cut";
  strEff[1] = "ntracks cut";
  strEff[2] = "ntracks + cluster size cut";
  TLegend *l1 = new TLegend(.15,.15,.35,.3);
  l1->SetBorderSize(0);
  l1->SetFillColor(kWhite);
  TLegend *l2 = new TLegend(.15,.74,.35,.89);
  l2->SetBorderSize(0);
  l2->SetFillColor(kWhite);
  for(int i = 0; i < 3; i++)
  {
    c1->cd(1);
    gEff[i]->GetYaxis()->SetTitle("efficiency");
    gEff[i]->SetTitle("module efficiency");
//    gEff[i]->SetLineWidth(2);
    string dOpt;
    if(i) dOpt = "PZ";
    else dOpt = "APZ";
    gEff[i]->Draw(dOpt.c_str());
    gEff[i]->SetLineColor(i+1);
    gEff[i]->SetMarkerColor(i+1);
  //gEffFit->SetLineWidth(2);
    l1->AddEntry(gEff[i], strEff[i].c_str(), "PL");

    c1->cd(2);
    gRate[i]->GetYaxis()->SetTitle("rate (Hz)");
    gRate[i]->SetTitle("muon rate");
//    gRateQuo->SetLineWidth(2);
    gRate[i]->Draw(dOpt.c_str());
    gRate[i]->SetLineColor(i+1);
    gRate[i]->SetMarkerColor(i+1);
//    gRateFit->SetLineWidth(2);
//  TLegend *l2 = new TLegend(.15,.75,.25,.85);
    l2->AddEntry(gRate[i], strEff[i].c_str(), "PL");
//  l2->AddEntry(gRateFit, "fitting", "PL");
//  l2->SetBorderSize(0);
//  l2->SetFillColor(kWhite);
//  l2->Draw();
  }

  TLine* increaseHVDate[3];
  TDatime incDate(2012, 3, 16, 0, 0, 0);
  double xIncDate = incDate.Convert();
  double ymin, ymax;
  c1->cd(1);
  ymin = gEff[0]->GetMinimum();
  ymax = gEff[0]->GetMaximum();
  increaseHVDate[0] = new TLine(xIncDate, ymin, xIncDate, ymax);
  increaseHVDate[0]->SetLineColor(kMagenta);
  increaseHVDate[0]->SetLineStyle(2);
  increaseHVDate[0]->Draw();
  TPaveLabel* HV;
  HV = new TPaveLabel(.5,.2,.9,.9,"7400V");
  HV->Draw();
  l1->Draw();
  c1->cd(2);
  l2->Draw();
}
