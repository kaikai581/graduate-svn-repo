#ifndef EVENT2DB_V043ALG_HPP
#define EVENT2DB_V043ALG_HPP
#include "Conventions/Site.h"
#include "Conventions/DetectorId.h"
#include "GaudiAlg/GaudiAlgorithm.h"
#include "GaudiKernel/Point3DTypes.h"


namespace sql
{
  class Connection;
  class Driver;
  class SQLException;
}


namespace DayaBay
{
  class CalibReadoutHeader;
  class UserDataHeader;
}


class Event2DB_v043Alg : public GaudiAlgorithm
{
public:
  /// Constructor has to be in this form
  Event2DB_v043Alg(const std::string&, ISvcLocator*);
  virtual ~Event2DB_v043Alg(){};
  
  /// Three mandatory member functions of any algorithm
  StatusCode initialize();
  StatusCode execute();
  StatusCode finalize();

private:
  std::string                  m_infilename;
  sql::Connection*             m_connection;
  sql::Driver*                 m_driver;
  DayaBay::UserDataHeader*     m_spall;
  int                          m_updatecount;
  int                          m_exeCntr;
  DayaBay::CalibReadoutHeader* m_calibRoHdr;
  std::map<Site::Site_t, std::map<DetectorId::DetectorId_t, std::string> >
                               m_referenceDetector;
  std::map<Site::Site_t, Gaudi::XYZPoint>
                               m_rpcRecOffset;

private:
  void                         printRuntimeErr(std::runtime_error&);
  void                         printSqlErr(sql::SQLException&);
  void                         processMuon();
  void                         processTrigger();
  void                         processAdCalib(int);
  void                         processAdRec(int);
  void                         processWsCalib(int);
  void                         processWsRec(int);
  void                         processRpcCalib(int);
  void                         processRpcRec(int);
};



#endif
