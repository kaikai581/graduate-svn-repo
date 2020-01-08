#include "AESTest.hpp"
#include "DataUtilities/DybArchiveList.h"
#include "Event/RecHeader.h"


using namespace std;
using namespace DayaBay;


AESTest::AESTest(const string& name, ISvcLocator* svcloc) :
GaudiAlgorithm(name, svcloc), m_execNum(0)
{
}


AESTest::~AESTest()
{
}



StatusCode AESTest::initialize()
{
  // Retrieve archive service
  StatusCode status = service("EventDataArchiveSvc", m_archiveSvc);
  if (status.isFailure())
  {
    Error("Service [EventDataArchiveSvc] not found", status);
  }
  
  return StatusCode::SUCCESS;
}



StatusCode AESTest::finalize()
{
  debug() << "finalize()" << endreq;
  
  return StatusCode::SUCCESS;
}




StatusCode AESTest::execute()
{
  SmartDataPtr<DybArchiveList> recList(m_archiveSvc,"/Event/Rec/AdSimple");
  if (recList == 0)
  {
    debug() << "No reconstructed event in archive list." << endreq;
    return StatusCode::SUCCESS;
  }
  
  
  RecHeader* curEvt = get<RecHeader>("/Event/Rec/AdSimple");
  info() << endreq;
  debug() << "current trigger number: " << curEvt->recTrigger().triggerNumber();
  debug() << "\tdetID:" << curEvt->context().GetDetId() << endreq;
  if(++m_execNum == 3)
  {
    recList->erase(recList->begin());
  }
  
  DybArchiveList::const_iterator recIt = recList->begin();
  int archNum = 0;
  for(; recIt!=recList->end(); recIt++)
  {
    RecHeader* archEvt = dynamic_cast<RecHeader*>(*recIt);
    debug() << "#" << ++archNum << "\t";
    debug() << archEvt->recTrigger().triggerNumber() << "\t";
    debug() << archEvt->context().GetDetId() << endreq;
  }
  
  info() << endreq;
  
  return StatusCode::SUCCESS;
}
