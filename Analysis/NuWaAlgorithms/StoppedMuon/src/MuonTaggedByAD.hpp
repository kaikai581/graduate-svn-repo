/*
 * This algorithm is based on He Miao's AESDemo.
 * 2012 Summer Shih-Kai
*/

#ifndef MUONTAGGEDBYAD_H
#define MUONTAGGEDBYAD_H

#include "GaudiAlg/GaudiAlgorithm.h"
/// DayaBay namespace is defoned in the below header file
#include "Event/UserDataHeader.h"


class MuonTaggedByAD : public GaudiAlgorithm
{
public:

  unsigned int m_evtNum;

  /// Constructor has to be in this form
  MuonTaggedByAD(const std::string&, ISvcLocator*);
  virtual ~MuonTaggedByAD();
  
  /// Three mandatory member functions of any algorithm
  StatusCode initialize();
  StatusCode execute();
  StatusCode finalize();


private:

  std::vector<short> hitAdId;


  IDataProviderSvc* p_archiveSvc;
  StatusCode promptOneAd(int, DayaBay::UserDataHeader*);
};


#endif
