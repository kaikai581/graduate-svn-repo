#include <sstream>
#include "Context/Context.h"
#include "Context/ServiceMode.h"
#include "Conventions/DetectorId.h"
#include "Conventions/SimFlag.h"
#include "cppconn/connection.h"
#include "cppconn/driver.h"
#include "cppconn/exception.h"
#include "cppconn/statement.h"
#include "DataSvc/ICableSvc.h"
#include "DetDesc/DetectorElement.h"
#include "DetDesc/IGeometryInfo.h"
#include "DetHelpers/IRpcGeomInfoSvc.h"
#include "RpcGeom2DBAlg.hpp"

#include "Event/RecRpcCluster.h" // included only for CLHEP namespace

#define DBHOST "tcp://heplinux3.phys.uh.edu:3306"
#define USER "sklin"
#define PASSWORD "aaaa5816"
#define DATABASE "MuonDB"

using namespace DayaBay;
using namespace DetectorId;
using namespace SimFlag;
using namespace Site;
using namespace sql;
using namespace std;


RpcGeom2DBAlg::RpcGeom2DBAlg(const string& name, ISvcLocator* svcloc) :
GaudiAlgorithm(name, svcloc)
{
}


StatusCode RpcGeom2DBAlg::execute()
{
  debug() << "execute()" << endreq;
  return StatusCode::SUCCESS;
}


StatusCode RpcGeom2DBAlg::finalize()
{
  debug() << "finalize()" << endreq;
  /// disconnect from the database
  m_connection->close();
  delete m_connection;
  
  return StatusCode::SUCCESS;
}


StatusCode RpcGeom2DBAlg::initialize()
{
  debug() << "initialize()" << endreq;
  
  /// Get RpcGeomInfo Service
  m_rpcGeomSvc = svc<IRpcGeomInfoSvc>("RpcGeomInfoSvc", true);
  if(!m_rpcGeomSvc) {
    error() << "Can't initialize Rpc geometry service." << endreq;
    return StatusCode::FAILURE;
  }
  /// get cable serivce
  /// Do not forget to specify --dbconf=offline_db in conjunction with
  /// CableSvc service.
  m_cableSvc = svc<ICableSvc>("CableSvc", true);
  
  /// start connection to database
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
    /// Note that if auto commit is turned off, one has to commit manually.
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
  
  processHall(kDayaBay, "/dd/Structure/Sites/db-rock");
  processHall(kLingAo,  "/dd/Structure/Sites/la-rock");
  processHall(kFar,     "/dd/Structure/Sites/far-rock");
  
  return StatusCode::SUCCESS;
}


void RpcGeom2DBAlg::processHall(Site_t hall, string detDesc)
{
  info() << "processing " << AsString(hall) << " site" << endreq;
  DetectorElement* de = getDet<DetectorElement>(detDesc);
  /// get site origin's coordinate w.r.t. global
  Gaudi::XYZPoint siteOrigin = de->geometry()->toGlobal(Gaudi::XYZPoint(0,0,0));
  info() << "site origin's coordinate in global system: ";
  info() << "(" << siteOrigin.X() << "," << siteOrigin.Y() << ",";
  info() << siteOrigin.Z() << ")" << endreq;
  /// get local ex, ey, ez coordinates in global system
  Gaudi::XYZVector ex = de->geometry()->toGlobal(Gaudi::XYZVector(1,0,0));
  Gaudi::XYZVector ey = de->geometry()->toGlobal(Gaudi::XYZVector(0,1,0));
  Gaudi::XYZVector ez = de->geometry()->toGlobal(Gaudi::XYZVector(0,0,1));
  info() << "orthogonal transformation matrix from local to global: " << endreq;
  info() << ex.X() << " " << ex.Y() << " " << ex.Z() << endreq;
  info() << ey.X() << " " << ey.Y() << " " << ey.Z() << endreq;
  info() << ez.X() << " " << ez.Y() << " " << ez.Z() << endreq;
  /// prepare statement to write transformation matrix
  stringstream matststr;
  matststr << "INSERT IGNORE INTO `local2global_matrix`";
  matststr << " SET `site` = " << hall;
  matststr << ", `Ogl_xx` = " << ex.X();
  matststr << ", `Ogl_xy` = " << ey.X();
  matststr << ", `Ogl_xz` = " << ez.X();
  matststr << ", `Ogl_yx` = " << ex.Y();
  matststr << ", `Ogl_yy` = " << ey.Y();
  matststr << ", `Ogl_yz` = " << ez.Y();
  matststr << ", `Ogl_zx` = " << ex.Z();
  matststr << ", `Ogl_zy` = " << ey.Z();
  matststr << ", `Ogl_zz` = " << ez.Z()<< ";";
  info() << matststr.str() << endreq;
  Statement* matst = m_connection->createStatement();
  matst -> executeUpdate(matststr.str());
  
  Context context(hall, kData, TimeStamp(2012,1,1,0,0,0), kRPC);
  ServiceMode svcMode(context, 0);
  
  vector<RpcSensor> rpcSensors = m_cableSvc->rpcSensors(svcMode);
  info() << "number of RPC strips: " << rpcSensors.size() << endreq;
  
  for(unsigned int i = 0; i < rpcSensors.size(); i++)
  {
    CLHEP::Hep3Vector striplocGlo = m_rpcGeomSvc->get(rpcSensors[i].fullPackedData())->globalPosition();
    CLHEP::Hep3Vector striplocLoc = m_rpcGeomSvc->get(rpcSensors[i].fullPackedData())->localPosition();
    CLHEP::Hep3Vector stripNormalGlo = m_rpcGeomSvc->get(rpcSensors[i].fullPackedData())->globalDirection();
    CLHEP::Hep3Vector stripNormalLoc = m_rpcGeomSvc->get(rpcSensors[i].fullPackedData())->localDirection();
    
    
    /// print debug information
    if( !((i+1)%100) )
    {
      debug() << "(row, col, layer, strip): (";
      debug() << rpcSensors[i].panelRow() << ", ";
      debug() << rpcSensors[i].panelColumn() << ", ";
      debug() << rpcSensors[i].layer() << ", ";
      debug() << rpcSensors[i].strip() << ")" << endreq;
      // print normal direction
      debug() << "global normal direction: (";
      debug() << stripNormalGlo.x() << ",";
      debug() << stripNormalGlo.y() << ",";
      debug() << stripNormalGlo.z() << ")" << endreq;
      debug() << " local normal direction: (";
      debug() << stripNormalLoc.x() << ",";
      debug() << stripNormalLoc.y() << ",";
      debug() << stripNormalLoc.z() << ")" << endreq;
      // print center coordinates
      debug() << "calculation: (";
      debug() << striplocGlo.x()-siteOrigin.X() << ",";
      debug() << striplocGlo.y()-siteOrigin.Y() << ",";
      debug() << striplocGlo.z()-siteOrigin.Z() << ")" << endreq;
      debug() << "direct: (";
      debug() << striplocLoc.x() << ",";
      debug() << striplocLoc.y() << ",";
      debug() << striplocLoc.z() << ")" << endreq;
      // print progress
      info() << i+1 << " RPC sensors processed" << endreq;
    }
    
    /// prepare insert statement of strip geometry
    stringstream ststr;
    ststr << "INSERT IGNORE INTO `rpc_strip_locations`";
    ststr << " SET `site` = " << hall;
    ststr << ", `detector` = " << kRPC;
    ststr << ", `row` = " << rpcSensors[i].panelRow();
    ststr << ", `col` = " << rpcSensors[i].panelColumn();
    ststr << ", `layer` = " << rpcSensors[i].layer();
    ststr << ", `strip` = " << rpcSensors[i].strip();
    ststr << ", `x_site` = " << striplocLoc.x();
    ststr << ", `y_site` = " << striplocLoc.y();
    ststr << ", `z_site` = " << striplocLoc.z();
    ststr << ", `x_shifted_global` = " << striplocGlo.x()-siteOrigin.X();
    ststr << ", `y_shifted_global` = " << striplocGlo.y()-siteOrigin.Y();
    ststr << ", `z_shifted_global` = " << striplocGlo.z()-siteOrigin.Z();
    ststr << ", `normal_xglobal` = " << stripNormalGlo.x();
    ststr << ", `normal_yglobal` = " << stripNormalGlo.y();
    ststr << ", `normal_zglobal` = " << stripNormalGlo.z() << ";";
    info() << ststr.str() << endreq;

    /// start inserting
    try
    {
      Statement* stmt = m_connection->createStatement();
      stmt -> executeUpdate(ststr.str());
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
    }
  }
}
