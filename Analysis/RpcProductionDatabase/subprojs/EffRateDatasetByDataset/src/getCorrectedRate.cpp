#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>
#include "TAxis.h"
#include "TFile.h"
#include "TGraphErrors.h"
#include "TROOT.h"
#include "utilities.h"


using namespace std;


void getCorrectedRate(const vector<string>& vIfn, string outputPath,
                      string hall, int row, int col)
{
  /// If both rate and efficiency graphs exist, calculate the corrected rate
  /// by dividing rate with efficiency.
  string strYear = getYear(vIfn[0]);
  string strDate = getDate(vIfn[0]);
  string strRate(Form("Rate_4Fold_EH%sR%dC%d_%s%s", hall.c_str(), row, col,
                      strYear.c_str(), strDate.c_str()));
  string strEff(Form("Eff_EH%sR%dC%d_%s%s", hall.c_str(), row, col,
                     strYear.c_str(), strDate.c_str()));

  /// check if both rate and efficiency graphs exist
  TFile fof(Form("%s/DsByDsInOneDay.root", outputPath.c_str()), "update");
  /// if any of the graphs doesn't exist, quit
  TObject *objRate = (TObject*)fof.Get(strRate.c_str());
  TObject *objEff = (TObject*)fof.Get(strEff.c_str());
  if(!(objRate && objEff))
    return;


  TGraphErrors* gRate = (TGraphErrors*)fof.Get(strRate.c_str());
  TGraphErrors* gEff = (TGraphErrors*)fof.Get(strEff.c_str());

  const int NRate = gRate->GetN();
  const int NEff = gEff->GetN();

  const int nPts = min(NRate, NEff);

  double x[nPts], ex[nPts], y[nPts], ey[nPts];
  double *xR, *exR, *xE, *exE;
  double *yR, *eyR, *yE, *eyE;

  xR = gRate->GetX();
  yR = gRate->GetY();
  xE = gEff->GetX();
  yE = gEff->GetY();
  exR = gRate->GetEX();
  eyR = gRate->GetEY();
  exE = gEff->GetEX();
  eyE = gEff->GetEY();

  for(int i = 0; i < nPts; i++)
  {
    if(NRate < NEff)
    {
      x[i] = xR[i];
      ex[i] = exR[i];
    }
    else
    {
      x[i] = xE[i];
      ex[i] = exE[i];
    }

    if(yE[i] != 0.)
      y[i] = yR[i]/yE[i];
    else
      y[i] = 0.;

    if(yR[i] == 0. || yE[i] == 0.)
      ey[i] = 0.;
    else
      ey[i] = y[i]*sqrt((eyR[i]/yR[i])*(eyR[i]/yR[i]) +
                        (eyE[i]/yE[i])*(eyE[i]/yE[i]));
  }


  string gName(Form("Corrected_Rate_4Fold_EH%sR%dC%d_%s%s", hall.c_str(),
                    row, col, strYear.c_str(), strDate.c_str()));
  TObject *obj = (TObject*)fof.Get(gName.c_str());
  if(obj) fof.Delete(Form("%s;*", gName.c_str()));

  TGraphErrors g(nPts, x, y, ex, ey);
  g.SetName(gName.c_str());
  g.SetTitle(gName.c_str());
  g.GetXaxis()->SetTimeDisplay(1);
  g.GetXaxis()->SetTimeOffset(0,"gmt");
  g.GetXaxis()->SetNdivisions(505);
  g.GetXaxis()->SetTimeFormat("%H:%M:%S");
  g.GetXaxis()->SetTitle("time (hh:mm:ss)");
  g.GetYaxis()->SetTitle("rate (Hz)");
  g.SetLineColor(kBlue);
  g.SetLineWidth(2);
  g.SetMarkerColor(kBlue);
  g.SetMarkerStyle(21);
  g.SetMarkerSize(.8);
  g.Write();
  fof.Close();
}
