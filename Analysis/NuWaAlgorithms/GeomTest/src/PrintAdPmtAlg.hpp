#ifndef PRINTADPMTALG_HPP
#define PRINTADPMTALG_HPP


#include "GaudiAlg/GaudiAlgorithm.h"

class ICableSvc;
class IPmtGeomInfoSvc;

class PrintAdPmtAlg : public GaudiAlgorithm
{
public:
  /// Constructor has to be in this form
  PrintAdPmtAlg(const std::string&, ISvcLocator*);
  virtual ~PrintAdPmtAlg(){};
  
  /// Three mandatory member functions of any algorithm
  StatusCode initialize();
  StatusCode execute();
  StatusCode finalize();

private:
  /// PMT Geometry Information Service
  IPmtGeomInfoSvc* m_pmtGeomSvc;
  /// cable service
  ICableSvc*       m_cableSvc;
};


#endif
