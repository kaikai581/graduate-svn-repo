#ifndef RPCGEOM2DBALG_HPP
#define RPCGEOM2DBALG_HPP

#include "Conventions/Site.h"
#include "GaudiAlg/GaudiAlgorithm.h"

namespace sql
{
  class Connection;
  class Driver;
}

class ICableSvc;
class IRpcGeomInfoSvc;

class RpcGeom2DBAlg : public GaudiAlgorithm
{
public:
  /// Constructor has to be in this form
  RpcGeom2DBAlg(const std::string&, ISvcLocator*);
  virtual ~RpcGeom2DBAlg(){};
  
  /// Three mandatory member functions of any algorithm
  StatusCode initialize();
  StatusCode execute();
  StatusCode finalize();

private:
  /// Rpc Geometry Information Service
  IRpcGeomInfoSvc* m_rpcGeomSvc;
  /// cable service
  ICableSvc*       m_cableSvc;
  
  /// mysql related
  sql::Connection* m_connection;
  sql::Driver*     m_driver;
  
  void processHall(Site::Site_t, std::string);
};


#endif
