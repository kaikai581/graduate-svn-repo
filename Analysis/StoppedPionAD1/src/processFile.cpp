#include <cstdlib>
#include <iostream>
#include "PerCalibReadoutEvent/PerCalibReadoutHeader.h"
#include "structopt.hpp"
#include "TEntryList.h"
#include "TFile.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TLeaf.h"
#include "TTree.h"
#include "utilities.hpp"


using namespace std;


bool aboveAD1(PerCalibReadoutHeader*);


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
  TFile fif(ifpn.c_str(), openOpt.c_str());
  TTree *rpcCalib = (TTree*)fif.Get("/Event/CalibReadout/CalibReadoutHeader");
  TTree* tCalibStat = (TTree*)fif.Get("/Event/Data/CalibStats");
  tCalibStat->SetTreeIndex(0);
  tCalibStat->BuildIndex("triggerNumber","context.mDetId");
  TTree* tSpall = (TTree*)fif.Get("/Event/Data/Physics/Spallation");
  tSpall->SetTreeIndex(0);
  tSpall->BuildIndex("triggerNumber_RPC","context.mDetId");

  if(!rpcCalib)
  {
    cerr << "calibration tree not found" << endl;
    return;
  }


	/// look at only RPC events, i.e. nHitsRpc>=1
  rpcCalib->Draw(">>elist", "nHitsRpc>=1", "entrylist");
  TEntryList *elist = (TEntryList*)gDirectory->Get("elist");

  /// dataset level storage
  PerCalibReadoutHeader* rh = 0;
  rpcCalib->SetBranchAddress("CalibReadout_CalibReadoutHeader", &rh);

  /// pin down RPC muon times
  
  /// time container in second
  vector<double> rpcMuonTime;
  /// time container in nanosecond
  vector<long>   rpcMuonTimeNs;
  /// water pool hit condition
  vector<int>    nHitIws;
  vector<int>    nHitOws;
  vector<float>  vecNPeSumAD1;

  Long64_t listEntries = elist->GetN();

	for(int entry = 0; entry < listEntries; entry++)
  {
    
    if(opt.nEvt>0 && entry>=opt.nEvt)
      break;
    
    rpcCalib->GetEntry(elist->GetEntry(entry));
    
    if(aboveAD1(rh))
    {
      rpcMuonTime.push_back((double)rh->triggerTimeSec+(double)rh->triggerTimeNanoSec*1e-9);
      long sec = (long)rh->triggerTimeSec;
      long nanoSec = (long)rh->triggerTimeNanoSec;
      rpcMuonTimeNs.push_back(sec*1000000000+nanoSec);

      /// get water pool information
      tSpall->GetEntryWithIndex(rh->triggerNumber, 8);
      nHitIws.push_back((int)tSpall->GetLeaf("calib_nHit_IWS")->GetValue());
      nHitOws.push_back((int)tSpall->GetLeaf("calib_nHit_OWS")->GetValue());
      vecNPeSumAD1.push_back(tSpall->GetLeaf("calib_nPESum_AD1")->GetValue());
    }
  }
  
  
  /// check point: output muon interarrival time above AD1
  TFile fof(Form("%s", getOutputPathName(ifpn, opt.outputPath, string("pidecay")).c_str()), "recreate");
  TH1F hMuDt("hMuDt", "muon interarrival time tagged by RPC", 100,0,1);
  for(unsigned int i = 0; i < rpcMuonTime.size(); i++)
  	if(i>=1)
  		hMuDt.Fill(rpcMuonTime[i]-rpcMuonTime[i-1]);
  hMuDt.Write();
  fof.Close();
  
  
  /// now that I've got the muon time above AD1, get the AD event time relative
  /// the muon time
  
  TTree* Ad1Rec = (TTree*)fif.Get("/Event/Rec/AdSimple");
  fif.cd();
  Ad1Rec->Draw(">>adlist", "energyStatus==1&&detector==1", "entrylist");
  TEntryList *adlist = (TEntryList*)gDirectory->Get("adlist");
	int Ad1Entries = adlist->GetN();




  unsigned int idxPtr = 1;
  const unsigned int nRpcMuons = rpcMuonTime.size();

  vector<double> vecEnergy;
  vector<double> vecTime;
  double eMax = 0;
  double dtMax = 0;

  double benergy = 0.;
  double bdtMuon = 0.;

  TFile fof2(Form("%s", getOutputPathName(ifpn, opt.outputPath, string("pidecay")).c_str()), "update");
  TTree tevst("evt", "energy vs time");
  tevst.Branch("energy", &benergy, "energy/D");
  tevst.Branch("dt", &bdtMuon, "dt/D");

  /// another tree with long int type time & unit ns
  TTree etl("etl", "energy vs time");
  double lenergy = 0.;
  long ldtMuon = 0;
  bool flasher = false;
  int nHit_IWS = -2;
  int nHit_OWS = -2;
  float nPESum_AD1 = 0.;
  etl.Branch("energy", &lenergy, "energy/D");
  etl.Branch("dt", &ldtMuon, "dt/L");
  etl.Branch("flasher", &flasher, "flasher/O");
  etl.Branch("nHit_IWS", &nHit_IWS, "nHit_IWS/I");
  etl.Branch("nHit_OWS", &nHit_OWS, "nHit_OWS/I");
  etl.Branch("nPESum_AD1", &nPESum_AD1, "nPESum_AD1/F");

  for(int entry = 0; entry < Ad1Entries; entry++)
  {
    if(opt.nEvt>0 && entry>=opt.nEvt)
      break;

    Ad1Rec->GetEntry(adlist->GetEntry(entry));

    float energy = Ad1Rec->GetLeaf("energy")->GetValue();
    if(energy>eMax) eMax = energy;
    int triggerTimeSec = (int)Ad1Rec->GetLeaf("triggerTimeSec")->GetValue();
    int triggerTimeNanoSec = (int)Ad1Rec->GetLeaf("triggerTimeNanoSec")->GetValue();
    int triggerNumber_AD1 = (int)Ad1Rec->GetLeaf("triggerNumber")->GetValue();
    double curTime = (double)triggerTimeSec+(double)triggerTimeNanoSec*1e-9;

    long sec = (long)triggerTimeSec;
    long nanosec = (long)triggerTimeNanoSec;
    long timeLong = sec*1000000000+nanosec;


    /// set flasher flag
    tCalibStat->GetEntryWithIndex(triggerNumber_AD1, 1);
    float quadrant = tCalibStat->GetLeaf("Quadrant")->GetValue();
    float maxq = tCalibStat->GetLeaf("MaxQ")->GetValue();
    float maxq2in = tCalibStat->GetLeaf("MaxQ_2inchPMT")->GetValue();

    if(quadrant*quadrant+(maxq/.45)*(maxq/.45)<1 && maxq2in<100)
      flasher = false;
    else flasher = true;


    for(unsigned int il = 1; il < rpcMuonTimeNs.size(); il++)
    {
      if(timeLong>=rpcMuonTimeNs[il-1]&&timeLong<rpcMuonTimeNs[il])
      {
        lenergy = (double)energy;
        ldtMuon = timeLong-rpcMuonTimeNs[il-1];
        nHit_IWS = nHitIws[il-1];
        nHit_OWS = nHitOws[il-1];
        nPESum_AD1 = vecNPeSumAD1[il-1];
        etl.Fill();
      }
    }

    /// subtract last muon time from the event time
    if(idxPtr < nRpcMuons)
    {
      if(curTime>=rpcMuonTime[idxPtr-1]&&curTime<rpcMuonTime[idxPtr])
      {
        vecEnergy.push_back((double)energy);
        double dt = curTime-rpcMuonTime[idxPtr-1];
        vecTime.push_back(dt);
        if(dt>dtMax) dtMax = dt;

        benergy = (double)energy;
        bdtMuon = dt;
        tevst.Fill();
      }
      else if(curTime>=rpcMuonTime[idxPtr])
      {
        vecEnergy.push_back((double)energy);
        double dt = curTime-rpcMuonTime[idxPtr];
        vecTime.push_back(dt);
        if(dt>dtMax) dtMax = dt;
        idxPtr++;

        benergy = (double)energy;
        bdtMuon = dt;
        tevst.Fill();
      }
    }


  }


  fof2.cd();
  TH2F hEvsT("hEvsT", "energy vs time", 1000, 0, 1e-5, 1000, 0, 60);
  for(unsigned int i = 0; i < vecEnergy.size(); i++)
  {
    //cout << vecEnergy[i] << " " << vecTime[i] << endl;
    hEvsT.Fill(vecTime[i], vecEnergy[i]);
  }
  hEvsT.Write();
  tevst.Write();
  etl.Write();
  fof2.Close();
  

  cout << "number of RPC muons: " << rpcMuonTime.size() << endl;
  cout << "number of AD1 events: " << vecEnergy.size() << endl;
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
