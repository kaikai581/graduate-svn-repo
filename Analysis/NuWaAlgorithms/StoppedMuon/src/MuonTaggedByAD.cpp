#include <sstream>
#include "DataUtilities/DybArchiveList.h"
#include "MuonTaggedByAD.hpp"
//#include "Event/HeaderObject.h"
//#include "Event/UserDataHeader.h"
#include "Event/RecHeader.h"
//#include "SpallData.h"


using namespace std;
using namespace DayaBay;


MuonTaggedByAD::MuonTaggedByAD(const string& name, ISvcLocator* svcloc) :
GaudiAlgorithm(name, svcloc), m_evtNum(0)
{
}


MuonTaggedByAD::~MuonTaggedByAD()
{
}


StatusCode MuonTaggedByAD::initialize()
{
  debug() << "initialize()" << endreq;
  
  // Retrieve archive service
  StatusCode status = service("EventDataArchiveSvc", p_archiveSvc);
  if (status.isFailure())
  {
    Error("Service [EventDataArchiveSvc] not found", status);
  }
  
  return StatusCode::SUCCESS;
}


StatusCode MuonTaggedByAD::execute()
{
  debug() << "execute() start " << endreq;
  
  DataObject* pObjSpall;
  if(eventSvc()->retrieveObject("/Event/Data/Physics/Spallation",pObjSpall).isFailure())
  {
    debug() << "No muon tag in this event" << endreq;
    return StatusCode::SUCCESS;
  }
  
  UserDataHeader* hdrSpall = dynamic_cast<UserDataHeader*>(pObjSpall);
  
  // print information
  //info() << "hitRPC: " << hdrSpall->getInt("hitRPC") << "\thitAD1: " << hdrSpall->getInt("hitAD1") << endreq;
  RecHeader* hRec = get<RecHeader>("/Event/Rec/AdSimple");
  info() << "current event time: " << hRec->recTrigger().triggerTime().GetSec();
  info() << "\t" << hRec->recTrigger().triggerTime().GetNanoSec() << endreq;
  info() << "detector: " << hRec->context().GetDetId() << endreq;
  
  
  for(int i = 1; i <= 4; i++)
  {
    StatusCode sc = promptOneAd(i, hdrSpall);
  }
  
  info() << "number of muon events processed: " << ++m_evtNum << endreq;
  info() << endreq;
  return StatusCode::SUCCESS;
}


StatusCode MuonTaggedByAD::finalize()
{
  debug() << "finalize()" << endreq;
  
  return StatusCode::SUCCESS;
}





StatusCode MuonTaggedByAD::promptOneAd(int i, UserDataHeader* hdrSpall)
{
  stringstream hitADN, triggerNumberADN;
  hitADN << "hitAD" << i;
  triggerNumberADN << "triggerNumber_AD" << i;
  
  if(hdrSpall->getInt(hitADN.str()) != 1) return StatusCode::SUCCESS;
  
  
  
  int trigNum = hdrSpall->getInt(triggerNumberADN.str());
  info() << "AD" << i << " get a muon hit" << endreq;
  info() << "trigger number: " << trigNum << endreq;
  
  // Retrieve RecHeader in AES to look for positron
  SmartDataPtr<DybArchiveList> reclist(p_archiveSvc,"/Event/Rec/AdSimple");
  if (0 == reclist)
  {
    info() << "No RecEvent in archive list." << endreq;
    return StatusCode::SUCCESS;
  }
  
  info() << "Number of RecHeader in ArchiveList: " << reclist->size() << endreq;
  
  for(DybArchiveList::const_iterator iter=reclist->begin(); iter!=reclist->end(); iter++)
  {
    RecHeader* muon = dynamic_cast<RecHeader*>(*iter);
    
    /// Trigger number and detector ID uniquely idetify the event.
    if(muon->recTrigger().triggerNumber() == trigNum &&
       muon->context().GetDetId() == i)
    {
      info() << "a match found" << endreq;
      info() << "sec: " << muon->timeStamp().GetSec() << "\t";
      info() << "nanosec: " << muon->timeStamp().GetNanoSec() << endreq;
      info() << "reconstructed energy: " << muon->recTrigger().energy() << endreq;
      
      /// Once found, quit the loop.
      ///break;
    }
    
    /// if IWS has hits, find its trigger time
    if(hdrSpall->getInt("hitIWS") == 1)
    {
      if(muon->recTrigger().triggerNumber() == hdrSpall->getInt("triggerNumber_IWS")
         && muon->context().GetDetId() == 5)
      {
        info() << "IWS:" << endreq;
        info() << "sec: " << muon->timeStamp().GetSec() << "\t";
        info() << "nanosec: " << muon->timeStamp().GetNanoSec() << endreq;
      }
    }
    
    /// if OWS has hits, find its trigger time
    if(hdrSpall->getInt("hitOWS") == 1)
    {
      if(muon->recTrigger().triggerNumber() == hdrSpall->getInt("triggerNumber_OWS")
         && muon->context().GetDetId() == 6)
      {
        info() << "OWS:" << endreq;
        info() << "sec: " << muon->timeStamp().GetSec() << "\t";
        info() << "nanosec: " << muon->timeStamp().GetNanoSec() << endreq;
      }
    }
    
    /// if RPC has hits, find its trigger time
    if(hdrSpall->getInt("hitRPC") == 1)
    {
      if(muon->recTrigger().triggerNumber() == hdrSpall->getInt("triggerNumber_RPC")
         && muon->context().GetDetId() == 7)
      {
        info() << "RPC:" << endreq;
        info() << "sec: " << muon->timeStamp().GetSec() << "\t";
        info() << "nanosec: " << muon->timeStamp().GetNanoSec() << endreq;
      }
    }
  }
  
  return StatusCode::SUCCESS;
}
