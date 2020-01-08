#ifndef MUONREDEFINITIONALG_HPP
#define MUONREDEFINITIONALG_HPP


#include <fstream>
#include "GaudiAlg/GaudiAlgorithm.h"
#include "Context/TimeStamp.h"
#include "Conventions/Detectors.h"



class MuonRedefinitionAlg : public GaudiAlgorithm
{
public:
  /// Constructor has to be in this form
  MuonRedefinitionAlg(const std::string&, ISvcLocator*);
  virtual ~MuonRedefinitionAlg(){};
  
  /// Three mandatory member functions of any algorithm
  StatusCode initialize();
  StatusCode execute();
  StatusCode finalize();

private:
  /// production file name
  std::string m_infilename;
  /// container for redefined muons
  std::map<DetectorId::DetectorId_t, std::vector<int> > m_myAdMuonList;
  std::map<DetectorId::DetectorId_t, std::vector<int> > m_myAdInterMuonList;
  
  /// trigger time of the last muon
  std::map<DetectorId::DetectorId_t, TimeStamp> m_lastMuonTime;
  
  /// flag for first encounter of an inter-muon event
  std::map<DetectorId::DetectorId_t, bool> m_isFirstInterMuonTrigger;
  
  /// output files
  std::ofstream m_outf;
};


#endif
