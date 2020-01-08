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


#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <tclap/CmdLine.h>
#include "TBranchElement.h"
#include "TChain.h"
#include "TFile.h"
#include "TH2I.h"
#include "TLeaf.h"
#include "TTree.h"
#include "TROOT.h"
#include "TEntryList.h"
#include "PerCalibReadoutEvent/PerCalibReadoutHeader.h"

using namespace std;

bool hasMuonHits(int, int, int *[]);

bool indexInRange(int, int);

string getOutputPathName(string&, string);

string getChainName(string);

bool fexists(const char *);

int main(int argc, char** argv)
{
  gROOT->ProcessLine(".x $ROOTIOTESTROOT/share/load.C");
  
  
  TCLAP::CmdLine cmd("Command description message", ' ', "0.9");
  TCLAP::ValueArg<string> infArg("i", "input",
  "Input file path name", false,
  "file/recon.Neutrino.0021355.Physics.EH3-Merged.KUP-P._0124.root", "string");
  TCLAP::ValueArg<string> ofArg("o", "output",
  "Output file path", false, "file/", "string");
  cmd.add(infArg);
  cmd.add(ofArg);
  cmd.parse(argc, argv);
  
  string infpn = infArg.getValue();
  string ofp = ofArg.getValue();
  string ofpn;

  ofpn = getOutputPathName(ofp, infpn);
  cout << "processing file: " << endl << infpn << endl;
  cout << "results will be output to: " << endl << ofpn << endl << endl;
  
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
  
  //TChain *treeSpal = new TChain("/Event/Data/Physics/Spallation");
  //TChain *treeCalibStats = new TChain("/Event/Data/CalibStats");
  //TChain *treeCalib = new TChain("/Event/CalibReadout/CalibReadoutHeader");
  
  TChain treeSpal("/Event/Data/Physics/Spallation");
  TChain treeCalibStats("/Event/Data/CalibStats");
  TChain treeCalib("/Event/CalibReadout/CalibReadoutHeader");
  
  string chainName = getChainName(infpn);
  if(chainName == "")
  {
    cout << "incorrect input file name" << endl;
    return -1;
  }
  cout << "files to be chained: " << chainName << endl;
  
  int spalStatus = treeSpal.Add(chainName.c_str());
  int calibStatsStatus = treeCalibStats.Add(chainName.c_str());
  int calibStatus = treeCalib.Add(chainName.c_str());
  if(!(spalStatus&calibStatsStatus&calibStatus))
  {
    cout << "chaining trees failed" << endl;
    return -1;
  }
  
  string cutMuon = "hitRPC==1&&(hitAD1==1||hitAD2==1||hitAD3==1||hitAD4==1";
  cutMuon += "||hitIWS==1||hitOWS==1)";
  treeSpal.Draw(">>elist", cutMuon.c_str(), "entrylist");
  TEntryList *elist = (TEntryList*)gDirectory->Get("elist");


  int tMu_ns, tMu_s;
  /// Note site used in reconstructed data is diffrernt from site in DaqCrate.
  /// For offline convention, please see
  /// http://dayabay.bnl.gov/dox/Conventions/html/namespaceSite.html
  /// For daq convention, please see
  /// http://dayabay.bnl.gov/dox/DaqEvent/html/
  /// DaqCrate_8cc.html#ae27fb919fe6fb7c8a00e2293f5f416b0
  int triggerNumber_RPC, mSec, mNanoSec, site;
  PerCalibReadoutHeader* rh = 0;
  treeSpal.SetBranchAddress("tMu_ns", &tMu_ns);
  treeSpal.SetBranchAddress("tMu_s", &tMu_s);
  treeSpal.SetBranchAddress("triggerNumber_RPC", &triggerNumber_RPC);
  
  cout << "starting rebuilding tree indeces" << endl;
  treeCalibStats.BuildIndex((char *)"triggerNumber", (char *)"context.mDetId");
  treeCalib.BuildIndex((char *)"triggerTimeSec", (char *)"triggerTimeNanoSec");
  
    
  /// diagonostic steps of TEntryList for TChain
  int nrows, ncols;
  int ***layerHitCount = 0, **passMuonCount = 0;
  
  Long64_t listEntries = elist->GetN();
  Int_t treenum = 0;
  treeSpal.SetEntryList(elist);
  

  for (Long64_t el = 0; el < listEntries; el++)
  {
    Long64_t treeEntry = elist->GetEntryAndTree(el, treenum);
    Long64_t chainEntry = treeEntry + treeSpal.GetTreeOffset()[treenum];

    treeSpal.GetEntry(chainEntry);

//    cout << "muon sec: " << tMu_s << "\t\t";
//    cout << "muon nanosec: " << tMu_ns << endl;
//    cout << "RPC trigger number: " << triggerNumber_RPC << endl;
    treeCalibStats.GetEntryWithIndex(triggerNumber_RPC, 7);
    mSec = (int)treeCalibStats.GetLeaf("context.mTimeStamp.mSec")->
            GetValue();
    mNanoSec = (int)treeCalibStats.GetLeaf("context.mTimeStamp.mNanoSec")->
                GetValue();
//    cout << "RPC in muon sec: " << mSec << "\t";
//    cout << "RPC in muon nanosec: " << mNanoSec << endl;
//    cout << "time difference between muon and RPC: ";
//    cout << mNanoSec - tMu_ns << endl;
    if(mNanoSec - tMu_ns)
    {
      cout << "in event " << triggerNumber_RPC;
      cout << " RPC is not the first" << endl;
    }
    
    treeCalib.SetBranchAddress("CalibReadout_CalibReadoutHeader", &rh);

    treeCalib.GetEntryWithIndex(mSec, mNanoSec);
    if(!rh)
    {
      cout << "RPC entry not found in calibration tree" << endl;
      continue;
    }
    
    if(!layerHitCount||!passMuonCount)
    {
      /// row runs from 0..7 for near sites; 0..10 for far site
      /// column runs from 1..9
      if(site == 0x100)
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
      for(int i = 0; i < nrows; i++)
      {
        layerHitCount[i] = new int* [ncols];
        passMuonCount[i] = new int [ncols];
        for(int j = 0; j < ncols; j++)
          layerHitCount[i][j] = new int [4];
      }
      /// initialize
      for(int i = 0; i < nrows; i++)
        for(int j = 0; j < ncols; j++)
        {
          passMuonCount[i][j] = 0;
          for(int k = 0; k < 4; k++)
            layerHitCount[i][j][k] = 0;
        }
    }
    
    unsigned int nHitsRpc = rh->nHitsRpc;
//    cout << "number of hit rpc strips: " << nHitsRpc << endl;
    
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
        if(sIdx == nHitsRpc - 1)
          nStripThisMod.push_back(nStrip);
      }
      else
      {
        nStrip++;
        nModules++;
      }
      lastModId.first = thisModId.first;
      lastModId.second = thisModId.second;
    }
//    cout << "number of hit modules: " << nModules << endl;
//    cout << "number of hit strips per module: ";
    for(unsigned int ni = 0; ni < nStripThisMod.size(); ni++)
      cout << nStripThisMod[ni] << " ";
    cout << endl;
    
    
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
      
//      cout << "number of hit layers: " << nLayer[nm] << endl;
      if(nLayerThisMod >= 3)
      {
        if(indexInRange(nrows, hitModRow[nm])&&
        indexInRange(ncols, hitModCol[nm]-1))
        {
          passMuonCount[hitModRow[nm]][hitModCol[nm]-1]++;
          for(int lIdx = 0; lIdx < 4; lIdx++)
            if(getHit[lIdx])
              layerHitCount[hitModRow[nm]][hitModCol[nm]-1][lIdx]++;
          for(unsigned int ns = 0; ns < nStripThisMod[nm]; ns++)
          {
//            cout << "row: " << hitModRow[nm] << " ";
//            cout << "column: " << hitModCol[nm] << " ";
//            cout << "layer: " << hitLayer[nm][ns] << " ";
//            cout << "strip: " << hitStrip[nm][ns] << endl;
          }
        }
//        else
//          cout << "row or column number out of range" << endl;
      }
    }
    cout << endl;
  }
  
  
  //if(layerHitCount&&passMuonCount)
  //{
    ///// print results layer by layer
    //for(int k = 0; k < 4; k++)
    //{
      //for(int i = 0; i < nrows; i++)
      //{
        //for(int j = 0; j < ncols; j++)
          //cout << layerHitCount[i][j][k] << " ";
        //cout << endl;
      //}
      //cout << endl << endl;
    //}
    
    ///// print result of muons
    //for(int i = 0; i < nrows; i++)
    //{
      //for(int j = 0; j < ncols; j++)
        //cout << passMuonCount[i][j] << " ";
      //cout << endl;
    //}
    
    ///// Write results to TFile.
    //if(hasMuonHits(nrows, ncols, passMuonCount))
    //{
      //TFile outTF(ofpn.c_str(), "recreate");
      //TH2I *layerCount[4];
      //for(int l = 0; l < 4; l++)
        //layerCount[l] = new TH2I(Form("l%d", l),
                                  //Form("hit count for layer %d", l),
                                  //ncols, 0, ncols, nrows, 0, nrows);
  
      //TH2I muonCount("m", "number of muons", ncols, 0, ncols,
                      //nrows, 0, nrows);
      
      ///// In SetBinContent(), the first 2 arguments are bin "count", not
      ///// bin "number"
      //for(int k = 0; k < 4; k++)
        //for(int i = 0; i < nrows; i++)
          //for(int j = 0; j < ncols; j++)
            //layerCount[k]->SetBinContent(j+1, i+1, layerHitCount[i][j][k]);
  
      //for(int i = 0; i < nrows; i++)
        //for(int j = 0; j < ncols; j++)
          //muonCount.SetBinContent(j+1, i+1, passMuonCount[i][j]);
      
      //for(int k = 0; k < 4; k++)
        //layerCount[k]->Write();
      //muonCount.Write();
  
      //for(int k = 0; k < 4; k++)
        //delete layerCount[k];
    //}
  //}
  
  cout << endl;
  cout << "number of entries in spallation tree: " << treeSpal.GetEntries();
  cout << endl;
  
  return 0;
}




bool fexists(const char *filename)
{
  ifstream ifile(filename);
  return ifile;
}


string getOutputPathName(string &ofp, string ins)
{
  string ofn;
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
    ofn = ins;
    ofn = ofn.substr(ofn.rfind('/')+1, ofn.length()-1-ofn.rfind('/'));
    ofn = ofn.substr(ofn.find('.')+1, ofn.length()-1-ofn.find('.'));
    ofn = "eff." + ofn;
  }
  return ofp + ofn;
}


string getChainName(string infpn)
{
  string path, name;
  size_t slashPos = infpn.rfind('/');
  stringstream wildCardName;
  
  if(infpn.substr(infpn.length()-5, 5) != ".root")
  {
    wildCardName << "";
    return wildCardName.str();
  }
  
  /// separate path from file name
  if(slashPos == string::npos)
  {
    path = "./";
    name = infpn;
  }
  else
  {
    path = infpn.substr(0, slashPos+1);
    name = infpn.substr(slashPos+1, infpn.length()-1-slashPos);
  }
  
  /// replace file number with wild card
  wildCardName << path << name.substr(0, name.rfind('_')) << '*';
  wildCardName << name.substr(name.rfind('.'), 5);
  
  return wildCardName.str();
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
