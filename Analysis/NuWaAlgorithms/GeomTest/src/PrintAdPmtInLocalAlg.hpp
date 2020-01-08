#ifndef PRINTADPMTINLOCAL_HPP
#define PRINTADPMTINLOCAL_HPP

#include "GaudiAlg/GaudiAlgorithm.h"

class ICableSvc;
class IPmtGeomInfoSvc;

class PrintAdPmtInLocalAlg : public GaudiAlgorithm
{
public:
  /// Constructor has to be in this form
  PrintAdPmtInLocalAlg(const std::string&, ISvcLocator*);
  virtual ~PrintAdPmtInLocalAlg(){};
  
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
