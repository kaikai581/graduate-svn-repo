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
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
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

bool fexists(const char *filename)
{
  ifstream ifile(filename);
  return ifile;
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

void noRpcList(string);

int main(int argc, char** argv)
{
  gROOT->SetMacroPath("/u/sklin/Work/Analysis/RPC/RpcEffRec/OfficialProduction\
PDSF/src");
  gROOT->ProcessLine(".L vecdict.h+");
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
    ofn = "eff." + ofn;
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
    {
      //umask(0);
      mkdir(ofp.c_str(), S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
    }
  }
  
  TFile f(infpn.c_str());

  TTree *treeSpal = (TTree*)f.Get("/Event/Data/Physics/Spallation");
  TTree *treeCalibStats = (TTree*)f.Get("/Event/Data/CalibStats");
  TTree *treeCalib = (TTree*)f.Get("/Event/CalibReadout/CalibReadoutHeader");
  
  if(!treeSpal)
  {
    cout << "spallation tree not found" << endl;
    return -1;
  }
  
  
  string cutMuon = "hitRPC==1&&(hitAD1==1||hitAD2==1||hitAD3==1||hitAD4==1";
  cutMuon += "||hitIWS==1||hitOWS==1)";
  treeSpal->Draw(">>elist", cutMuon.c_str(), "entrylist");
  TEntryList *elist = (TEntryList*)f.Get("elist");

  int tMu_ns, tMu_s;
  /// Note site used in reconstructed data is diffrernt from site in DaqCrate.
  /// For offline convention, please see
  /// http://dayabay.bnl.gov/dox/Conventions/html/namespaceSite.html
  /// For daq convention, please see
  /// http://dayabay.bnl.gov/dox/DaqEvent/html/
  /// DaqCrate_8cc.html#ae27fb919fe6fb7c8a00e2293f5f416b0
  int triggerNumber_RPC, mSec, mNanoSec, site;
//  vector<unsigned int> &row, &column, &layer, &strip;
  PerCalibReadoutHeader* rh = 0;
  treeSpal->SetBranchAddress("tMu_ns", &tMu_ns);
  treeSpal->SetBranchAddress("tMu_s", &tMu_s);
  treeSpal->SetBranchAddress("triggerNumber_RPC", &triggerNumber_RPC);
  
  rebuildIndex(treeCalibStats, (char *)"triggerNumber" ,
              (char *)"context.mDetId");
  rebuildIndex(treeCalib, (char *)"triggerTimeSec" ,
              (char *)"triggerTimeNanoSec");
  
  
  int nrows, ncols;
  int ***layerHitCount = 0, **passMuonCount = 0;
  int **threeHitCount = 0, **fourHitCount = 0;
  int nentry = treeSpal->GetEntries();
  for(int entry = 0; entry < nentry; entry++)
  {
    //if(entry>=25)
      //break;
    if(elist->Contains(entry, treeSpal))
    {
      treeSpal->GetEntry(entry);
      cout << "muon sec: " << tMu_s << "\t\t";
      cout << "muon nanosec: " << tMu_ns << endl;
      cout << "RPC trigger number: " << triggerNumber_RPC << endl;
      treeCalibStats->GetEntryWithIndex(triggerNumber_RPC, 7);
      mSec = (int)treeCalibStats->GetLeaf("context.mTimeStamp.mSec")->GetValue();
      mNanoSec = (int)treeCalibStats->GetLeaf("context.mTimeStamp.mNanoSec")
                  ->GetValue();
      cout << "RPC in muon sec: " << mSec << "\t";
      cout << "RPC in muon nanosec: " << mNanoSec << endl;
      cout << "time difference between muon and RPC: ";
      cout << mNanoSec - tMu_ns << endl;
      if(mNanoSec - tMu_ns)
      {
        cout << "in event " << triggerNumber_RPC;
        cout << " RPC is not the first" << endl;
      }
      //treeCalib->GetEntry(11);
      treeCalib->SetBranchAddress("CalibReadout_CalibReadoutHeader", &rh);
//      vector<unsigned int>* strip = (vector<unsigned int>*)((TBranchElement*)treeCalib->GetBranch("rpcStrip"))->GetObject();
      treeCalib->GetEntryWithIndex(mSec, mNanoSec);
      if(!rh)
      {
        cout << "RPC entry not found in calibration tree" << endl;
        continue;
      }
      
      site = (int)treeCalib->GetLeaf("site")->GetValue();
      cout << "site: " << site << endl;
      //detector = treeCalib->GetLeaf("detector")->GetValue();
      //cout << "detector: " << detector << endl;
      
      
      if(!layerHitCount||!passMuonCount)
      {
        /// row runs from 0..7 for near sites; 0..10 for far site
        /// column runs from 1..9
        if(site == 4)
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
      
//      row = (vector<unsigned int>*)treeCalib->GetLeaf("rpcRow")->
//              GetValuePointer();
      //column = (vector<unsigned int>*)treeCalib->GetLeaf("rpcColumn")->
              //GetValuePointer();
      //layer = (vector<unsigned int>*)treeCalib->GetLeaf("rpcLayer")->
              //GetValuePointer();
      //strip = (vector<unsigned int>*)treeCalib->GetLeaf("rpcStrip")->
              //GetValuePointer();
              
      //vector<unsigned int>& row = getLeafVectorUI("rpcRow", treeCalib);
      //vector<unsigned int>& column = getLeafVectorUI("rpcColumn", treeCalib);
      //vector<unsigned int>& layer = getLeafVectorUI("rpcLayer", treeCalib);
      //vector<unsigned int>& strip = getLeafVectorUI("rpcStrip", treeCalib);

      //cout << "number of hit strips: " << strip->size() << endl;
      //cout << "row of readout rpc moduls: " << rh->rpcRow.size() << endl;
      
      //for(int j = 0; j < strip.size(); j++)
      //{
        //unsigned int thisrow = row[j];
        //unsigned int thiscol = column[j];
        //unsigned int thislayer = layer[j];
        //cout << "row: " << thisrow << "\t\t";
        //cout << "col: " << thiscol << endl;
        //cout << "number of layers: " << thislayer << endl;
        
        //if(thisrow < 0 || thisrow >= nrows)
        //{
          //cerr << "row number out of range" << endl;
          //continue;
        //}
        
        //if(thiscol < 1 || thiscol > ncols)
        //{
          //cerr << "column number out of range" << endl;
          //continue;
        //}
        
        //if(thislayer >= 3)
        //{
          //passMuonCount[thisrow][thiscol-1]++;
          //bool getHit[4] = {false};
          //for(int k = 0; k < (*strip)[j].size(); k++)
            //getHit[(*strip)[j][k]/8] = true;
          //for(int k = 0; k < 4; k++)
            //if(getHit[k])
              //layerHitCount[thisrow][thiscol-1][k]++;
        //}
        
        //for(int k = 0; k < (*strip)[j].size(); k++)
          //cout << (*strip)[j][k] << " ";
        //cout << endl;
      //}
      unsigned int nHitsRpc = rh->nHitsRpc;
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
      cout << "number of hit modules: " << nModules << endl;
      cout << "number of hit strips per module: ";
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
        
        cout << "number of hit layers: " << nLayer[nm] << endl;
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
              cout << "row: " << hitModRow[nm] << " ";
              cout << "column: " << hitModCol[nm] << " ";
              cout << "layer: " << hitLayer[nm][ns] << " ";
              cout << "strip: " << hitStrip[nm][ns] << endl;
            }
            /// record whether the readout is 3 or 4 hit
            if(nLayerThisMod == 3)
              threeHitCount[hitModRow[nm]][hitModCol[nm]-1]++;
            else if(nLayerThisMod == 4)
              fourHitCount[hitModRow[nm]][hitModCol[nm]-1]++;
          }
          else
            cout << "row or column number out of range" << endl;
        }
      }
      cout << endl;
    }
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
  for(int i = 0; i < nrows; i++)
  {
    for(int j = 0; j < ncols; j++)
      cout << passMuonCount[i][j] << " ";
    cout << endl;
  }

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
      }
    
    for(int k = 0; k < 4; k++)
      layerCount[k]->Write();
    muonCount.Write();
    hThreeHit.Write();
    hFourHit.Write();

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
