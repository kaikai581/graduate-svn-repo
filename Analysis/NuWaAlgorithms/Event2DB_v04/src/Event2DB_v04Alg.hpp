#ifndef EVENT2DB_V04ALG_HPP
#define EVENT2DB_V04ALG_HPP
#include "GaudiAlg/GaudiAlgorithm.h"


namespace sql
{
  class Connection;
  class Driver;
  class SQLException;
}


namespace DayaBay
{
  class CalibReadoutHeader;
  class ReadoutHeader;
  class UserDataHeader;
}


class Event2DB_v04Alg : public GaudiAlgorithm
{
public:
  /// Constructor has to be in this form
  Event2DB_v04Alg(const std::string&, ISvcLocator*);
  virtual ~Event2DB_v04Alg(){};
  
  /// Three mandatory member functions of any algorithm
  StatusCode initialize();
  StatusCode execute();
  StatusCode finalize();

private:
  sql::Connection*             m_connection;
  sql::Driver*                 m_driver;
  IDataProviderSvc*            m_archiveSvc;
  DayaBay::UserDataHeader*     m_spall;
  int                          m_updatecount;
  int                          m_exeCntr;
  DayaBay::CalibReadoutHeader* m_calibRoHdr;
  DayaBay::ReadoutHeader*      m_roHdr;

private:
  void                         printRuntimeErr(std::runtime_error&);
  void                         printSqlErr(sql::SQLException&);
  void                         processMuon();
  void                         processTrigger();
  void                         processAd(int);
  void                         processWs(int);
  void                         processRpc(int);
};



#endif
