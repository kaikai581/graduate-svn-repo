#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <set>
#include <string>
#include "TChain.h"
#include "TEntryList.h"
#include "TF1.h"
#include "TFile.h"
#include "TH1F.h"
#include "TROOT.h"
#include "structopt.hpp"
#include "utilities.hpp"


#define DT4FOLDBINSIZE 0.05 // bin size of 4-fold dt histogram in second
#define DT3FOLDBINSIZE 0.25 // bin size of 3-fold dt histogram in second


using namespace std;


unsigned int getAbsentLayer(vector<unsigned int>);
void write2ROOT(vector<double> [], string, string, int, int, int);
void fitDtHist(string, string, int, int, int, double [], double []);
void getLayerEffErr(double [], double [], double [], double []);
void getRealRateErr(double[], double[], double[], double[], double&, double&);
void getEffByQuotient(string, string, int, int, int, bool, unsigned int,
                      unsigned int[], vector<double>&);



void eventLoop(option_t opt)
{
  /// check if the input information is valid
  if(!checkInputValidity(opt.inputPath, opt.hall))
  {
    cerr << "Input path or hall number invalid" << endl;
    exit(1);
  }
  
  
  TChain rpcChain("rpcTree");
  
  if(!isRootFile(opt.inputPath))
    rpcChain.Add(Form("%s/*EH%d*.root", opt.inputPath.c_str(), opt.hall));
  else
    rpcChain.Add(Form("%s", opt.inputPath.c_str()));
  
  
  string forcedTriggerCut = "mRpcFromRot==1";
  string moduleCut = Form("mRpcRow==%d&&mRpcColumn==%d", opt.row, opt.col);
  
  
  //string totCut = forcedTriggerCut + "&&" + moduleCut;
  string totCut;
  if(opt.strCut == "") totCut = moduleCut;
  else totCut = opt.strCut + "&&" + moduleCut;
  
  
  /// make cut on the tree chain
  rpcChain.Draw(">>elist", totCut.c_str(), "entrylist");
  TEntryList *elist = (TEntryList*)gDirectory->Get("elist");
  Long64_t listEntries = elist->GetN();
  Int_t treenum = 0;
  rpcChain.SetEntryList(elist);
  
  
  /// print number of events information
//  if(opt.dbgFlag)
//  {
    cout << "number of readouts in the chain: ";
    cout << rpcChain.GetEntries() << endl;
    
    cout << "number of non-forced trigger events in the specified module: ";
    cout << listEntries << endl << endl;
//  }
  
  
  /// load dictionary for vector<vector<unsigned int> >
  loadDictionary();
  /// setup container variables
  unsigned int triggerTimeSec;
  unsigned int triggerTimeNanoSec;
  vector<bool>* mRpcFromRot = new vector<bool>;
  vector<unsigned int>* mRpcRow = new vector<unsigned int>;
  vector<unsigned int>* mRpcColumn = new vector<unsigned int>;
  vector<vector<unsigned int> >* mRpcLayer = new vector<vector<unsigned int> >;
  vector<unsigned int>* mNLayers = new vector<unsigned int>;
  rpcChain.SetBranchAddress("mRpcFromRot", &mRpcFromRot);
  rpcChain.SetBranchAddress("triggerTimeSec", &triggerTimeSec);
  rpcChain.SetBranchAddress("triggerTimeNanoSec", &triggerTimeNanoSec);
  rpcChain.SetBranchAddress("mRpcRow", &mRpcRow);
  rpcChain.SetBranchAddress("mRpcColumn", &mRpcColumn);
  rpcChain.SetBranchAddress("mRpcLayer", &mRpcLayer);
  rpcChain.SetBranchAddress("mNLayers", &mNLayers);
  
  
  /// results storage:
  /// Array [0] stores 4 fold trigger times.
  /// Array [i] stores 3 fold trigger times with missing layer i.
  vector<double> nFoldTime[5];
  vector<double> nFoldDt[5];
  vector<double> triggerTime, triggerDt;
  unsigned int N4 = 0, N3[4] = {0}, noiseN[4] = {0}, NForcedTrigger = 0;
  
  
  for (Long64_t el = 0; el < listEntries; el++)
  {
    /// quit if reaches specified number of events
    if(opt.nEvt > 0 && el >= opt.nEvt)
      break;
    
    
    Long64_t treeEntry = elist->GetEntryAndTree(el, treenum);
    Long64_t chainEntry = treeEntry + rpcChain.GetTreeOffset()[treenum];
    

    rpcChain.GetEntry(chainEntry);
    
    
    /// check if number of readout modules is consistent
    bool nModuleConsistent = (mRpcRow->size() == mRpcColumn->size()) &&
                             (mRpcColumn->size() == mRpcLayer->size()) &&
                             (mRpcLayer->size() == mNLayers->size());
    if(!nModuleConsistent)
    {
      cerr << "number of readout modules inconsistent" << endl;
      continue;
    }
    
    unsigned int nModules = mRpcRow->size();
    /// loop over modules in the readout containing the specified module
    for(unsigned int mIdx = 0; mIdx < nModules; mIdx++)
    {
      if((int)mRpcRow->at(mIdx)==opt.row && (int)mRpcColumn->at(mIdx)==opt.col)
      {
        if(mRpcFromRot->at(mIdx))
        {
          double curTime = (double)triggerTimeSec+(double)triggerTimeNanoSec*1e-9;
          
          /// trigger from ROT
          triggerTime.push_back(curTime);
          if(triggerTime.size() > 1)
            triggerDt.push_back(triggerTime[triggerTime.size()-1]-
                                triggerTime[triggerTime.size()-2]);
          
          /// 4-fold trigger
          if(mNLayers->at(mIdx) == 4)
          {
            nFoldTime[0].push_back(curTime);
            if(nFoldTime[0].size() > 1)
              nFoldDt[0].push_back(nFoldTime[0][nFoldTime[0].size()-1]-
                                   nFoldTime[0][nFoldTime[0].size()-2]);
  
  
            /// increment number of 4-fold triggers
            N4++;
  
            /// output warnings when dt is not reasonable
            /// Usually when a run is stopped and a new run is started,
            /// a large dt gap occurs. Since different runs are chained together,
            /// unless there is some way to separate runs by their file names,
            /// fixing dt bin size could be a good temperary solution.
            if(nFoldDt[0].size() >= 1)
            {
              if(nFoldDt[0][nFoldDt[0].size()-1] > 100.)
              {
                cerr << "time since last 4-fold trigger is way too large" << endl;
                cerr << "tree number: " << treenum << "\t";
                cerr << "tree entry: " << treeEntry << endl;
                cerr << "trigger time sec: " << triggerTimeSec << "\t";
                cerr << "trigger time nano sec: " << triggerTimeNanoSec << endl;
              }
            }
          }
          else if(mNLayers->at(mIdx) == 3)
          {
            unsigned int absentLayer = getAbsentLayer(mRpcLayer->at(mIdx));
            nFoldTime[absentLayer].push_back(curTime);
            if(nFoldTime[absentLayer].size() > 1)
              nFoldDt[absentLayer].push_back(nFoldTime[absentLayer][nFoldTime[absentLayer].size()-1]-nFoldTime[absentLayer][nFoldTime[absentLayer].size()-2]);
            
            /// increment N3[i], number of 3-fold triggers, where i is
            /// (absent layer ID) - 1
            N3[absentLayer-1]++;
          }
          else
          {
            cerr << "layer multiplicity is neither 3 nor 4" << endl;
            continue;
          }
            
            
            
            
          /// print debug information
          if(opt.dbgFlag)
          {
            cout << "trigger time: ";
            cout << curTime << endl;
            
            cout << "RPC row: ";
            cout << mRpcRow->at(mIdx) << " ";
            cout << endl;
            
            cout << "RPC col: ";
            cout << mRpcColumn->at(mIdx) << " ";
            cout << endl;
            
            cout << "layer multiplicity: ";
            cout << mNLayers->at(mIdx) << " ";
            cout << endl;
            
            cout << "time since last trigger: ";
            if(mNLayers->at(mIdx) == 4)
            {
              if(nFoldTime[0].size() > 1)
                cout << nFoldTime[0][nFoldTime[0].size()-1]-
                nFoldTime[0][nFoldTime[0].size()-2] << endl;
              else
                cout << 0. << endl;
            }
            else if(mNLayers->at(mIdx) == 3)
            {
              unsigned int absentLayer = getAbsentLayer(mRpcLayer->at(mIdx));
              if(nFoldTime[absentLayer].size() > 1)
                cout << nFoldTime[absentLayer][nFoldTime[absentLayer].size()-1]-
                nFoldTime[absentLayer][nFoldTime[absentLayer].size()-2] << endl;
              else
                cout << 0. << endl;
            }
          }
          if(opt.dbgFlag)
            cout << endl;
        }
        /// forced trigger
        else
        {
          NForcedTrigger++;
          for(unsigned int lIdx = 0; lIdx < mRpcLayer->at(mIdx).size(); lIdx++)
            noiseN[mRpcLayer->at(mIdx)[lIdx]-1]++;
        }
      }

    }

  }
  /// output forced trigger information to screen and file ///
  cout << "number of forced triggers: " << NForcedTrigger << endl;
  cout << "number of forced triggers layer by layer:";
  for(int i = 0; i < 4; i++)
    cout << " " << noiseN[i];
  cout << endl;
  /// end of forced trigger output ///////////////////////////
/// end of TChain loop /////////////////////////////////////////////////////////
  
  
  
/// output results /////////////////////////////////////////////////////////////
  write2ROOT(nFoldDt, opt.inputPath, opt.outputPath, opt.hall, opt.row, opt.col);
  double rate[5], erate[5];
  fitDtHist(opt.inputPath, opt.outputPath, opt.hall, opt.row, opt.col, rate, erate);
  
  cout << "4-fold rate and error: ";
  cout << rate[0] << " " << erate[0] << endl;
  for(int i = 1; i < 5; i++)
  {
    cout << "3-fold rate and error with layer " << i << " absent: ";
    cout << rate[i] << " " << erate[i] << endl;
  }
  
  
  /// calculate layer efficiencies
  double layerEff[4], layerEffErr[4];
  getLayerEffErr(rate, erate, layerEff, layerEffErr);
  cout << "layer efficiency and error: " << endl;
  for(int i = 0; i < 4; i++)
  {
    cout << "layer " << i+1 << ": ";
    cout << layerEff[i] << " " << layerEffErr[i] << endl;
  }




  /// calculate module efficiency and error
  double moduleEff = 0., moduleEffErr = 0.;
  getModuleEffErr(layerEff, layerEffErr, moduleEff, moduleEffErr);
  cout << endl;
  cout << "module efficiency and error: " << endl;
  cout << moduleEff << " " << moduleEffErr << endl;
  
  
  
  /// calculate real rate and error
  double realRate = 0., eRealRate = 0.;
  getRealRateErr(rate, erate, layerEff, layerEffErr, realRate, eRealRate);
  cout << "real rate and error: " << endl;
  cout << realRate << " " << eRealRate << endl;
  
  
  
  /// output results to a file
  eff2File("ByFitting", opt.inputPath, opt.outputPath, opt.hall, opt.row, opt.col, moduleEff, moduleEffErr, realRate,
           eRealRate, layerEff, layerEffErr);
  
  
    
  /// output dt array for debugging
  if(opt.dbgFlag)
  {
    ofstream timeInfo("dt4Fold.txt");
    sort(nFoldDt[0].begin(), nFoldDt[0].end());
    for(unsigned int k = 0; k < nFoldDt[0].size(); k++)
      timeInfo << nFoldDt[0][k] << endl;
    timeInfo.close();
  }
  

  /// a simpler way of getting layer efficiency
  getEffByQuotient(opt.inputPath, opt.outputPath, opt.hall, opt.row, opt.col, opt.dbgFlag, N4, N3, triggerDt);
}
/// end of event loop function /////////////////////////////////////////////////




unsigned int getAbsentLayer(vector<unsigned int> hitLayer)
{
  unsigned int fl[] = {1, 2, 3, 4};
  set<unsigned int> absentLayer(fl, fl+4);
  for(unsigned int i = 0; i < hitLayer.size(); i++)
    absentLayer.erase(hitLayer[i]);

  return *(absentLayer.begin());
}



void write2ROOT(vector<double> nFoldDt[], string ifp, string ofp, int hall,
                int row, int col)
{
  string year = getYear(ifp);
  string date = getDate(ifp);
  
  
  TFile fof(Form("%s/effbyfitting.root", ofp.c_str()), "update");
  
  /// 4-fold interarrival time
  string h4fName = Form("dt4FoldEH%dR%dC%d_%s%s", hall, row, col, year.c_str(),
                        date.c_str());

  double maxDt = *max_element(nFoldDt[0].begin(), nFoldDt[0].end());
  TH1F h4f(h4fName.c_str(), h4fName.c_str(), (int)(maxDt/DT4FOLDBINSIZE), 0,
           maxDt);

  for(unsigned int i = 0; i < nFoldDt[0].size(); i++)
    h4f.Fill(nFoldDt[0][i]);


  /// override if exists
  TObject *obj = (TObject*)fof.Get(h4fName.c_str());
  if(obj) fof.Delete(Form("%s;*", h4fName.c_str()));
  h4f.Write();
  
  /// 3-fold interarrival time
  string h3fName[4];
  
  for(int i = 0; i < 4; i++)
  {
    h3fName[i] = Form("dt3FoldLayer%dAbsentEH%dR%dC%d_%s%s", i+1, hall, row,
                      col, year.c_str(), date.c_str());

    if(nFoldDt[i+1].size())
      maxDt = *max_element(nFoldDt[i+1].begin(), nFoldDt[i+1].end());
    else maxDt = 10;
    TH1F h3f(h3fName[i].c_str(), h3fName[i].c_str(), (int)(maxDt/DT3FOLDBINSIZE)
             , 0, maxDt);
    
    for(unsigned int j = 0; j < nFoldDt[i+1].size(); j++)
      h3f.Fill(nFoldDt[i+1][j]);
    /// override if exists
    TObject *obj2 = (TObject*)fof.Get(h3fName[i].c_str());
    if(obj2) fof.Delete(Form("%s;*", h3fName[i].c_str()));
    h3f.Write();
  }
  
  fof.Close();
  
}


void fitDtHist(string ifp, string ofp, int hall, int row, int col,
               double rate[], double erate[])
{
  string year = getYear(ifp);
  string date = getDate(ifp);
  
  TFile tif(Form("%s/effbyfitting.root", ofp.c_str()));
  
  string hName = Form("dt4FoldEH%dR%dC%d_%s%s", hall, row, col, year.c_str(),
                      date.c_str());
  TH1F* hDt = (TH1F*)tif.Get(hName.c_str());
  TF1 ffit("ffit", "expo");
  /// fit the histogram with quiet mode
  hDt->Fit("ffit", "Q");
  
  rate[0] = ffit.GetParameter(1);
  erate[0] = ffit.GetParError(1);
  
  /// 3-fold fitting
  for(unsigned int i = 1; i < 5; i++)
  {
    hName = Form("dt3FoldLayer%dAbsentEH%dR%dC%d_%s%s", i, hall, row,
                 col, year.c_str(), date.c_str());
    hDt = (TH1F*)tif.Get(hName.c_str());
    hDt->Fit("ffit", "Q");
    
    rate[i] = ffit.GetParameter(1);
    erate[i] = ffit.GetParError(1);
  }
  cout << "fitDtHist called" << endl;
}



/// calculate layer efficiency with Dr. Lau's fascinating idea
void getLayerEffErr(double rate[], double erate[], double lEff[],
                    double lEffErr[])
{
  if(rate[0] == 0.)
  {
    cerr << "4-fold efficiency is zero. This method breaks down." << endl;
    exit(1);
  }
  
  double beta[4], ebeta[4];
  double re2 = erate[0]/rate[0];
  for(int i = 0; i < 4; i++)
  {
    beta[i] = rate[i+1]/rate[0];
    double re1 = 0.;
    if(rate[i+1] != 0.) re1 = erate[i+1]/rate[i+1];
    ebeta[i] = beta[i]*sqrt(re1*re1+re2*re2);

    
    /// calculate layer efficiencies and their errors
    lEff[i] = 1./(1.+beta[i]);
    lEffErr[i] = ebeta[i]/(1.+beta[i])/(1.+beta[i]);
  }
}




void getRealRateErr(double fitr[], double efitr[], double le[], double ele[],
                    double& rr, double& err)
{
  double fourProduct = 1.;
  for(int i = 0; i < 4; i++)
    fourProduct *= le[i];

  if(fourProduct == 0.)
  {
    cerr << "One of the layer efficiencies is 0. ";
    cerr << "This method breaks down." << endl;
    exit(1);
  }
  rr = fabs(fitr[0]/fourProduct);
  
  /// calculate error in real rate
  double t0 = efitr[0]/fourProduct;
  double t1 = fitr[0]*ele[0]/fourProduct/le[0];
  double t2 = fitr[0]*ele[1]/fourProduct/le[1];
  double t3 = fitr[0]*ele[2]/fourProduct/le[2];
  double t4 = fitr[0]*ele[3]/fourProduct/le[3];
  err = sqrt(t0*t0+t1*t1+t2*t2+t3*t3+t4*t4);
}
