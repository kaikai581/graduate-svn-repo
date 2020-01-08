/*
 * 
 * In order to utilize the officially tagged muons in NuWa, 2 pass method is
 * used since the tagged muons are not syncronized with other readout headers.
 * This causes problems of looking back into the previous event loops when a
 * tagged muon is encountered. Two pass method is of course the simplest
 * solution to this problem.
 * 
 * Shih-Kai 2013 Feb. @ UH
 * 
 */



#ifndef FIRSTPASSALG_HPP
#define FIRSTPASSALG_HPP

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


class FirstPassAlg : public GaudiAlgorithm
{
public:
  /// Constructor has to be in this form
  FirstPassAlg(const std::string&, ISvcLocator*);
  virtual ~FirstPassAlg(){};
  
  /// Three mandatory member functions of any algorithm
  StatusCode initialize();
  StatusCode execute();
  StatusCode finalize();

private:
  sql::Connection*             m_connection;
  sql::Driver*                 m_driver;
  DayaBay::UserDataHeader*     m_spall;
  int                          m_updatecount;
  int                          m_exeCntr;
  DayaBay::CalibReadoutHeader* m_calibRoHdr;
  std::string                  m_fileNumber;
  std::string                  m_runNumber;
  //std::map<Site::Site_t, std::map<DetectorId::DetectorId_t, std::string> >
                               //m_referenceDetector;
  //std::map<Site::Site_t, Gaudi::XYZPoint>
                               //m_rpcRecOffset;

private:
  void                         printRuntimeErr(std::runtime_error&);
  void                         printSqlErr(sql::SQLException&);
  //void                         processMuon();
  //void                         processTrigger();
  //void                         processAdCalib(int);
  //void                         processAdRec(int);
  //void                         processWsCalib(int);
  //void                         processWsRec(int);
  //void                         processRpcCalib(int);
  //void                         processRpcRec(int);
};


#endif
