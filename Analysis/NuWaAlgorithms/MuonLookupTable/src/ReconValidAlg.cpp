#include "Event/RecHeader.h"
#include "Event/RecRpcHeader.h"
#include "Event/UserDataHeader.h"
#include "ReconValidAlg.hpp"
#include "TEntryList.h"
#include "TFile.h"
#include "TLeaf.h"
#include "TTree.h"

using namespace DayaBay;
using namespace DetectorId;
using namespace ROOT::Math;
using namespace std;

ReconValidAlg::ReconValidAlg(const string& name, ISvcLocator* svcloc) :
GaudiAlgorithm(name, svcloc), exeCntr(1)
{
  declareProperty("InFileName", m_infilename = "input.root", "input root file name");
  declareProperty("WindowStart", m_windowStart = 20., "start of the time window after a muon in microsecond");
  declareProperty("WindowEnd", m_windowEnd = 1000., "end of the time window after a muon in microsecond");
}


StatusCode ReconValidAlg::execute()
{
  
  RecHeader* recHeader = get<RecHeader>("/Event/Rec/AdSimple");
  const RecTrigger& recAdTrig = recHeader->recTrigger();
  /// time stamp of the current event
  TimeStamp curEvtTime = recAdTrig.triggerTime();
  
  /// process AD events
  if(recAdTrig.detector().isAD() && !isFlasher() &&
     recAdTrig.energyStatus() == 1 && recAdTrig.positionStatus() == 1)
  {

    DetectorId_t detId = recAdTrig.detector().detectorId();
    int trigNum = recAdTrig.triggerNumber();
    triggerId curTrigId(detId, trigNum);
    
    if(m_muonLookupTable.find(curTrigId) != m_muonLookupTable.end())
    {
      info() << "AD muon found" << endreq;
      m_muonDataBuffer[m_adMuonId[detId]].m_adMuon[detId].triggerNumber = trigNum;
      m_muonDataBuffer[m_adMuonId[detId]].m_adMuon[detId].dtAdMuNanoSec = 0;
      m_muonDataBuffer[m_adMuonId[detId]].m_adMuon[detId].triggerTime = recAdTrig.triggerTime();
      m_muonDataBuffer[m_adMuonId[detId]].m_adMuon[detId].energy = recAdTrig.energy();
      m_muonDataBuffer[m_adMuonId[detId]].m_adMuon[detId].position = XYZPoint(recAdTrig.position().x(), recAdTrig.position().y(), recAdTrig.position().z());
      m_muonDataBuffer[m_adMuonId[detId]].m_detectorFinished[detId] = true;
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

    //if(getMuon(cut.str()))
    //{
      //string muonId;
      //registerWsMuon(detId, trigNum, muonId);
      //m_muonDataBuffer[muonId].m_detectorFinished[detId] = true;
      ////info() << "muon id: " << muonId << endreq;
    //}
  }
  
  /// deal with RPC
  RecRpcHeader* recRpc = get<RecRpcHeader>("/Event/Rec/RpcSimple");
  const RecRpcTrigger recRpcTrigger = recRpc->recTrigger();
  if(recRpcTrigger.detector().isRPC())
  {
    int trigNum = recRpcTrigger.triggerNumber();
    DetectorId_t detId = recRpcTrigger.detector().detectorId();
    triggerId curTrigId(detId, trigNum);

    //if(getMuon(cut.str()))
    //{
      //string muonId;
      //registerRpcMuon(detId, trigNum, muonId);
      //m_muonDataBuffer[muonId].m_detectorFinished[detId] = true;
      ////info() << "muon id: " << muonId << endreq;
    //}
  }
  
  /// Check if any muon is finished.
  /// If yes, process them and erase them from the muon list.
  map<string, SingleMuonData>::iterator it = m_muonDataBuffer.begin();
  vector<string> completeList;
  //for(; it != m_muonDataBuffer.end(); it++)
  //{
    //if(it->second.allDetectorsFinished())
    //{
      //if(it->second.includeAd())
      //{
        //map<DetectorId_t, AdData>::iterator detIt = it->second.m_adMuon.begin();
        //TimeStamp upperTime = detIt->second.triggerTime;
        //for(; detIt != it->second.m_adMuon.end(); detIt++)
          //if( upperTime < detIt->second.triggerTime )
            //upperTime = detIt->second.triggerTime;
        
        //upperTime.Add(m_windowEnd*1e-6);
        ////info() << upperTime.AsString() << endreq;
        //if(curEvtTime > upperTime)
        //{
          
          //if(it->second.m_adNeutron.size())
          //{
            //map<DetectorId_t, vector<AdData> >::iterator neuAdIt = it->second.m_adNeutron.begin();
            //for(; neuAdIt != it->second.m_adNeutron.end(); neuAdIt++)
            //{
              //info() <<  "number of neutron candidates in AD" << (int)neuAdIt->first << ": " << neuAdIt->second.size() << endreq;
              //for(unsigned int kk = 0; kk < neuAdIt->second.size(); kk++)
                //info() << "time after muon(ns): " << neuAdIt->second[kk].dtAdMuNanoSec << endreq;
            //}
          //}
          //completeList.push_back(it->first);
        //}
      //}
      ///// If no AD involved, delete directly.
      //else completeList.push_back(it->first);
      ////completeList.push_back(it->first);
    //}
  //}
  debug() << "size of muon list before: " << m_muonDataBuffer.size() << endreq;
  for(unsigned int ii = 0; ii < completeList.size(); ii++)
    m_muonDataBuffer.erase(completeList[ii]);
  debug() << "size of muon list: " << m_muonDataBuffer.size() << endreq;
  
  /// output progress
  if(exeCntr%1000 == 0)
    info() << exeCntr << " events are processed" << endreq;
  exeCntr++;
  
  return StatusCode::SUCCESS;
}


void ReconValidAlg::fillTree()
{
  m_outfile->cd();
  m_outtree->Fill();
}


StatusCode ReconValidAlg::finalize()
{
  m_infile->Close();
  delete m_infile;
  
  //m_outfile->cd();
  //m_outtree->Write();
  //m_outfile->Close();
  //delete m_outfile;
  
  return StatusCode::SUCCESS;
}


StatusCode ReconValidAlg::initialize()
{
  m_infile = new TFile(m_infilename.c_str());
  m_treeSpal = (TTree*)m_infile->Get("/Event/Data/Physics/Spallation");
  
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
    
    //vector<triggerId> hitmap;
    
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
    //if(hitAD1 == 1) hitmap.push_back(triggerId(kAD1,triggerNumberAD1));
    //if(hitAD2 == 1) hitmap.push_back(triggerId(kAD2,triggerNumberAD2));
    //if(hitAD3 == 1) hitmap.push_back(triggerId(kAD3,triggerNumberAD3));
    //if(hitAD4 == 1) hitmap.push_back(triggerId(kAD4,triggerNumberAD4));
    //if(hitIWS == 1) hitmap.push_back(triggerId(kIWS,triggerNumberIWS));
    //if(hitOWS == 1) hitmap.push_back(triggerId(kOWS,triggerNumberOWS));
    //if(hitRPC == 1) hitmap.push_back(triggerId(kRPC,triggerNumberRPC));
    
    /// store this muon for recording its hit pattern
    string muId = muonId.str();
    m_muonProgressTable[muId] = SingleMuonProgress();
    if(hitAD1 == 1)
    {
      m_muonLookupTable[triggerId(kAD1,triggerNumberAD1)] = muId;
      m_muonProgressTable[muId].m_detectorFinished[kAD1] = false;
    }
    if(hitAD2 == 1)
    {
      m_muonLookupTable[triggerId(kAD2,triggerNumberAD2)] = muId;
      m_muonProgressTable[muId].m_detectorFinished[kAD2] = false;
    }
    if(hitAD3 == 1)
    {
      m_muonLookupTable[triggerId(kAD3,triggerNumberAD3)] = muId;
      m_muonProgressTable[muId].m_detectorFinished[kAD3] = false;
    }
    if(hitAD4 == 1)
    {
      m_muonLookupTable[triggerId(kAD4,triggerNumberAD4)] = muId;
      m_muonProgressTable[muId].m_detectorFinished[kAD4] = false;
    }
    if(hitIWS == 1)
    {
      m_muonLookupTable[triggerId(kIWS,triggerNumberIWS)] = muId;
      m_muonProgressTable[muId].m_detectorFinished[kIWS] = false;
    }
    if(hitOWS == 1)
    {
      m_muonLookupTable[triggerId(kOWS,triggerNumberOWS)] = muId;
      m_muonProgressTable[muId].m_detectorFinished[kOWS] = false;
    }
    if(hitRPC == 1)
    {
      m_muonLookupTable[triggerId(kRPC,triggerNumberRPC)] = muId;
      m_muonProgressTable[muId].m_detectorFinished[kRPC] = false;
    }
    
    //for(unsigned int i = 0; i < hitmap.size(); i++)
    //{
      //m_muonLookupTable[hitmap[i]].m_muonId = muonId.str();
      //m_muonLookupTable[hitmap[i]].m_hitMap = hitmap;
    //}
    
    if(!((ent+1)%10000))
      info() << ent+1 << " muons booked" << endreq;
    
  }
  
  //m_outfile = new TFile("output.root", "recreate");
  //m_outtree = new TTree("t", "muon tree");
  //m_outtree->Branch("eMu", &m_eMu, "eMu/D");
  return StatusCode::SUCCESS;
}


bool ReconValidAlg::isFlasher()
{
  UserDataHeader* calibStats = get<UserDataHeader>("/Event/Data/CalibStats");
  
  float Quadrant = calibStats->getFloat("Quadrant");
  float MaxQ     = calibStats->getFloat("MaxQ");
  float flasherScale = Quadrant*Quadrant+(MaxQ/.45)*(MaxQ/.45);
  
  if(flasherScale < 1.) return false;
  
  return true;
}

/*
bool ReconValidAlg::muonInProgressTable(string muId)
{
  return (m_muonProgressTable.find(muId) != m_muonProgressTable.end());
}


bool ReconValidAlg::getMuon(string cut)
{
  m_infile->cd();
  m_treeSpal->Draw(">>elist", cut.c_str(), "entrylist");
  m_entryList = (TEntryList*)gDirectory->Get("elist");
  
  return (bool)m_entryList->GetN();
}


void ReconValidAlg::registerAdMuon(DetectorId_t detId, int trigNum)
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


void ReconValidAlg::registerRpcMuon(DetectorId_t detId, int trigNum, string& id)
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


void ReconValidAlg::registerWsMuon(DetectorId_t detId, int trigNum, string& id)
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
*/
