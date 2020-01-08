/*
 * This algorithm is based on Chao's CoincidenceTight.
 * The idea is to loop over each AD event, assuming they are Michel electron
 * events, and look back long enough to search for a muon.
 * 
 * 2012 Summer Shih-Kai
*/

#ifndef ELECTRONLOOKBACK_HPP
#define ELECTRONLOOKBACK_HPP


#include "GaudiAlg/GaudiAlgorithm.h"
/// DayaBay namespace is defoned in the below header files
#include "Event/RecHeader.h"
#include "Event/UserDataHeader.h"


class ElectronLookBack : public GaudiAlgorithm
{
public:
  /// Constructor has to be in this form
  ElectronLookBack(const std::string&, ISvcLocator*);
  virtual ~ElectronLookBack();
  
  /// Three mandatory member functions of any algorithm
  StatusCode initialize();
  StatusCode execute();
  StatusCode finalize();


private:
  /// variables
  DayaBay::RecHeader* m_michel;
  IDataProviderSvc*   p_archiveSvc;
  std::vector<int>    m_pairedList; // list of the execution numbers of paired muons
  
  /// functions
  void printDebugInfo(DayaBay::UserDataHeader*);
  void delist();
};



#endif
