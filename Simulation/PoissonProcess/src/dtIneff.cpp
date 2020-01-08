/*
 * This program simulates a Poisson process with some detection inefficiency.
*/

#include <cmath>
#include <iostream>
#include "tclap/CmdLine.h"
#include "TFile.h"
#include "TH1F.h"
#include "TRandom3.h"

using namespace std;

int main(int argc, char** argv)
{
  /// parsing command line arguments
  TCLAP::CmdLine cmd("Command description message", ' ', "0.9");
  TCLAP::ValueArg<int> nEvtArg("n", "number",
  "Number of events", false, 1000000, "integer");
//  TCLAP::ValueArg<double> timeIntervalArg("t", "time",
//  "Time interval", false, 1000000., "double");
  TCLAP::ValueArg<double> lambdaArg("l", "lambda",
  "Average counts", false, 1., "double");
  TCLAP::ValueArg<double> effArg("e", "efficiency",
  "Detection efficiency", false, .7, "double");
  TCLAP::ValueArg<string> ofpathnameArg("o", "output",
    "Output file pathname", false, "results.root", "string");
  cmd.add(nEvtArg);
//  cmd.add(timeIntervalArg);
  cmd.add(lambdaArg);
  cmd.add(effArg);
  cmd.add(ofpathnameArg);
  cmd.parse(argc, argv);

  int nEvt = nEvtArg.getValue();
//  double timeInterval = timeIntervalArg.getValue();
  double lambda = lambdaArg.getValue();
  double eff = effArg.getValue();
  string ofpn = ofpathnameArg.getValue();
  
  
  /// also study some other efficiencies
  double eff2 = eff/2.;
  double eff4 = eff/4.;


  /// set x range in dt histogram without inefficiency
  double xmax = ceil(log((double)nEvt)/lambda);
  /// set x range in dt histogram with inefficiency
  double xmaxIneff = ceil(log((double)nEvt*eff*eff)/eff/lambda);
  double xmaxIneff2 = ceil(log((double)nEvt*eff2*eff2)/eff2/lambda);
  double xmaxIneff4 = ceil(log((double)nEvt*eff4*eff4)/eff4/lambda);
  /// set # of histogram bins according to xmax & xmaxIneff
  int nBin = xmax*10;
  int nBinIneff = xmaxIneff*10;
  int nBinIneff2 = xmaxIneff2*10;
  int nBinIneff4 = xmaxIneff4*10;

  /// Poisson random variable
  TRandom3 poiRV(0);
  /// efficiency random variable
  TRandom3 effRV(0);
  TRandom3 effRV2(0);
  TRandom3 effRV4(0);

  /// time series of event arrival time
  vector<double> timeSeries(1, 0.);
  vector<double> timeSeriesIneff;
  vector<double> timeSeriesIneff2;
  vector<double> timeSeriesIneff4;
  if(effRV.Uniform() < eff) timeSeriesIneff.push_back(0.);
  if(effRV2.Uniform() < eff2) timeSeriesIneff2.push_back(0.);
  if(effRV4.Uniform() < eff4) timeSeriesIneff4.push_back(0.);

  TH1F hDt("hDt", "interarrival time of events registered with a fully efficient detector", nBin, 0, xmax);
  for(int i = 0; i < nEvt; i++)
  {
    double curDt = poiRV.Exp(1./lambda);
    timeSeries.push_back(timeSeries[timeSeries.size()-1]+curDt);
    if(effRV.Uniform() < eff)
      timeSeriesIneff.push_back(timeSeries[timeSeries.size()-1]);
    if(effRV2.Uniform() < eff2)
      timeSeriesIneff2.push_back(timeSeries[timeSeries.size()-1]);
    if(effRV4.Uniform() < eff4)
      timeSeriesIneff4.push_back(timeSeries[timeSeries.size()-1]);
    hDt.Fill(curDt);
  }

  TH1F hDtIneff(Form("hDtEff%d",(int)(eff*100)), "interarrival time of events registered with an inefficient detector", nBinIneff, 0, xmaxIneff);
  for(unsigned int i = 1; i < timeSeriesIneff.size(); i++)
    hDtIneff.Fill(timeSeriesIneff[i]-timeSeriesIneff[i-1]);


  TH1F hDtIneff2(Form("hDtEff%d",(int)(eff2*100)), "interarrival time of events registered with an inefficient detector", nBinIneff2, 0, xmaxIneff2);
  for(unsigned int i = 1; i < timeSeriesIneff2.size(); i++)
    hDtIneff2.Fill(timeSeriesIneff2[i]-timeSeriesIneff2[i-1]);


  TH1F hDtIneff4(Form("hDtEff%d",(int)(eff4*100)), "interarrival time of events registered with an inefficient detector", nBinIneff4, 0, xmaxIneff4);
  for(unsigned int i = 1; i < timeSeriesIneff4.size(); i++)
    hDtIneff4.Fill(timeSeriesIneff4[i]-timeSeriesIneff4[i-1]);

  TFile tof(ofpn.c_str(), "recreate");
  
  hDt.SetLineColor(kBlue);
  hDtIneff.SetLineColor(kRed);
  hDtIneff2.SetLineColor(kGreen);
  hDtIneff4.SetLineColor(kMagenta);
  hDt.SetLineWidth(2);
  hDtIneff.SetLineWidth(2);
  hDtIneff2.SetLineWidth(2);
  hDtIneff4.SetLineWidth(2);
  hDt.SetTitle("");
  hDtIneff.SetTitle("");
  hDtIneff2.SetTitle("");
  hDtIneff4.SetTitle("");
  hDtIneff.SetMaximum(hDt.GetMaximum());
  hDtIneff2.SetMaximum(hDt.GetMaximum());
  hDtIneff4.SetMaximum(hDt.GetMaximum());
  
  hDt.GetXaxis()->SetTitle("#Deltat (s)");
  hDtIneff.GetXaxis()->SetTitle("#Deltat (s)");
  hDtIneff2.GetXaxis()->SetTitle("#Deltat (s)");
  hDtIneff4.GetXaxis()->SetTitle("#Deltat (s)");
  
  hDt.Write();
  hDtIneff.Write();
  hDtIneff2.Write();
  hDtIneff4.Write();
  tof.Close();

  return 0;
}

