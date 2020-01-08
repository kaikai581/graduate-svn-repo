#ifndef MUONTREEALG_HPP
#define MUONTREEALG_HPP

#include "Conventions/Detectors.h"
#include "GaudiAlg/GaudiAlgorithm.h"
#include "MuonData.hpp"

class TEntryList;
class TFile;
class TTree;

class MuonTreeAlg : public GaudiAlgorithm
{
public:
  /// Constructor has to be in this form
  MuonTreeAlg(const std::string&, ISvcLocator*);
  virtual ~MuonTreeAlg(){};
  
  /// Three mandatory member functions of any algorithm
  StatusCode initialize();
  StatusCode execute();
  StatusCode finalize();

private:
  /// configurables
  std::string m_infilename;
  double      m_windowStart;
  double      m_windowEnd;
  
  int exeCntr;
  TEntryList* m_entryList;
  TFile* m_infile;
  TFile* m_outfile;
  TTree* m_treeSpal;
  /// containers of tree elements
  //int m_tMu_s;
  //int m_tMu_ns;
  //int m_hitAD1;
  //int m_hitAD2;
  //int m_hitAD3;
  //int m_hitAD4;
  //int m_hitIWS;
  //int m_hitOWS;
  //int m_hitRPC;
  TTree* m_outtree;
  double m_eMu;
  std::map<DetectorId::DetectorId_t, std::string > m_adMuonId;
  std::map<std::string, MuonData> m_muonList;
  //std::map<std::string, int> m_testList;
  std::map<std::pair<DetectorId::DetectorId_t, int>, MuonLookupStruct> m_muonLookupTable;
  
private:
  void fillTree();
  bool isFlasher();
  bool getMuon(std::string);
  void registerAdMuon(DetectorId::DetectorId_t, int);
  void registerWsMuon(DetectorId::DetectorId_t, int, std::string&);
  void registerRpcMuon(DetectorId::DetectorId_t, int, std::string&);
  bool isNeutronCandidate();
};

#endif
