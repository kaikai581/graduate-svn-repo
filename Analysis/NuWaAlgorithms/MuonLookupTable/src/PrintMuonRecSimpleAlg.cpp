#include "Event/UserDataHeader.h"
#include "PrintMuonRecSimpleAlg.hpp"

using namespace DayaBay;
using namespace std;

PrintMuonRecSimpleAlg::PrintMuonRecSimpleAlg(const string& name, ISvcLocator* svcloc) :
GaudiAlgorithm(name, svcloc)
{
}


StatusCode PrintMuonRecSimpleAlg::execute()
{
  if(exist<UserDataHeader>(evtSvc(),"/Event/Data/Physics/MuonRecSimple"))
  {
    UserDataHeader* muRec;
    muRec = get<UserDataHeader>("/Event/Data/Physics/MuonRecSimple");
    
    if(muRec->getIntArray("DetectorId").size() >= 3)
    {
      info() << "detector IDs: ";
      for(unsigned int i = 0; i < muRec->getIntArray("DetectorId").size(); i++)
        info() << muRec->getIntArray("DetectorId")[i] << " ";
      info() << endreq;
      
      info() << "(start,end): (";
      info() << muRec->getInt("StartIndex") << ",";
      info() << muRec->getInt("EndIndex") << ")" << endreq;
    }
    
  }
  
  return StatusCode::SUCCESS;
}


StatusCode PrintMuonRecSimpleAlg::finalize()
{
  return StatusCode::SUCCESS;
}


StatusCode PrintMuonRecSimpleAlg::initialize()
{
  return StatusCode::SUCCESS;
}
