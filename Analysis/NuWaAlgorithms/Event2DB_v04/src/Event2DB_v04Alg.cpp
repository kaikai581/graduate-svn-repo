#include <sstream>
#include "cppconn/connection.h"
#include "cppconn/driver.h"
#include "cppconn/exception.h"
#include "cppconn/prepared_statement.h"
#include "cppconn/resultset.h"
#include "cppconn/statement.h"
#include "Context/Context.h"
#include "DataUtilities/DybArchiveList.h"
#include "Event/CalibReadoutHeader.h"
#include "Event/CalibReadoutPmtCrate.h"
#include "Event/CalibReadoutRpcCrate.h"
#include "Event/ReadoutHeader.h"
#include "Event/RecHeader.h"
#include "Event/RecRpcHeader.h"
#include "Event/UserDataHeader.h"
#include "Event2DB_v04Alg.hpp"



#define DBHOST "tcp://heplinux9.phys.uh.edu:3306"
#define USER "sklin"
#define PASSWORD "aaaa5816"
#define DATABASE "`EventDisplayDB_v0.4`"



using namespace DayaBay;
using namespace sql;
using namespace std;


Event2DB_v04Alg::Event2DB_v04Alg(const string& name, ISvcLocator* svcloc) :
GaudiAlgorithm(name, svcloc), m_updatecount(0), m_exeCntr(1)
{
}


StatusCode Event2DB_v04Alg::initialize()
{
  // Retrieve archive service
  StatusCode status = service("EventDataArchiveSvc", m_archiveSvc);
  if (status.isFailure())
  {
    Error("Service [EventDataArchiveSvc] not found", status);
  }
  
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
    printSqlErr(e);
    return StatusCode::FAILURE;
  }
  
  return StatusCode::SUCCESS;
}



StatusCode Event2DB_v04Alg::finalize()
{
  debug() << "finalize()" << endreq;
  
  m_connection->close();
  delete m_connection;
  
  return StatusCode::SUCCESS;
}



StatusCode Event2DB_v04Alg::execute()
{
  debug() << "execute()" << endreq;
  
  /// For every event, get its calibration readout.
  m_calibRoHdr = get<CalibReadoutHeader>("/Event/CalibReadout/CalibReadoutHeader");
  if(!m_calibRoHdr) {
    error() << "Can't get calibration readout header." << endreq;
    return StatusCode::FAILURE;
  }
  
  const CalibReadout* calibReadout = m_calibRoHdr->calibReadout();
  if(!calibReadout) {
    error()<<"Failed to get CalibReadout from header"<<endreq;
    return StatusCode::FAILURE;
  }
  
  if( !(calibReadout->detector().isAD() ||
        calibReadout->detector().isWaterShield() ||
        calibReadout->detector().isRPC()) ) {
    info() << "this execution doesn't contain a valid detector." << endreq;
    return StatusCode::SUCCESS;
  }
  
  processTrigger();
  
  /// test if a muon exists in this execution
  if (!(exist<UserDataHeader>(evtSvc(),"/Event/Data/Physics/Spallation")))
    return StatusCode::SUCCESS;
  
  debug() << "a muon event found" << endreq;
  m_spall = get<UserDataHeader>("/Event/Data/Physics/Spallation");
  
  processMuon();
  
  /// output progress
  if(m_exeCntr%1000 == 0)
    info() << m_exeCntr << " events are processed" << endreq;
  m_exeCntr++;
  
  return StatusCode::SUCCESS;
}


void Event2DB_v04Alg::printRuntimeErr(std::runtime_error& e)
{
  error() << "ERROR: runtime_error in " << __FILE__;
  error() << " (" << __func__ << ") on line " << __LINE__ << endl;
  error() << "ERROR: " << e.what() << endl;
}


void Event2DB_v04Alg::printSqlErr(SQLException& e)
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


void Event2DB_v04Alg::processAd(int triggerId)
{
  const CalibReadout* calibReadout = m_calibRoHdr->calibReadout();
  const Context& context = m_calibRoHdr->context();
  
  /// write adPmtCalibStats table
  
  const CalibReadoutPmtCrate* pmtCrate = dynamic_cast<const CalibReadoutPmtCrate*>(calibReadout);
  
  CalibReadoutPmtCrate::PmtChannelReadouts channels = pmtCrate->channelReadout();
  
  CalibReadoutPmtCrate::PmtChannelReadouts::const_iterator channelIter,
  channelDone = channels.end();
  
  int totInsPmt = 0;
  
  for(channelIter=channels.begin();channelIter!=channelDone;++channelIter)
  {
    const CalibReadoutPmtChannel& channel = *channelIter;
    
    AdPmtSensor pmtId(channel.pmtSensorId().fullPackedData());
    
    stringstream insstr;
    
    insstr << "INSERT IGNORE INTO adPmtCalibStats (triggerId, adPmtId, ";
    insstr << "totCharge)";
    insstr << " SELECT " << triggerId;
    insstr << ", adPmtId";
    insstr << ", " << channel.sumCharge();
    insstr << " FROM adPmt";
    insstr << " WHERE hallId = " << (int)context.GetSite();
    insstr << " AND detectorId = " << (int)context.GetDetId();
    insstr << " AND ring = " << pmtId.ring();
    insstr << " AND col = " << pmtId.column();
    insstr << ";";
    
    debug() << insstr.str() << endreq;
    
    try
    {
      /// execute the insert query
      Statement* stmt = m_connection->createStatement();
      totInsPmt += stmt->executeUpdate(insstr.str());
      delete stmt;
    }
    catch (SQLException &e)
    {
      printSqlErr(e);
      return;
    }
    catch (std::runtime_error &e)
    {
      printRuntimeErr(e);
      return;
    }
  }
  
  info() << totInsPmt << " AD PMT charge inserted" << endreq;
  
}


void Event2DB_v04Alg::processMuon()
{
  const Context& context = m_spall->context();
  
  stringstream insstr;
  
  insstr << "INSERT IGNORE INTO muon";
  insstr << " SET timeStampSec = " << (int)context.GetTimeStamp().GetSec();
  insstr << ", timeStampNanoSec = " << (int)context.GetTimeStamp().GetNanoSec();
  insstr << ", hallId = " << (int)context.GetSite();
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
    return;
  }
  catch (std::runtime_error &e)
  {
    printRuntimeErr(e);
    return;
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
      return;
    }
    while (res->next()) {
      muonId = res->getInt("muonId");
      debug() << "muon id: " << muonId << endreq;
    }
    delete res;
    delete stmt;
  }
  catch (SQLException &e)
  {
    printSqlErr(e);
    return;
  }
  catch (std::runtime_error &e)
  {
    printRuntimeErr(e);
    return;
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
  map<int, string> trigNumName, dtMuName;
  
  trigNumName[1] = "triggerNumber_AD1";
  trigNumName[2] = "triggerNumber_AD2";
  trigNumName[3] = "triggerNumber_AD3";
  trigNumName[4] = "triggerNumber_AD4";
  trigNumName[5] = "triggerNumber_IWS";
  trigNumName[6] = "triggerNumber_OWS";
  trigNumName[7] = "triggerNumber_RPC";
  
  dtMuName[1] = "dtAD1_ms";
  dtMuName[2] = "dtAD2_ms";
  dtMuName[3] = "dtAD3_ms";
  dtMuName[4] = "dtAD4_ms";
  dtMuName[5] = "dtIWS_ms";
  dtMuName[6] = "dtOWS_ms";
  dtMuName[7] = "dtRPC_ms";
  
  int totDet = 0;
  
  for(int detId = 1; detId <= 7; detId++)
  {
    if(detHitMap[detId] != 1) continue;
    
    stringstream insstr2;
    
    insstr2 << "INSERT IGNORE INTO muonTriggerConstituent";
    insstr2 << " (triggerId, muonId, dtMu)";
    insstr2 << " SELECT triggerId, " << muonId << ", ";
    insstr2 << m_spall->getFloat(dtMuName[detId]);
    insstr2 << " FROM `trigger` WHERE";
    insstr2 << " triggerNumber = " << m_spall->getInt(trigNumName[detId]);
    insstr2 << " AND detectorId = " << detId;
    insstr2 << " AND hallId = " << (int)context.GetSite();
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
      return;
    }
    catch (std::runtime_error &e)
    {
      printRuntimeErr(e);
      return;
    }
  }
  
  info() << totDet << " detectors in this muon inserted" << endreq;
  
}


void Event2DB_v04Alg::processRpc(int triggerId)
{
  const CalibReadout* calibReadout = m_calibRoHdr->calibReadout();
  const Context& context = m_calibRoHdr->context();

  /// write rpcStripCalib table
  
  const CalibReadoutRpcCrate* rpcCrate = dynamic_cast<const CalibReadoutRpcCrate*>(calibReadout);
  
  CalibReadoutRpcCrate::RpcChannelReadouts channels = rpcCrate->channelReadout();
  
  CalibReadoutRpcCrate::RpcChannelReadouts::const_iterator channelIter,
  channelDone = channels.end();
  
  int totInsStrip = 0;
  
  for(channelIter=channels.begin();channelIter!=channelDone;++channelIter)
  {
    const CalibReadoutRpcChannel& channel = *channelIter;
    
    RpcSensor rpcId(channel.rpcSensorId().fullPackedData());
    
    stringstream insstr;
    
    insstr << "INSERT IGNORE INTO rpcStripCalib (triggerId, rpcStripId)";
    insstr << " SELECT " << triggerId;
    insstr << ", rpcStripId";
    insstr << " FROM rpcStrip";
    insstr << " WHERE hallId = " << (int)context.GetSite();
    insstr << " AND detectorId = " << (int)context.GetDetId();
    insstr << " AND row = " << rpcId.panelRow();
    insstr << " AND col = " << rpcId.panelColumn();
    insstr << " AND layer = " << rpcId.layer();
    insstr << " AND strip = " << rpcId.strip();
    insstr << ";";
    
    debug() << insstr.str() << endreq;
    
    try
    {
      /// execute the insert query
      Statement* stmt = m_connection->createStatement();
      totInsStrip += stmt->executeUpdate(insstr.str());
      delete stmt;
    }
    catch (SQLException &e)
    {
      printSqlErr(e);
      return;
    }
    catch (std::runtime_error &e)
    {
      printRuntimeErr(e);
      return;
    }
  }
  
  info() << totInsStrip << " RPC strips inserted" << endreq;
  
}


void Event2DB_v04Alg::processTrigger()
{
  const CalibReadout* calibReadout = m_calibRoHdr->calibReadout();
  
  const Context& context = m_calibRoHdr->context();
  stringstream insstr;
  insstr << "INSERT IGNORE INTO `trigger`";
  insstr << " SET `hallId` = " << (int)context.GetSite();
  insstr << ", `detectorId` = " << (int)context.GetDetId();
  insstr << ", `dateNTime` = '" << context.GetTimeStamp().AsString("SQL") << "'";
  insstr << ", `triggerNumber` = " << calibReadout->triggerNumber();
  insstr << ", `triggerTimeSec` = " << calibReadout->triggerTime().GetSec();
  insstr << ", `triggerTimeNanoSec` = " << calibReadout->triggerTime().GetNanoSec();
  insstr << ";";
  
  debug() << insstr.str() << endreq;
  
  try
  {
    /// filling muon trigger table
    Statement* stmt = m_connection->createStatement();
    int updatecount = stmt->executeUpdate(insstr.str());
    info() << updatecount << " trigger inserted" << endreq;
    delete stmt;
    
    /// retrieve the id just inserted
    stringstream qrystr;
    qrystr << "SELECT triggerId FROM `trigger` WHERE";
    qrystr << " hallId = " << (int)context.GetSite();
    qrystr << " AND detectorId = " << (int)context.GetDetId();
    qrystr << " AND triggerNumber = " << calibReadout->triggerNumber();
    qrystr << ";";
    
    debug() << qrystr.str() << endreq;
   
    int triggerId = 0;
    
    try
    {
      Statement* stmt = m_connection->createStatement();
      ResultSet* res;
      res = stmt->executeQuery(qrystr.str());
      /// check if the result has exactly one record
      if(res->rowsCount() != 1) {
        error() << "no or multiple trigger id" << endreq;
        return;
      }
      while (res->next()) {
        triggerId = res->getInt("triggerId");
        debug() << "trigger id: " << triggerId << endreq;
      }
      delete res;
      delete stmt;
    }
    catch (SQLException &e)
    {
      printSqlErr(e);
      return;
    }
    catch (std::runtime_error &e)
    {
      printRuntimeErr(e);
      return;
    }
    
    if(calibReadout->detector().isAD())          processAd(triggerId);
    if(calibReadout->detector().isWaterShield()) processWs(triggerId);
    if(calibReadout->detector().isRPC())         processRpc(triggerId);
  }
  catch (SQLException &e)
  {
    printSqlErr(e);
    return;
  }
  catch (std::runtime_error &e)
  {
    printRuntimeErr(e);
    return;
  }
}


void Event2DB_v04Alg::processWs(int triggerId)
{
  const CalibReadout* calibReadout = m_calibRoHdr->calibReadout();
  const Context& context = m_calibRoHdr->context();

  /// write wsPmtCalibStats table
  
  const CalibReadoutPmtCrate* pmtCrate = dynamic_cast<const CalibReadoutPmtCrate*>(calibReadout);
  
  CalibReadoutPmtCrate::PmtChannelReadouts channels = pmtCrate->channelReadout();
  
  CalibReadoutPmtCrate::PmtChannelReadouts::const_iterator channelIter,
  channelDone = channels.end();
  
  int totInsPmt = 0;
  
  for(channelIter=channels.begin();channelIter!=channelDone;++channelIter)
  {
    const CalibReadoutPmtChannel& channel = *channelIter;
    
    PoolPmtSensor pmtId(channel.pmtSensorId().fullPackedData());
    
    stringstream insstr;
    
    insstr << "INSERT IGNORE INTO wsPmtCalibStats (triggerId, wsPmtId, ";
    insstr << "totCharge)";
    insstr << " SELECT " << triggerId;
    insstr << ", wsPmtId";
    insstr << ", " << channel.sumCharge();
    insstr << " FROM wsPmt";
    insstr << " WHERE hallId = " << (int)context.GetSite();
    insstr << " AND detectorId = " << (int)context.GetDetId();
    insstr << " AND wallNumber = " << pmtId.wallNumber();
    insstr << " AND wallSpot = " << pmtId.wallSpot();
    insstr << " AND inwardFacing = " << (int)pmtId.inwardFacing();
    insstr << ";";
    
    debug() << insstr.str() << endreq;
    
    try
    {
      /// execute the insert query
      Statement* stmt = m_connection->createStatement();
      totInsPmt += stmt->executeUpdate(insstr.str());
      delete stmt;
    }
    catch (SQLException &e)
    {
      printSqlErr(e);
      return;
    }
    catch (std::runtime_error &e)
    {
      printRuntimeErr(e);
      return;
    }
  }
  
  info() << totInsPmt << " Pool PMT charge inserted" << endreq;
  
}
