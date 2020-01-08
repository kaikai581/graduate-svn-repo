/*
 * 
 * To run this algorithm, make sure muon information is already written into
 * a MySQL database with "Muon2DB" package.
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

#ifndef NEUTRONALG_HPP
#define NEUTRONALG_HPP

#include "GaudiAlg/GaudiAlgorithm.h"

namespace sql
{
  class Connection;
  class Driver;
  class SQLException;
}

class NeutronAlg : public GaudiAlgorithm
{
public:
  /// Constructor has to be in this form
  NeutronAlg(const std::string&, ISvcLocator*);
  virtual ~NeutronAlg(){};
  
  /// Three mandatory member functions of any algorithm
  StatusCode initialize();
  StatusCode execute();
  StatusCode finalize();

private:
  sql::Connection*             m_connection;
  sql::Driver*                 m_driver;
  int                          m_exeCntr;
  std::string                  m_fileNumber;
  std::string                  m_runNumber;

private:
  void                         printRuntimeErr(std::runtime_error&);
  void                         printSqlErr(sql::SQLException&);
};

#endif
