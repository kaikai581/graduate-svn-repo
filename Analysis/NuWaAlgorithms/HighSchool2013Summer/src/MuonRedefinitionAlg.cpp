#include <algorithm>
#include "Event/CalibReadoutHeader.h"
#include "Event/CalibReadout.h"
#include "Event/CalibReadoutPmtCrate.h"
#include "Event/RecHeader.h"
#include "Event/UserDataHeader.h"
#include "MuonRedefinitionAlg.hpp"
#include "TFile.h"
#include "TLeaf.h"
#include "TTree.h"


using namespace DayaBay;
using namespace DetectorId;
using namespace std;


MuonRedefinitionAlg::MuonRedefinitionAlg(const string& name, ISvcLocator* svcloc) :
GaudiAlgorithm(name, svcloc)
{
  declareProperty("InFileName", m_infilename = "input.root", "input root file name");
  
  m_lastMuonTime[kAD1] = TimeStamp(2011,1,1,0,0,0);
  m_lastMuonTime[kAD2] = TimeStamp(2011,1,1,0,0,0);
  m_lastMuonTime[kAD3] = TimeStamp(2011,1,1,0,0,0);
  m_lastMuonTime[kAD4] = TimeStamp(2011,1,1,0,0,0);
  
  m_isFirstInterMuonTrigger[kAD1] = false;
  m_isFirstInterMuonTrigger[kAD2] = false;
  m_isFirstInterMuonTrigger[kAD3] = false;
  m_isFirstInterMuonTrigger[kAD4] = false;
}



StatusCode MuonRedefinitionAlg::execute()
{
  RecHeader* recHeader = get<RecHeader>("/Event/Rec/AdSimple");
  const RecTrigger& recAdTrig = recHeader->recTrigger();
  
  /// only process AD triggers
  if(!recAdTrig.detector().isAD()) return StatusCode::SUCCESS;
  
  CalibReadoutHeader* calibReadoutHeader = get<CalibReadoutHeader>("/Event/CalibReadout/CalibReadoutHeader");
  const CalibReadout* calibReadout = calibReadoutHeader->calibReadout();
  const CalibReadoutPmtCrate* pmtCrate = dynamic_cast<const CalibReadoutPmtCrate*>(calibReadout);
  
  /// get calib statistics for PMT gain
  UserDataHeader* calibStats = get<UserDataHeader>("/Event/Data/CalibStats");
  float npesum = calibStats->getFloat("nPESum");
  
  DetectorId_t detId = recAdTrig.detector().detectorId();
  int trigNum = recAdTrig.triggerNumber();
  
  /// if this is a muon event
  if(find(m_myAdMuonList[detId].begin(), m_myAdMuonList[detId].end(), trigNum) != m_myAdMuonList[detId].end())
  {
    m_lastMuonTime[detId] = recAdTrig.triggerTime();
    m_isFirstInterMuonTrigger[detId] = true;
  }
  
  /// if this is an inter-muon event
  if(find(m_myAdInterMuonList[detId].begin(), m_myAdInterMuonList[detId].end(), trigNum) != m_myAdInterMuonList[detId].end())
  {
    info() << "inter-muon det " << (int)detId << " trig_num " << trigNum << endreq;
    
    TimeStamp dt(recAdTrig.triggerTime());
    dt.Subtract(m_lastMuonTime[detId]);
    if( dt.GetSeconds() > 1e-3 )
    {
      
      /// write out muon data
      if(m_isFirstInterMuonTrigger[detId])
      {
        m_outf << "Muon time: " << m_lastMuonTime[detId].GetSec() << " ";
        m_outf << m_lastMuonTime[detId].GetNanoSec() << " " << (int)detId << endl;
        m_isFirstInterMuonTrigger[detId] = false;
      }
      
      /// write out inter muon event data
      m_outf << "\t";
      m_outf << "Event time: " << recAdTrig.triggerTime().GetSec() << " ";
      m_outf << recAdTrig.triggerTime().GetNanoSec() << endl;
      
      /// retrive pmt hits
      CalibReadoutPmtCrate::PmtChannelReadouts channels = pmtCrate->channelReadout();
      CalibReadoutPmtCrate::PmtChannelReadouts::const_iterator chIt;
      
      /// PMT loop
      for(chIt = channels.begin(); chIt != channels.end(); chIt++)
      {
        const CalibReadoutPmtChannel& channel = *chIt;
        AdPmtSensor pmtId(channel.pmtSensorId().fullPackedData());
        
        /// hit loop in a PMT
        for(unsigned int hitIdx = 0; hitIdx < channel.size(); hitIdx++)
        {
          if(pmtId.ring() > 0)
          {
            m_outf << "\t\t";
            m_outf << (pmtId.ring()-1)+(pmtId.column()-1)*8 << " ";
            m_outf << channel.time(hitIdx) << " ";
            m_outf << channel.charge(hitIdx)*recAdTrig.energy()/npesum << endl;
          }
        }
      }
    }
  }
  
  
  return StatusCode::SUCCESS;
}



StatusCode MuonRedefinitionAlg::initialize()
{
  TFile inprodfile(m_infilename.c_str());
  TTree* tspall = (TTree*)inprodfile.Get("/Event/Data/Physics/Spallation");
  
  /// Check if only one AD is triggered. If true, this is a candidate of
  /// something interesting.
  int nent = tspall->GetEntries();
  for(int ent = 0; ent < nent; ent++)
  {
    tspall->GetEntry(ent);
    
    int hitAD1 = (int)tspall->GetLeaf("hitAD1")->GetValue();
    int hitAD2 = (int)tspall->GetLeaf("hitAD2")->GetValue();
    int hitAD3 = (int)tspall->GetLeaf("hitAD3")->GetValue();
    int hitAD4 = (int)tspall->GetLeaf("hitAD4")->GetValue();
    int hitIWS = (int)tspall->GetLeaf("hitIWS")->GetValue();
    int hitOWS = (int)tspall->GetLeaf("hitOWS")->GetValue();
    int hitRPC = (int)tspall->GetLeaf("hitRPC")->GetValue();
    int triggerNumberAD1 = (int)tspall->GetLeaf("triggerNumber_AD1")->GetValue();
    int triggerNumberAD2 = (int)tspall->GetLeaf("triggerNumber_AD2")->GetValue();
    int triggerNumberAD3 = (int)tspall->GetLeaf("triggerNumber_AD3")->GetValue();
    int triggerNumberAD4 = (int)tspall->GetLeaf("triggerNumber_AD4")->GetValue();
    int triggerNumberIWS = (int)tspall->GetLeaf("triggerNumber_IWS")->GetValue();
    int triggerNumberOWS = (int)tspall->GetLeaf("triggerNumber_OWS")->GetValue();
    int triggerNumberRPC = (int)tspall->GetLeaf("triggerNumber_RPC")->GetValue();
    
    /// if an official AD muon is along with any muon detector trigger,
    /// define this as a customized AD muon.
    if(hitIWS == 1 || hitOWS == 1 || hitRPC == 1)
    {
      if(hitAD1 == 1) m_myAdMuonList[kAD1].push_back(triggerNumberAD1);
      if(hitAD2 == 1) m_myAdMuonList[kAD2].push_back(triggerNumberAD2);
      if(hitAD3 == 1) m_myAdMuonList[kAD3].push_back(triggerNumberAD3);
      if(hitAD4 == 1) m_myAdMuonList[kAD4].push_back(triggerNumberAD4);
      continue;
    }
    
    if(hitAD1 == 1) m_myAdInterMuonList[kAD1].push_back(triggerNumberAD1);
    if(hitAD2 == 1) m_myAdInterMuonList[kAD2].push_back(triggerNumberAD2);
    if(hitAD3 == 1) m_myAdInterMuonList[kAD3].push_back(triggerNumberAD3);
    if(hitAD4 == 1) m_myAdInterMuonList[kAD4].push_back(triggerNumberAD4);
    
    
    
  }
  
  inprodfile.Close();
  
  size_t startPos = m_infilename.find_first_of(".");
  size_t endPos = m_infilename.find_last_of(".");
  
  string outfn = "Intermuon" + m_infilename.substr(startPos, endPos-startPos) + ".txt";

  m_outf.open(outfn.c_str());
  return StatusCode::SUCCESS;
}



StatusCode MuonRedefinitionAlg::finalize()
{
  info() << m_myAdMuonList[kAD1].size() << endreq;
  info() << m_myAdMuonList[kAD2].size() << endreq;
  info() << m_myAdMuonList[kAD3].size() << endreq;
  info() << m_myAdMuonList[kAD4].size() << endreq;
  info() << m_myAdInterMuonList[kAD1].size() << endreq;
  info() << m_myAdInterMuonList[kAD2].size() << endreq;
  info() << m_myAdInterMuonList[kAD3].size() << endreq;
  info() << m_myAdInterMuonList[kAD4].size() << endreq;
  m_outf.close();
  return StatusCode::SUCCESS;
}
