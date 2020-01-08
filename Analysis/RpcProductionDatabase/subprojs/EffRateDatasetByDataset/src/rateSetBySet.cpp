#include <vector>
#include <string>
#include "TDatime.h"
#include "TF1.h"
#include "TFile.h"
#include "TGraphErrors.h"
#include "TH1F.h"
#include "TTimeStamp.h"
#include "TTree.h"
#include "utilities.h"

using namespace std;

struct FittingPrarmeters_t
{
  /// fitting function = exp(p0+p1*x)
  double p0;
  double ep0; // error of p0
  double p1;
  double ep1; // error of p1
};


string getNFold(string);


void rateSetBySet(const vector<string>& vIfn, string outputPath, string hall,
                  int row, int col, string dtVarName, unsigned int nSet,
                  bool dbgFlag)
{
  vector<FittingPrarmeters_t> fittingParameters;
  vector<TDatime> dtStartTime;
  vector<TDatime> dtEndTime;


  for(unsigned int i = 0; i < vIfn.size(); i++)
  {
    /// quit loop if reaching the specified number of datasets to be processed
    if(nSet > 0 && i >= nSet)
      break;


    TFile tif(vIfn[i].c_str());
    TTree* rpcTree = (TTree*)tif.Get("rpcTree");

    /// extract triggerTimeSec and triggerTimeNanoSec
    unsigned int ttSec, ttNanoSec;
    rpcTree->SetBranchAddress("triggerTimeSec", &ttSec);
    rpcTree->SetBranchAddress("triggerTimeNanoSec", &ttNanoSec);


    unsigned int year, month, day, hour, min, sec;
    rpcTree->GetEntry(0);
    TTimeStamp tsStartTime(ttSec, ttNanoSec);
    tsStartTime.GetDate(kTRUE, 0, &year, &month, &day);
    tsStartTime.GetTime(kTRUE, 0, &hour, &min, &sec);
    TDatime curStartTime(year, month, day, hour, min, sec);
    dtStartTime.push_back(curStartTime);
    if(dbgFlag)
    {
      cout << "dataset start time: " << endl;
      cout << curStartTime.AsString() << endl;
    }

    rpcTree->GetEntry(rpcTree->GetEntries()-1);
    TTimeStamp tsEndTime(ttSec, ttNanoSec);
    tsEndTime.GetDate(kTRUE, 0, &year, &month, &day);
    tsEndTime.GetTime(kTRUE, 0, &hour, &min, &sec);
    TDatime curEndTime(year, month, day, hour, min, sec);
    dtEndTime.push_back(curEndTime);
    if(dbgFlag)
    {
      cout << "dataset end time: " << endl;
      cout << curEndTime.AsString() << endl;
    }


    char* modCut = Form("mRpcRow==%d&&mRpcColumn==%d&&%s>0", row, col,
                        dtVarName.c_str());
    rpcTree->Draw(Form("%s>>hDt", dtVarName.c_str()), modCut);
    TH1F* hDt = (TH1F*)gDirectory->Get("hDt");
    TF1 ffit("ffit", "expo");
    /// fit the histogram with quiet mode
    hDt->Fit("ffit", "Q");

    FittingPrarmeters_t curFitParam;
    curFitParam.p0 = ffit.GetParameter(0);
    curFitParam.ep0	= ffit.GetParError(0);
    curFitParam.p1 = ffit.GetParameter(1);
    curFitParam.ep1	= ffit.GetParError(1);
    fittingParameters.push_back(curFitParam);

  }



  const unsigned int nPts = fittingParameters.size();
  double x[nPts], ex[nPts], y[nPts], ey[nPts];


  string sYear = getYear(vIfn[0]);
  string sDate = getDate(vIfn[0]);
  char* gName = Form("Rate_%s_EH%sR%dC%d_%s%s", getNFold(dtVarName).c_str(),
                     hall.c_str(), row, col, sYear.c_str(), sDate.c_str());


  TFile fof(Form("%s/DsByDsInOneDay.root", outputPath.c_str()), "update");
  /// delete if the graph already exists
  TObject *obj = (TObject*)fof.Get(gName);
  if(obj)
    fof.Delete(Form("%s;*", gName));

  for(unsigned int i = 0; i < nPts; i++)
  {
    x[i] = dtEndTime[i].Convert();
    ex[i] = 0.;
    y[i] = -fittingParameters[i].p1;
    ey[i] = fittingParameters[i].ep1;
  }
  TGraphErrors gRate(nPts, x, y, ex, ey);
  gRate.SetName(gName);
  gRate.SetTitle(gName);
  gRate.GetXaxis()->SetTimeDisplay(1);
  gRate.GetXaxis()->SetTimeOffset(0,"gmt");
  gRate.GetXaxis()->SetNdivisions(505);
  gRate.GetXaxis()->SetTimeFormat("%H:%M:%S");
  gRate.GetXaxis()->SetTitle("time (hh:mm:ss)");
  gRate.GetYaxis()->SetTitle("rate (Hz)");
  gRate.SetLineColor(kBlue);
  gRate.SetLineWidth(2);
  gRate.SetMarkerColor(kBlue);
  gRate.SetMarkerStyle(21);
  gRate.SetMarkerSize(.8);
  gRate.Write();
  fof.Close();
}





string getNFold(string varName)
{
  string strRes;
  if(varName.find("4Fold") != string::npos)
  {
    strRes = "4Fold";
    return strRes;
  }

  if(varName.find("3Fold") != string::npos)
  {
    strRes = "3Fold";
    return strRes;
  }

  strRes = "3or4Fold";
  return strRes;
}
