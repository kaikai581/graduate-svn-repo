#include <algorithm>
#include <iostream>
#include <fstream>
#include "TAxis.h"
#include "TGraphErrors.h"
#include "TFile.h"
#include "TROOT.h"


using namespace std;


string filePath = "/home/sklin/Data/sklin/Data/DayaBayLocalData/RootFiles/RpcProductionTree";


void PlotDaily(string eh = "EH3", int row = 3, int col = 4, string leaf = "dtInSec4Fold")
{
  vector<string> vYear;
  vector<string> vDate;
  vector<double> vRate;
  vector<double> vError;
  unsigned int dateIdx = 0;
  
  ofstream ofn("daily_rate.txt");
  
  
  gROOT->ProcessLine(Form(".! ls %s>year", filePath.c_str()));
  ifstream fYear("year");
  vector<string> year;
  string curYear;
  while(fYear >> curYear)
    year.push_back(curYear);
  
  
  /// process p12b
  for(unsigned int i = 0; i < year.size(); i++)
  {
    string yearPath = filePath + "/" + year[i];
    gROOT->ProcessLine(Form(".! ls %s/p12b/Neutrino>date", yearPath.c_str()));
    ifstream fDate("date");
    vector<string> date;
    string curDate;
    while(fDate >> curDate)
    {
      date.push_back(curDate);
      vYear.push_back(year[i]);
      vDate.push_back(curDate);
      string ifpn = yearPath + "/p12b/Neutrino/" + curDate + "/" + "*" + eh + "*.root";
      gROOT->ProcessLine(Form(".x Interarrival.C+(\"%s\",%d,%d,\"%s\")",ifpn.c_str(),row,col,leaf.c_str()));
      ifstream ftemp("rate_error.txt");
      double rate, error;
      ftemp >> rate >> error;
      ftemp.close();
      vRate.push_back(rate);
      vError.push_back(error);
      ofn << year[i] << " " << curDate << " " << rate << " " << error << endl;
    }
  }
  
  /// process kup
  for(unsigned int i = 0; i < year.size(); i++)
  {
    string yearPath = filePath + "/" + year[i];
    gROOT->ProcessLine(Form(".! ls %s/kup/Neutrino>date", yearPath.c_str()));
    ifstream fDate("date");
    vector<string> date;
    string curDate;
    while(fDate >> curDate)
    {
      date.push_back(curDate);
      vector<string>::iterator itYear = std::find(vYear.begin(), vYear.end(), year[i]);
      vector<string>::iterator itDate = std::find(vDate.begin(), vDate.end(), curDate);
      if(itYear!=vYear.end()&&itDate!=vDate.end())
      {
        cout << "fuck you!" << endl;
        
      }
      else
      {
        vYear.push_back(year[i]);
        vDate.push_back(curDate);
        string ifpn = yearPath + "/kup/Neutrino/" + curDate + "/" + "*" + eh + "*.root";
        gROOT->ProcessLine(Form(".x Interarrival.C+(\"%s\",%d,%d,\"%s\")",ifpn.c_str(),row,col,leaf.c_str()));
        ifstream ftemp("rate_error.txt");
        double rate, error;
        ftemp >> rate >> error;
        ftemp.close();
        vRate.push_back(rate);
        vError.push_back(error);
        ofn << year[i] << " " << curDate << " " << rate << " " << error << endl;
      }
    }
    
    fDate.close();
    gROOT->ProcessLine(".! rm -f date");
  }
  
  ofn.close();
  
  const int N = vDate.size();
  double x[N], y[N], ex[N], ey[N];
  for(int i = 0; i < N; i++)
  {
    int thisYear = atoi(vYear[i].c_str());
    int thisDate = atoi(vDate[i].c_str());
    TDatime thisYearDate(thisYear, thisDate/100., thisDate%100, 0, 0, 0);
    x[i] = thisYearDate.Convert();
    ex[i] = 0.;
    y[i] = vRate[i];
    ey[i] = vError[i];
  }
  
  
  TFile tof("InterarrivalRate.root","update");
  const char* gName = Form("Rate%sR%dC%d%s", eh.c_str(), row, col, leaf.c_str());
  TObject *obj = (TObject*)tof.Get(gName);
  if(obj) tof.Delete(Form("%s;*", gName));
  
  TGraphErrors *g = new TGraphErrors(N, x, y, ex, ey);
  g->SetName(gName);
  g->SetTitle(Form("%s rate of %sR%dC%d", leaf.c_str(), eh.c_str(), row, col));
  g->GetXaxis()->SetTimeDisplay(1);
  g->GetXaxis()->SetTimeFormat("%m/%d/%Y");
  g->GetXaxis()->SetNdivisions(-505);
  g->GetXaxis()->SetTimeOffset(0,"gmt");
  g->GetXaxis()->SetTitle("date (mm/dd/yyyy)");
  g->GetYaxis()->SetTitle("rate (Hz)");
  g->SetLineWidth(2);
  g->Draw("APL");
  
  g->Write();
  tof.Close();
    
  fYear.close();
  gROOT->ProcessLine(".! rm -f year");
}
