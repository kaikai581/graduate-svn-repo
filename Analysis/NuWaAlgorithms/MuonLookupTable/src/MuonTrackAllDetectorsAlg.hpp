#ifndef MUONTRACKALLDETECTORSALG_HPP
#define MUONTRACKALLDETECTORSALG_HPP


#include "Conventions/Detectors.h"
#include "GaudiAlg/GaudiAlgorithm.h"
#include "MuonData.hpp"
#include "MuonTrackTreeVars.hpp"

class TFile;
class TTree;

typedef std::pair<DetectorId::DetectorId_t, int> triggerId;

class MuonTrackAllDetectorsAlg : public GaudiAlgorithm
{
public:
  /// Constructor has to be in this form
  MuonTrackAllDetectorsAlg(const std::string&, ISvcLocator*);
  virtual ~MuonTrackAllDetectorsAlg(){};
  
  /// Three mandatory member functions of any algorithm
  StatusCode initialize();
  StatusCode execute();
  StatusCode finalize();
  
private:

  /// variables
  std::string m_infilename;
  std::string m_outfilename;
  // progress counter
  int exeCntr;
  
  TFile* m_infile;
  TTree* m_treeSpal;
  
  std::map<triggerId, std::string> m_muonLookupTable;
  std::map<std::string, SingleMuonProgress> m_muonProgressTable;
  std::map<std::string, SingleMuonBuffer> m_muonDataBuffer;
  
  // output variables
  TFile* m_outfile;
  TTree* m_outtree;
  MuonTrackTreeVars m_trvar;
  
  // geometry information
  std::map<Site::Site_t, std::map<DetectorId::DetectorId_t, std::string> > m_referenceDetector;
  std::map<Site::Site_t, ROOT::Math::XYZPoint> m_rpcRecOffset;
  std::map<Site::Site_t, std::map<DetectorId::DetectorId_t, ROOT::Math::XYZPoint> > m_oavCtr;
  
  /// methods
  singleTrack connect2Points(ROOT::Math::XYZPoint, ROOT::Math::XYZPoint);
  void fillOutputTree(std::string);
  void fitLeastSquaresLine(std::string);
  std::vector<double> getAngCTR(Site::Site_t, DetectorId::DetectorId_t, singleTrack, ROOT::Math::XYZPoint);
  std::vector<std::pair<ROOT::Math::XYZPoint, ROOT::Math::XYZPoint> > getTrackOavIntersections(Site::Site_t, DetectorId::DetectorId_t, singleTrack);
  std::string makeMuonId(int, int);
  bool isFlasher();
  void makeTrack(std::string);
  bool wsPositionValid(ROOT::Math::XYZPoint);
};


#endif
