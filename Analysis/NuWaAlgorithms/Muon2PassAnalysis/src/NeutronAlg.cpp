#include <string>

#include "NeutronAlg.hpp"

#include "cppconn/exception.h"
#include "Math/AxisAngle.h"


using namespace ROOT::Math;
using namespace sql;
using namespace std;


NeutronAlg::NeutronAlg(const string& name, ISvcLocator* svcloc) :
GaudiAlgorithm(name, svcloc), m_exeCntr(1)
{
  declareProperty("RunNum", m_runNumber = "0021358", "run number of the file");
  declareProperty("FileNum", m_fileNumber = "0001", "file number of the file");
}


StatusCode NeutronAlg::execute()
{
  
  /// output progress
  if(m_exeCntr%1000 == 0)
    info() << m_exeCntr << " events are processed" << endreq;
  m_exeCntr++;
  
  return StatusCode::SUCCESS;
}


StatusCode NeutronAlg::finalize()
{
  return StatusCode::SUCCESS;
}


StatusCode NeutronAlg::initialize()
{
  return StatusCode::SUCCESS;
}


void NeutronAlg::printRuntimeErr(std::runtime_error& e)
{
  error() << "ERROR: runtime_error in " << __FILE__;
  error() << " (" << __func__ << ") on line " << __LINE__ << endl;
  error() << "ERROR: " << e.what() << endl;
}


void NeutronAlg::printSqlErr(SQLException& e)
{
  error() << "ERROR: SQLException in " << __FILE__;
  error() << " (" << __func__<< ") on line " << __LINE__ << endl;
  error() << "ERROR: " << e.what();
  error() << " (MySQL error code: " << e.getErrorCode();
  error() << ", SQLState: " << e.getSQLState() << ")" << endl;
  
  if (e.getErrorCode() == 1047)
  {
    error() << "\nYour server does not seem to support Prepared Statements at all. ";
    error() << "Perhaps MYSQL < 4.1?" << endl;
  }
}
