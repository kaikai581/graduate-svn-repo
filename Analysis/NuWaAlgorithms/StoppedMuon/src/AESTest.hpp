#ifndef AESTEST_HPP
#define AESTEST_HPP


#include "GaudiAlg/GaudiAlgorithm.h"


class AESTest : public GaudiAlgorithm
{
public:
  /// Constructor has to be in this form
  AESTest(const std::string&, ISvcLocator*);
  virtual ~AESTest();
  
  /// Three mandatory member functions of any algorithm
  StatusCode initialize();
  StatusCode execute();
  StatusCode finalize();


private:
  IDataProviderSvc*   m_archiveSvc;
  int                 m_execNum;
};



#endif
