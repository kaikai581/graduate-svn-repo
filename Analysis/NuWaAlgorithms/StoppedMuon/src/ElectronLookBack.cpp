#include <sstream>
#include "ElectronLookBack.hpp"
#include "Event/RegistrationSequence.h"
#include "DataUtilities/DybArchiveList.h"


using namespace std;
using namespace DayaBay;





ElectronLookBack::ElectronLookBack(const string& name, ISvcLocator* svcloc) :
GaudiAlgorithm(name, svcloc), m_michel(0)
{
}


ElectronLookBack::~ElectronLookBack()
{
}



void ElectronLookBack::delist()
{
  vector<string> objpath;
  objpath.push_back(RegistrationSequenceLocation::Default);

  RegistrationSequence* seq = get<RegistrationSequence>(evtSvc(), RegistrationSequenceLocation::Default);
  IRegistrationSequence::Registrations& regs = const_cast<IRegistrationSequence::Registrations&>(seq->registrations());
  IRegistrationSequence::Registrations::iterator regit;
  for(regit = regs.begin(); regit != regs.end(); regit++) {
    objpath.push_back(regit->path());
    info() << regit->path() << endreq;
  }

  //for(unsigned int i=0; i<objpath.size(); i++) {
    //SmartDataPtr<DybArchiveList> archlist(p_archiveSvc,objpath[i]);
    //if(archlist==0) continue;
    //if(archlist->size()==0) continue;
    //archlist->erase(archlist->begin());
  //}
}




StatusCode ElectronLookBack::initialize()
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



StatusCode ElectronLookBack::execute()
{
  debug() << "execute() start " << endreq;
  
  
  // Retrieve RecHeader in AES to look for muons.
  // If there are no muons at all, quit this loop.
  SmartDataPtr<DybArchiveList> muonList(p_archiveSvc,"/Event/Data/Physics/Spallation");
  if (0 == muonList)
  {
    debug() << "No muons in archive list." << endreq;
    return StatusCode::SUCCESS;
  }
  
  
  debug() << muonList->size() << " muons in archive" << endreq;
  
  m_michel = get<RecHeader>("/Event/Rec/AdSimple");
  
  /// looking for muons in the same AD as the current delayed event
  for(DybArchiveList::const_iterator iter=muonList->begin(); iter!=muonList->end(); iter++)
  {
    UserDataHeader* muon = dynamic_cast<UserDataHeader*>(*iter);
    
    stringstream hitAdId;
    hitAdId << "hitAD" << m_michel->context().GetDetId();
    
    /// if in the same AD
    if(muon->getInt(hitAdId.str()) == 1)
    {
      printDebugInfo(muon);
      
      delist();
      /// Once found, the electron will be associated with it and match no more.
      break;
    }
  }
  
  
  return StatusCode::SUCCESS;
}




StatusCode ElectronLookBack::finalize()
{
  debug() << "finalize()" << endreq;
  
  return StatusCode::SUCCESS;
}





void ElectronLookBack::printDebugInfo(UserDataHeader* muon)
{
  info() << "AD" << m_michel->context().GetDetId() << " got a muon hit" << endreq;
  
  info() << "delayed signal time: " << m_michel->recTrigger().triggerTime().GetSec();
  info() << "\t" << m_michel->recTrigger().triggerTime().GetNanoSec() << endreq;
  
  
  info() << endreq;
}

