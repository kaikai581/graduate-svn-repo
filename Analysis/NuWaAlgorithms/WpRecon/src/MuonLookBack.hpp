/*
 * This algorithm is to loop over officially tagged muons.
 * Then look back to find the associated AD, WP and RPC data.
 * Note: the output is just coincidence events without any energy cuts.
 * 
 * 2012 Fall Shih-Kai
*/

#ifndef MUONLOOKBACK_HPP
#define MUONLOOKBACK_HPP


#include <queue>
#include <deque>
#include "Conventions/Site.h"
#include "GaudiKernel/Point3DTypes.h"
#include "GaudiKernel/Vector3DTypes.h"
#include "GaudiAlg/GaudiAlgorithm.h"
#include "MyDataModel.hpp"
#include "TreeVariables.hpp"


namespace DayaBay
{
  class CalibReadoutHeader;
  class UserDataHeader;
}

class IPmtGeomInfoSvc;
class IRpcGeomInfoSvc;
class TFile;
class TTree;


class MuonLookBack : public GaudiAlgorithm
{
public:
  /// Constructor has to be in this form
  MuonLookBack(const std::string&, ISvcLocator*);
  virtual ~MuonLookBack(){};
  
  /// Three mandatory member functions of any algorithm
  StatusCode initialize();
  StatusCode execute();
  StatusCode finalize();

private:
  /// Rpc Geometry Information Service
  /// maintain 2 queues for forming coincidence
  int exeCntr;
  Gaudi::XYZPoint m_gblAdCenter[4];
  IPmtGeomInfoSvc* m_pmtGeomSvc;
  IRpcGeomInfoSvc* m_rpcGeomSvc;
  std::queue<MyDataModel::AdTrigger>  m_adMuonQue[4];
  std::queue<MyDataModel::AdTrigger>  m_adMichQue[4];
  std::queue<MyDataModel::WpTrigger>  m_wpTrigQue[2];
  std::queue<MyDataModel::RpcTrigger> m_rpcTrigQue;
  std::set<MyDataModel::eventId>      m_retriggerSet;
  std::map<MyDataModel::eventId, int> m_nNeutronOfficial;
  DayaBay::CalibReadoutHeader*        m_crHdr;
  DayaBay::UserDataHeader*            m_spall;
  /// stack storing any unpaired prompt signals
  std::deque<MyDataModel::AdTrigger>  m_muonPrompt[4];
  std::queue<MyDataModel::DecayMuon>  m_decayMuons;
  /// get muon track information: the intersection of the muon track with OAV
  //Gaudi::XYZPoint                     getIntersection(int);
  /// return t, the straight line parameter
  void                                getIntersection(MyDataModel::AdTrigger&, double&, double&, int&, int&);
  
  /// variable and function to bind muons from different detectors
  MyDataModel::MuonLookup             m_muonLookup;
  void bindWpMuon(MyDataModel::eventId&, int);
  void bindRpcMuon(MyDataModel::eventId&);
  
  /// variables for hardcoded RPC geometry transformation...
  double x_offset[4];
  double y_offset[4];
  double z_offset[4];
  
  /// AD local coordinate translation
  std::map<Site::Site_t, std::map<DetectorId::DetectorId_t, Gaudi::XYZPoint> > adLocalTranslation;
  
  void processAd();
  void processWp();
  void processRpc();
  
  bool flasherTest();
  void formCoincidence(std::deque<MyDataModel::AdTrigger>&,std::queue<MyDataModel::AdTrigger>&);
  double neutronDistance2MuonTrack(const MyDataModel::RpcTrigger&, const MyDataModel::AdTrigger&, const MyDataModel::AdTrigger&);
  double neutronDistanceAlongMuonTrack(const MyDataModel::RpcTrigger&, const MyDataModel::AdTrigger&, const MyDataModel::AdTrigger&);
  
  /// ROOT variables
  TFile*        m_rootfile;
  std::string   m_rootfilename;
  TTree*        m_tree;
  TTree*        m_trIwsRec;
  TTree*        m_trOwsRec;
  TTree*        m_trTraComp;
  TreeVariables m_tv;
  SingleWsRecTreeVars m_IwsVars;
  SingleWsRecTreeVars m_OwsVars;
  TrackComp     m_TraComp;
  
  /// configurable variables
  double        m_windowStart;
  double        m_windowEnd;
  
  void fillTree();
  void fillIwsRecTree();
  void fillOwsRecTree();
  void fillTrackCompTree();
  
  /// calculate cosine of two given vectors
  double getCosine(Gaudi::XYZVector, Gaudi::XYZVector);
};



#endif
