#include <sstream>
#include <sys/unistd.h> /// for hostname retrieval
#include "boost/algorithm/string.hpp"
#include "cppconn/connection.h"
#include "cppconn/driver.h"
#include "cppconn/exception.h"
#include "cppconn/prepared_statement.h"
#include "cppconn/resultset.h"
#include "cppconn/statement.h"
#include "Context/Context.h"
#include "DataUtilities/DybArchiveList.h"
#include "DetDesc/DetectorElement.h"
#include "DetDesc/IGeometryInfo.h"
#include "Event/CalibReadoutHeader.h"
#include "Event/CalibReadoutPmtCrate.h"
#include "Event/CalibReadoutRpcCrate.h"
#include "Event/ReadoutHeader.h"
#include "Event/RecHeader.h"
#include "Event/RecRpcHeader.h"
#include "Event/UserDataHeader.h"
#include "Event2DB_v043Alg.hpp"



#define DBHOST "tcp://heplinux3.phys.uh.edu:3306"
#define USER "sklin"
#define PASSWORD "aaaa5816"
#define DATABASE "EventDisplayDB_v043"



using namespace DetectorId;
using namespace Site;
using namespace DayaBay;
using namespace sql;
using namespace std;


Event2DB_v043Alg::Event2DB_v043Alg(const string& name, ISvcLocator* svcloc) :
GaudiAlgorithm(name, svcloc), m_updatecount(0), m_exeCntr(1)
{
  
  /// declare properties for module arguments
  declareProperty("InFileName", m_infilename = "input.root", "input root file name");
  
  /// assign the reference detector strings
  m_referenceDetector[kDayaBay][kAD1] = "/dd/Structure/DayaBay/db-rock/db-ows/db-curtain/db-iws/db-ade1/db-sst1/db-oil1";
  m_referenceDetector[kDayaBay][kAD2] = "/dd/Structure/DayaBay/db-rock/db-ows/db-curtain/db-iws/db-ade2/db-sst2/db-oil2";
  m_referenceDetector[kDayaBay][kIWS] = "/dd/Structure/DayaBay/db-rock/db-ows";
  m_referenceDetector[kDayaBay][kOWS] = "/dd/Structure/DayaBay/db-rock/db-ows";
  m_referenceDetector[kDayaBay][kRPC] = "/dd/Structure/DayaBay/db-rock/db-rpc";
  
  m_referenceDetector[kLingAo][kAD1] = "/dd/Structure/DayaBay/la-rock/la-ows/la-curtain/la-iws/la-ade1/la-sst1/la-oil1";
  m_referenceDetector[kLingAo][kAD2] = "/dd/Structure/DayaBay/la-rock/la-ows/la-curtain/la-iws/la-ade2/la-sst2/la-oil2";
  m_referenceDetector[kLingAo][kIWS] = "/dd/Structure/DayaBay/la-rock/la-ows";
  m_referenceDetector[kLingAo][kOWS] = "/dd/Structure/DayaBay/la-rock/la-ows";
  m_referenceDetector[kLingAo][kRPC] = "/dd/Structure/DayaBay/la-rock/la-rpc";
  
  m_referenceDetector[kFar][kAD1] = "/dd/Structure/DayaBay/far-rock/far-ows/far-curtain/far-iws/far-ade1/far-sst1/far-oil1";
  m_referenceDetector[kFar][kAD2] = "/dd/Structure/DayaBay/far-rock/far-ows/far-curtain/far-iws/far-ade2/far-sst2/far-oil2";
  m_referenceDetector[kFar][kAD3] = "/dd/Structure/DayaBay/far-rock/far-ows/far-curtain/far-iws/far-ade3/far-sst3/far-oil3";
  m_referenceDetector[kFar][kAD4] = "/dd/Structure/DayaBay/far-rock/far-ows/far-curtain/far-iws/far-ade4/far-sst4/far-oil4";
  m_referenceDetector[kFar][kIWS] = "/dd/Structure/DayaBay/far-rock/far-ows";
  m_referenceDetector[kFar][kOWS] = "/dd/Structure/DayaBay/far-rock/far-ows";
  m_referenceDetector[kFar][kRPC] = "/dd/Structure/DayaBay/far-rock/far-rpc";
  
  /// assign RPC offsets
  m_rpcRecOffset[kDayaBay] = Gaudi::XYZPoint(2500.,-500.,12500.);
  m_rpcRecOffset[kLingAo] = Gaudi::XYZPoint(2500.,-500.,12500.);
  m_rpcRecOffset[kFar] = Gaudi::XYZPoint(5650.,500.,12500.);
}


StatusCode Event2DB_v043Alg::initialize()
{
  string url(DBHOST);
  
  /// if this alg is run locally, set the url to localhost
  char hostname[128];
  gethostname(hostname, sizeof(hostname));
  if(string(hostname) == "heplinux3") url = "localhost";
  
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
    // NOTE: Auto commit greatly reduces the performance!
    m_connection->setAutoCommit(0);
    
    /* select appropriate database schema */
    m_connection->setSchema(database);
    
    info() << "Successfully connected to the database" << endreq;
  }
  catch(SQLException &e)
  {
    printSqlErr(e);
    return StatusCode::FAILURE;
  }
  
  vector<std::string> strs;
  boost::split(strs, m_infilename, boost::is_any_of("/"));
  unsigned int strsize = strs.size();
  info() << strs[strsize-5] << endreq;
  info() << strs[strsize-2] << endreq;
  boost::split(strs, m_infilename, boost::is_any_of("."));
  strsize = strs.size();
  string runstr = strs[strsize-2];
  runstr = runstr.substr(1,runstr.size()-1);
  info() << atoi(strs[strsize-6].c_str()) << endreq;
  info() << atoi(runstr.c_str()) << endreq;
  
  return StatusCode::SUCCESS;
}



StatusCode Event2DB_v043Alg::finalize()
{
  debug() << "finalize()" << endreq;
  
  if(!m_connection->getAutoCommit()) m_connection->commit();
  m_connection->close();
  delete m_connection;
  
  return StatusCode::SUCCESS;
}



StatusCode Event2DB_v043Alg::execute()
{
  info() << "execute()" << endreq;
  
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


void Event2DB_v043Alg::printRuntimeErr(std::runtime_error& e)
{
  error() << "ERROR: runtime_error in " << __FILE__;
  error() << " (" << __func__ << ") on line " << __LINE__ << endl;
  error() << "ERROR: " << e.what() << endl;
}


void Event2DB_v043Alg::printSqlErr(SQLException& e)
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


void Event2DB_v043Alg::processAdCalib(int triggerId)
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
    
    insstr << "INSERT IGNORE INTO calibAdPmt (detectorTriggerId, adPmtId, ";
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
    
    int calibAdPmtId = -1;
    
    stringstream qrystr;
    
    qrystr << "SELECT c.calibAdPmtId from calibAdPmt c";
    qrystr << " INNER JOIN (";
    qrystr << " SELECT adPmtId from adPmt WHERE hallId = " << (int)context.GetSite();
    qrystr << " AND detectorId = " << (int)context.GetDetId();
    qrystr << " AND ring = " << pmtId.ring();
    qrystr << " AND col = " << pmtId.column();
    qrystr << " ) a ON c.adPmtId = a.adPmtId";
    qrystr << " WHERE c.detectorTriggerId = " << triggerId;
    
    debug() << qrystr.str() << endreq;
    
    try
    {
      Statement* stmt = m_connection->createStatement();
      ResultSet* res;
      res = stmt->executeQuery(qrystr.str());
      /// check if the result has exactly one record
      if(res->rowsCount() != 1) {
        error() << "no or multiple AD PMT id" << endreq;
        return;
      }
      while (res->next())
        calibAdPmtId = res->getInt("calibAdPmtId");
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
    
    /// start filling TDC table
    int totInsTdc = 0;
    
    for(unsigned int hitIdx = 0; hitIdx < channel.size(); hitIdx++)
    {
      stringstream instdc;
      
      instdc << "INSERT IGNORE INTO calibAdPmtTdc";
      instdc << " SET calibAdPmtId = " << calibAdPmtId;
      instdc << ", time_ps = " << (int)(channel.time(hitIdx)*1000);
      instdc << ", charge = " << channel.charge(hitIdx);
      instdc << ";";
      
      try
      {
        /// execute the insert query
        Statement* stmt = m_connection->createStatement();
        totInsTdc += stmt->executeUpdate(instdc.str());
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
    
    info() << totInsTdc << " TDC hits inserted" << endreq;
  }
  
  info() << totInsPmt << " AD PMT charge inserted" << endreq;
  
}


void Event2DB_v043Alg::processAdRec(int triggerId)
{
  RecHeader* recHeader = get<RecHeader>("/Event/Rec/AdSimple");
  if(!recHeader) {
    error() << "failed to get AD reconstructed header" << endreq;
    return;
  }
  
  const Context& context = recHeader->context();
  const RecTrigger& recAdTrig = recHeader->recTrigger();
  
  DetectorElement* de = getDet<DetectorElement>(m_referenceDetector[context.GetSite()][context.GetDetId()]);
  Gaudi::XYZPoint gblAdRec = de->geometry()->toGlobal(Gaudi::XYZPoint(recAdTrig.position().x(),recAdTrig.position().y(),recAdTrig.position().z()));
  
  stringstream insstr;
  
  insstr << "INSERT IGNORE INTO recAdSimple";
  insstr << " SET detectorTriggerId = " << triggerId;
  insstr << ", energy = " << recAdTrig.energy();
  insstr << ", energyStatus = " << recAdTrig.energyStatus();
  insstr << ", x = " << gblAdRec.X();
  insstr << ", y = " << gblAdRec.Y();
  insstr << ", z = " << gblAdRec.Z();
  insstr << ", positionStatus = " << recAdTrig.positionStatus();
  insstr << ";";
  
  debug() << insstr.str() << endreq;
  
  try
  {
    /// execute the insert query
    Statement* stmt = m_connection->createStatement();
    info() << stmt->executeUpdate(insstr.str()) << " AD reconstruction record(s) inserted" << endreq;
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


void Event2DB_v043Alg::processMuon()
{
  const Context& context = m_spall->context();
  const TimeStamp& ts = m_spall->timeStamp();
  
  
  /// get run number and file number
  vector<string> strs;
  boost::split(strs, m_infilename, boost::is_any_of("."));
  unsigned int strsize = strs.size();
  string fnstr = strs[strsize-2];
  fnstr = fnstr.substr(1,fnstr.size()-1);

  
  stringstream insstr;
  
  insstr << "INSERT IGNORE INTO muon";
  insstr << " SET timeStampSec = " << (int)context.GetTimeStamp().GetSec();
  insstr << ", timeStampNanoSec = " << (int)context.GetTimeStamp().GetNanoSec();
  insstr << ", hallId = " << (int)context.GetSite();
  insstr << ", dateNTime = '" << string(ts.AsString("s")) << "'";
  insstr << ", runNumber = " << atoi(strs[strsize-6].c_str());
  insstr << ", fileNumber = " << atoi(fnstr.c_str());
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
    insstr2 << " (detectorTriggerId, muonId, dtMu_ms)";
    insstr2 << " SELECT detectorTriggerId, " << muonId << ", ";
    insstr2 << m_spall->getFloat(dtMuName[detId]);
    insstr2 << " FROM detectorTrigger WHERE";
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


void Event2DB_v043Alg::processRpcCalib(int triggerId)
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
    
    insstr << "INSERT IGNORE INTO calibRpcStrip (detectorTriggerId, rpcStripId)";
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


void Event2DB_v043Alg::processRpcRec(int triggerId)
{
  RecRpcHeader* recHeader = get<RecRpcHeader>("/Event/Rec/RpcSimple");
  if(!recHeader) {
    error() << "failed to get RPC reconstructed header" << endreq;
    return;
  }
  
  const Context& context = recHeader->context();
  const RecRpcTrigger& recRpcTrig = recHeader->recTrigger();
  const vector<const RecRpcCluster*> clusters = recRpcTrig.clusters();
  
  Site_t curSite = context.GetSite();
  
  DetectorElement* de = getDet<DetectorElement>(m_referenceDetector[curSite][context.GetDetId()]);
  Gaudi::XYZPoint gblOrigin = de->geometry()->toGlobal(Gaudi::XYZPoint(0,0,0));
  
  for(unsigned int clIdx = 0; clIdx < clusters.size(); clIdx++)
  {
    Gaudi::XYZPoint recPos;
    recPos.SetX(clusters[clIdx]->position().x());
    recPos.SetY(clusters[clIdx]->position().y());
    recPos.SetZ(clusters[clIdx]->position().z());
    
    /// RPC coordinate is hard coded in RpcSimple algorithm.
    /// It is with respect to water pool coordinate.
    /// Subtract offset to transform to site coordinate.
    recPos = recPos - m_rpcRecOffset[curSite];
    Gaudi::XYZPoint gblPos = de->geometry()->toGlobal(recPos);
    
    stringstream insstr;
    
    insstr << "INSERT IGNORE INTO recRpcSimple";
    insstr << " SET detectorTriggerId = " << triggerId;
    insstr << ", clusterId = " << clIdx+1;
    insstr << ", isTelescope = " << (int)(clusters[clIdx]->type() == RecRpcCluster::kTelescopeRPC);
    insstr << ", x = " << gblPos.X();
    insstr << ", y = " << gblPos.Y();
    insstr << ", z = " << gblPos.Z();
    insstr << ";";
    
    debug() << insstr.str() << endreq;
    
    try
    {
      /// execute the insert query
      Statement* stmt = m_connection->createStatement();
      info() << stmt->executeUpdate(insstr.str()) << " RPC reconstruction record(s) inserted" << endreq;
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
}


void Event2DB_v043Alg::processTrigger()
{
  const CalibReadout* calibReadout = m_calibRoHdr->calibReadout();
  
  /// deal with trigger
  stringstream trigCondHexStr;
  trigCondHexStr << "0x" << std::hex << (int)calibReadout->triggerType();
  
  const Context& context = m_calibRoHdr->context();
  stringstream insstr;
  insstr << "INSERT IGNORE INTO detectorTrigger";
  insstr << " SET hallId = " << (int)context.GetSite();
  insstr << ", detectorId = " << (int)context.GetDetId();
  insstr << ", dateNTime = '" << context.GetTimeStamp().AsString("SQL") << "'";
  insstr << ", triggerNumber = " << calibReadout->triggerNumber();
  insstr << ", triggerTimeSec = " << calibReadout->triggerTime().GetSec();
  insstr << ", triggerTimeNanoSec = " << calibReadout->triggerTime().GetNanoSec();
  insstr << ", triggerConditionHex = '" << trigCondHexStr.str() << "'";
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
    qrystr << "SELECT detectorTriggerId FROM detectorTrigger WHERE";
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
        triggerId = res->getInt("detectorTriggerId");
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
    
    /// fill triggerCondition table
    stringstream trigCondStr(Trigger::AsString(calibReadout->triggerType()));
    string trigName;
    while (trigCondStr >> trigName) {
      stringstream insTrigCond;
      insTrigCond << "INSERT IGNORE INTO triggerCondition";
      insTrigCond << " SET detectorTriggerId = " << triggerId;
      insTrigCond << ", triggerName = '" << trigName << "';";
      try
      {
        Statement* stmt = m_connection->createStatement();
        int updatecount = stmt->executeUpdate(insTrigCond.str());
        info() << updatecount << " trigger condition inserted" << endreq;
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
    
    if(calibReadout->detector().isAD())
    {
      processAdCalib(triggerId);
      processAdRec(triggerId);
    }
    if(calibReadout->detector().isWaterShield())
    {
      processWsCalib(triggerId);
      processWsRec(triggerId);
    }
    if(calibReadout->detector().isRPC())
    {
      processRpcCalib(triggerId);
      processRpcRec(triggerId);
    }
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


void Event2DB_v043Alg::processWsCalib(int triggerId)
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
    
    insstr << "INSERT IGNORE INTO calibWsPmt (detectorTriggerId, wsPmtId, ";
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
    
    int calibWsPmtId = -1;
    
    stringstream qrystr;
    
    qrystr << "SELECT c.calibWsPmtId from calibWsPmt c";
    qrystr << " INNER JOIN (";
    qrystr << " SELECT wsPmtId from wsPmt WHERE hallId = " << (int)context.GetSite();
    qrystr << " AND detectorId = " << (int)context.GetDetId();
    qrystr << " AND wallNumber = " << pmtId.wallNumber();
    qrystr << " AND wallSpot = " << pmtId.wallSpot();
    qrystr << " AND inwardFacing = " << (int)pmtId.inwardFacing();
    qrystr << " ) a ON c.wsPmtId = a.wsPmtId";
    qrystr << " WHERE c.detectorTriggerId = " << triggerId;
    
    debug() << qrystr.str() << endreq;
    
    try
    {
      Statement* stmt = m_connection->createStatement();
      ResultSet* res;
      res = stmt->executeQuery(qrystr.str());
      /// check if the result has exactly one record
      if(res->rowsCount() != 1) {
        error() << "no or multiple water pool PMT id" << endreq;
        return;
      }
      while (res->next())
        calibWsPmtId = res->getInt("calibWsPmtId");
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
    
    /// start filling TDC table
    int totInsTdc = 0;
    
    for(unsigned int hitIdx = 0; hitIdx < channel.size(); hitIdx++)
    {
      stringstream instdc;
      
      instdc << "INSERT IGNORE INTO calibWsPmtTdc";
      instdc << " SET calibWsPmtId = " << calibWsPmtId;
      instdc << ", time_ps = " << (int)(channel.time(hitIdx)*1000);
      instdc << ", charge = " << channel.charge(hitIdx);
      instdc << ";";
      
      try
      {
        /// execute the insert query
        Statement* stmt = m_connection->createStatement();
        totInsTdc += stmt->executeUpdate(instdc.str());
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
    
    info() << totInsTdc << " TDC hits inserted" << endreq;
  }
  
  info() << totInsPmt << " Pool PMT charge inserted" << endreq;
  
}


void Event2DB_v043Alg::processWsRec(int triggerId)
{
  RecHeader* recHeader = get<RecHeader>("/Event/Rec/PoolSimple");
  if(!recHeader) {
    error() << "failed to get water pool reconstructed header" << endreq;
    return;
  }
  
  const Context& context = recHeader->context();
  const RecTrigger& recWsTrig = recHeader->recTrigger();
  
  DetectorElement* de = getDet<DetectorElement>(m_referenceDetector[context.GetSite()][context.GetDetId()]);
  /// note that WS reconstruction points' unit is in m
  Gaudi::XYZPoint gblWsRec = de->geometry()->toGlobal(Gaudi::XYZPoint(recWsTrig.position().x()*1000.,recWsTrig.position().y()*1000.,recWsTrig.position().z()*1000.));
  
  stringstream insstr;
  
  insstr << "INSERT IGNORE INTO recPoolSimple";
  insstr << " SET detectorTriggerId = " << triggerId;
  insstr << ", x = " << gblWsRec.X();
  insstr << ", y = " << gblWsRec.Y();
  insstr << ", z = " << gblWsRec.Z();
  insstr << ";";
  
  debug() << insstr.str() << endreq;
  
  try
  {
    /// execute the insert query
    Statement* stmt = m_connection->createStatement();
    info() << stmt->executeUpdate(insstr.str()) << " water pool reconstruction record(s) inserted" << endreq;
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
