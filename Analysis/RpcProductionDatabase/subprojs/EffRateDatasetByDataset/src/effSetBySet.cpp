#include <climits> // where PATH_MAX is defined
#include <cmath>
#include <dirent.h>
#include <iostream>
#include <string>
#include <vector>
#include "TAxis.h"
#include "TDatime.h"
#include "TEntryList.h"
#include "TFile.h"
#include "TGraphErrors.h"
#include "TROOT.h"
#include "TTimeStamp.h"
#include "TTree.h"


using namespace std;


void loadDictionary();
void getCount(vector<unsigned int>*, vector<unsigned int>*,
              vector<vector<unsigned int> >*, int&, vector<int>&, int, int);
/// function for getting layer efficiencies as well as their errors
vector<double> getLayerEffErr(int, const vector<int>&, vector<double>&);
double getModuleEffErr(const vector<double>&, const vector<double>&, double&);

void saveEff2ROOT(const vector<TTimeStamp>&, string, string, int, int,
                  const vector<double>&, const vector<double>&);


void effSetBySet(const vector<string>& vIfn, string outputPath, string hall,
                  int row, int col, unsigned int nSet, bool dbgFlag)
{
  
  loadDictionary();


  /// array of hit count in module and layer level of datasets in one day
  vector<int> mCountDay;
  vector<vector<int> > lCountDay;
  /// container of all kinds of efficiencies of each dataset
  vector<vector<double> > layerEffDay;
  vector<vector<double> > layerEffErrDay;
  vector<double> moduleEffDay;
  vector<double> moduleEffErrDay;
  vector<TTimeStamp> datasetTimeDay;

  
  for(unsigned int i = 0; i < vIfn.size(); i++)
  {
    /// quit loop if reaching the specified number of datasets to be processed
    if(nSet > 0 && i >= nSet)
      break;


    TFile fif(vIfn[i].c_str());
    TTree* rpcTree = (TTree*)fif.Get("rpcTree");


    /// restrict to one module
    string modCut = Form("mRpcRow==%d&&mRpcColumn==%d", row, col);
    string hitPatternCut = "mIs1Strip==1";
    string forcedTriggerCut = "mRpcFromRot==1";
    string totalCut = modCut + "&&" + hitPatternCut + "&&" + forcedTriggerCut;


    /// make cut on the root file
    rpcTree->Draw(">>elist", totalCut.c_str(), "entrylist");
    TEntryList *elist = (TEntryList*)gDirectory->Get("elist");
    Long64_t listEntries = elist->GetN();

    rpcTree->SetEntryList(elist);


    /// setup container variables
    vector<unsigned int>* mRpcRow = new vector<unsigned int>;
    vector<unsigned int>* mRpcColumn = new vector<unsigned int>;
    vector<vector<unsigned int> >* mRpcLayer = new vector<vector<unsigned int> >;
    rpcTree->SetBranchAddress("mRpcRow", &mRpcRow);
    rpcTree->SetBranchAddress("mRpcColumn", &mRpcColumn);
    rpcTree->SetBranchAddress("mRpcLayer", &mRpcLayer);
    /// extract triggerTimeSec and triggerTimeNanoSec
    unsigned int ttSec, ttNanoSec;
    rpcTree->SetBranchAddress("triggerTimeSec", &ttSec);
    rpcTree->SetBranchAddress("triggerTimeNanoSec", &ttNanoSec);
    int mCountDs = 0;
    vector<int> lCountDs(4);


    /// loop over events passing the cut
    for (Long64_t el = 0; el < listEntries; el++)
    {
      Long64_t treeEntry = elist->GetEntry(el);

      rpcTree->GetEntry(treeEntry);

      /// get module and layer count
      getCount(mRpcRow, mRpcColumn, mRpcLayer, mCountDs, lCountDs, row, col);

      if(dbgFlag)
      {

        for(unsigned int mIdx = 0; mIdx < mRpcRow->size(); mIdx++)
        {
          cout << "RPC ID (row, col): ";
          cout << "(" << (*mRpcRow)[mIdx] << ", " << (*mRpcColumn)[mIdx] << ") "
               << endl;
          cout << "RPC layers: (";
          for(unsigned int lIdx = 0; lIdx < mRpcLayer->at(mIdx).size(); lIdx++)
          {
            cout << (*mRpcLayer)[mIdx][lIdx];
            if(lIdx != (*mRpcLayer)[mIdx].size()-1)
              cout << ", ";
            else
              cout << ")";
          }
          cout << endl;
        }
        cout << endl;
      }
    }



    vector<double> layerEffErrDs(4);
    double moduleEffErrDs(0.);
    vector<double> layerEffDs = getLayerEffErr(mCountDs, lCountDs,
                                               layerEffErrDs);
    double moduleEffDs = getModuleEffErr(layerEffDs, layerEffErrDs,
                                         moduleEffErrDs);

    TTimeStamp datasetTime(ttSec, ttNanoSec);

    /// store the count results into dataset array
    mCountDay.push_back(mCountDs);
    lCountDay.push_back(lCountDs);
    /// store the efficiency results into dataset array
    moduleEffDay.push_back(moduleEffDs);
    moduleEffErrDay.push_back(moduleEffErrDs);
    /// store dataset time
    datasetTimeDay.push_back(datasetTime);

    /// output count results
    if(dbgFlag)
    {
      cout << "number of triggers: " << mCountDs << endl;
      cout << "counts per layer:";
      for(unsigned int i = 0; i < lCountDs.size(); i++)
        cout << " " << lCountDs[i];
      cout << endl;
      cout << "layer efficiencies:";
      for(unsigned int i = 0; i < layerEffDs.size(); i++)
        cout << " " << layerEffDs[i];
      cout << endl;
      cout << "error of layer efficiencies:";
      for(unsigned int i = 0; i < layerEffErrDs.size(); i++)
        cout << " " << layerEffErrDs[i];
      cout << endl;
      cout << "calculated module efficiency:" << endl;
      cout << moduleEffDs << endl;
      cout << "error of calculated module efficiency:" << endl;
      cout << moduleEffErrDs << endl << endl;
    }
  }

  /// save efficiency results to root files
  saveEff2ROOT(datasetTimeDay, outputPath, hall, row, col, moduleEffDay,
               moduleEffErrDay);
}




void loadDictionary()
{
  char macroPath[PATH_MAX];
  getcwd(macroPath, PATH_MAX);
  gROOT->SetMacroPath(Form("%s/include", macroPath));
  gROOT->ProcessLine(".L vecdict.h+");
}




void getCount(vector<unsigned int>* mRow, vector<unsigned int>* mCol,
              vector<vector<unsigned int> >* mLyr, int& mCnt, vector<int>& lCnt,
              int row, int col)
{
  for(unsigned int mIdx = 0; mIdx < mRow->size(); mIdx++)
  {
    if(mRow->at(mIdx) == (unsigned int)row &&
       mCol->at(mIdx) == (unsigned int)col)
    {
      mCnt++;
      for(unsigned int lIdx = 0; lIdx < mLyr->at(mIdx).size(); lIdx++)
        lCnt[mLyr->at(mIdx)[lIdx]-1]++;
    }
  }
}




vector<double> getLayerEffErr(int mCnt, const vector<int>& lCnt,
                              vector<double>& lErr)
{
  vector<double> lEff(4);
  for(unsigned int i = 0; i < 4; i++)
  {
    if(mCnt > 0)
      lEff[i] = (double)lCnt[i]/(double)mCnt;
    else
      lEff[i] = 0.;

    /// calculate errors according to the paper
    int n = mCnt;
    int k = lCnt[i];
    if(n) lErr[i] = sqrt((double)(k+1)/(n+2)*(k+2)/(n+3)-
                         (double)(k+1)*(k+1)/(n+2)/(n+2));
    else lErr[i] = 0.;
  }
  return lEff;
}




double getModuleEffErr(const vector<double>& lEff,
                       const vector<double>& lErr, double& mErr)
{
  double mEff = 0.;
  double fourProduct = 1.;
  for(int l = 0; l < 4; l++)
  {
    double threeProduct = 1-lEff[l];
    for(int p = 1; p < 4; p++)
      threeProduct *= lEff[(l+p)%4];
    mEff += threeProduct;
    fourProduct *= lEff[l];
  }
  mEff += fourProduct;


  /// calculate error according to layer errors
  double D1f, D2f, D3f, D4f;
  double x1, x2, x3, x4;
  double dx1, dx2, dx3, dx4;
  x1 = lEff[0];
  dx1 = lErr[0];
  x2 = lEff[1];
  dx2 = lErr[1];
  x3 = lEff[2];
  dx3 = lErr[2];
  x4 = lEff[3];
  dx4 = lErr[3];
  D1f = x3*x4+x2*x4+x2*x3-3*x2*x3*x4;
  D2f = x1*x3+x3*x4+x1*x4-3*x1*x3*x4;
  D3f = x1*x2+x2*x4+x1*x4-3*x1*x2*x4;
  D4f = x1*x2+x2*x3+x1*x3-3*x1*x2*x3;
  D1f *= dx1;
  D2f *= dx2;
  D3f *= dx3;
  D4f *= dx4;
  mErr = sqrt(D1f*D1f+D2f*D2f+D3f*D3f+D4f*D4f);
  /// end of error calculation


  return mEff;
}




void saveEff2ROOT(const vector<TTimeStamp>& dsTime, string outputPath,
                  string hall, int row, int col, const vector<double>& eff,
                  const vector<double>& err)
{
  const unsigned int nPts = eff.size();

  double x[nPts], ex[nPts], y[nPts], ey[nPts];


  unsigned int year, month, day, hour, min, sec;
  unsigned int yyyymmdd;
  yyyymmdd = dsTime[0].GetDate(kTRUE, 0, &year, &month, &day);

  string gName(Form("Eff_EH%sR%dC%d_%d", hall.c_str(), row, col, yyyymmdd));

  TFile fof(Form("%s/DsByDsInOneDay.root", outputPath.c_str()), "update");
  /// delete if the graph already exists
  TObject *obj = (TObject*)fof.Get(gName.c_str());
  if(obj)
    fof.Delete(Form("%s;*", gName.c_str()));

  for(unsigned int dIdx = 0; dIdx < nPts; dIdx++)
  {
    dsTime[dIdx].GetDate(kTRUE, 0, &year, &month, &day);
    dsTime[dIdx].GetTime(kTRUE, 0, &hour, &min, &sec);

    TDatime curDsTime(year, month, day, hour, min, sec);


    x[dIdx] = curDsTime.Convert();
    ex[dIdx] = 0.;
    y[dIdx] = eff[dIdx];
    ey[dIdx] = err[dIdx];

  }

  TGraphErrors gEff(nPts, x, y, ex, ey);
  gEff.SetName(gName.c_str());
  gEff.SetTitle(gName.c_str());
  gEff.GetXaxis()->SetTimeDisplay(1);
  gEff.GetXaxis()->SetTimeOffset(0,"gmt");
  gEff.GetXaxis()->SetNdivisions(505);
  gEff.GetXaxis()->SetTimeFormat("%H:%M:%S");
  gEff.GetXaxis()->SetTitle("time (hh:mm:ss)");
  gEff.GetYaxis()->SetTitle("efficiency");
  gEff.SetLineColor(kBlue);
  gEff.SetLineWidth(2);
  gEff.SetMarkerColor(kBlue);
  gEff.SetMarkerStyle(21);
  gEff.SetMarkerSize(.8);
  gEff.Write();
  fof.Close();
}
