#include <utility>

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

void EffMuonMultDet(string infn = "")
{
  gROOT->ProcessLine(".x $ROOTIOTESTROOT/share/load.C");
  gROOT->SetMacroPath("/home/sklin/Work/Analysis/RpcEffRec/CrossDetector/\
ScriptOnProduction");
  gROOT->ProcessLine(".L vecdict.h+");
  
  //const string inputPath = "../OfficialProduction/file/";
  //string fileName =
                  //"recon.Neutrino.0021361.Physics.EH2-Merged.KUP-P._0001.root";
  //fileName = inputPath + fileName;
  string fileName = "/home/sklin/RemoteMounts/PDSFDayaBayDaya/exp/dayabay/2011/p12a/NoTag/1223/recon.NoTag.0021029.Physics.EH1-Merged.P12A-P._0001.root";
  TFile f(fileName.c_str());

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
  /// note site used in reconstructed data is diffrernt from site in DaqCrate
  /// for offline convention, please see
  /// http://dayabay.bnl.gov/dox/Conventions/html/namespaceSite.html
  /// for daq convention, please see
  /// http://dayabay.bnl.gov/dox/DaqEvent/html/\
  /// DaqCrate_8cc.html#ae27fb919fe6fb7c8a00e2293f5f416b0
  int triggerNumber_RPC, mSec, mNanoSec, site, detector;
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
      mSec = treeCalibStats->GetLeaf("context.mTimeStamp.mSec")->GetValue();
      mNanoSec = treeCalibStats->GetLeaf("context.mTimeStamp.mNanoSec")
                  ->GetValue();
      cout << "RPC in muon sec: " << mSec << "\t";
      cout << "RPC in muon nanosec: " << mNanoSec << endl;

      treeCalib->SetBranchAddress("CalibReadout_CalibReadoutHeader", &rh);

      treeCalib->GetEntryWithIndex(mSec, mNanoSec);
      
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
      cout << "number of hit rpc strips: " << nHitsRpc << endl;
      
      /// 2 passes method: first pass to determine the number of
      /// hit modules in this trigger
      pair<unsigned int, unsigned int> lastModId;
      pair<unsigned int, unsigned int> thisModId;
      vector<unsigned int> nStripThisMod;
      unsigned int nModules = 0;
      unsigned int nStrip = 0;
      
      for(int sIdx = 0; sIdx < nHitsRpc; sIdx++)
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
      unsigned int stripIdx = 0;
      vector<unsigned int> nLayer;
      for(unsigned int nm = 0; nm < nModules; nm++)
      {
        layerThisMod.clear();
        stripThisMod.clear();
        bool getHit[4] = {false};
        for(unsigned int ns = 0; ns < nStripThisMod[nm]; ns++)
        {
          if(ns == 0)
          {
            hitModRow.push_back(rh->rpcRow[stripIdx]);
            hitModCol.push_back(rh->rpcColumn[stripIdx]);
          }
          layerThisMod.push_back(rh->rpcLayer[stripIdx]);
          getHit[rh->rpcLayer[stripIdx]-1] = true;
          stripThisMod.push_back(rh->rpcStrip[stripIdx]);
          stripIdx++;
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
          passMuonCount[hitModRow[nm]][hitModCol[nm]-1]++;
          for(int lIdx = 0; lIdx < 4; lIdx++)
            if(getHit[lIdx])
              layerHitCount[hitModRow[nm]][hitModCol[nm]-1][lIdx]++;
        }
        for(unsigned int ns = 0; ns < nStripThisMod[nm]; ns++)
        {
          cout << "row: " << hitModRow[nm] << " ";
          cout << "column: " << hitModCol[nm] << " ";
          cout << "layer: " << hitLayer[nm][ns] << " ";
          cout << "strip: " << hitStrip[nm][ns] << endl;
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
    TFile outTF("test.root", "recreate");
    TH2I *layerCount[4];
    for(int l = 0; l < 4; l++)
      layerCount[l] = new TH2I(Form("l%d", l),
                                Form("hit count for layer %d", l),
                                ncols, 0, ncols, nrows, 0, nrows);

    TH2I muonCount("m", "number of muons", ncols, 0, ncols,
                    nrows, 0, nrows);
    
    /// In SetBinContent(), the first 2 arguments are bin "count", not
    /// bin "number"
    for(int k = 0; k < 4; k++)
      for(int i = 0; i < nrows; i++)
        for(int j = 0; j < ncols; j++)
          layerCount[k]->SetBinContent(j+1, i+1, layerHitCount[i][j][k]);

    for(int i = 0; i < nrows; i++)
      for(int j = 0; j < ncols; j++)
        muonCount.SetBinContent(j+1, i+1, passMuonCount[i][j]);
    
    for(k = 0; k < 4; k++)
      layerCount[k]->Write();
    muonCount.Write();
    
    delete [] layerCount;
  }
  //cout.setf(std::ios::boolalpha);
  //cout << hasMuonHits(nrows, ncols, passMuonCount) << endl;
  
  f.Close();
  return;
}
