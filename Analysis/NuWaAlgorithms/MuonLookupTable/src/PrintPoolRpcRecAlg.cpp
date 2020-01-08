#include "Event/RecHeader.h"
#include "PrintPoolRpcRecAlg.hpp"

using namespace DayaBay;
using namespace std;

PrintPoolRpcRecAlg::PrintPoolRpcRecAlg(const string& name, ISvcLocator* svcloc) :
GaudiAlgorithm(name, svcloc)
{
}


StatusCode PrintPoolRpcRecAlg::execute()
{
  RecHeader* recPoolHeader = get<RecHeader>("/Event/Rec/PoolSimple");
  const RecTrigger& recPoolTrig = recPoolHeader->recTrigger();
  if(recPoolTrig.detector().isWaterShield())
  {
    info() << "detector: " << (int)recPoolTrig.detector().detectorId() << endreq;
    info() << "trigger number: " << recPoolTrig.triggerNumber() << endreq;
    info() << recPoolTrig.position().x() << " ";
    info() << recPoolTrig.position().y() << " ";
    info() << recPoolTrig.position().z() << endreq;
  }
  
  return StatusCode::SUCCESS;
}


StatusCode PrintPoolRpcRecAlg::finalize()
{
  return StatusCode::SUCCESS;
}


StatusCode PrintPoolRpcRecAlg::initialize()
{
  return StatusCode::SUCCESS;
}
