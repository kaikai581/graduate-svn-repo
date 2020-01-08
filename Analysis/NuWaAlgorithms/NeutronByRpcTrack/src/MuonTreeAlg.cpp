#include "Event/RecHeader.h"
#include "Event/RecRpcHeader.h"
#include "Event/UserDataHeader.h"
#include "MuonTreeAlg.hpp"
#include "TEntryList.h"
#include "TFile.h"
#include "TLeaf.h"
#include "TTree.h"

using namespace DayaBay;
using namespace DetectorId;
using namespace ROOT::Math;
using namespace std;

MuonTreeAlg::MuonTreeAlg(const string& name, ISvcLocator* svcloc) :
GaudiAlgorithm(name, svcloc), exeCntr(1)
{
  declareProperty("InFileName", m_infilename = "input.root", "input root file name");
  declareProperty("WindowStart", m_windowStart = 20., "start of the time window after a muon in microsecond");
  declareProperty("WindowEnd", m_windowEnd = 1000., "end of the time window after a muon in microsecond");
}


StatusCode MuonTreeAlg::execute()
{
  
  RecHeader* recHeader = get<RecHeader>("/Event/Rec/AdSimple");
  const RecTrigger& recAdTrig = recHeader->recTrigger();
  /// time stamp of the current event
  TimeStamp curEvtTime = recAdTrig.triggerTime();
  
  /// process AD events
  if(recAdTrig.detector().isAD() && !isFlasher() &&
     recAdTrig.energyStatus() == 1 && recAdTrig.positionStatus() == 1)
  {
    /*
    int trigNum = recAdTrig.triggerNumber();
    stringstream cut;
    int detIdNum = recAdTrig.detector().detectorId();
    cut << "triggerNumber_AD" << detIdNum << "==" << trigNum;
    
    DetectorId_t detId = recAdTrig.detector().detectorId();
    if(getMuon(cut.str()))
    {
      //fillTree();
      registerAdMuon(detId, trigNum);
      m_muonList[m_adMuonId[detId]].m_adMuon[detId].triggerNumber = trigNum;
      m_muonList[m_adMuonId[detId]].m_adMuon[detId].dtAdMuNanoSec = 0;
      m_muonList[m_adMuonId[detId]].m_adMuon[detId].triggerTime = recAdTrig.triggerTime();
      m_muonList[m_adMuonId[detId]].m_adMuon[detId].energy = recAdTrig.energy();
      m_muonList[m_adMuonId[detId]].m_adMuon[detId].position = XYZPoint(recAdTrig.position().x(), recAdTrig.position().y(), recAdTrig.position().z());
      m_muonList[m_adMuonId[detId]].m_detectorFinished[detId] = true;
      debug() << "muon id: " << m_adMuonId[detId] << endreq;
    }*/
    DetectorId_t detId = recAdTrig.detector().detectorId();
    int trigNum = recAdTrig.triggerNumber();
    pair<DetectorId_t, int> curTrigId(detId, trigNum);
    
    if(m_muonLookupTable.find(curTrigId) != m_muonLookupTable.end())
    {
      info() << "AD muon found" << endreq;
      m_muonList[m_adMuonId[detId]].m_adMuon[detId].triggerNumber = trigNum;
      m_muonList[m_adMuonId[detId]].m_adMuon[detId].dtAdMuNanoSec = 0;
      m_muonList[m_adMuonId[detId]].m_adMuon[detId].triggerTime = recAdTrig.triggerTime();
      m_muonList[m_adMuonId[detId]].m_adMuon[detId].energy = recAdTrig.energy();
      m_muonList[m_adMuonId[detId]].m_adMuon[detId].position = XYZPoint(recAdTrig.position().x(), recAdTrig.position().y(), recAdTrig.position().z());
      m_muonList[m_adMuonId[detId]].m_detectorFinished[detId] = true;
    }
    else /// neutron candidate
    {
      TimeStamp curTime = recAdTrig.triggerTime();
      DetectorId_t detId = recAdTrig.detector().detectorId();
      
      if(m_muonList.find(m_adMuonId[detId]) != m_muonList.end())
      {
        AdData curNeu;
        curNeu.triggerNumber = recAdTrig.triggerNumber();
        curNeu.triggerTime = recAdTrig.triggerTime();
        curNeu.energy = recAdTrig.energy();
        curNeu.position = XYZPoint(recAdTrig.position().x(), recAdTrig.position().y(), recAdTrig.position().z());
        
        TimeStamp stime, etime, dtstamp(recAdTrig.triggerTime());
        dtstamp.Subtract(m_muonList[m_adMuonId[detId]].m_adMuon[detId].triggerTime);
        curNeu.dtAdMuNanoSec = dtstamp.GetNanoSec();
        
        stime = m_muonList[m_adMuonId[detId]].m_adMuon[detId].triggerTime;
        etime = m_muonList[m_adMuonId[detId]].m_adMuon[detId].triggerTime;
        stime.Add(m_windowStart*1e-6);
        etime.Add(m_windowEnd*1e-6);
        if(recAdTrig.triggerTime() > stime && recAdTrig.triggerTime() < etime)
          m_muonList[m_adMuonId[detId]].m_adNeutron[detId].push_back(curNeu);
      }
    }
  }
  
  /// deal with water pool
  recHeader = get<RecHeader>("/Event/Rec/PoolSimple");
  const RecTrigger& recPoolTrig = recHeader->recTrigger();
  if(recPoolTrig.detector().isWaterShield())
  {
    int trigNum = recPoolTrig.triggerNumber();
    stringstream cut;
    DetectorId_t detId = recPoolTrig.detector().detectorId();
    if(detId == kIWS) cut << "triggerNumber_IWS==" << trigNum;
    if(detId == kOWS) cut << "triggerNumber_OWS==" << trigNum;

    if(getMuon(cut.str()))
    {
      string muonId;
      registerWsMuon(detId, trigNum, muonId);
      m_muonList[muonId].m_detectorFinished[detId] = true;
      //info() << "muon id: " << muonId << endreq;
    }
  }
  
  /// deal with RPC
  RecRpcHeader* recRpc = get<RecRpcHeader>("/Event/Rec/RpcSimple");
  const RecRpcTrigger recRpcTrigger = recRpc->recTrigger();
  if(recRpcTrigger.detector().isRPC())
  {
    int trigNum = recRpcTrigger.triggerNumber();
    stringstream cut;
    DetectorId_t detId = recRpcTrigger.detector().detectorId();
    cut << "triggerNumber_RPC==" << trigNum;

    if(getMuon(cut.str()))
    {
      string muonId;
      registerRpcMuon(detId, trigNum, muonId);
      m_muonList[muonId].m_detectorFinished[detId] = true;
      //info() << "muon id: " << muonId << endreq;
    }
  }
  
  /// Check if any muon is finished.
  /// If yes, process them and erase them from the muon list.
  map<string, MuonData>::iterator it = m_muonList.begin();
  vector<string> completeList;
  for(; it != m_muonList.end(); it++)
  {
    if(it->second.allDetectorsFinished())
    {
      if(it->second.includeAd())
      {
        map<DetectorId_t, AdData>::iterator detIt = it->second.m_adMuon.begin();
        TimeStamp upperTime = detIt->second.triggerTime;
        for(; detIt != it->second.m_adMuon.end(); detIt++)
          if( upperTime < detIt->second.triggerTime )
            upperTime = detIt->second.triggerTime;
        
        upperTime.Add(m_windowEnd*1e-6);
        //info() << upperTime.AsString() << endreq;
        if(curEvtTime > upperTime)
        {
          
          if(it->second.m_adNeutron.size())
          {
            map<DetectorId_t, vector<AdData> >::iterator neuAdIt = it->second.m_adNeutron.begin();
            for(; neuAdIt != it->second.m_adNeutron.end(); neuAdIt++)
            {
              info() <<  "number of neutron candidates in AD" << (int)neuAdIt->first << ": " << neuAdIt->second.size() << endreq;
              for(unsigned int kk = 0; kk < neuAdIt->second.size(); kk++)
                info() << "time after muon(ns): " << neuAdIt->second[kk].dtAdMuNanoSec << endreq;
            }
          }
          completeList.push_back(it->first);
        }
      }
      /// If no AD involved, delete directly.
      else completeList.push_back(it->first);
      //completeList.push_back(it->first);
    }
  }
  debug() << "size of muon list before: " << m_muonList.size() << endreq;
  for(unsigned int ii = 0; ii < completeList.size(); ii++)
    m_muonList.erase(completeList[ii]);
  debug() << "size of muon list: " << m_muonList.size() << endreq;
  
  /// output progress
  if(exeCntr%1000 == 0)
    info() << exeCntr << " events are processed" << endreq;
  exeCntr++;
  
  return StatusCode::SUCCESS;
}


void MuonTreeAlg::fillTree()
{
  m_outfile->cd();
  m_outtree->Fill();
}


StatusCode MuonTreeAlg::finalize()
{
  m_infile->Close();
  delete m_infile;
  
  //m_outfile->cd();
  //m_outtree->Write();
  //m_outfile->Close();
  //delete m_outfile;
  
  return StatusCode::SUCCESS;
}


StatusCode MuonTreeAlg::initialize()
{
  m_infile = new TFile(m_infilename.c_str());
  m_treeSpal = (TTree*)m_infile->Get("/Event/Data/Physics/Spallation");
  //m_treeSpal->SetBranchAddress("tMu_s", &m_tMu_s);
  //m_treeSpal->SetBranchAddress("tMu_ns", &m_tMu_ns);
  //m_treeSpal->SetBranchAddress("hitAD1", &m_hitAD1);
  //m_treeSpal->SetBranchAddress("hitAD2", &m_hitAD2);
  //m_treeSpal->SetBranchAddress("hitAD3", &m_hitAD3);
  //m_treeSpal->SetBranchAddress("hitAD4", &m_hitAD4);
  //m_treeSpal->SetBranchAddress("hitIWS", &m_hitIWS);
  //m_treeSpal->SetBranchAddress("hitOWS", &m_hitOWS);
  //m_treeSpal->SetBranchAddress("hitRPC", &m_hitRPC);
  for(int ent = 0; ent < m_treeSpal->GetEntries(); ent++)
  {
    m_treeSpal->GetEntry(ent);
    
    stringstream muonId;
    int tMu_s = (int)m_treeSpal->GetLeaf("tMu_s")->GetValue();
    int tMu_ns = (int)m_treeSpal->GetLeaf("tMu_ns")->GetValue();
    muonId << tMu_s;
    /// zero padding
    unsigned int ndigits;
    ndigits = tMu_ns > 0 ? (unsigned int) log10 ((double) tMu_ns) + 1 : 1;
    for(int i = 0; i < 9-(int)ndigits; i++) muonId << 0;
    muonId << tMu_ns;
    
    vector<pair<DetectorId_t, int> > hitmap;
    
    int hitAD1 = (int)m_treeSpal->GetLeaf("hitAD1")->GetValue();
    int hitAD2 = (int)m_treeSpal->GetLeaf("hitAD2")->GetValue();
    int hitAD3 = (int)m_treeSpal->GetLeaf("hitAD3")->GetValue();
    int hitAD4 = (int)m_treeSpal->GetLeaf("hitAD4")->GetValue();
    int hitIWS = (int)m_treeSpal->GetLeaf("hitIWS")->GetValue();
    int hitOWS = (int)m_treeSpal->GetLeaf("hitOWS")->GetValue();
    int hitRPC = (int)m_treeSpal->GetLeaf("hitRPC")->GetValue();
    int triggerNumberAD1 = (int)m_treeSpal->GetLeaf("triggerNumber_AD1")->GetValue();
    int triggerNumberAD2 = (int)m_treeSpal->GetLeaf("triggerNumber_AD2")->GetValue();
    int triggerNumberAD3 = (int)m_treeSpal->GetLeaf("triggerNumber_AD3")->GetValue();
    int triggerNumberAD4 = (int)m_treeSpal->GetLeaf("triggerNumber_AD4")->GetValue();
    int triggerNumberIWS = (int)m_treeSpal->GetLeaf("triggerNumber_IWS")->GetValue();
    int triggerNumberOWS = (int)m_treeSpal->GetLeaf("triggerNumber_OWS")->GetValue();
    int triggerNumberRPC = (int)m_treeSpal->GetLeaf("triggerNumber_RPC")->GetValue();
    if(hitAD1 == 1) hitmap.push_back(pair<DetectorId::DetectorId_t, int>(kAD1,triggerNumberAD1));
    if(hitAD2 == 1) hitmap.push_back(pair<DetectorId::DetectorId_t, int>(kAD2,triggerNumberAD2));
    if(hitAD3 == 1) hitmap.push_back(pair<DetectorId::DetectorId_t, int>(kAD3,triggerNumberAD3));
    if(hitAD4 == 1) hitmap.push_back(pair<DetectorId::DetectorId_t, int>(kAD4,triggerNumberAD4));
    if(hitIWS == 1) hitmap.push_back(pair<DetectorId::DetectorId_t, int>(kIWS,triggerNumberIWS));
    if(hitOWS == 1) hitmap.push_back(pair<DetectorId::DetectorId_t, int>(kOWS,triggerNumberOWS));
    if(hitRPC == 1) hitmap.push_back(pair<DetectorId::DetectorId_t, int>(kRPC,triggerNumberRPC));
    
    for(unsigned int i = 0; i < hitmap.size(); i++)
    {
      m_muonLookupTable[hitmap[i]].m_muonId = muonId.str();
      m_muonLookupTable[hitmap[i]].m_hitMap = hitmap;
    }
    
    if(!((ent+1)%10000))
      info() << ent+1 << " muons booked" << endreq;
    
  }
  
  //m_outfile = new TFile("output.root", "recreate");
  //m_outtree = new TTree("t", "muon tree");
  //m_outtree->Branch("eMu", &m_eMu, "eMu/D");
  return StatusCode::SUCCESS;
}


bool MuonTreeAlg::isFlasher()
{
  UserDataHeader* calibStats = get<UserDataHeader>("/Event/Data/CalibStats");
  
  float Quadrant = calibStats->getFloat("Quadrant");
  float MaxQ     = calibStats->getFloat("MaxQ");
  float flasherScale = Quadrant*Quadrant+(MaxQ/.45)*(MaxQ/.45);
  
  if(flasherScale < 1.) return false;
  
  return true;
}


bool MuonTreeAlg::getMuon(string cut)
{
  m_infile->cd();
  m_treeSpal->Draw(">>elist", cut.c_str(), "entrylist");
  m_entryList = (TEntryList*)gDirectory->Get("elist");
  
  return (bool)m_entryList->GetN();
}


void MuonTreeAlg::registerAdMuon(DetectorId_t detId, int trigNum)
{
  debug() << "detector: " << (int)detId << endreq;
  m_treeSpal->GetEntry(m_entryList->GetEntry(0));
  
  stringstream muonId;
  int tMu_s = (int)m_treeSpal->GetLeaf("tMu_s")->GetValue();
  int tMu_ns = (int)m_treeSpal->GetLeaf("tMu_ns")->GetValue();
  //muonId << m_tMu_s;
  muonId << tMu_s;
  /// zero padding
  unsigned int ndigits;
  //ndigits = m_tMu_ns > 0 ? (unsigned int) log10 ((double) m_tMu_ns) + 1 : 1;
  ndigits = tMu_ns > 0 ? (unsigned int) log10 ((double) tMu_ns) + 1 : 1;
  for(int i = 0; i < 9-(int)ndigits; i++) muonId << 0;
  //muonId << m_tMu_ns;
  muonId << tMu_ns;
  m_adMuonId[detId] = muonId.str();
  
  if(m_muonList.find(muonId.str()) == m_muonList.end())
  {
    vector<DetectorId_t> hitmap;
    //if(m_hitAD1 == 1) hitmap.push_back(kAD1);
    //if(m_hitAD2 == 1) hitmap.push_back(kAD2);
    //if(m_hitAD3 == 1) hitmap.push_back(kAD3);
    //if(m_hitAD4 == 1) hitmap.push_back(kAD4);
    //if(m_hitIWS == 1) hitmap.push_back(kIWS);
    //if(m_hitOWS == 1) hitmap.push_back(kOWS);
    //if(m_hitRPC == 1) hitmap.push_back(kRPC);
    int hitAD1 = (int)m_treeSpal->GetLeaf("hitAD1")->GetValue();
    int hitAD2 = (int)m_treeSpal->GetLeaf("hitAD2")->GetValue();
    int hitAD3 = (int)m_treeSpal->GetLeaf("hitAD3")->GetValue();
    int hitAD4 = (int)m_treeSpal->GetLeaf("hitAD4")->GetValue();
    int hitIWS = (int)m_treeSpal->GetLeaf("hitIWS")->GetValue();
    int hitOWS = (int)m_treeSpal->GetLeaf("hitOWS")->GetValue();
    int hitRPC = (int)m_treeSpal->GetLeaf("hitRPC")->GetValue();
    if(hitAD1 == 1) hitmap.push_back(kAD1);
    if(hitAD2 == 1) hitmap.push_back(kAD2);
    if(hitAD3 == 1) hitmap.push_back(kAD3);
    if(hitAD4 == 1) hitmap.push_back(kAD4);
    if(hitIWS == 1) hitmap.push_back(kIWS);
    if(hitOWS == 1) hitmap.push_back(kOWS);
    if(hitRPC == 1) hitmap.push_back(kRPC);
    m_muonList[muonId.str()] = MuonData(hitmap);
  }
  
  m_muonList[muonId.str()].m_triggerId[detId] = trigNum;
}


void MuonTreeAlg::registerRpcMuon(DetectorId_t detId, int trigNum, string& id)
{
  debug() << "detector: " << (int)detId << endreq;
  m_treeSpal->GetEntry(m_entryList->GetEntry(0));
  
  stringstream muonId;
  int tMu_s = (int)m_treeSpal->GetLeaf("tMu_s")->GetValue();
  int tMu_ns = (int)m_treeSpal->GetLeaf("tMu_ns")->GetValue();
  //muonId << m_tMu_s;
  muonId << tMu_s;
  /// zero padding
  unsigned int ndigits;
  //ndigits = m_tMu_ns > 0 ? (unsigned int) log10 ((double) m_tMu_ns) + 1 : 1;
  ndigits = tMu_ns > 0 ? (unsigned int) log10 ((double) tMu_ns) + 1 : 1;
  for( int i = 0; i < 9-(int)ndigits; i++) muonId << 0;
  //muonId << m_tMu_ns;
  muonId << tMu_ns;
  id = muonId.str();
  
  if(m_muonList.find(muonId.str()) == m_muonList.end())
  {
    vector<DetectorId_t> hitmap;
    //if(m_hitAD1 == 1) hitmap.push_back(kAD1);
    //if(m_hitAD2 == 1) hitmap.push_back(kAD2);
    //if(m_hitAD3 == 1) hitmap.push_back(kAD3);
    //if(m_hitAD4 == 1) hitmap.push_back(kAD4);
    //if(m_hitIWS == 1) hitmap.push_back(kIWS);
    //if(m_hitOWS == 1) hitmap.push_back(kOWS);
    //if(m_hitRPC == 1) hitmap.push_back(kRPC);
    int hitAD1 = (int)m_treeSpal->GetLeaf("hitAD1")->GetValue();
    int hitAD2 = (int)m_treeSpal->GetLeaf("hitAD2")->GetValue();
    int hitAD3 = (int)m_treeSpal->GetLeaf("hitAD3")->GetValue();
    int hitAD4 = (int)m_treeSpal->GetLeaf("hitAD4")->GetValue();
    int hitIWS = (int)m_treeSpal->GetLeaf("hitIWS")->GetValue();
    int hitOWS = (int)m_treeSpal->GetLeaf("hitOWS")->GetValue();
    int hitRPC = (int)m_treeSpal->GetLeaf("hitRPC")->GetValue();
    if(hitAD1 == 1) hitmap.push_back(kAD1);
    if(hitAD2 == 1) hitmap.push_back(kAD2);
    if(hitAD3 == 1) hitmap.push_back(kAD3);
    if(hitAD4 == 1) hitmap.push_back(kAD4);
    if(hitIWS == 1) hitmap.push_back(kIWS);
    if(hitOWS == 1) hitmap.push_back(kOWS);
    if(hitRPC == 1) hitmap.push_back(kRPC);
    m_muonList[muonId.str()] = MuonData(hitmap);
  }
  
  m_muonList[id].m_triggerId[detId] = trigNum;
}


void MuonTreeAlg::registerWsMuon(DetectorId_t detId, int trigNum, string& id)
{
  debug() << "detector: " << (int)detId << endreq;
  m_treeSpal->GetEntry(m_entryList->GetEntry(0));
  
  stringstream muonId;
  int tMu_s = (int)m_treeSpal->GetLeaf("tMu_s")->GetValue();
  int tMu_ns = (int)m_treeSpal->GetLeaf("tMu_ns")->GetValue();
  muonId << tMu_s;
  /// zero padding
  unsigned int ndigits;
  ndigits = tMu_ns > 0 ? (unsigned int) log10 ((double) tMu_ns) + 1 : 1;
  for( int i = 0; i < 9-(int)ndigits; i++) muonId << 0;
  muonId << tMu_ns;
  id = muonId.str();
  
  if(m_muonList.find(muonId.str()) == m_muonList.end())
  {
    vector<DetectorId_t> hitmap;
    int hitAD1 = (int)m_treeSpal->GetLeaf("hitAD1")->GetValue();
    int hitAD2 = (int)m_treeSpal->GetLeaf("hitAD2")->GetValue();
    int hitAD3 = (int)m_treeSpal->GetLeaf("hitAD3")->GetValue();
    int hitAD4 = (int)m_treeSpal->GetLeaf("hitAD4")->GetValue();
    int hitIWS = (int)m_treeSpal->GetLeaf("hitIWS")->GetValue();
    int hitOWS = (int)m_treeSpal->GetLeaf("hitOWS")->GetValue();
    int hitRPC = (int)m_treeSpal->GetLeaf("hitRPC")->GetValue();
    if(hitAD1 == 1) hitmap.push_back(kAD1);
    if(hitAD2 == 1) hitmap.push_back(kAD2);
    if(hitAD3 == 1) hitmap.push_back(kAD3);
    if(hitAD4 == 1) hitmap.push_back(kAD4);
    if(hitIWS == 1) hitmap.push_back(kIWS);
    if(hitOWS == 1) hitmap.push_back(kOWS);
    if(hitRPC == 1) hitmap.push_back(kRPC);
    m_muonList[muonId.str()] = MuonData(hitmap);
  }
  
  m_muonList[id].m_triggerId[detId] = trigNum;
}
