#include "Event/CalibReadout.h"
#include "Event/CalibReadoutHeader.h"
#include "Event/RecHeader.h"
#include "Event/RecTrigger.h"
#include "Event/UserDataHeader.h"
#include "InterRpcAlignRightAlg.hpp"


using namespace std;
using namespace DayaBay;


InterRpcAlignRightAlg::InterRpcAlignRightAlg(const string& name, ISvcLocator* svcloc)
: GaudiAlgorithm(name, svcloc)
{
  declareProperty("FileName", m_rootfilename = "output.root", "root file name");
}


StatusCode InterRpcAlignRightAlg::execute()
{
  CalibReadoutHeader* calibReadoutHdr = get<CalibReadoutHeader>( CalibReadoutHeader::defaultLocation() );
  const CalibReadout* calibReadout = calibReadoutHdr->calibReadout();
  
  unsigned int detector = calibReadoutHdr->context().GetDetId();
  if(detector <= 4) // AD trigger
  {
    TreeVariables temptv;
    RecHeader* rh = get<RecHeader>( "/Event/Rec/AdSimple" );
    RecTrigger rt = rh->recTrigger();
    
    /// only deal with events with valid reconstructed energies
    if(rt.energyStatus() != 1) return StatusCode::SUCCESS;

    temptv.site = (int)rt.detector().site();
    temptv.triggerNumber = rt.triggerNumber();
    temptv.triggerTime = calibReadout->triggerTime();
    temptv.e = rt.energy();
    temptv.detector = detector;
    temptv.isFlasher = flasherTest();
    temptv.triggerType = calibReadout->triggerType();
    
    m_tv.push_back(temptv);
  }
  else if(detector == 7) // RPC trigger
  {
    TimeStamp rpcTime = calibReadout->triggerTime();
    
    /// used for counting number of events in a fixed window before a RPC hits
    m_coinEvts.clear();
    int counterInWin = 0;
    
    for(unsigned int i = 0; i < m_tv.size(); i++)
    {
      TimeStamp dtstamp = m_tv[i].triggerTime.CloneAndSubtract(rpcTime);
      TimeStamp posdt = rpcTime.CloneAndSubtract(m_tv[i].triggerTime);
      m_tvContiner.dt = dtstamp.GetSec()*1000000000+dtstamp.GetNanoSec();
      debug() << posdt.GetSec() << endreq;
      debug() << posdt.GetNanoSec() << endreq;
      m_tvContiner.e = m_tv[i].e;
      m_tvContiner.detector = m_tv[i].detector;
      m_tvContiner.isFlasher = m_tv[i].isFlasher;
      debug() << m_tvContiner.isFlasher << endreq;
      m_tvContiner.hitIWS = m_tv[i].hitIWS;
      m_tvContiner.hitOWS = m_tv[i].hitOWS;
      m_tvContiner.triggerType = m_tv[i].triggerType;
      m_tree->Fill();
      
      TimeStamp inverseDt = rpcTime.CloneAndSubtract(m_tv[i].triggerTime);
      /// If the current event happen to be in the coincidence window before
      /// the RPC hit, store it in the vector.
      if(inverseDt.GetSec() == 0 && inverseDt.GetNanoSec() < 1000000) {
        m_coinEvts.push_back(m_tv[i]);
        counterInWin++;
      }
    }
    
    info() << "number of evnets in this window: " << counterInWin << endreq;
    
    if(m_coinEvts.size() == 2) {
      
      m_tvCoin.site = m_coinEvts[0].site;
      m_tvCoin.detector = m_coinEvts[0].detector;
      m_tvCoin.pTimeSec = m_coinEvts[0].triggerTime.GetSec();
      m_tvCoin.pTimeNanoSec = m_coinEvts[0].triggerTime.GetNanoSec();
      m_tvCoin.dTimeSec = m_coinEvts[1].triggerTime.GetSec();
      m_tvCoin.dTimeNanoSec = m_coinEvts[1].triggerTime.GetNanoSec();
      m_tvCoin.pTriggerNumber = m_coinEvts[0].triggerNumber;
      m_tvCoin.dTriggerNumber = m_coinEvts[1].triggerNumber;
      m_tvCoin.pE = m_coinEvts[0].e;
      m_tvCoin.dE = m_coinEvts[1].e;
      m_tvCoin.dt = m_coinEvts[1].triggerTime.CloneAndSubtract(m_coinEvts[0].triggerTime).GetSeconds();
      m_trCoin->Fill();
      
    }
    m_tv.clear();
  }
  return StatusCode::SUCCESS;
}


StatusCode InterRpcAlignRightAlg::finalize()
{
  m_rootfile->Write();
  delete m_tree;
  delete m_trCoin;
  delete m_rootfile;
  return StatusCode::SUCCESS;
}


StatusCode InterRpcAlignRightAlg::initialize()
{
  debug() << "initialize()" << endreq;
  
  // Retrieve archive service
  //StatusCode status = service("EventDataArchiveSvc", m_archiveSvcPtr);
  //if (status.isFailure())
    //Error("Service [EventDataArchiveSvc] not found", status);
  m_rootfile = new TFile(m_rootfilename.c_str(), "RECREATE");
  m_tree = new TTree("intrpc", "AD events between RPC triggers aligned to the end of time window");
  m_tree->Branch("dt", &m_tvContiner.dt, "dt/L");
  m_tree->Branch("e", &m_tvContiner.e, "e/D");
  m_tree->Branch("detector", &m_tvContiner.detector, "detector/I");
  m_tree->Branch("isFlasher", &m_tvContiner.isFlasher, "isFlasher/O");
  m_tree->Branch("hitIWS", &m_tvContiner.hitIWS, "hitIWS/I");
  m_tree->Branch("hitOWS", &m_tvContiner.hitOWS, "hitOWS/I");
  m_tree->Branch("triggerType", &m_tvContiner.triggerType, "triggerType/I");
  
  m_trCoin = new TTree("mycoin", "coincidence in a time window just before a muon");
  m_trCoin->Branch("site", &m_tvCoin.site, "site/I");
  m_trCoin->Branch("detector", &m_tvCoin.detector, "detector/I");
  m_trCoin->Branch("pTriggerNumber", &m_tvCoin.pTriggerNumber, "pTriggerNumber/I");
  m_trCoin->Branch("dTriggerNumber", &m_tvCoin.dTriggerNumber, "dTriggerNumber/I");
  m_trCoin->Branch("pTimeSec", &m_tvCoin.pTimeSec, "pTimeSec/I");
  m_trCoin->Branch("pTimeNanoSec", &m_tvCoin.pTimeNanoSec, "pTimeNanoSec/I");
  m_trCoin->Branch("dTimeSec", &m_tvCoin.dTimeSec, "dTimeSec/I");
  m_trCoin->Branch("dTimeNanoSec", &m_tvCoin.dTimeNanoSec, "dTimeNanoSec/I");
  m_trCoin->Branch("pE", &m_tvCoin.pE, "pE/D");
  m_trCoin->Branch("dE", &m_tvCoin.dE, "dE/D");
  m_trCoin->Branch("dt", &m_tvCoin.dt, "dt/D");
  return StatusCode::SUCCESS;
}


bool InterRpcAlignRightAlg::flasherTest()
{
  UserDataHeader* calibStats = get<UserDataHeader>( "/Event/Data/CalibStats" );
  
  float Quadrant = calibStats->getFloat("Quadrant");
  float MaxQ     = calibStats->getFloat("MaxQ");
  float flasherScale = Quadrant*Quadrant+(MaxQ/.45)*(MaxQ/.45);
  
  if(flasherScale < 1.) return false;
  
  return true;
}
