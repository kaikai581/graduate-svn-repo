#ifndef MUONTREEALG_HPP
#define MUONTREEALG_HPP

#include "Conventions/Detectors.h"
#include "GaudiAlg/GaudiAlgorithm.h"
#include "MuonData.hpp"

class TEntryList;
class TFile;
class TTree;

typedef std::pair<DetectorId::DetectorId_t, int> triggerId;

class ReconValidAlg : public GaudiAlgorithm
{
public:
  /// Constructor has to be in this form
  ReconValidAlg(const std::string&, ISvcLocator*);
  virtual ~ReconValidAlg(){};
  
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
  std::map<std::string, SingleMuonData> m_muonDataBuffer;
  //std::map<std::pair<DetectorId::DetectorId_t, int>, MuonLookupStruct> m_muonLookupTable;
  std::map<triggerId, std::string> m_muonLookupTable;
  /// record the progress of each muon
  std::map<std::string, SingleMuonProgress> m_muonProgressTable;
  
private:
  void fillTree();
  //bool getMuon(std::string);
  bool isFlasher();
  bool isNeutronCandidate();
  /// return if the muon is already in the progress table
  //bool muonInProgressTable(std::string);
  //void registerAdMuon(DetectorId::DetectorId_t, int);
  //void registerWsMuon(DetectorId::DetectorId_t, int, std::string&);
  //void registerRpcMuon(DetectorId::DetectorId_t, int, std::string&);
};

#endif
