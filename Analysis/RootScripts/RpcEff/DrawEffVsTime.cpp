#include <iostream>
#include <sstream>
#include <sys/stat.h>
#include <vector>
#include "TAxis.h"
#include "TCanvas.h"
#include "TDatime.h"
#include "TFile.h"
#include "TGraphErrors.h"
#include "TH2F.h"
#include "TLegend.h"

using namespace std;

bool fexist(const char*);
void getSingleEff(TFile*, TH2F*, float[], float[], int, int);
void setGraphStyle(TGraphErrors*, int);

/// global variables
int dateIn2012[] = {
  101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115,
  116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130,
  131, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214,
  215, 216, 217
};

void DrawEffVsTime(int hn = 1, int row = -1, int col = -1, int avg = 0)
{
  stringstream infn;
  infn << "eff.EH" << hn << ".root";
  cout << "opening file: " << endl;
  cout << infn.str() << endl;
  const int ndays = sizeof(dateIn2012)/sizeof(int);
  float x[ndays], y[ndays] = {0.};
  float xerr[ndays] = {0.}, yerr[ndays] = {0.};
  for(int i = 0; i < ndays; i++)
  {
    TDatime curDate(2012, dateIn2012[i]/100, dateIn2012[i]%100, 0, 0, 0);
    x[i] = curDate.Convert();
  }
    
  if(!fexist(infn.str().c_str()))
  {
    cout << "file does not exist" << endl;
    return;
  }
  TFile of("EffPlot.root", "recreate");
  TFile *inf = new TFile(infn.str().c_str());
  
  int nrows = 0, ncols = 9;
  if(hn == 1 || hn == 2)
    nrows = 8;
  else if(hn == 3)
    nrows = 11;
  
  TGraphErrors **ge;
  TH2F *heff = 0;
  float minEff = 1., minErr = 1.;
  float maxEff = 0., maxErr = 0.;
  TLegend *l = new TLegend(.91,.5,.97,.95);
  l->SetFillColor(kWhite);
  l->SetBorderSize(0);
  if(row != -1 && col != -1)
  {
    getSingleEff(inf, heff, y, yerr, row, col);
    ge = new TGraphErrors* [1];
    ge[0] = new TGraphErrors(ndays, x, y, xerr, yerr);
    setGraphStyle(ge[0], 7);
    const char* gName = Form("EH%dR%dC%d", hn, row, col);
    ge[0]->SetTitle(gName);
    ge[0]->Draw("AP");
    l->AddEntry(ge[0], gName, "LP");
    l->Draw();
    of.cd();
    ge[0]->Write();
    inf->cd();
  }
  else
  {
    /// No average. Plot side by side.
    if(!avg)
    {
      if(row == -1&&col != -1)
      {
        ge = new TGraphErrors* [nrows];
        for(int rIdx = 0; rIdx < nrows; rIdx++)
        {
          getSingleEff(inf, heff, y, yerr, rIdx, col);
          for(int dIdx = 0; dIdx < ndays; dIdx++)
          {
            if(y[dIdx] > 0. && y[dIdx] < minEff)
            {
              minEff = y[dIdx];
              minErr = yerr[dIdx];
            }
            if(y[dIdx] > maxEff)
            {
              maxEff = y[dIdx];
              maxErr = yerr[dIdx];
            }
          }
          ge[rIdx] = new TGraphErrors(ndays, x, y, xerr, yerr);
          setGraphStyle(ge[rIdx], rIdx);
          if(y[0] > 0.)
            l->AddEntry(ge[rIdx], Form("R%dC%d", rIdx, col), "LP");
        }
        for(int rIdx = 0; rIdx < nrows; rIdx++)
        {
          if(!rIdx)
          {
            ge[rIdx]->SetMaximum(maxEff+maxErr);
            ge[rIdx]->SetMinimum(minEff-minErr);
            ge[rIdx]->Draw("APL");
          }
          else
            ge[rIdx]->Draw("PL");
          of.cd();
          ge[rIdx]->Write();
          inf->cd();
        }
        l->Draw();
      }
      else if(row != -1&&col == -1)
      {
        ge = new TGraphErrors* [ncols];
        for(int cIdx = 0; cIdx < ncols; cIdx++)
        {
          getSingleEff(inf, heff, y, yerr, row, cIdx);
          for(int dIdx = 0; dIdx < ndays; dIdx++)
          {
            if(y[dIdx] > 0. && y[dIdx] < minEff)
            {
              minEff = y[dIdx];
              minErr = yerr[dIdx];
            }
            if(y[dIdx] > maxEff)
            {
              maxEff = y[dIdx];
              maxErr = yerr[dIdx];
            }
          }
          ge[cIdx] = new TGraphErrors(ndays, x, y, xerr, yerr);
          setGraphStyle(ge[cIdx], cIdx);
          if(y[0] > 0.)
            l->AddEntry(ge[cIdx], Form("R%dC%d", row, cIdx), "LP");
        }
        for(int cIdx = 0; cIdx < ncols; cIdx++)
        {
          if(!cIdx)
          {
            ge[cIdx]->SetMaximum(maxEff+maxErr);
            ge[cIdx]->SetMinimum(minEff-minErr);
            ge[cIdx]->Draw("APL");
          }
          else
            ge[cIdx]->Draw("PL");
          of.cd();
          ge[cIdx]->Write();
          inf->cd();
        }
        l->Draw();
      }
      else if(row == -1 && col == -1)
      {
        ge = new TGraphErrors* [nrows*ncols];
        for(int rIdx = 0; rIdx < nrows; rIdx++)
          for(int cIdx = 0; cIdx < ncols; cIdx++)
          {
            getSingleEff(inf, heff, y, yerr, rIdx, cIdx);
            for(int dIdx = 0; dIdx < ndays; dIdx++)
            {
              if(y[dIdx] > 0. && y[dIdx] < minEff)
              {
                minEff = y[dIdx];
                minErr = yerr[dIdx];
              }
              if(y[dIdx] > maxEff)
              {
                maxEff = y[dIdx];
                maxErr = yerr[dIdx];
              }
            }
            ge[rIdx*ncols+cIdx] = new TGraphErrors(ndays, x, y, xerr, yerr);
            setGraphStyle(ge[rIdx*ncols+cIdx], rIdx*ncols+cIdx);
            if(y[0] > 0.)
              l->AddEntry(ge[rIdx*ncols+cIdx],
                          Form("R%dC%d", rIdx, cIdx), "LP");
          }
        for(int rIdx = 0; rIdx < nrows; rIdx++)
          for(int cIdx = 0; cIdx < ncols; cIdx++)
          {
            if(!(rIdx*ncols+cIdx))
            {
              ge[rIdx*ncols+cIdx]->SetMaximum(maxEff+maxErr);
              ge[rIdx*ncols+cIdx]->SetMinimum(minEff-minErr);
              ge[rIdx*ncols+cIdx]->Draw("APL");
            }
            else
              ge[rIdx*ncols+cIdx]->Draw("PL");
            of.cd();
            ge[rIdx*ncols+cIdx]->Write();
            inf->cd();
          }
        l->Draw();
      }
    }
  }
  of.cd();
  l->Write();
  inf->cd();
  of.Close();
  return;
}


bool fexist(const char *filename)
{
  struct stat buffer;
  /// From stat.h manual:
  /// Upon successful completion, 0 shall be returned.
  /// Otherwise, -1 shall be returned and errno set to indicate the error.
  if (stat(filename, &buffer) == 0) return true;
  return false;
}


void getSingleEff(TFile *inf, TH2F* heff, float y[], float yerr[],
                  int row, int col)
{
  const int ndays = sizeof(dateIn2012)/sizeof(int);
  for(int i = 0; i < ndays; i++)
  {
    heff = (TH2F*)inf->Get(Form("eff20120%d", dateIn2012[i]));
    y[i] = heff->GetBinContent(col+1, row+1);
    yerr[i] = heff->GetBinError(col+1, row+1);
  }
}


void setGraphStyle(TGraphErrors* g, int idx)
{
  g->GetXaxis()->SetTimeDisplay(1);
  g->GetXaxis()->SetTimeFormat("%m/%d/%Y");
  g->GetXaxis()->SetNdivisions(-505);
  g->GetXaxis()->SetTimeOffset(0,"gmt");
  g->GetXaxis()->SetTitle("date (mm/dd/yyyy)");
  g->GetYaxis()->SetTitle("efficiency");
  g->SetMarkerStyle(idx%9+1);
  g->SetMarkerColor(idx%9+1);
  g->SetLineColor(idx%9+1);
  g->SetTitle("");
}
