#include <algorithm>
#include <fstream>
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
#include "TROOT.h"

using namespace std;

/// global variables
int dateIn2012[] = {
  101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115,
  116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130,
  131, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214,
  215, 216, 217
};

const string prepath =
  "/eliza7/dayabay/scratch/sklin/RootFile/RpcRate/2012/p12b/Neutrino";


bool fexists(const char*);
void accDataPerDay(string, vector<vector<float> >&,
                   vector<vector<float> >&, const char*);
void getDailyDataErr(vector<vector<float> >&, vector<vector<float> >&, int);
void printArray(vector<vector<float> >);
void writeResults(int, int, int, vector<float>, vector<float>, const char*,
                  int, int);


void DrawRateVsTime(int hn = 1, int row = 1, int col = 1)
{
  const int nDays = sizeof(dateIn2012)/sizeof(int);
  
  
  vector<float> rate4h;
  vector<float> rate4hErr;
  vector<float> rate3h;
  vector<float> rate3hErr;
  /// loop for days
  for(int dIdx = 0; dIdx < nDays; dIdx++)
  {
    stringstream fullpath;
    fullpath << prepath << "/" << 0 << dateIn2012[dIdx];
    gROOT->ProcessLine(Form(".! ls %s/*EH%d*.root>inflist",
                       fullpath.str().c_str(), hn));
    ifstream inflist("inflist");
    string fn;
    
    /// result container for one day
    vector<vector<float> > rate4hPerDay;
    vector<vector<float> > rate3hPerDay;
    vector<vector<float> > rate4hErrPerDay;
    vector<vector<float> > rate3hErrPerDay;
    int nFiles = 0;
    while(inflist >> fn)
    {
      nFiles++;
      accDataPerDay(fn, rate4hPerDay, rate4hErrPerDay, "h4hr");
      accDataPerDay(fn, rate3hPerDay, rate3hErrPerDay, "hge3hr");
    }
    getDailyDataErr(rate4hPerDay, rate4hErrPerDay, nFiles);
    getDailyDataErr(rate3hPerDay, rate3hErrPerDay, nFiles);
    
    /// write daily results into one array
    if(row >= 0 && row < (int)rate4hPerDay.size()
       && col >= 0 && col < (int)rate4hPerDay[0].size())
    {
      rate4h.push_back(rate4hPerDay[row][col]);
      rate4hErr.push_back(rate4hErrPerDay[row][col]);
      rate3h.push_back(rate3hPerDay[row][col]);
      rate3hErr.push_back(rate3hErrPerDay[row][col]);
    }
    else
      cout << "row or column out of range" << endl;
//    printArray(rate4hPerDay);
//    printArray(rate4hErrPerDay);
  }
  
  writeResults(hn, row, col, rate4h, rate4hErr, "4 hit rate", 860, 21);
  writeResults(hn, row, col, rate3h, rate3hErr, "3 hit rate", 880, 21);
  
  if(fexists("inflist"))
    gROOT->ProcessLine(".! rm -f inflist");
}


bool fexists(const char *filename)
{
  struct stat buffer;
  /// From stat.h manual:
  /// Upon successful completion, 0 shall be returned.
  /// Otherwise, -1 shall be returned and errno set to indicate the error.
  if (stat(filename, &buffer) == 0) return true;
  return false;
}


void accDataPerDay(string fn, vector<vector<float> >& resArr,
                   vector<vector<float> >& errArr, const char* histName)
{
  TFile tinf(fn.c_str());
  TH1F *h = (TH1F*)tinf.Get(histName);
  const int ncols = h->GetNbinsX();
  const int nrows = h->GetNbinsY();
  
  if(!resArr.size())
    for(int i = 0; i < nrows; i++)
    {
      vector<float> rowData(ncols, 0.);
      resArr.push_back(rowData);
    }
  if(!errArr.size())
    for(int i = 0; i < nrows; i++)
    {
      vector<float> rowData(ncols, 0.);
      errArr.push_back(rowData);
    }
    
  for(int i = 0; i < nrows; i++)
    for(int j = 0; j < ncols; j++)
    {
      resArr[i][j] += h->GetBinContent(j+1, i+1);
      float err = h->GetBinError(j+1, i+1);
      errArr[i][j] += err*err;
    }
}

void getDailyDataErr(vector<vector<float> >& resArr,
                     vector<vector<float> >& errArr, int N)
{
  for(unsigned int i = 0; i < resArr.size(); i++)
    for(unsigned int j = 0; j < resArr[i].size(); j++)
    {
      resArr[i][j] /= (float)N;
      if(i == 0 || i == resArr.size()-1)
        if(j != 4)
          resArr[i][j] = 0.;
    }
  
  for(unsigned int i = 0; i < errArr.size(); i++)
    for(unsigned int j = 0; j < errArr[i].size(); j++)
    {
      errArr[i][j] = sqrt(errArr[i][j]);
      errArr[i][j] /= (float)N;
      if(i == 0 || i == errArr.size()-1)
        if(j != 4)
          errArr[i][j] = 0.;
    }
}

void printArray(vector<vector<float> > arr)
{
  for(unsigned int i = 0; i < arr.size(); i++)
  {
    for(unsigned int j = 0; j < arr[i].size(); j++)
      cout << arr[i][j] << " ";
    cout << endl;
  }
}


void writeResults(int hn, int row, int col, vector<float> res,
                  vector<float> err, const char* title, int color, int marker)
{
  TFile of(Form("rate.EH%d.root", hn), "update");
  const int N = res.size();
  float x[N], xerr[N], y[N], yerr[N];
  
  /// used for determining graph title
  string strippedTitle(title);
  strippedTitle.erase(remove_if(strippedTitle.begin(),
    strippedTitle.end(), ::isspace), strippedTitle.end());
  cout << strippedTitle << endl;
  
  
  
  for(int i = 0; i < N; i++)
  {
    TDatime curDate(2012, dateIn2012[i]/100, dateIn2012[i]%100, 0, 0, 0);
    x[i] = curDate.Convert();
    xerr[i] = 0.;
    y[i] = res[i];
    yerr[i] = err[i];
  }
  
  
  const char* gName = Form("g%sEH%dR%dC%d",
                           strippedTitle.c_str(), hn, row, col);
  TObject *obj = (TObject*)of.Get(gName);
  if(obj)
    of.Delete(Form("%s;*", gName));
  
  TGraphErrors g(N, x, y, xerr, yerr);
  g.SetName(gName);
  g.SetTitle(Form("%s for EH%dR%dC%d", title, hn, row, col));
  g.GetXaxis()->SetTimeDisplay(1);
  g.GetXaxis()->SetTimeFormat("%m/%d/%Y");
  g.GetXaxis()->SetNdivisions(-505);
  g.GetXaxis()->SetTimeOffset(0,"gmt");
  g.GetXaxis()->SetTitle("date (mm/dd/yyyy)");
  g.GetYaxis()->SetTitle("rate (Hz)");
  g.SetLineColor(color);
  g.SetMarkerColor(color);
  g.SetMarkerStyle(marker);
  g.SetLineWidth(2);
  g.Write();
  of.Close();
}
