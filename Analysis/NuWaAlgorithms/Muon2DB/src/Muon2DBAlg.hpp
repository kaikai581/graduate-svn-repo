#ifndef MUON2DBALG_HPP
#define MUON2DBALG_HPP


#include "GaudiAlg/GaudiAlgorithm.h"


namespace sql
{
  class Connection;
  class Driver;
}


namespace DayaBay
{
  class UserDataHeader;
}


class Muon2DBAlg : public GaudiAlgorithm
{
public:
  /// Constructor has to be in this form
  Muon2DBAlg(const std::string&, ISvcLocator*);
  virtual ~Muon2DBAlg(){};
  
  /// Three mandatory member functions of any algorithm
  StatusCode initialize();
  StatusCode execute();
  StatusCode finalize();

private:
  sql::Connection*         m_connection;
  sql::Driver*             m_driver;
  IDataProviderSvc*        m_archiveSvc;
  DayaBay::UserDataHeader* m_hdrSp;
  int                      m_updatecount;
};



#endif
