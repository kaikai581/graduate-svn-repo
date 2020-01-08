#include <sstream>
#include "cppconn/connection.h"
#include "cppconn/driver.h"
#include "cppconn/exception.h"
#include "cppconn/prepared_statement.h"
#include "cppconn/resultset.h"
#include "cppconn/statement.h"
#include "Event/UserDataHeader.h"
#include "FirstPassAlg.hpp"


#define DBHOST "tcp://heplinux3.phys.uh.edu:3306"
#define USER "sklin"
#define PASSWORD "aaaa5816"
#define DATABASE "MuonFirstPass_v01"


using namespace DetectorId;
using namespace Site;
using namespace DayaBay;
using namespace sql;
using namespace std;


FirstPassAlg::FirstPassAlg(const string& name, ISvcLocator* svcloc) :
GaudiAlgorithm(name, svcloc), m_updatecount(0), m_exeCntr(1)
{
  declareProperty("RunNum", m_runNumber = "0021358", "run number of the file");
  declareProperty("FileNum", m_fileNumber = "0001", "file number of the file");
}


StatusCode FirstPassAlg::initialize()
{
  /// open database connection
  string url(DBHOST);
  const string user(USER);
  const string password(PASSWORD);
  const string database(DATABASE);
  
  try
  {
    m_driver = get_driver_instance();

    /* create a database connection using the Driver */
    m_connection = m_driver->connect(url, user, password);
    
    /* turn off the autocommit */
    // Note that if auto commit is turned off, one has to commit manually.
    // m_connection->commit()
    m_connection->setAutoCommit(1);
    
    /* select appropriate database schema */
    m_connection->setSchema(database);
    
    info() << "Successfully connected to the database" << endreq;
  }
  catch(SQLException &e)
  {
    error() << "ERROR: SQLException in " << __FILE__;
		error() << " (" << __func__<< ") on line " << __LINE__ << endreq;
		error() << "ERROR: " << e.what();
		error() << " (MySQL error code: " << e.getErrorCode();
		error() << ", SQLState: " << e.getSQLState() << ")" << endreq;
    return StatusCode::FAILURE;
  }
  
  /// insert the dataset information
  stringstream ststr;
  /// prepare insert statement
  ststr << "INSERT IGNORE INTO dataset";
  ststr << " SET runNumber = " << atoi(m_runNumber.c_str());
  ststr << ", fileNumber = " << atoi(m_fileNumber.c_str());
  ststr << ";";
  debug() << ststr.str() << endreq;
  
  try
  {
    /// execute the insert query
    Statement* stmt = m_connection->createStatement();
    stmt->executeUpdate(ststr.str());
    delete stmt;
  }
  catch (SQLException &e)
  {
    printSqlErr(e);
    return StatusCode::FAILURE;
  }
  catch (std::runtime_error &e)
  {
    printRuntimeErr(e);
    return StatusCode::FAILURE;
  }
  
  return StatusCode::SUCCESS;
}


StatusCode FirstPassAlg::finalize()
{
  debug() << "finalize()" << endreq;
  
  m_connection->close();
  delete m_connection;
  
  return StatusCode::SUCCESS;
}


void FirstPassAlg::printRuntimeErr(std::runtime_error& e)
{
  error() << "ERROR: runtime_error in " << __FILE__;
  error() << " (" << __func__ << ") on line " << __LINE__ << endl;
  error() << "ERROR: " << e.what() << endl;
}


void FirstPassAlg::printSqlErr(SQLException& e)
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


StatusCode FirstPassAlg::execute()
{
  /// test if a muon exists in this execution
  if (!(exist<UserDataHeader>(evtSvc(),"/Event/Data/Physics/Spallation")))
    return StatusCode::SUCCESS;
  
  info() << "a muon event found" << endreq;
  m_spall = get<UserDataHeader>("/Event/Data/Physics/Spallation");
  
  /* start inserting muons */
  const Context& context = m_spall->context();
  
  stringstream insstr;
  
  insstr << "INSERT IGNORE INTO muon";
  insstr << " SET timeStampSec = " << (int)context.GetTimeStamp().GetSec();
  insstr << ", timeStampNanoSec = " << (int)context.GetTimeStamp().GetNanoSec();
  insstr << ", hallId = " << (int)context.GetSite();
  insstr << ", runNumber = " << atoi(m_runNumber.c_str());
  insstr << ", fileNumber = " << atoi(m_fileNumber.c_str());
  insstr << ";";
  
  try
  {
    /// execute the insert query
    Statement* stmt = m_connection->createStatement();
    info() << stmt->executeUpdate(insstr.str()) << " muon inserted" << endreq;
    delete stmt;
  }
  catch (SQLException &e)
  {
    printSqlErr(e);
    return StatusCode::FAILURE;
  }
  catch (std::runtime_error &e)
  {
    printRuntimeErr(e);
    return StatusCode::FAILURE;
  }
  
  /// get the current muon id
  stringstream qrymu;
  
  qrymu << "SELECT muonId from muon WHERE";
  qrymu << " timeStampSec = " << (int)context.GetTimeStamp().GetSec();
  qrymu << " AND timeStampNanoSec = " << (int)context.GetTimeStamp().GetNanoSec();
  qrymu << " AND hallId = " << (int)context.GetSite();
  qrymu << ";";
  
  int muonId = -1;
  
  try
  {
    Statement* stmt = m_connection->createStatement();
    ResultSet* res;
    res = stmt->executeQuery(qrymu.str());
    /// check if the result has exactly one record
    if(res->rowsCount() != 1) {
      error() << "no or multiple muon id" << endreq;
      return StatusCode::FAILURE;
    }
    while (res->next()) {
      muonId = res->getInt("muonId");
      info() << "muon id: " << muonId << endreq;
    }
    delete res;
    delete stmt;
  }
  catch (SQLException &e)
  {
    printSqlErr(e);
    return StatusCode::FAILURE;
  }
  catch (std::runtime_error &e)
  {
    printRuntimeErr(e);
    return StatusCode::FAILURE;
  }
  
  
  /// for each detector, if it is triggered, insert the trigger Id
  map<int, int> detHitMap;
  
  detHitMap[1] = m_spall->getInt("hitAD1");
  detHitMap[2] = m_spall->getInt("hitAD2");
  detHitMap[3] = m_spall->getInt("hitAD3");
  detHitMap[4] = m_spall->getInt("hitAD4");
  detHitMap[5] = m_spall->getInt("hitIWS");
  detHitMap[6] = m_spall->getInt("hitOWS");
  detHitMap[7] = m_spall->getInt("hitRPC");
  
  /// strings for corresponding trigger numbers and dt to muon
  map<int, string> trigNumStr;
  
  trigNumStr[1] = "triggerNumber_AD1";
  trigNumStr[2] = "triggerNumber_AD2";
  trigNumStr[3] = "triggerNumber_AD3";
  trigNumStr[4] = "triggerNumber_AD4";
  trigNumStr[5] = "triggerNumber_IWS";
  trigNumStr[6] = "triggerNumber_OWS";
  trigNumStr[7] = "triggerNumber_RPC";
  
  int totDet = 0;
  
  for(int detId = 1; detId <= 7; detId++)
  {
    if(detHitMap[detId] != 1) continue;
    
    stringstream insstr2;
    
    insstr2 << "INSERT IGNORE INTO muonConstituent";
    insstr2 << " SET triggerNumber=" << m_spall->getInt(trigNumStr[detId]);
    insstr2 << ", detectorId=" << detId;
    insstr2 << ", muonId=" << muonId;
    insstr2 << ";";
    
    try
    {
      /// execute the insert query
      Statement* stmt = m_connection->createStatement();
      totDet += stmt->executeUpdate(insstr2.str());
      delete stmt;
    }
    catch (SQLException &e)
    {
      printSqlErr(e);
      return StatusCode::FAILURE;
    }
    catch (std::runtime_error &e)
    {
      printRuntimeErr(e);
      return StatusCode::FAILURE;
    }
  }
  
  info() << totDet << " detectors in this muon inserted" << endreq;
  
  /// output progress
  if(m_exeCntr%1000 == 0)
    info() << m_exeCntr << " events are processed" << endreq;
  m_exeCntr++;
  
  return StatusCode::SUCCESS;
}
