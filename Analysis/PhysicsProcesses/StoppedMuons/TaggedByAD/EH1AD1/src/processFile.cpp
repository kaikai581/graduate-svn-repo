#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include "CalibEvent.hpp"
#include "PerCalibReadoutEvent/PerCalibReadoutHeader.h"
#include "RecADEvent.hpp"
#include "structopt.hpp"
#include "TaggedMuonEvent.hpp"
#include "TEntryList.h"
#include "TFile.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TLeaf.h"
#include "TreeVariables.hpp"
#include "TTree.h"
#include "utilities.hpp"


using namespace std;


void assignBranch(TTree&);


void processFile(string ifpn)
{
  option_t opt;

  unsigned int runNumber = atoi(getRunNumber(ifpn).c_str());
  unsigned int fileNumber = atoi(getFileNumber(ifpn).c_str());

  /// exclude non-physics runs
  if(!runNumber) return;
  cout << "processing run " << runNumber << " file " << fileNumber << endl;


  /// open the TFile with the option set by overwrite flag
  string openOpt;
  if(opt.owFlag) openOpt = "update";
  else           openOpt = "read";


  /// get trees: CalibReadoutHeader and Spallation
  /// rebuild index for retrieving data
  TFile fif(ifpn.c_str(), openOpt.c_str());
  TTree* tSpall = (TTree*)fif.Get("/Event/Data/Physics/Spallation");
  TTree* tCalibStats = (TTree*)fif.Get("/Event/Data/CalibStats");
  tCalibStats->SetTreeIndex(0);
  tCalibStats->BuildIndex("triggerNumber","context.mDetId");
  

  TTree* tCalib = (TTree*)fif.Get("/Event/CalibReadout/CalibReadoutHeader");
  tCalib->SetTreeIndex(0);
  tCalib->BuildIndex("triggerNumber", "context.mDetId");
  // Initialize PerCalibReadoutHeader!!!
  // This GOD DAMN trifle cost me ONE WEEK!!!
  PerCalibReadoutHeader* rh = 0;
  tCalib->SetBranchAddress("CalibReadout_CalibReadoutHeader", &rh);
  
  
  
  TTree* tAdRec = (TTree*)fif.Get("/Event/Rec/AdSimple");
  tAdRec->SetTreeIndex(0);
  tAdRec->BuildIndex("triggerNumber", "context.mDetId");

  if(!tSpall)
  {
    cerr << "spallation tree not found" << endl;
    return;
  }
  
  if(!tCalibStats)
  {
    cerr << "calibration statistics tree not found" << endl;
    return;
  }
  
  
  if(!tCalib)
  {
    cerr << "calibration statistics tree not found" << endl;
    return;
  }
  


  /*
   * find muon candidates
   */
  /// For each event in the spallation tree with AD hits, if it has energy
  /// 80 < E < 700 MeV, it is regarded as a stopped muon candidate.
  
  /// In spallation tree, find events with either AD hits.
  string anyADCut("hitAD1==1||hitAD2==1||hitAD3==1||hitAD4==1");
  tSpall->Draw(">>elist", anyADCut.c_str(), "entrylist");
  TEntryList *elist = (TEntryList*)gDirectory->Get("elist");
  
  /// output container from AD's ID to a vector of stopped muon candidates
  map<short, vector<RecADEvent> > stoppedMuonsInDs;
  map<short, vector<TaggedMuonEvent> > spallMuInDs;
  
  Long64_t listEntries = elist->GetN();
  /// reference time so that time stamps can be simply represented with
  /// an integer
  long t0;
  /// first pass: find all stopped muon candidates
	for(Long64_t entry = 0; entry < listEntries; entry++)
  {
    
    if(opt.nEvt>0 && entry>=opt.nEvt) break;
    TaggedMuonEvent thisTag(tSpall, elist->GetEntry(entry));
    
    if(entry == 0)
    {
      long sec = (long)tSpall->GetLeaf("context.mTimeStamp.mSec")->GetValue();
      long nanosec = (long)tSpall->GetLeaf("context.mTimeStamp.mNanoSec")
                                 ->GetValue();
      t0 = sec*1000000000+nanosec;
      
      if(opt.dbgFlag)
        cout << "reference time: " << t0 << endl;
    }
    
    for(int adId = 1; adId <= 4; adId++)
    {
      if(thisTag.triggerNumberAD[adId-1] > 0)
      {

        RecADEvent curEvt;
        curEvt.getEntryWithIndex(tAdRec, thisTag.triggerNumberAD[adId-1], adId);

        if(curEvt.isStoppedMuon(tCalibStats))
        {
          curEvt.fillNPESum(tCalibStats);
          map<short, vector<RecADEvent> >::iterator it = stoppedMuonsInDs.find(adId);
          if(it == stoppedMuonsInDs.end())
            stoppedMuonsInDs[adId] = vector<RecADEvent>(1, curEvt);
          else
            stoppedMuonsInDs[adId].push_back(curEvt);
          
          map<short, vector<TaggedMuonEvent> >::iterator it2 = spallMuInDs.find(adId);
          if(it2 == spallMuInDs.end())
            spallMuInDs[adId] = vector<TaggedMuonEvent>(1, thisTag);
          else
            spallMuInDs[adId].push_back(thisTag);
          
          /// print muon candidates debug information
          if(opt.dbgFlag)
          {
            for(int i = 0; i < 4; i++)
            {
              //cout << "triggerNumber_AD" << i+1 << ": ";
              //cout << thisTag.triggerNumberAD[i] << endl;
            }
            cout << "triggerNumber_RPC: " << spallMuInDs[adId][spallMuInDs[adId].size()-1].triggerNumberRPC << endl;
          }
        }
      }
    }

  }
  
  if(opt.dbgFlag)
  {
    for(map<short, vector<RecADEvent> >::iterator it = stoppedMuonsInDs.begin();
        it != stoppedMuonsInDs.end(); it++)
    {
      cout << "number of prompt muon candidates in AD" << it->first << ": ";
      cout << it->second.size() << endl;
    }
  }
  
  
  /// second pass: find Michel electrons
  Long64_t nentries = tAdRec->GetEntries();

  map<short, unsigned int> muPtr;
  for(map<short, vector<RecADEvent> >::iterator it = stoppedMuonsInDs.begin();
      it != stoppedMuonsInDs.end(); it++)
    muPtr[it->first] = stoppedMuonsInDs[it->first].size()-1;
  
  map<short, map<unsigned int, vector<RecADEvent> > > michelInDs;
  
  
  TFile fof(Form("%s", getOutputPathName(ifpn, opt.outputPath,
            string("stoppedmu")).c_str()), "recreate");
  TTree tStoppedMu("tsmu", "tree of stopped muons");
  assignBranch(tStoppedMu);
  TreeVariables tv;
  
  for(Long64_t entry = nentries-1; entry >= 0; entry--)
  {
    if(opt.nEvt>0 && entry<=nentries-1-opt.nEvt) break;
    
    RecADEvent curEvt(tAdRec, tCalibStats, entry);
    /// exclude detectors other than ADs
    if(curEvt.detector > 4) continue;
    
    vector<RecADEvent> muonsInAnAD = stoppedMuonsInDs[curEvt.detector];
    
    
    while(curEvt.position2Window(muonsInAnAD[muPtr[curEvt.detector]].triggerTimeInNs) == RecADEvent::left && muPtr[curEvt.detector] >= 1) muPtr[curEvt.detector]--;
    if(curEvt.position2Window(muonsInAnAD[muPtr[curEvt.detector]].triggerTimeInNs) == RecADEvent::in)
    {
      //CalibEvent ce(tCalib, spallMuInDs[curEvt.detector][muPtr[curEvt.detector]].triggerNumberRPC, 7);
      int byte = tCalib->GetEntryWithIndex(spallMuInDs[curEvt.detector][muPtr[curEvt.detector]].triggerNumberRPC, 7);
      CalibEvent ce(rh, byte);
      
      if(opt.dbgFlag)
      {
        if(curEvt.isMichel())
        {
          cout << "found Michel electrons in AD" << curEvt.detector << endl;
          cout << "triggerNumber of Michel: " << curEvt.triggerNumber << endl;
          
          cout << "RPC trigger Number: " << spallMuInDs[curEvt.detector][muPtr[curEvt.detector]].triggerNumberRPC << endl;
          cout << "RPC bytes read in CalibReadoutHeader: " << ce.byte << endl;
          cout << "RPC trigger time: " << ce.triggerTimeInNs << endl;
          if(ce.byte > 0) cout << "nhits RPC: " << ce.rh->nHitsRpc << endl;
          else cout << "nhits RPC: 0" << endl;
          
          
          cout << endl;
        }
      }
      
      /// write to tree
      tv.detector = curEvt.detector;
      tv.triggerNumberMu = muonsInAnAD[muPtr[curEvt.detector]].triggerNumber;
      tv.triggerTimeMu = muonsInAnAD[muPtr[curEvt.detector]].triggerTimeInNs;
      tv.muTime = muonsInAnAD[muPtr[curEvt.detector]].triggerTimeInNs-t0;
      tv.nPESumMu = muonsInAnAD[muPtr[curEvt.detector]].nPESum;
      tv.xmu = muonsInAnAD[muPtr[curEvt.detector]].x;
      tv.ymu = muonsInAnAD[muPtr[curEvt.detector]].y;
      tv.zmu = muonsInAnAD[muPtr[curEvt.detector]].z;
      tv.energyMu = muonsInAnAD[muPtr[curEvt.detector]].energy;
      tv.triggerNumberE = curEvt.triggerNumber;
      tv.triggerTimeE = curEvt.triggerTimeInNs;
      tv.eTime = curEvt.triggerTimeInNs-t0;
      tv.nPESumE = curEvt.nPESum;
      tv.xe = curEvt.x;
      tv.ye = curEvt.y;
      tv.ze = curEvt.z;
      tv.energyE = curEvt.energy;
      /// root histograms cannot handle subtractions of long integers
      if(curEvt.isMichel())
        tv.dt = curEvt.triggerTimeInNs-muonsInAnAD[muPtr[curEvt.detector]].triggerTimeInNs;
      else tv.dt = -1;
      tv.isMichel = curEvt.isMichel();
      tv.triggerNumberRpc = spallMuInDs[curEvt.detector][muPtr[curEvt.detector]].triggerNumberRPC;
      tv.triggerTimeRpc = ce.triggerTimeInNs;
      tv.rpcTime = ce.triggerTimeInNs-t0;
      tv.nModules = ce.mRpcReadout.size();
      /// clear vector containers
      tv.rpcRow.clear();
      tv.rpcCol.clear();
      tv.rpcLayer.clear();
      tv.rpcStrip.clear();
      tv.rpcFromRot.clear();

      for(CalibEvent::mEvent::iterator it = ce.mRpcReadout.begin(); it != ce.mRpcReadout.end(); it++)
      {
        tv.rpcRow.push_back(it->first.first);
        tv.rpcCol.push_back(it->first.second);
        
        vector<unsigned int> firedLayers;
        for(unsigned int l = 0; l < it->second.size(); l++)
        {
          firedLayers.push_back(l+1);
          tv.rpcStrip.push_back(it->second);
        }
        tv.rpcLayer.push_back(firedLayers);
        
        tv.rpcFromRot.push_back(ce.mRpcFromRot[it->first]);
      }
      tStoppedMu.Fill();
    }

  }
  
  tStoppedMu.Write();
  fof.Close();

}







void assignBranch(TTree& tr)
{
  TreeVariables tv;
  
  tr.Branch("detector", &tv.detector, "delayed/S");
  tr.Branch("triggerNumberMu", &tv.triggerNumberMu, "triggerNumberMu/i");
  tr.Branch("energyMu", &tv.energyMu, "energyMu/F");
  tr.Branch("triggerTimeMu", &tv.triggerTimeMu, "triggerTimeMu/L");
  tr.Branch("nPESumMu", &tv.nPESumMu, "nPESumMu/F");
  tr.Branch("xmu", &tv.xmu, "xmu/F");
  tr.Branch("ymu", &tv.ymu, "ymu/F");
  tr.Branch("zmu", &tv.zmu, "zmu/F");
  tr.Branch("muTime", &tv.muTime, "muTime/D");
  tr.Branch("triggerNumberE", &tv.triggerNumberE, "triggerNumberE/i");
  tr.Branch("energyE", &tv.energyE, "energyE/F");
  tr.Branch("triggerTimeE", &tv.triggerTimeE, "triggerTimeE/L");
  tr.Branch("eTime", &tv.eTime, "eTime/D");
  tr.Branch("nPESumE", &tv.nPESumE, "nPESumE/F");
  tr.Branch("xe", &tv.xe, "xe/F");
  tr.Branch("ye", &tv.ye, "ye/F");
  tr.Branch("ze", &tv.ze, "ze/F");
  //tr.Branch("dt", &tv.dt, "dt/L");
  tr.Branch("isMichel", &tv.isMichel, "isMichel/O");
  
  tr.Branch("triggerTimeRpc", &tv.triggerTimeRpc, "triggerTimeRpc/L");
  tr.Branch("rpcTime", &tv.rpcTime, "rpcTime/D");
  tr.Branch("triggerNumberRpc", &tv.triggerNumberRpc, "triggerNumberRpc/I");
  tr.Branch("nModules", &tv.nModules, "nModules/i");
  tr.Branch("rpcRow", &tv.rpcRow);
  tr.Branch("rpcCol", &tv.rpcCol);
  tr.Branch("rpcLayer", &tv.rpcLayer);
  tr.Branch("rpcStrip", &tv.rpcStrip);
  tr.Branch("rpcFromRot", &tv.rpcFromRot);
}

