/*
 * This script record inter-muon event only if its energy > 30MeV.
*/


#include <fstream>
#include <iostream>
#include <sstream>
#include "PerCalibReadoutEvent/PerCalibReadoutHeader.h"
#include "TEntryList.h"
#include "TFile.h"
#include "TLeaf.h"
#include "TTree.h"


string infn("/disk1/data/2011/p12e/Neutrino/1224/recon.Neutrino.0021223.Physics.EH3-Merged.P12E-P._0001.root");


double ChargeEnergyRatio(TFile& f, int tn, map<int, vector<pair<float, float> > >& hits, double& evt_en)
{
  TTree* tAdSimple = (TTree*)f.Get("/Event/Rec/AdSimple");
  
  stringstream tnsel;
  tnsel << "detector==1&&triggerNumber==" << tn;
  
  tAdSimple->Draw(">>elist2", tnsel.str().c_str(), "entrylist");
  TEntryList* elist2 = (TEntryList*)gDirectory->Get("elist2");
  tAdSimple->GetEntry(elist2->GetEntry(0));
  
  map<int, vector<pair<float, float> > >::iterator it = hits.begin();
  double totchg = 0;
  for(; it != hits.end(); it++)
    for(unsigned int i = 0; i < it->second.size(); i++)
      totchg += it->second[i].second;
  
  evt_en = tAdSimple->GetLeaf("energy")->GetValue();
  return tAdSimple->GetLeaf("energy")->GetValue()/totchg;
}


void InterMuonPmts30MeV()
{
  TFile inf(infn.c_str());
  
  TTree* tSpall = (TTree*)inf.Get("/Event/Data/Physics/Spallation");
  TTree* tCalibRoHdr = (TTree*)inf.Get("/Event/CalibReadout/CalibReadoutHeader");
  
  /// store 100 muons
  int nmu = 1;
  int nentries = tSpall->GetEntries();
  vector<double> muTrigNum;
  for(int ent = 0; ent < nentries; ent++)
  {
    if(nmu > 10) break;
    tSpall->GetEntry(ent);
    int triggerNum = (int)tSpall->GetLeaf("triggerNumber_AD1")->GetValue();
    
    if(triggerNum >= 0)
    {
      nmu++;
      muTrigNum.push_back(triggerNum);
    }
  }
  
  /// retrieve calibration information from CalibReadoutHeader
  ofstream outf("output_energy.txt");
  PerCalibReadoutHeader* rh = 0;
  tCalibRoHdr->SetBranchAddress("CalibReadout_CalibReadoutHeader", &rh);
  for(unsigned int i = 0; i < muTrigNum.size()-1; i++)
  {
    stringstream tnsel;
    tnsel << "detector==1&&triggerNumber>=" << muTrigNum[i] << "&&triggerNumber<" << muTrigNum[i+1];
    
    tCalibRoHdr->Draw(">>elist", tnsel.str().c_str(), "entrylist");
    TEntryList* elist = (TEntryList*)gDirectory->Get("elist");
    tCalibRoHdr->GetEntry(elist->GetEntry(0));
    
    int tMu_s = rh->triggerTimeSec;
    int tMu_ns = rh->triggerTimeNanoSec;
    double tMu = tMu_s + tMu_ns*1e-9;
    
    outf << "Muon time: " << tMu_s << " ";
    outf << tMu_ns << endl;
    
    int nlist = elist->GetN();
    for(int nli = 1; nli < nlist; nli++)
    {
      tCalibRoHdr->GetEntry(elist->GetEntry(nli));
      
      int tEv_s = rh->triggerTimeSec;
      int tEv_ns = rh->triggerTimeNanoSec;
      double tEv = tEv_s + tEv_ns*1e-9;
      map<int, vector<pair<float, float> > > pmtHits;
      
      if(tEv - tMu > 1e-3)
      {
        outf << "\t";
        outf << "Event time: " << tEv_s << " ";
        outf << tEv_ns << endl;
        
        int nHitsAD = (int)tCalibRoHdr->GetLeaf("nHitsAD")->GetValue();
        for(int j = 0; j < nHitsAD; j++)
        {
          int ring = rh->ring[j]-1;
          int column = rh->column[j]-1;
          pmtHits[ring*24+column].push_back(pair<float, float>(rh->timeAD[j], rh->chargeAD[j]));
        }
        
        for(int ring = 0; ring < 8; ring++)
          for(int col = 0; col < 24; col++)
            if(pmtHits.find(ring*24+col) == pmtHits.end())
              pmtHits[ring*24+col].push_back(pair<float, float>(0,0));
        
        /// get the charge energy ratio and the total energy
        double evt_en;
        double ceratio = ChargeEnergyRatio(inf, rh->triggerNumber, pmtHits, evt_en);
        bool first_evt_over_30MeV = true;
        
        map<int, vector<pair<float, float> > >::iterator it = pmtHits.begin();
        for(; it != pmtHits.end(); it++)
        {
          if()
          for(unsigned int k = 0; k < it->second.size(); k++)
          {
            outf << "\t\t";
            outf << it->first << " ";
            outf << it->second[k].first << " ";
            outf << it->second[k].second*ceratio << endl;
          }
        }
      }
    }
    
    cout << i+1 << " muons finished" << endl;
  }
  
  outf.close();
}
