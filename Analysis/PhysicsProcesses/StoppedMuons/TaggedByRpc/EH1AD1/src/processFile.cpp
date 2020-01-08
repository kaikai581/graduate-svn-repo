#include <algorithm>
#include <cstdlib>
#include <iostream>
#include "AdEvent.hpp"
#include "PerCalibReadoutEvent/PerCalibReadoutHeader.h"
#include "structopt.hpp"
#include "TEntryList.h"
#include "TFile.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TLeaf.h"
#include "TreeVariables.hpp"
#include "TTree.h"
#include "utilities.hpp"


using namespace std;


bool aboveAD1(PerCalibReadoutHeader*);
void assignBranch(TTree&);
//void findMichelElectrons(TTree*, AdEvent&, long, vector<AdEvent>&);
//void findMichelElectrons(TTree*, AdEvent&, vector<AdEvent>&);
void findMichelElectrons(TTree*, vector<vector<AdEvent> >&,
     vector<vector<AdEvent> >&);
void findStoppedMuons(TTree*, TTree*, long, long, long&, vector<AdEvent>&);
bool isFlasher(TTree*);


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
  TTree* tCalibStat = (TTree*)fif.Get("/Event/Data/CalibStats");
  tCalibStat->SetTreeIndex(0);
  tCalibStat->BuildIndex("triggerNumber","context.mDetId");
  TTree* tCalib = (TTree*)fif.Get("/Event/CalibReadout/CalibReadoutHeader");
  tCalib->SetTreeIndex(0);
  tCalib->BuildIndex("triggerNumber","context.mDetId");
  TTree* tAd1Rec = (TTree*)fif.Get("/Event/Rec/AdSimple");
  tAd1Rec->SetTreeIndex(0);
  tAd1Rec->BuildIndex("triggerNumber", "context.mDetId");

  if(!tSpall)
  {
    cerr << "spallation tree not found" << endl;
    return;
  }
  
  if(!tCalibStat)
  {
    cerr << "calibration statistics tree not found" << endl;
    return;
  }
  
  if(!tCalib)
  {
    cerr << "calibration tree not found" << endl;
    return;
  }


	/// In spallation tree, find events with both AD1 and RPC hit.
  /// Also by Daniel's recipe it is required nPESum_AD1>3000 &&
  /// nPESum_AD1<140000
  tSpall->Draw(">>elist", "hitAD1==1&&hitRPC==1&&calib_nPESum_AD1>3000&&calib_nPESum_AD1<140000", "entrylist");
  TEntryList *elist = (TEntryList*)gDirectory->Get("elist");

  /// dataset level storage
  PerCalibReadoutHeader* rh = 0;
  tCalib->SetBranchAddress("CalibReadout_CalibReadoutHeader", &rh);

  /// pin down RPC muon times
  
  /// time container in nanosecond
  vector<long>   rpcMuonTimeNs;
  
  
  TFile fof(Form("%s", getOutputPathName(ifpn, opt.outputPath,
            string("mudecay")).c_str()), "recreate");
  TTree tMu("tmu", "tree of muon decay");
  assignBranch(tMu);

  /*
   * find muon candidates
   */
  /// For each RPC tagged muons, store the AD trigger number of any events
  /// within 200 us from the RPC trigger time.
  /// The following container stores stopped muon data
  /// There could be multiple muon and Michel electron candidates.
  /// Thus the vector containers in the pair.
  vector<pair<vector<AdEvent>, vector<AdEvent> > > dsStoppedMuons;
  vector<vector<AdEvent> > muonsInDs, mesInDs;
  
  Long64_t listEntries = elist->GetN();
  long ad1Entry = 0;

	for(int entry = 0; entry < listEntries; entry++)
  {
    
    if(opt.nEvt>0 && entry>=opt.nEvt) break;
    
    tSpall->GetEntry(elist->GetEntry(entry));
    long sec = (long)tSpall->GetLeaf("tMu_s")->GetValue();
    long nanoSec = (long)tSpall->GetLeaf("tMu_ns")->GetValue();
    //float nPESumAD1 = (float)tSpall->GetLeaf("calib_nPESum_AD1")->GetValue();
    int triggerNumberRpc = (int)tSpall->GetLeaf("triggerNumber_RPC")
                                        ->GetValue();
    
    tCalib->GetEntryWithIndex(triggerNumberRpc, 7);
    
    vector<AdEvent> muons, michcelElectrons;
    //TreeVariables tv;
    
    //if(aboveAD1(rh))
    //{
      /// record RPC muon times
      rpcMuonTimeNs.push_back(sec*1000000000+nanoSec);
      
      
      /// find muon candidates with Daniel's criteria
      if(rpcMuonTimeNs.size()>=2)
      {
        unsigned int mtIdx = rpcMuonTimeNs.size()-1;
        findStoppedMuons(tAd1Rec, tCalibStat, rpcMuonTimeNs[mtIdx-1],
        rpcMuonTimeNs[mtIdx], ad1Entry, muons);
      }
      /// find Michel electron candidates
      if(muons.size()==1)
      {
        //findMichelElectrons(tAd1Rec, muons[0], ad1Entry, michcelElectrons);
        //findMichelElectrons(tAd1Rec, muons[0], michcelElectrons);
        //cout << muons[0].triggerNumber << " " << muons[0].triggerTime << endl;
        muonsInDs.push_back(muons);
      }
      
      //if(muons.size()==1&&michcelElectrons.size()==1)
      //{
        //cout << muons[0].energy << " " <<  michcelElectrons[0].energy << endl;
      //}
    //}
  }
  cout << "number of prompt muon candidates: " << muonsInDs.size() << endl;
  
  /// find Michel electrons based on the muon candidates
  findMichelElectrons(tAd1Rec, muonsInDs, mesInDs);
  
  
  /// check point: output muon interarrival time tagged by RPC modules above AD1
  //TFile fof(Form("%s", getOutputPathName(ifpn, opt.outputPath,
            //string("mudecay")).c_str()), "recreate");
  fof.cd();
  TH1F hMuDt("hMuDt", "muon interarrival time tagged by RPC", 100,0,1000000000);
  for(unsigned int i = 0; i < rpcMuonTimeNs.size(); i++)
  	if(i>=1)
  		hMuDt.Fill(rpcMuonTimeNs[i]-rpcMuonTimeNs[i-1]);
  hMuDt.Write();
  fof.Close();
  

  cout << "number of RPC muons: " << rpcMuonTimeNs.size() << endl;
}





bool aboveAD1(PerCalibReadoutHeader* rh)
{
  unsigned int nHitsRpc = rh->nHitsRpc;;
  vector<unsigned int> rpcRow = rh->rpcRow;
  vector<unsigned int> rpcColumn = rh->rpcColumn;
  vector<unsigned int> rpcLayer = rh->rpcLayer;
  vector<unsigned int> rpcStrip = rh->rpcStrip;
  vector<bool> rpcFromRot = rh->rpcFromRot;
  
  bool nModChk = (nHitsRpc==rpcRow.size())&&(nHitsRpc==rpcColumn.size());
  nModChk = nModChk&&(nHitsRpc==rpcLayer.size())&&(nHitsRpc==rpcStrip.size());
  nModChk = nModChk&&(nHitsRpc==rpcFromRot.size());
  
  if(!nModChk)
  	cerr << "number of triggered module inconsistent" << endl;
  
  for(unsigned int m = 0; m < nHitsRpc; m++)
  {
  	if(!rpcFromRot[m]) return false;
    /// which modules to choose is determined by the other program resides in
    /// ProductionGeometryCheck
    if(rpcRow[m]>=2&&rpcRow[m]<=5&&rpcColumn[m]>=5&&rpcColumn[m]<=8)
  		return true;
  }
  
  return false;
}





void assignBranch(TTree& tr)
{
  TreeVariables tv;
  
  tr.Branch("delayed", &tv.delayed, "delayed/O");
  tr.Branch("dt", &tv.dt, "dt/i");
  tr.Branch("energy", &tv.energy, "energy/F");
  tr.Branch("flasher", &tv.flasher, "flasher/O");
  tr.Branch("multiplicity", &tv.multiplicity, "multiplicity/i");
  tr.Branch("nHit_IWS", &tv.nHit_IWS, "nHit_IWS/I");
  tr.Branch("nHit_OWS", &tv.nHit_OWS, "nHit_OWS/I");
  tr.Branch("nPESum_AD1", &tv.nPESum_AD1, "nPESum_AD1/F");
  tr.Branch("prompt", &tv.prompt, "prompt/O");
  
}





//void findMichelElectrons(TTree* tadrec, AdEvent& mu, //long entry,
                         //vector<AdEvent>& mes)
//{
  //int nentry = tadrec->GetEntries();
  ////for(int i = entry; i < nentry; i++)
  //for(int i = 0; i < nentry; i++)
  //{
    //tadrec->GetEntry(i);
    
    //long triggerTimeSec = (long)tadrec->GetLeaf("triggerTimeSec")
                                //->GetValue();
    //long triggerTimeNanoSec = (long)tadrec->GetLeaf("triggerTimeNanoSec")
                                    //->GetValue();
    //long eventTime = triggerTimeSec*1000000000+triggerTimeNanoSec;
    //float eventEnergy = (float)tadrec->GetLeaf("energy")->GetValue();
    //int energyStatus = (int)tadrec->GetLeaf("energyStatus")->GetValue();
    //if(energyStatus != 1) continue;
    //int triggerNumber = (int)tadrec->GetLeaf("triggerNumber")->GetValue();
    
    //if(eventTime-mu.triggerTime>500 && eventTime-mu.triggerTime<20000)
      //if(eventEnergy>20 && eventEnergy<60)
      //{
        //AdEvent curEvt;
        //curEvt.energy = eventEnergy;
        //curEvt.triggerTime = eventTime;
        //curEvt.triggerNumber = triggerNumber;
        //mes.push_back(curEvt);
      //}
    
    //if(eventTime-mu.triggerTime>=20000) return;
  //}
//}





void findMichelElectrons(TTree* trec, vector<vector<AdEvent> >& mu,
                         vector<vector<AdEvent> >& me)
{
  Long64_t nentry = trec->GetEntries();
  
  Long64_t entryMin = 0;
  
  for(unsigned int i = 0; i < mu.size(); i++)
  {
    long tmin = mu[i][0].triggerTime+500;
    long tmax = mu[i][0].triggerTime+20000;
    
    vector<AdEvent> meInterval;
    for(Long64_t entry = entryMin; entry < nentry; entry++)
    {
      trec->GetEntry(entry);
      long triggerTimeSec = (long)trec->GetLeaf("triggerTimeSec")->GetValue();
      long triggerTimeNanoSec = (long)trec->GetLeaf("triggerTimeNanoSec")
           ->GetValue();
      long time = triggerTimeSec*1000000000+triggerTimeNanoSec;
      float energy = (float)trec->GetLeaf("energy")->GetValue();
      int detector = (int)trec->GetLeaf("detector")->GetValue();
      int triggerNumber = (int)trec->GetLeaf("triggerNumber")->GetValue();
      
      if(detector==1)
      {
        if(time <= tmin) entryMin = entry;
        if(time>tmin && time<tmax)
          if(energy>20 && energy<60)
          {
            AdEvent michel;
            michel.triggerNumber = triggerNumber;
            michel.triggerTime = time;
            michel.energy = energy;
            meInterval.push_back(michel);
          }
      }
      if(time>=tmax) break;
    }
    me.push_back(meInterval);
    //cout << i+1 << " muons finished" << endl;
    if(meInterval.size()) cout << endl << "Michel electrons found!" << endl << endl;
  }
}





void findStoppedMuons(TTree* tadrec, TTree* tcs, long tmin, long tmax,
     long& entry, vector<AdEvent>& muons)
{
  
  /// search for AD prompt events due to muons:
  /// time window 200us after RPC muon trigger
  //tmax = min(tmax, 200000+tmin);
  tmax = 200000+tmin;
  
  tadrec->GetEntry(entry);
  long triggerTimeSec = (long)tadrec->GetLeaf("triggerTimeSec")->GetValue();
  long triggerTimeNanoSec = (long)tadrec->GetLeaf("triggerTimeNanoSec")
                            ->GetValue();
  long triggerTimeAd = triggerTimeSec*1000000000+triggerTimeNanoSec;
  
  while(triggerTimeAd<tmin)
  {
    entry++;
    tadrec->GetEntry(entry);
    triggerTimeSec = (long)tadrec->GetLeaf("triggerTimeSec")->GetValue();
    triggerTimeNanoSec = (long)tadrec->GetLeaf("triggerTimeNanoSec")
                               ->GetValue();
    triggerTimeAd = triggerTimeSec*1000000000+triggerTimeNanoSec;
  }
  while(triggerTimeAd>=tmin && triggerTimeAd<tmax)
  {
    tadrec->GetEntry(entry);
    triggerTimeSec = (long)tadrec->GetLeaf("triggerTimeSec")->GetValue();
    triggerTimeNanoSec = (long)tadrec->GetLeaf("triggerTimeNanoSec")
                               ->GetValue();
    triggerTimeAd = triggerTimeSec*1000000000+triggerTimeNanoSec;
    
    /// energy information in AdSimple tree
    int energyStatus = (int)tadrec->GetLeaf("energyStatus")->GetValue();
    float energy = (float)tadrec->GetLeaf("energy")->GetValue();
    int triggerNumber = (int)tadrec->GetLeaf("triggerNumber")->GetValue();
    int detector = (int)tadrec->GetLeaf("detector")->GetValue();
    
    if(energyStatus==1 && energy>20 && energy<90 && detector==1)
    {
      AdEvent mu;
      mu.energy = energy;
      mu.triggerTime = triggerTimeAd;
      mu.triggerNumber = triggerNumber;
      muons.push_back(mu);
    }
    entry++;
  }

}





bool isFlasher(TTree* tcs, int tn)
{
  bool flasher = false;
  
  /// set flasher flag
  tcs->GetEntryWithIndex(tn, 1);
  float quadrant = tcs->GetLeaf("Quadrant")->GetValue();
  float maxq = tcs->GetLeaf("MaxQ")->GetValue();
  float maxq2in = tcs->GetLeaf("MaxQ_2inchPMT")->GetValue();

  if(quadrant*quadrant+(maxq/.45)*(maxq/.45)<1 && maxq2in<100)
    return false;
  
  return true;
  
}
