#include <iostream>
#include <string>
#include <vector>
#include <tclap/CmdLine.h>
#include "TBranchElement.h"
#include "TFile.h"
#include "TLeaf.h"
#include "TTree.h"
#include "TROOT.h"
#include "TEntryList.h"

using namespace std;

void rebuildIndex(TTree*, char*, char*);

vector<int>& getLeafVectorI(const char* leafName, TTree* tree){
  void* objPtr = (dynamic_cast<TBranchElement*>(tree->GetBranch(leafName)))->GetObject();
  return *((vector<int>*)(objPtr));  
}

vector<vector<int> >* getLeafVectorVectorI(const char* leafName, TTree* tree){
  void* objPtr = (dynamic_cast<TBranchElement*>(tree->GetBranch(leafName)))->GetObject();
  return (vector<vector<int> >*)(objPtr);  
}

int main()
{
  gROOT->SetMacroPath("/home/sklin/Data/sklin/Data/Analysis/RpcEffRec/\
CrossDetector/MyOwnProduction/src");
  gROOT->ProcessLine(".L vecdict.h+");

	TCLAP::CmdLine cmd("Command description message", ' ');
  const string inputPath = "file/";
  string fileName =
                  "recon.Neutrino.0017656.Physics.EH1-Merged.P12A-P._0001.root";
  fileName = inputPath + fileName;
  TFile f(fileName.c_str());
  fileName = "file/";
  fileName += "rpcraw.Neutrino.0017656.Physics.EH1-Merged.SFO-1._0001.root";

  TFile fCalib(fileName.c_str());

  TTree *treeSpal = (TTree*)f.Get("/Event/Data/Physics/Spallation");
  TTree *treeCalibStats = (TTree*)f.Get("/Event/Data/CalibStats");
  TTree *treeCalib = (TTree*)fCalib.Get("t");
  
  if(!treeSpal)
  {
    cout << "spallation tree not found" << endl;
    return -1;
  }
  
  
  string cutMuon = "hitRPC==1&&(hitAD1==1||hitAD2==1||hitAD3==1||hitAD4==1";
  cutMuon += "||hitIWS==1||hitOWS==1)";
  f.cd();
  treeSpal->Draw(">>elist", cutMuon.c_str(), "entrylist");
  TEntryList *elist = (TEntryList*)f.Get("elist");

  int tMu_ns, tMu_s;
  float dtAD1_ms, dtAD2_ms, dtAD3_ms, dtAD4_ms, dtIWS_ms, dtOWS_ms, dtRPC_ms;
  int triggerNumber_RPC, mSec, mNanoSec, roNum, site;
  vector<vector<int> >* strip;
  vector<int> *row, *col, *nLayers;
  treeSpal->SetBranchAddress("tMu_ns", &tMu_ns);
  treeSpal->SetBranchAddress("tMu_s", &tMu_s);
  treeSpal->SetBranchAddress("dtAD1_ms", &dtAD1_ms);
  treeSpal->SetBranchAddress("dtAD2_ms", &dtAD2_ms);
  treeSpal->SetBranchAddress("dtAD3_ms", &dtAD3_ms);
  treeSpal->SetBranchAddress("dtAD4_ms", &dtAD4_ms);
  treeSpal->SetBranchAddress("dtIWS_ms", &dtIWS_ms);
  treeSpal->SetBranchAddress("dtOWS_ms", &dtOWS_ms);
  treeSpal->SetBranchAddress("dtRPC_ms", &dtRPC_ms);
  treeSpal->SetBranchAddress("triggerNumber_RPC", &triggerNumber_RPC);
  
  rebuildIndex(treeCalibStats, (char *)"triggerNumber" ,
              (char *)"context.mDetId");
  rebuildIndex(treeCalib, (char *)"triggerTimeSec" ,
              (char *)"triggerTimeNanoSec");
  //treeCalibStats->SetBranchAddress("context.mTimeStamp.mSec", &mSec);
  //treeCalibStats->SetBranchAddress("context.mTimeStamp.mNanoSec", &mNanoSec);
  //treeCalibStats->GetEntryWithIndex(-1,7);
  //cout << mSec << " " << mNanoSec << endl;
  //mSec = treeCalibStats->GetLeaf("context.mTimeStamp.mSec")->GetValue();
  //mNanoSec = treeCalibStats->GetLeaf("context.mTimeStamp.mNanoSec")->GetValue();
  //cout << mSec << " " << mNanoSec << endl;
  //treeCalib->SetBranchAddress("hitRow", &row);
  //treeCalib->SetBranchAddress("hitCol", &col);
  //treeCalib->SetBranchAddress("nLayers", &nLayers);
  //treeCalib->SetBranchAddress("stripId", &strip);
  
  
  int nrows, ncols;
  int ***layerHitCount = 0, **passMuonCount = 0;
  int nentry = treeSpal->GetEntries();
  for(int i = 0; i < nentry; i++)
  {
    //if(i>=25)
      //break;
    if(elist->Contains(i, treeSpal))
    {
      treeSpal->GetEntry(i);
      cout << "muon sec: " << tMu_s << "\t\t";
      cout << "muon nanosec: " << tMu_ns << endl;
      //cout << dtAD1_ms << " " << dtAD2_ms << " " << dtAD3_ms << " ";
      //cout << dtAD4_ms << " " << dtIWS_ms << " " << dtOWS_ms << " ";
      //cout << dtRPC_ms << endl;
      cout << "RPC trigger number: " << triggerNumber_RPC << endl;
      treeCalibStats->GetEntryWithIndex(triggerNumber_RPC, 7);
      mSec = treeCalibStats->GetLeaf("context.mTimeStamp.mSec")->GetValue();
      mNanoSec = treeCalibStats->GetLeaf("context.mTimeStamp.mNanoSec")
                  ->GetValue();
      cout << "RPC in muon sec: " << mSec << "\t";
      cout << "RPC in muon nanosec: " << mNanoSec << endl;
      treeCalib->GetEntryWithIndex(mSec, mNanoSec);
      roNum = treeCalib->GetLeaf("roNum")->GetValue();
      site = treeCalib->GetLeaf("site")->GetValue();
      cout << "site: " << site << endl;
      
      
      if(!layerHitCount||!passMuonCount)
      {
        /// row runs from 0..7 for near sites; 0..10 for far site
        /// column runs from 1..9
        if(site == 3)
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
      
      
      strip = (vector<vector<int> >*)treeCalib->
              GetLeaf("stripId")->GetValuePointer();
      row = (vector<int>*)treeCalib->GetLeaf("hitRow")->GetValuePointer();
      col = (vector<int>*)treeCalib->GetLeaf("hitCol")->GetValuePointer();
      nLayers = (vector<int>*)treeCalib->GetLeaf("nLayers")->GetValuePointer();

      //strip = getLeafVectorVectorI("stripId", treeCalib);
      //row = getLeafVectorI("hitRow", treeCalib);
      //col = getLeafVectorI("hitCol", treeCalib);
      //nLayers = getLeafVectorI("nLayers", treeCalib);
      
      if(!strip)
      {
        cout << "RPC entry not found in calibration tree" << endl;
        continue;
      }
      
      for(int j = 0; j < strip->size(); j++)
      {
        int thisrow = (*row)[j];
        int thiscol = (*col)[j];
        int thisnl = (*nLayers)[j];
        cout << "row: " << thisrow << "\t\t";
        cout << "col: " << thiscol << endl;
        cout << "number of layers: " << thisnl << endl;
        
        if(thisrow < 0 || thisrow >= nrows)
        {
          cerr << "row number out of range" << endl;
          continue;
        }
        
        if(thiscol < 1 || thiscol > ncols)
        {
          cerr << "column number out of range" << endl;
          continue;
        }
        
        if(thisnl >= 3)
        {
          passMuonCount[thisrow][thiscol-1]++;
          bool getHit[4] = {false};
          for(int k = 0; k < (*strip)[j].size(); k++)
            getHit[(*strip)[j][k]/8] = true;
          for(int k = 0; k < 4; k++)
            if(getHit[k])
              layerHitCount[thisrow][thiscol-1][k]++;
        }
        
        for(int k = 0; k < (*strip)[j].size(); k++)
          cout << (*strip)[j][k] << " ";
        cout << endl;
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

  cout << endl;
  cout << "number of entries in spallation tree: " << nentry << endl;
  
  f.Close();
  return 0;
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
