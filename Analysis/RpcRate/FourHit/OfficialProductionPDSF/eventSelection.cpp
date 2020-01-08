/// 2012 Feb 3:
/// Finally I got this program compiled successfully.
/// Key points are:
/// 1. Generate dictionary for PerCalibReadoutEvent
/// 2. Link to libPerCalibReadoutEvent.so

/// 2012 Jan 22:
/// Chao replied my mayday.
/// CalibReadoutHeader is a RootIO class. One should use class method to
/// retrieve branches.
/// NOTE THAT to run this program, nuwa should be set up.


/// As of 2012 Jan 17, this program NEVER worked.
/// The problem is on the vector retrieval.
/// Plan to go back to my own production file.


#include <algorithm>
#include <climits> // where PATH_MAX is defined
#include <cmath>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "Conventions/Site.h"
#include "tclap/CmdLine.h"
#include "TBranchElement.h"
#include "TFile.h"
#include "TH2I.h"
#include "TLeaf.h"
#include "TTree.h"
#include "TROOT.h"
#include "TEntryList.h"
#include "PerCalibReadoutEvent/PerCalibReadoutHeader.h"

using namespace std;

bool fexists(const char *);
bool hasMuonHits(int, int, int *[]);
bool indexInRange(int, int);
void globalSetting();
void rebuildIndex(TTree*, char*, char*);
void noRpcList(string);
template<class T> T** allocModulewise(int, int);
void calcHitRate(int, int, int**, vector<unsigned int>**,
                vector<unsigned int>**, float**, float**);
void initArray(int**, int, int);
void printModuleResults(int, int, int**);
void printModuleResults(int, int, float**);



int main(int argc, char** argv)
{
  globalSetting();
  
  
  TCLAP::CmdLine cmd("Command description message", ' ', "0.9");
  TCLAP::ValueArg<string> infArg("i", "input",
  "Input file path name", false,
  "file/recon.Neutrino.0021355.Physics.EH3-Merged.KUP-P._0124.root", "string");
  TCLAP::ValueArg<string> ofArg("o", "output",
  "Output file path", false, "file/", "string");
  TCLAP::ValueArg<int> nEvtArg("n", "number",
  "Number of events", false, -1, "integer");
  TCLAP::SwitchArg dbgArg("g", "debug", "Print debug info", false);
  cmd.add(infArg);
  cmd.add(ofArg);
  cmd.add(nEvtArg);
  cmd.add(dbgArg);
  cmd.parse(argc, argv);
  
  
  int nEvt = nEvtArg.getValue();
  bool dbgInfo = dbgArg.getValue();
  string infpn = infArg.getValue();
  string ofp = ofArg.getValue();
  string ofn;
  string ofpn;
  if(ofp.substr(ofp.length()-5, 5) == ".root")
  {
    if(ofp.rfind('/') == string::npos)
    {
      ofn = ofp;
      ofp = "./";
    }
    else
    {
      ofn = ofp.substr(ofp.rfind('/')+1, ofp.length()-1-ofp.rfind('/'));
      ofp = ofp.substr(0, ofp.rfind('/'));
    }
  }
  else
  {
    ofn = infpn;
    ofn = ofn.substr(ofn.rfind('/')+1, ofn.length()-1-ofn.rfind('/'));
    ofn = ofn.substr(ofn.find('.')+1, ofn.length()-1-ofn.find('.'));
    ofn = "rate." + ofn;
  }
  /// check if path is terminated with '/'
  if(ofp.rfind('/') != ofp.length()-1)
    ofp += '/';
  ofpn = ofp + ofn;
  cout << "processing file: " << endl << infpn << endl << endl;
  
  if(!fexists(infpn.c_str()))
  {
    cerr << "Failed to open input file." << endl;
    return -1;
  }
  
  /// Check if the output path exists. If not, create it.
  if(ofp != "./")
  {
    struct stat st;
    if((ofp.c_str(), &st))
      mkdir(ofp.c_str(), S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
  }
  
  TFile f(infpn.c_str());


  TTree *treeCalib = (TTree*)f.Get("/Event/CalibReadout/CalibReadoutHeader");
  
  /// Note site used in reconstructed data is diffrernt from site in DaqCrate.
  /// For offline convention, please see
  /// http://dayabay.bnl.gov/dox/Conventions/html/namespaceSite.html
  /// For daq convention, please see
  /// http://dayabay.bnl.gov/dox/DaqEvent/html/
  /// DaqCrate_8cc.html#ae27fb919fe6fb7c8a00e2293f5f416b0

  int site;

  PerCalibReadoutHeader* rh = 0;
  
  
  int nrows, ncols;
  int ***layerHitCount = 0, **passMuonCount = 0;
  int **threeHitCount = 0, **fourHitCount = 0;
  float **ge3HitRate = 0, **fourHitRate = 0;
  float **ge3RateErr = 0, **fourRateErr = 0;

  int nentry = treeCalib->GetEntries();
  vector<unsigned int> **ge3HitTimeSec = 0, **ge3HitTimeNanoSec = 0;
  vector<unsigned int> **fourHitTimeSec = 0, **fourHitTimeNanoSec = 0;
  for(int entry = 0; entry < nentry; entry++)
  {
    if(nEvt>=0 && entry>=nEvt)
      break;


    treeCalib->SetBranchAddress("CalibReadout_CalibReadoutHeader", &rh);

    treeCalib->GetEntry(entry);
    if(!rh)
    {
      cout << "RPC entry not found in calibration tree" << endl;
      continue;
    }
    
    site = (int)treeCalib->GetLeaf("site")->GetValue();
    if(dbgInfo)
      cout << "site: " << site << endl;

    
    
    if(!layerHitCount||!passMuonCount)
    {
      /// row runs from 0..7 for near sites; 0..10 for far site
      /// column runs from 1..9
      if(site == Site::kFar)
      {
        nrows = 11;
        ncols = 9;
      }
      else
      {
        nrows = 8;
        ncols = 9;
      }
      
      layerHitCount = new int** [nrows];
      passMuonCount = new int* [nrows];
      threeHitCount = new int* [nrows];
      fourHitCount = new int* [nrows];
      for(int i = 0; i < nrows; i++)
      {
        layerHitCount[i] = new int* [ncols];
        passMuonCount[i] = new int [ncols];
        threeHitCount[i] = new int [ncols];
        fourHitCount[i] = new int [ncols];
        for(int j = 0; j < ncols; j++)
          layerHitCount[i][j] = new int [4];
      }
      /// initialize
      for(int i = 0; i < nrows; i++)
        for(int j = 0; j < ncols; j++)
        {
          passMuonCount[i][j] = 0;
          threeHitCount[i][j] = 0;
          fourHitCount[i][j] = 0;
          for(int k = 0; k < 4; k++)
            layerHitCount[i][j][k] = 0;
        }
    }
    
    
    /// allocate all kinds of results storage
    if(!fourHitTimeSec)
      fourHitTimeSec = allocModulewise<vector<unsigned int> >(nrows, ncols);
    if(!fourHitTimeNanoSec)
      fourHitTimeNanoSec = allocModulewise<vector<unsigned int> >(nrows, ncols);
    if(!ge3HitTimeSec)
      ge3HitTimeSec = allocModulewise<vector<unsigned int> >(nrows, ncols);
    if(!ge3HitTimeNanoSec)
      ge3HitTimeNanoSec = allocModulewise<vector<unsigned int> >(nrows, ncols);
    if(!ge3HitRate)
      ge3HitRate = allocModulewise<float>(nrows, ncols);
    if(!fourHitRate)
      fourHitRate = allocModulewise<float>(nrows, ncols);
    if(!ge3RateErr)
      ge3RateErr = allocModulewise<float>(nrows, ncols);
    if(!fourRateErr)
      fourRateErr = allocModulewise<float>(nrows, ncols);
    
    unsigned int triggerTimeSec = rh->triggerTimeSec;
    unsigned int triggerTimeNanoSec = rh->triggerTimeNanoSec;
    unsigned int nHitsRpc = rh->nHitsRpc;
    if(dbgInfo)
      cout << "number of hit rpc strips: " << nHitsRpc << endl;
    
    /// 2 passes method: first pass to determine the number of
    /// hit modules in this trigger
    pair<unsigned int, unsigned int> lastModId;
    pair<unsigned int, unsigned int> thisModId;
    vector<unsigned int> nStripThisMod;
    unsigned int nModules = 0;
    unsigned int nStrip = 0;
    
    for(unsigned int sIdx = 0; sIdx < nHitsRpc; sIdx++)
    {
      thisModId.first = rh->rpcRow[sIdx];
      thisModId.second = rh->rpcColumn[sIdx];
      if(sIdx >= 1)
      {
        if(!(thisModId == lastModId))
        {
          nModules++;
          nStripThisMod.push_back(nStrip);
          nStrip = 1;
        }
        else
          nStrip++;
      }
      else
      {
        nStrip++;
        nModules++;
      }
      if(sIdx == nHitsRpc - 1)
        nStripThisMod.push_back(nStrip);
      lastModId.first = thisModId.first;
      lastModId.second = thisModId.second;
    }
    if(dbgInfo)
    {
      cout << "number of hit modules: " << nModules << endl;
      cout << "number of hit strips per module: ";
      for(unsigned int ni = 0; ni < nStripThisMod.size(); ni++)
        cout << nStripThisMod[ni] << " ";
      cout << endl;
    }    
    
    /// 2 passes method: second pass to get data and store them
    /// module by module
    vector<unsigned int> hitModRow;
    vector<unsigned int> hitModCol;
    vector<unsigned int> layerThisMod;
    vector<vector<unsigned int> > hitLayer;
    vector<unsigned int> stripThisMod;
    vector<vector<unsigned int> > hitStrip;
    vector<unsigned int> nLayer;
    nStrip = 0;
    for(unsigned int nm = 0; nm < nModules; nm++)
    {
      layerThisMod.clear();
      stripThisMod.clear();
      bool getHit[4] = {false};

      for(unsigned int ns = 0; ns < nStripThisMod[nm]; ns++)
      {
        if(ns == 0)
        {
          hitModRow.push_back(rh->rpcRow[nStrip]);
          hitModCol.push_back(rh->rpcColumn[nStrip]);
        }
        layerThisMod.push_back(rh->rpcLayer[nStrip]);
        getHit[rh->rpcLayer[nStrip]-1] = true;
        stripThisMod.push_back(rh->rpcStrip[nStrip]);
        nStrip++;
      }
      unsigned int nLayerThisMod = 0;
      for(int lIdx = 0; lIdx < 4; lIdx++)
        if(getHit[lIdx])
          nLayerThisMod++;
      nLayer.push_back(nLayerThisMod);
      hitLayer.push_back(layerThisMod);
      hitStrip.push_back(stripThisMod);
      
      if(dbgInfo)
        cout << "number of hit layers: " << nLayer[nm] << endl;
      if(nLayerThisMod >= 3)
      {
        if(indexInRange(nrows, hitModRow[nm])&&
        indexInRange(ncols, hitModCol[nm]-1))
        {
          passMuonCount[hitModRow[nm]][hitModCol[nm]-1]++;
          ge3HitTimeSec[hitModRow[nm]][hitModCol[nm]-1].push_back(triggerTimeSec);
          ge3HitTimeNanoSec[hitModRow[nm]][hitModCol[nm]-1].push_back(triggerTimeNanoSec);
          for(int lIdx = 0; lIdx < 4; lIdx++)
            if(getHit[lIdx])
              layerHitCount[hitModRow[nm]][hitModCol[nm]-1][lIdx]++;
          if(dbgInfo)
            for(unsigned int ns = 0; ns < nStripThisMod[nm]; ns++)
            {
              cout << "row: " << hitModRow[nm] << " ";
              cout << "column: " << hitModCol[nm] << " ";
              cout << "layer: " << hitLayer[nm][ns] << " ";
              cout << "strip: " << hitStrip[nm][ns] << endl;
            }
          /// record whether the readout is 3 or 4 hit
          if(nLayerThisMod == 3)
            threeHitCount[hitModRow[nm]][hitModCol[nm]-1]++;
          else if(nLayerThisMod == 4)
          {
            fourHitCount[hitModRow[nm]][hitModCol[nm]-1]++;
            fourHitTimeSec[hitModRow[nm]][hitModCol[nm]-1].push_back(triggerTimeSec);
            fourHitTimeNanoSec[hitModRow[nm]][hitModCol[nm]-1].push_back(triggerTimeNanoSec);
          }
        }
        else
          cout << "row or column number out of range" << endl;
      }
    }
    if(dbgInfo)
      cout << endl;
    }

  
  /// print results layer by layer
  for(int k = 0; k < 4; k++)
  {
    for(int i = 0; i < nrows; i++)
    {
      for(int j = 0; j < ncols; j++)
        cout << layerHitCount[i][j][k] << " ";
      cout << endl;
    }
    cout << endl << endl;
  }
  
  /// print result of muons
  cout << "3-hit count:" << endl;
  printModuleResults(nrows, ncols, passMuonCount);
  /// print 4-hit count
  cout << "4-hit count:" << endl;
  printModuleResults(nrows, ncols, fourHitCount);

  /// calculate 3-hit rate
  cout << "3-hit rate:" << endl;
  calcHitRate(nrows, ncols, passMuonCount, ge3HitTimeSec, ge3HitTimeNanoSec,
              ge3HitRate, ge3RateErr);
  cout << "3-hit rate error:" << endl;
  printModuleResults(nrows, ncols, ge3RateErr);
  /// calculate 4-hit rate
  cout << "4-hit rate:" << endl;
  calcHitRate(nrows, ncols, fourHitCount, fourHitTimeSec, fourHitTimeNanoSec,
              fourHitRate, fourRateErr);
  cout << "4-hit rate error:" << endl;
  printModuleResults(nrows, ncols, fourRateErr);

  /// Write results to TFile.
  if(hasMuonHits(nrows, ncols, passMuonCount))
  {
    TFile outTF(ofpn.c_str(), "recreate");
    TH2I *layerCount[4];
    for(int l = 0; l < 4; l++)
      layerCount[l] = new TH2I(Form("l%d", l),
                                Form("hit count for layer %d", l),
                                ncols, 0, ncols, nrows, 0, nrows);

    TH2I muonCount("m", "number of muons", ncols, 0, ncols,
                    nrows, 0, nrows);
    TH2I hThreeHit("h3h", "number of 3 hit readout", ncols, 0, ncols,
                    nrows, 0, nrows);
    TH2I hFourHit("h4h", "number of 4 hit readout", ncols, 0, ncols,
                    nrows, 0, nrows);
    TH2F hGe3HitRate("hge3hr", "#geq3 hit rate", ncols, 0, ncols,
                  nrows, 0, nrows);
    TH2F h4HitRate("h4hr", "4 hit rate", ncols, 0, ncols,
                  nrows, 0, nrows);
    
    /// In SetBinContent(), the first 2 arguments are bin "count", not
    /// bin "number"
    for(int k = 0; k < 4; k++)
      for(int i = 0; i < nrows; i++)
        for(int j = 0; j < ncols; j++)
          layerCount[k]->SetBinContent(j+1, i+1, layerHitCount[i][j][k]);

    for(int i = 0; i < nrows; i++)
      for(int j = 0; j < ncols; j++)
      {
        muonCount.SetBinContent(j+1, i+1, passMuonCount[i][j]);
        hThreeHit.SetBinContent(j+1, i+1, threeHitCount[i][j]);
        hFourHit.SetBinContent(j+1, i+1, fourHitCount[i][j]);
        hGe3HitRate.SetBinContent(j+1, i+1, ge3HitRate[i][j]);
        hGe3HitRate.SetBinError(j+1, i+1, ge3RateErr[i][j]);
        h4HitRate.SetBinContent(j+1, i+1, fourHitRate[i][j]);
        h4HitRate.SetBinError(j+1, i+1, fourRateErr[i][j]);
      }
    
    for(int k = 0; k < 4; k++)
      layerCount[k]->Write();
    muonCount.Write();
    hThreeHit.Write();
    hFourHit.Write();
    hGe3HitRate.Write();
    h4HitRate.Write();

    for(int k = 0; k < 4; k++)
      delete layerCount[k];
  }
  else
    noRpcList(infpn);

  cout << endl;
  cout << "number of entries in spallation tree: " << nentry << endl;
  
  f.Close();
  return 0;
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


void globalSetting()
{
  char macroPath[PATH_MAX];
  getcwd(macroPath, PATH_MAX);
  gROOT->SetMacroPath(Form("%s/src", macroPath));
  gROOT->ProcessLine(".L vecdict.h+");
  gROOT->ProcessLine(".x $ROOTIOTESTROOT/share/load.C");
}


bool hasMuonHits(int nrows, int ncols, int *passMuonCount[])
{
  bool hit = false;
  for(int i = 0; i < nrows; i++)
    for(int j = 0; j < ncols; j++)
      if(passMuonCount[i][j])
        hit = true;
  
  return hit;
}


bool indexInRange(int arraySize, int curIdx)
{
  if(curIdx < arraySize)
    return true;
  else
    return false;
}



void noRpcList(string infpn)
{
  vector<string> vFileName;
  if(fexists("NoRpcFileList.txt"))
  {
    string thisFileName;
    ifstream fList("NoRpcFileList.txt");
    while(fList >> thisFileName)
      vFileName.push_back(thisFileName);
    fList.close();
    
    vector<string>::iterator itEnd = vFileName.end();
    if(find(vFileName.begin(), itEnd, infpn) == itEnd)
    {
      ofstream ofList;
      ofList.open("NoRpcFileList.txt", ios::out|ios::app);
      ofList << infpn << endl;
      ofList.close();
    }
  }
  else
  {
    ofstream ofList("NoRpcFileList.txt");
    ofList << infpn << endl;
    ofList.close();
  }
  return;
}


template<class T> T** allocModulewise(int nrows, int ncols)
{
  T** retPtr;
  retPtr = new T* [nrows];
  for(int i = 0; i < nrows; i++)
    retPtr[i] = new T [ncols];
  return retPtr;
}


void calcHitRate(int nrows, int ncols, int** hitCount,
                vector<unsigned int>** hitTimeSec,
                vector<unsigned int>** hitTimeNanoSec,
                float** hitRate, float** hitRateErr)
{
  for(int i = 0; i < nrows; i++)
  {
    for(int j = 0; j < ncols; j++)
    {
      if(hitTimeSec[i][j].size() >= 2)
      {
        double Dt = (hitTimeSec[i][j][hitTimeSec[i][j].size()-1]-hitTimeSec[i][j][0])+(hitTimeNanoSec[i][j][hitTimeNanoSec[i][j].size()-1]-hitTimeNanoSec[i][j][0])*1e-9;
        if(Dt > 0.)
        {
          hitRate[i][j] = (float)hitCount[i][j]/Dt;
          if(hitCount[i][j])
            hitRateErr[i][j] = sqrt((float)hitCount[i][j])/Dt;
          else
            hitRateErr[i][j] = 0.;
        }
        else
        {
          hitRate[i][j] = -1.;
          hitRateErr[i][j] = 0.;
        }
        cout << hitRate[i][j] << " ";
      }
      else
        cout << 0 << " ";
    }
    cout << endl;
  }
  cout << endl;
}


void initArray(int** aPtr, int nrows, int ncols)
{
  for(int i = 0; i < nrows; i++)
    for(int j = 0; j < ncols; j++)
      aPtr[i][j] = 0;
}


void printModuleResults(int nrows, int ncols, int** aPtr)
{
  for(int i = 0; i < nrows; i++)
  {
    for(int j = 0; j < ncols; j++)
      cout << aPtr[i][j] << " ";
    cout << endl;
  }
  cout << endl;
}


void printModuleResults(int nrows, int ncols, float** aPtr)
{
  for(int i = 0; i < nrows; i++)
  {
    for(int j = 0; j < ncols; j++)
      cout << aPtr[i][j] << " ";
    cout << endl;
  }
  cout << endl;
}


void rebuildIndex(TTree *t, char* mainIdx, char* minorIdx)
{
  // Disable pre-existing index in the calib stats trees
  //  (Another reason ROOT is frustrating; we must manually do this)
  t->SetTreeIndex(0);
  
  // Build a new look-up index for the 'unfriendly' tree
  //  (Trigger number and detector id uniquely identify an entry)
  t->BuildIndex(mainIdx, minorIdx);
}
