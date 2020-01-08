#include <map>
#include <sstream>
#include "cppconn/connection.h"
#include "cppconn/driver.h"
#include "cppconn/exception.h"
#include "cppconn/prepared_statement.h"
#include "cppconn/resultset.h"
#include "cppconn/statement.h"
#include "DataUtilities/DybArchiveList.h"
#include "Event/CalibReadoutHeader.h"
#include "Event/CalibReadoutPmtCrate.h"
#include "Event/RecHeader.h"
#include "Event/RecRpcHeader.h"
#include "Event/UserDataHeader.h"
#include "Muon2DBAlg.hpp"



#define DBHOST "tcp://heplinux3.phys.uh.edu:3306"
#define USER "sklin"
#define PASSWORD "aaaa5816"
#define DATABASE "MuonDB"



using namespace DayaBay;
using namespace sql;
using namespace std;


Muon2DBAlg::Muon2DBAlg(const string& name, ISvcLocator* svcloc) :
GaudiAlgorithm(name, svcloc), m_updatecount(0)
{
}



StatusCode Muon2DBAlg::initialize()
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
    error() << "ERROR: SQLException in " << __FILE__;
		error() << " (" << __func__<< ") on line " << __LINE__ << endreq;
		error() << "ERROR: " << e.what();
		error() << " (MySQL error code: " << e.getErrorCode();
		error() << ", SQLState: " << e.getSQLState() << ")" << endreq;
    return StatusCode::FAILURE;
  }
  
  return StatusCode::SUCCESS;
}



StatusCode Muon2DBAlg::finalize()
{
  debug() << "finalize()" << endreq;
  
  m_connection->close();
  delete m_connection;
  
  return StatusCode::SUCCESS;
}



StatusCode Muon2DBAlg::execute()
{
  debug() << "execute()" << endreq;
  
  /// retrieve only muon events
  DataObject* pObjSpall;
  if(eventSvc()->retrieveObject("/Event/Data/Physics/Spallation",pObjSpall).isFailure())
  {
    debug() << "No muon tag in this event" << endreq;
    return StatusCode::SUCCESS;
  }
  
  m_hdrSp = dynamic_cast<UserDataHeader*>(pObjSpall);
  
  /// output debug information
  // execution number is an integer
  debug() << "execution number: " << m_hdrSp->execNumber() << endreq;
  debug() << "trigger time: " << m_hdrSp->timeStamp().AsString("SQL") << endreq;
  debug() << "trigger time: " << m_hdrSp->timeStamp().GetSec() << "\t";
  debug() << m_hdrSp->timeStamp().GetNanoSec() << endreq;
  debug() << "   muon time: " << m_hdrSp->getInt("tMu_s") << "\t";
  debug() << m_hdrSp->getInt("tMu_ns") << endreq;
  
  /// map from detector ID to trigger number
  map<int, int> adTrigMap;
  map<int, int> wpTrigMap;
  map<int, int> rpcTrigMap;
  
  if(m_hdrSp->getInt("hitAD1") == 1)
    adTrigMap[1] = m_hdrSp->getInt("triggerNumber_AD1");
  if(m_hdrSp->getInt("hitAD2") == 1)
    adTrigMap[2] = m_hdrSp->getInt("triggerNumber_AD2");
  if(m_hdrSp->getInt("hitAD3") == 1)
    adTrigMap[3] = m_hdrSp->getInt("triggerNumber_AD3");
  if(m_hdrSp->getInt("hitAD4") == 1)
    adTrigMap[4] = m_hdrSp->getInt("triggerNumber_AD4");
  if(m_hdrSp->getInt("hitIWS") == 1)
    wpTrigMap[5] = m_hdrSp->getInt("triggerNumber_IWS");
  if(m_hdrSp->getInt("hitOWS") == 1)
    wpTrigMap[6] = m_hdrSp->getInt("triggerNumber_OWS");
  if(m_hdrSp->getInt("hitRPC") == 1)
    rpcTrigMap[7] = m_hdrSp->getInt("triggerNumber_RPC");
  debug() << adTrigMap.size() << " ADs ";
  debug() << wpTrigMap.size() << " WPs ";
  debug() << rpcTrigMap.size() << " RPCs are hit" << endreq;
  
  /// start database table filling
  stringstream ststr;
  /// prepare insert statement
  ststr << "INSERT IGNORE INTO `muon_triggers`";
  ststr << " SET `exec_num` = " << m_hdrSp->execNumber();
  ststr << ", `trigger_time` = '" << m_hdrSp->timeStamp().AsString("SQL") << "'";
  ststr << ", `tMu_s` = " << m_hdrSp->getInt("tMu_s");
  ststr << ", `tMu_ns` = " << m_hdrSp->getInt("tMu_ns") << ";";
  debug() << ststr.str() << endreq;
  
  int muonTrigId = 0;
  try
  {
    /// filling muon trigger table
    Statement* stmt = m_connection->createStatement();
    m_updatecount = stmt -> executeUpdate(ststr.str());
    ResultSet* res;
    res = stmt->executeQuery("SELECT LAST_INSERT_ID()");
    // Muon trigger ID will be used as a foreign key in sub-detector tables.
    if(res->next())
      muonTrigId = res->getInt(1);
    info() << m_updatecount << " rows inserted" << endreq;
    
    /// filling sub-detector trigger table
    /// AD
    for(map<int, int>::iterator it=adTrigMap.begin(); it!=adTrigMap.end(); it++)
    {

      // Retrieve AD RecHeader in AES to look for this AD muon
      SmartDataPtr<DybArchiveList> adRecHist(m_archiveSvc,"/Event/Rec/AdSimple");
      SmartDataPtr<DybArchiveList> adCalibHist(m_archiveSvc,"/Event/CalibReadout/CalibReadoutHeader");
      DybArchiveList::const_iterator itCalib = adCalibHist->begin();
      // AD trigger ID used as PMT table's foreign key
      int adTrigId = 0;
      
      if (0 == adRecHist) warning() << "No RecEvent in archive list." << endreq;
      else
      {
        for(DybArchiveList::const_iterator iter=adRecHist->begin(); iter!=adRecHist->end(); iter++)
        {
          RecHeader* adRecEvt = dynamic_cast<RecHeader*>(*iter);
          /// Trigger number and detector ID uniquely idetify the event.
          if(adRecEvt->recTrigger().triggerNumber() == (unsigned int)it->second &&
             adRecEvt->context().GetDetId() == it->first)
          {
            stringstream insstr;
            insstr << "INSERT IGNORE INTO `ad_triggers`";
            insstr << " SET `detector_id` = " << it->first;
            insstr << ", `trigger_time` = '" << adRecEvt->timeStamp().AsString("SQL") << "'";
            insstr << ", `energy` = " << adRecEvt->recTrigger().energy();
            insstr << ", `energy_status` = " << adRecEvt->recTrigger().energyStatus();
            insstr << ", `x` = " << adRecEvt->recTrigger().position().x();
            insstr << ", `y` = " << adRecEvt->recTrigger().position().y();
            insstr << ", `z` = " << adRecEvt->recTrigger().position().z();
            insstr << ", `position_status` = " << adRecEvt->recTrigger().positionStatus();
            insstr << ", `muon_triggers_trigger_id` = " << muonTrigId;
            insstr << ";";
            m_updatecount = stmt->executeUpdate(insstr.str());
            info() << m_updatecount << " AD rows inserted" << endreq;
            res = stmt->executeQuery("SELECT LAST_INSERT_ID()");
            if(res->next())
              adTrigId = res->getInt(1);
            break;
          }
          itCalib++;
        } // end of reconstructed AES loop
      }
      
      CalibReadoutHeader* adCalibEvt = dynamic_cast<CalibReadoutHeader*>(*itCalib);
      if(adCalibEvt->context().GetDetId() != it->first ||
         adCalibEvt->calibReadout()->triggerNumber() != (unsigned int)it->second)
      {
        error() << "calibrated readout is not aligned with reconstructed readout" << endreq;
        return StatusCode::FAILURE;
      }
      
      const CalibReadoutPmtCrate* adCalibPmt = dynamic_cast<const CalibReadoutPmtCrate*>(adCalibEvt->calibReadout());
      CalibReadoutPmtCrate::PmtChannelReadouts channels = adCalibPmt->channelReadout();
      CalibReadoutPmtCrate::PmtChannelReadouts::iterator ci = channels.begin();
      /// Loop over channel data
      for(; ci!=channels.end(); ci++)
      {
        stringstream insstr;
        insstr << "INSERT IGNORE INTO `ad_pmt_hits`";
        insstr << " SET `time_since_trigger` = " << ci->earliestTime();
        insstr << ", `charge` = " << ci->sumCharge();
        const AdPmtSensor pmtId(ci->pmtSensorId().fullPackedData());
        insstr << ", `ring` = " << pmtId.ring();
        insstr << ", `column` = " << pmtId.column();
        insstr << ", `ad_triggers_trigger_id` = " << adTrigId;
        insstr << ";";
        m_updatecount = stmt->executeUpdate(insstr.str());
        info() << m_updatecount << " AD PMT inserted" << endreq;
      }
    } // end of AD ID loop


    /// filling sub-detector trigger table
    /// water pool
    for(map<int, int>::iterator it=wpTrigMap.begin(); it!=wpTrigMap.end(); it++)
    {
      // Retrieve WP RecHeader in AES to look for this WP muon
      // Attention: as for now, there is no water pool reconstruction in the production yet
      // skip WP reconstruction
      SmartDataPtr<DybArchiveList> wpCalibHist(m_archiveSvc,"/Event/CalibReadout/CalibReadoutHeader");
      DybArchiveList::const_iterator itCalib = wpCalibHist->begin();
      // WP trigger ID used as PMT table's foreign key
      int wpTrigId = 0;
      
      if (0 == wpCalibHist) warning() << "No WP calibrated event in archive list." << endreq;
      else
      {
        for(; itCalib!=wpCalibHist->end(); itCalib++)
        {
          CalibReadoutHeader* wpCalibEvt = dynamic_cast<CalibReadoutHeader*>(*itCalib);
          /// Trigger number and detector ID uniquely idetify the event.
          if(wpCalibEvt->calibReadout()->triggerNumber() == (unsigned int)it->second &&
             wpCalibEvt->context().GetDetId() == it->first)
          {
            /// fill WP trigger table
            stringstream insstr;
            insstr << "INSERT IGNORE INTO `wp_triggers`";
            insstr << " SET `detector_id` = " << it->first;
            insstr << ", `trigger_time` = '" << wpCalibEvt->timeStamp().AsString("SQL") << "'";
            insstr << ", `muon_triggers_trigger_id` = " << muonTrigId;
            insstr << ";";
            m_updatecount = stmt->executeUpdate(insstr.str());
            info() << m_updatecount << " WP rows inserted" << endreq;
            res = stmt->executeQuery("SELECT LAST_INSERT_ID()");
            if(res->next())
              wpTrigId = res->getInt(1);

            const CalibReadoutPmtCrate* wpCalibPmt = dynamic_cast<const CalibReadoutPmtCrate*>(wpCalibEvt->calibReadout());
            CalibReadoutPmtCrate::PmtChannelReadouts channels = wpCalibPmt->channelReadout();
            CalibReadoutPmtCrate::PmtChannelReadouts::iterator chit = channels.begin();
            for(; chit != channels.end(); chit++)
            {
              stringstream insstr2;
              insstr2 << "INSERT IGNORE INTO `wp_pmt_hits`";
              insstr2 << " SET `time_since_trigger` = " << chit->earliestTime();
              insstr2 << ", `charge` = " << chit->sumCharge();
              const PoolPmtSensor pmtId(chit->pmtSensorId().fullPackedData());
              insstr2 << ", `wall_number` = " << pmtId.wallNumber();
              insstr2 << ", `wall_spot` = " << pmtId.wallSpot();
              insstr2 << ", `inward_facing` = " << pmtId.inwardFacing();
              insstr2 << ", `wp_triggers_trigger_id` = " << wpTrigId;
              insstr2 << ";";
              m_updatecount = stmt->executeUpdate(insstr2.str());
              info() << m_updatecount << " WP PMT inserted" << endreq;
            }
            
            /// quit the loop once the event is found in the AES
            break;
          }
        } // end of WP AES loop
      }
    } // end of WP detector loop
    
    /// filling sub-detector trigger table
    /// RPC
    for(map<int, int>::iterator it=rpcTrigMap.begin(); it!=rpcTrigMap.end(); it++)
    {
      // Retrieve RPC RecHeader in AES to look for this RPC muon
      SmartDataPtr<DybArchiveList> rpcRecHist(m_archiveSvc,"/Event/Rec/RpcSimple");
      // RPC trigger ID used as strip table's foreign key
      int rpcTrigId = 0;
      
      if (0 == rpcRecHist) warning() << "No RPC reconstructed event in archive list." << endreq;
      else
      {
        DybArchiveList::const_iterator itRec = rpcRecHist->begin();
        for(; itRec!=rpcRecHist->end(); itRec++)
        {
          RecRpcHeader* rpcRecEvt = dynamic_cast<RecRpcHeader*>(*itRec);
          if(rpcRecEvt->recTrigger().triggerNumber() == (unsigned int)it->second &&
             rpcRecEvt->context().GetDetId() == it->first)
          {
            /// fill RPC trigger table
            stringstream insstr;
            insstr << "INSERT IGNORE INTO `rpc_triggers`";
            insstr << "SET `trigger_time` = '" << rpcRecEvt->timeStamp().AsString("SQL") << "'";
            insstr << ", `muon_triggers_trigger_id` = " << muonTrigId;
            insstr << ";";
            m_updatecount = stmt->executeUpdate(insstr.str());
            info() << m_updatecount << " RPC rows inserted" << endreq;
            res = stmt->executeQuery("SELECT LAST_INSERT_ID()");
            if(res->next())
              rpcTrigId = res->getInt(1);
            
            vector<const RecRpcCluster*> clusters = rpcRecEvt->recTrigger().clusters();
            vector<const RecRpcCluster*>::iterator clit = clusters.begin();
            for(; clit != clusters.end(); clit++)
            {
              stringstream insstr2;
              insstr2 << "INSERT IGNORE INTO `rpc_strip_hits`";
              insstr2 << " SET `x` = " << (*clit)->position().x();
              insstr2 << ", `y` = " << (*clit)->position().y();
              insstr2 << ", `z` = " << (*clit)->position().z();
              insstr2 << ", `rpc_triggers_trigger_id` = " << rpcTrigId;
              insstr2 << ";";
              m_updatecount = stmt->executeUpdate(insstr2.str());
              info() << m_updatecount << " RPC points inserted" << endreq;
            }
            
            /// quit the loop once the event is found in the AES
            break;
          }
        } // end of RPC AES loop
      }
    } // end of RPC detector loop
    
    delete stmt;
  }
  catch (SQLException &e)
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
    return StatusCode::FAILURE;
  }
  catch (std::runtime_error &e)
  {
    error() << "ERROR: runtime_error in " << __FILE__;
    error() << " (" << __func__ << ") on line " << __LINE__ << endl;
    error() << "ERROR: " << e.what() << endl;
    return StatusCode::FAILURE;
  }
  
  return StatusCode::SUCCESS;
}
