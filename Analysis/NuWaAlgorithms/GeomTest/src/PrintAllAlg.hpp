#ifndef PRINTALLALG_HPP
#define PRINTALLALG_HPP

#include "GaudiAlg/GaudiAlgorithm.h"

class ICableSvc;
class IPmtGeomInfoSvc;
class IRpcGeomInfoSvc;

class PrintAllAlg : public GaudiAlgorithm
{
public:
  /// Constructor has to be in this form
  PrintAllAlg(const std::string&, ISvcLocator*);
  virtual ~PrintAllAlg(){};
  
  /// Three mandatory member functions of any algorithm
  StatusCode initialize();
  StatusCode execute();
  StatusCode finalize();

private:
  /// PMT Geometry Information Service
  IPmtGeomInfoSvc* m_pmtGeomSvc;
  /// Rpc Geometry Information Service
  IRpcGeomInfoSvc* m_rpcGeomSvc;
  /// cable service
  ICableSvc*       m_cableSvc;
};


#endif
