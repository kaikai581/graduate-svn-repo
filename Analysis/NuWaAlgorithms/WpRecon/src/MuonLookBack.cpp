#include <utility>
#include <sstream>
#include "DetDesc/DetectorElement.h"
#include "DetDesc/IGeometryInfo.h"
#include "DetHelpers/IPmtGeomInfoSvc.h"
#include "DetHelpers/IRpcGeomInfoSvc.h"
#include "Event/CalibReadoutHeader.h"
#include "Event/CalibReadout.h"
#include "Event/CalibReadoutPmtCrate.h"
#include "Event/CalibReadoutRpcCrate.h"
#include "Event/UserDataHeader.h"
#include "Event/RecHeader.h"
#include "Event/RecRpcHeader.h"
#include "Event/RecRpcTrigger.h"
#include "MuonLookBack.hpp"
#include "TFile.h"
#include "TH1F.h"
#include "TMath.h"
#include "TParameter.h"
#include "TTree.h"


using namespace DetectorId;
using namespace Site;
using namespace std;
using namespace DayaBay;


MuonLookBack::MuonLookBack(const string& name, ISvcLocator* svcloc) :
GaudiAlgorithm(name, svcloc), exeCntr(1)
{
  declareProperty("FileName", m_rootfilename = "output.root", "root file name");
  declareProperty("WindowStart", m_windowStart = 0.5, "start of the time window after a muon in microsecond");
  declareProperty("WindowEnd", m_windowEnd = 20, "end of the time window after a muon in microsecond");
  
  //EH1
  x_offset[1] = 2500.0; 
  y_offset[1] = -500.0;
  z_offset[1] = 12500.0;
  //EH2
  x_offset[2] = 2500.0; 
  y_offset[2] = -500.0;
  z_offset[2] = 12500.0;
  //EH3
  x_offset[3] = 5650.0; 
  y_offset[3] = 500.0;
  z_offset[3] = 12500.0;
  
  // AD local coordinate system is relative to AD center.
  // Therefore to get to the pool-center coordinate, each AD event needs
  // translation.
  adLocalTranslation[kDayaBay][kAD1] = Gaudi::XYZPoint(5742./2,0,0);
  adLocalTranslation[kDayaBay][kAD2] = Gaudi::XYZPoint(-5742./2,0,0);
  adLocalTranslation[kLingAo][kAD1] = Gaudi::XYZPoint(5742./2,0,0);
  adLocalTranslation[kLingAo][kAD2] = Gaudi::XYZPoint(-5742./2,0,0);
  adLocalTranslation[kFar][kAD1] = Gaudi::XYZPoint(-5742./2,5742./2,0);
  adLocalTranslation[kFar][kAD2] = Gaudi::XYZPoint(5742./2,5742./2,0);
  adLocalTranslation[kFar][kAD3] = Gaudi::XYZPoint(-5742./2,-5742./2,0);
  adLocalTranslation[kFar][kAD4] = Gaudi::XYZPoint(5742./2,-5742./2,0);
}


StatusCode MuonLookBack::initialize()
{
  /// Get PmtGeomInfo Service
  m_pmtGeomSvc = svc<IPmtGeomInfoSvc>("PmtGeomInfoSvc", true);
  if(!m_pmtGeomSvc) {
    error() << "Can't initialize Pmt geometry service." << endreq;
    return StatusCode::FAILURE;
  }
  /// Get RpcGeomInfo Service
  m_rpcGeomSvc = svc<IRpcGeomInfoSvc>("RpcGeomInfoSvc", true);
  if(!m_rpcGeomSvc) {
    error() << "Can't initialize Rpc geometry service." << endreq;
    return StatusCode::FAILURE;
  }
  
  /// prepare for ROOT output
  m_rootfile = new TFile(m_rootfilename.c_str(), "RECREATE");
  m_tree = new TTree("dmucoin", "muon decay coincidence events");
  m_tree->Branch("site", &m_tv.site, "site/I");
  m_tree->Branch("detector", &m_tv.detector, "detector/I");
  m_tree->Branch("triggerNumberMu", &m_tv.triggerNumberMu, "triggerNumberMu/I");
  m_tree->Branch("triggerNumberIws", &m_tv.triggerNumberIws, "triggerNumberIws/I");
  m_tree->Branch("triggerNumberOws", &m_tv.triggerNumberOws, "triggerNumberOws/I");
  m_tree->Branch("triggerNumberRpc", &m_tv.triggerNumberRpc, "triggerNumberRpc/I");
  m_tree->Branch("tAdMuSec", &m_tv.tAdMuSec, "tAdMuSec/I");
  m_tree->Branch("tAdMuNanoSec", &m_tv.tAdMuNanoSec, "tAdMuNanoSec/I");
  m_tree->Branch("eMu", &m_tv.eMu, "eMu/D");
  m_tree->Branch("nMi", &m_tv.nMi, "nMi/I");
  m_tree->Branch("hitIws", &m_tv.hitIws, "hitIws/I");
  m_tree->Branch("nHitsIws", &m_tv.nHitsIws, "nHitsIws/I");
  m_tree->Branch("hitOws", &m_tv.hitOws, "hitOws/I");
  m_tree->Branch("nHitsOws", &m_tv.nHitsOws, "nHitsOws/I");
  m_tree->Branch("hitRpc", &m_tv.hitRpc, "hitRpc/I");
  m_tree->Branch("nHitsRpc", &m_tv.nHitsRpc, "nHitsRpc/I");
  m_tree->Branch("nRpcMi", &m_tv.nRpcMi, "nRpcMi/I");
  m_tree->Branch("muTheta", &m_tv.muTheta, "muTheta/D");
  m_tree->Branch("muPhi", &m_tv.muPhi, "muPhi/D");
  // variable arrays
  m_tree->Branch("triggerNumberMi", m_tv.triggerNumberMi, "triggerNumberMi[nMi]/I");
  m_tree->Branch("isRetrigger", m_tv.isRetrigger, "isRetrigger[nMi]/I");
  m_tree->Branch("nMaxPmtMi", m_tv.nMaxPmtMi, "nMaxPmtMi[nMi]/I");
  m_tree->Branch("tMaxPmtMi", m_tv.tMaxPmtMi, "tMaxPmtMi[nMi]/D");
  m_tree->Branch("tAdMiSec", m_tv.tAdMiSec, "tAdMiSec[nMi]/I");
  m_tree->Branch("tAdMiNanoSec", m_tv.tAdMiNanoSec, "tAdMiNanoSec[nMi]/I");
  m_tree->Branch("eMi", m_tv.eMi, "eMi[nMi]/D");
  m_tree->Branch("dtMuMi", m_tv.dtMuMi, "dtMuMi[nMi]/D");
  m_tree->Branch("dlMuMi", m_tv.dlMuMi, "dlMuMi[nMi]/D");
  m_tree->Branch("miZone", m_tv.miZone, "miZone[nMi]/I");
  m_tree->Branch("dtRpcMi", m_tv.dtRpcMi, "dtRpcMi[nRpcMi]/D");
  m_tree->Branch("dlRpcMi", m_tv.dlRpcMi, "dlRpcMi[nRpcMi]/D");
  // global reconstructed position
  m_tree->Branch("xMu", &m_tv.xMu, "xMu/D");
  m_tree->Branch("yMu", &m_tv.yMu, "yMu/D");
  m_tree->Branch("zMu", &m_tv.zMu, "zMu/D");
  // cocal reconstructed position
  m_tree->Branch("xlMu", &m_tv.xlMu, "xlMu/D");
  m_tree->Branch("ylMu", &m_tv.ylMu, "ylMu/D");
  m_tree->Branch("zlMu", &m_tv.zlMu, "zlMu/D");
  m_tree->Branch("rho2Mu", &m_tv.rho2Mu, "rho2Mu/D");
  m_tree->Branch("xMi", m_tv.xMi, "xMi[nMi]/D");
  m_tree->Branch("yMi", m_tv.yMi, "yMi[nMi]/D");
  m_tree->Branch("zMi", m_tv.zMi, "zMi[nMi]/D");
  m_tree->Branch("rho2Mi", m_tv.rho2Mi, "rho2Mi[nMi]/D");
  m_tree->Branch("xRpc", &m_tv.xRpc, "xRpc/D");
  m_tree->Branch("yRpc", &m_tv.yRpc, "yRpc/D");
  m_tree->Branch("zRpc", &m_tv.zRpc, "zRpc/D");
  m_tree->Branch("xlRpc", &m_tv.xlRpc, "xlRpc/D");
  m_tree->Branch("ylRpc", &m_tv.ylRpc, "ylRpc/D");
  m_tree->Branch("zlRpc", &m_tv.zlRpc, "zlRpc/D");
  m_tree->Branch("tRim", m_tv.tRim, "tRim[nRpcMi]/D");
  m_tree->Branch("xRim", m_tv.xRim, "xRim[nRpcMi]/D");
  m_tree->Branch("yRim", m_tv.yRim, "yRim[nRpcMi]/D");
  m_tree->Branch("zRim", m_tv.zRim, "zRim[nRpcMi]/D");
  m_tree->Branch("dlRimMi", m_tv.dlRimMi, "dlRimMi[nRpcMi]/D");
  // information between muon and RPC reconstructed positions
  m_tree->Branch("dtRpcMu", &m_tv.dtRpcMu, "dtRpcMu/D");
  m_tree->Branch("dlRpcMu", &m_tv.dlRpcMu, "dlRpcMu/D");
  m_tree->Branch("dlRimMu", &m_tv.dlRimMu, "dlRimMu/D");
  m_tree->Branch("dlMuRim", &m_tv.dlMuRim, "dlMuRim/D");
  m_tree->Branch("tRimMu", &m_tv.tRimMu, "tRimMu/D");
  m_tree->Branch("xRimMu", &m_tv.xRimMu, "xRimMu/D");
  m_tree->Branch("yRimMu", &m_tv.yRimMu, "yRimMu/D");
  m_tree->Branch("zRimMu", &m_tv.zRimMu, "zRimMu/D");
  m_tree->Branch("tRimMuOut", &m_tv.tRimMuOut, "tRimMuOut/D");
  m_tree->Branch("xRimMuOut", &m_tv.xRimMuOut, "xRimMuOut/D");
  m_tree->Branch("yRimMuOut", &m_tv.yRimMuOut, "yRimMuOut/D");
  m_tree->Branch("zRimMuOut", &m_tv.zRimMuOut, "zRimMuOut/D");
  m_tree->Branch("inPosMu", &m_tv.inPosMu, "inPosMu/I");
  m_tree->Branch("outPosMu", &m_tv.outPosMu, "outPosMu/I");
  m_tree->Branch("xlBot", &m_tv.xlBot, "xlBot/D");
  m_tree->Branch("ylBot", &m_tv.ylBot, "ylBot/D");
  m_tree->Branch("zlBot", &m_tv.zlBot, "zlBot/D");
  m_tree->Branch("dIwsBot", &m_tv.dIwsBot, "dIwsBot/D");
  m_tree->Branch("dOwsBot", &m_tv.dOwsBot, "dOwsBot/D");
  // water pool PMT coordinates
  m_tree->Branch("xIws", m_tv.xIws, "xIws[nHitsIws]/D");
  m_tree->Branch("yIws", m_tv.yIws, "yIws[nHitsIws]/D");
  m_tree->Branch("zIws", m_tv.zIws, "zIws[nHitsIws]/D");
  m_tree->Branch("xlIws", m_tv.xlIws, "xlIws[nHitsIws]/D");
  m_tree->Branch("ylIws", m_tv.ylIws, "ylIws[nHitsIws]/D");
  m_tree->Branch("zlIws", m_tv.zlIws, "zlIws[nHitsIws]/D");
  m_tree->Branch("firstHitTimeIws", m_tv.firstHitTimeIws, "firstHitTimeIws[nHitsIws]/D");
  m_tree->Branch("totChargeIws", m_tv.totChargeIws, "totChargeIws[nHitsIws]/D");
  m_tree->Branch("xOws", m_tv.xOws, "xOws[nHitsOws]/D");
  m_tree->Branch("yOws", m_tv.yOws, "yOws[nHitsOws]/D");
  m_tree->Branch("zOws", m_tv.zOws, "zOws[nHitsOws]/D");
  m_tree->Branch("xlOws", m_tv.xlOws, "xlOws[nHitsOws]/D");
  m_tree->Branch("ylOws", m_tv.ylOws, "ylOws[nHitsOws]/D");
  m_tree->Branch("zlOws", m_tv.zlOws, "zlOws[nHitsOws]/D");
  m_tree->Branch("firstHitTimeOws", m_tv.firstHitTimeOws, "firstHitTimeOws[nHitsOws]/D");
  m_tree->Branch("totChargeOws", m_tv.totChargeOws, "totChargeOws[nHitsOws]/D");
  m_tree->Branch("xlIwsBot", &m_tv.xlIwsBot, "xlIwsBot/D");
  m_tree->Branch("ylIwsBot", &m_tv.ylIwsBot, "ylIwsBot/D");
  m_tree->Branch("zlIwsBot", &m_tv.zlIwsBot, "zlIwsBot/D");
  m_tree->Branch("xlOwsBot", &m_tv.xlOwsBot, "xlOwsBot/D");
  m_tree->Branch("ylOwsBot", &m_tv.ylOwsBot, "ylOwsBot/D");
  m_tree->Branch("zlOwsBot", &m_tv.zlOwsBot, "zlOwsBot/D");
  // general accounting
  m_tree->Branch("nNeutron", &m_tv.nNeutron, "nNeutron/I");
  m_tree->Branch("nNeutronOfficial", &m_tv.nNeutronOfficial, "nNeutronOfficial/I");
  m_tree->Branch("nNeutronTight", &m_tv.nNeutronTight, "nNeutronTight/I");
  m_tree->Branch("nNeutronTight2", &m_tv.nNeutronTight2, "nNeutronTight2/I");
  m_tree->Branch("nAfterPulse", &m_tv.nAfterPulse, "nAfterPulse/I");
  // neutron properties
  m_tree->Branch("dlNeutron", m_tv.dlNeutron, "dlNeutron[nNeutron]/D");
  m_tree->Branch("zNeutron", m_tv.zNeutron, "zNeutron[nNeutron]/D");
  m_tree->Branch("zDepth", m_tv.zDepth, "zDepth[nNeutron]/D");
  m_tree->Branch("nZone", m_tv.nZone, "nZone[nNeutron]/I");
  
  /// inner water pool reconstruction tree
  m_trIwsRec = new TTree("iwsrec", "inner water pool muon reconstruction");
  m_trIwsRec->Branch("xRpc", &m_IwsVars.xRpc, "xRpc/D");
  m_trIwsRec->Branch("yRpc", &m_IwsVars.yRpc, "yRpc/D");
  m_trIwsRec->Branch("zRpc", &m_IwsVars.zRpc, "zRpc/D");
  m_trIwsRec->Branch("xWp", &m_IwsVars.xWp, "xWp/D");
  m_trIwsRec->Branch("yWp", &m_IwsVars.yWp, "yWp/D");
  m_trIwsRec->Branch("zWp", &m_IwsVars.zWp, "zWp/D");
  m_trIwsRec->Branch("nHit1400", &m_IwsVars.nHit1400, "nHit1400/I");
  m_trIwsRec->Branch("nHit1500", &m_IwsVars.nHit1500, "nHit1500/I");
  m_trIwsRec->Branch("xWp1400", &m_IwsVars.xWp1400, "xWp1400/D");
  m_trIwsRec->Branch("yWp1400", &m_IwsVars.yWp1400, "yWp1400/D");
  m_trIwsRec->Branch("zWp1400", &m_IwsVars.zWp1400, "zWp1400/D");
  m_trIwsRec->Branch("xWp1500", &m_IwsVars.xWp1500, "xWp1500/D");
  m_trIwsRec->Branch("yWp1500", &m_IwsVars.yWp1500, "yWp1500/D");
  m_trIwsRec->Branch("zWp1500", &m_IwsVars.zWp1500, "zWp1500/D");
  m_trIwsRec->Branch("detector", &m_IwsVars.detector, "detector/I");
  m_trIwsRec->Branch("thetaAd", &m_IwsVars.thetaAd, "thetaAd/D");
  m_trIwsRec->Branch("phiAd", &m_IwsVars.phiAd, "phiAd/D");
  m_trIwsRec->Branch("thetaWp", &m_IwsVars.thetaWp, "thetaWp/D");
  m_trIwsRec->Branch("phiWp", &m_IwsVars.phiWp, "phiWp/D");
  m_trIwsRec->Branch("angleBetween", &m_IwsVars.angleBetween, "angleBetween/D");
  m_trIwsRec->Branch("angleBetween1400", &m_IwsVars.angleBetween1400, "angleBetween1400/D");
  m_trIwsRec->Branch("angleBetween1500", &m_IwsVars.angleBetween1500, "angleBetween1500/D");
  
  /// outer water pool reconstruction tree
  m_trOwsRec = new TTree("owsrec", "outer water pool muon reconstruction");
  m_trOwsRec->Branch("xRpc", &m_OwsVars.xRpc, "xRpc/D");
  m_trOwsRec->Branch("yRpc", &m_OwsVars.yRpc, "yRpc/D");
  m_trOwsRec->Branch("zRpc", &m_OwsVars.zRpc, "zRpc/D");
  m_trOwsRec->Branch("xWp", &m_OwsVars.xWp, "xWp/D");
  m_trOwsRec->Branch("yWp", &m_OwsVars.yWp, "yWp/D");
  m_trOwsRec->Branch("zWp", &m_OwsVars.zWp, "zWp/D");
  m_trOwsRec->Branch("nHit1400", &m_OwsVars.nHit1400, "nHit1400/I");
  m_trOwsRec->Branch("nHit1500", &m_OwsVars.nHit1500, "nHit1500/I");
  m_trOwsRec->Branch("xWp1400", &m_OwsVars.xWp1400, "xWp1400/D");
  m_trOwsRec->Branch("yWp1400", &m_OwsVars.yWp1400, "yWp1400/D");
  m_trOwsRec->Branch("zWp1400", &m_OwsVars.zWp1400, "zWp1400/D");
  m_trOwsRec->Branch("xWp1500", &m_OwsVars.xWp1500, "xWp1500/D");
  m_trOwsRec->Branch("yWp1500", &m_OwsVars.yWp1500, "yWp1500/D");
  m_trOwsRec->Branch("zWp1500", &m_OwsVars.zWp1500, "zWp1500/D");
  m_trOwsRec->Branch("detector", &m_OwsVars.detector, "detector/I");
  m_trOwsRec->Branch("thetaAd", &m_OwsVars.thetaAd, "thetaAd/D");
  m_trOwsRec->Branch("phiAd", &m_OwsVars.phiAd, "phiAd/D");
  m_trOwsRec->Branch("thetaWp", &m_OwsVars.thetaWp, "thetaWp/D");
  m_trOwsRec->Branch("phiWp", &m_OwsVars.phiWp, "phiWp/D");
  m_trOwsRec->Branch("angleBetween", &m_OwsVars.angleBetween, "angleBetween/D");
  m_trOwsRec->Branch("angleBetween1400", &m_OwsVars.angleBetween1400, "angleBetween1400/D");
  m_trOwsRec->Branch("angleBetween1500", &m_OwsVars.angleBetween1500, "angleBetween1500/D");
  
  // tree for track comparison
  m_trTraComp = new TTree("tracomp", "comparison between different tracks");
  m_trTraComp->Branch("cosAdTele", &m_TraComp.cosAdTele, "cosAdTele/D");
  
  
  // store the window start time and end time
  TParameter<float> wstart("windowstart", m_windowStart);
  TParameter<float> wend("windowend", m_windowEnd);
  wstart.Write();
  wend.Write();
  
  return StatusCode::SUCCESS;
}


StatusCode MuonLookBack::execute()
{
  
  /// For every event, start with calibration readout.
  m_crHdr = get<CalibReadoutHeader>("/Event/CalibReadout/CalibReadoutHeader");
  if(!m_crHdr) {
    error() << "Can't get calibration readout header." << endreq;
    return StatusCode::FAILURE;
  }
  
  const CalibReadout* calibReadout = m_crHdr->calibReadout();
  if(!calibReadout) {
    error()<<"Failed to get CalibReadout from header"<<endreq;
    return StatusCode::FAILURE;
  }
  
  if( !(calibReadout->detector().isAD() ||
        calibReadout->detector().isWaterShield() ||
        calibReadout->detector().isRPC()) ) {
    info() << "this execution doesn't contain a valid detector." << endreq;
    return StatusCode::SUCCESS;
  }
  
  if(calibReadout->detector().isAD())          processAd();
  if(calibReadout->detector().isWaterShield()) processWp();
  if(calibReadout->detector().isRPC())         processRpc();
  
  /// output progress
  if(exeCntr%1000 == 0)
    info() << exeCntr << " events are processed" << endreq;
  exeCntr++;
  
  /// Get muon event and trace the queue to extract detailed data.
  if (!(exist<UserDataHeader>(evtSvc(),"/Event/Data/Physics/Spallation")))
    return StatusCode::SUCCESS;
  
  debug() << "a muon event found" << endreq;
  m_spall = get<UserDataHeader>("/Event/Data/Physics/Spallation");
  
  /// store the retrigger events tagged in the official production files
  int nRetriggers = m_spall->getInt("nRetriggers");
  vector<int> rtTrigNum = m_spall->getIntArray("triggerNumber_rt");
  vector<int> rtDetId = m_spall->getIntArray("detectorId_rt");
  for(int i = 0; i < nRetriggers; i++)
    m_retriggerSet.insert(MyDataModel::eventId(rtTrigNum[i], (DetectorId_t)rtDetId[i]));
  
  /// number of spallation neutrons from official production files
  int nSpall = m_spall->getInt("nSpall");
  vector<int> spallDetId = m_spall->getIntArray("detectorId_sp");

  vector<MyDataModel::AdTrigger>  admuon;
  vector<MyDataModel::WpTrigger>  wpmuon;
  vector<MyDataModel::RpcTrigger> rpcmuon;
  
  /// locate AD data in the queue
  for(unsigned int i = 0; i < 4; i++)
  {
    m_muonPrompt[i].clear();
    stringstream var;
    stringstream tn;
    var << "hitAD" << i+1;
    tn << "triggerNumber_AD" << i+1;
    
    if(m_spall->getInt(var.str()) == 1)
    {
      MyDataModel::eventId curId(m_spall->getInt(tn.str()),(DetectorId_t)(i+1));
      
      /// store number of spallation neutrons from official production files
      m_nNeutronOfficial[curId] = 0;
      for(int ii = 0; ii < nSpall; ii++)
        if(curId.second == spallDetId[ii]) m_nNeutronOfficial[curId]++;
      
      while (!m_adMuonQue[i].empty())
      {
        if(m_adMuonQue[i].front().id == curId)
        {
          debug() << "an AD" << i+1 << " muon match in the queue found" << endreq;
          m_muonPrompt[i].push_back(m_adMuonQue[i].front());
          m_adMuonQue[i].pop();
          break;
        }
        m_adMuonQue[i].pop();
      }
      if(!m_muonPrompt[i].size()) error() << "no AD" << i+1 << " muon match in the queue found" << endreq;
      /// if WP or RPC also has hits, bind them together
      if(m_spall->getInt("hitIWS") == 1) bindWpMuon(curId, 5);
      if(m_spall->getInt("hitOWS") == 1) bindWpMuon(curId, 6);
      if(m_spall->getInt("hitRPC") == 1) bindRpcMuon(curId);
      
      /// print bind debug information
      debug() << "number of events in IWS, OWS, RPC: ";
      debug() << m_muonLookup[curId].iwsTrig.size() << ", ";
      debug() << m_muonLookup[curId].owsTrig.size() << ", ";
      debug() << m_muonLookup[curId].rpcTrig.size() << endreq;
    }

    if(m_muonPrompt[i].size() > 0) formCoincidence(m_muonPrompt[i], m_adMichQue[i]);
    
    for(unsigned int jj = 0; jj < m_muonPrompt[i].size(); jj++)
      debug() << i << " " << m_muonPrompt[i][jj].id.first << endreq;
  }
  
  while(!m_decayMuons.empty()) {
    fillTree();
    fillIwsRecTree();
    fillOwsRecTree();
    fillTrackCompTree();
    m_muonLookup.erase(m_decayMuons.front().prompt.id);
    m_decayMuons.pop();
  }
  
  m_retriggerSet.clear();
  m_nNeutronOfficial.clear();
  
  return StatusCode::SUCCESS;
}


StatusCode MuonLookBack::finalize()
{
  for(int i = 0; i < 4; i++) {
    info() << "number of AD" << i+1 << " triggers in the queue: " << m_adMuonQue[i].size() << endreq;
  }
  for(int i = 0; i < 2; i++) {
    info() << "number of WP" << i+5 << " triggers in the queue: " << m_wpTrigQue[i].size() << endreq;
  }
  info() << "number of RPC triggers in the queue: " << m_rpcTrigQue.size() << endreq;
  
  m_rootfile->Write();
  delete m_tree;
  delete m_trIwsRec;
  delete m_trOwsRec;
  delete m_trTraComp;
  delete m_rootfile;
  
  return StatusCode::SUCCESS;
}


bool MuonLookBack::flasherTest()
{
  UserDataHeader* calibStats = get<UserDataHeader>("/Event/Data/CalibStats");
  
  float Quadrant = calibStats->getFloat("Quadrant");
  float MaxQ     = calibStats->getFloat("MaxQ");
  float flasherScale = Quadrant*Quadrant+(MaxQ/.45)*(MaxQ/.45);
  
  if(flasherScale < 1.) return false;
  
  return true;
}


void MuonLookBack::processAd()
{
  
  MyDataModel::AdTrigger adTrig;
  
  RecHeader* recHeader = get<RecHeader>("/Event/Rec/AdSimple");
  if(!recHeader) {
    error() << "failed to get AD reconstructed header" << endreq;
    return;
  }
  
  /// start filling AD data
  const RecTrigger& recAdTrig = recHeader->recTrigger();
  adTrig.id.first = recAdTrig.triggerNumber();
  adTrig.id.second = recAdTrig.detector().detectorId();
  adTrig.site = (int)recAdTrig.detector().site();
  adTrig.ts = recAdTrig.triggerTime();
  adTrig.energy = recAdTrig.energy();
  adTrig.energyStatus = recAdTrig.energyStatus();
  adTrig.x = recAdTrig.position().x();
  adTrig.y = recAdTrig.position().y();
  adTrig.z = recAdTrig.position().z();
  adTrig.positionStatus = recAdTrig.positionStatus();
  
  const CalibReadoutPmtCrate* pmtCrate = dynamic_cast<const CalibReadoutPmtCrate*>(m_crHdr->calibReadout());
  CalibReadoutPmtCrate::PmtChannelReadouts channels = pmtCrate->channelReadout();
  adTrig.nHits = channels.size();
  
  /// record the timing information
  CalibReadoutPmtCrate::PmtChannelReadouts::const_iterator channelIter,
  channelDone = channels.end();
  
  for(channelIter=channels.begin();channelIter!=channelDone;++channelIter) {
    const CalibReadoutPmtChannel& channel = *channelIter;
    AdPmtSensor pmtId(channel.pmtSensorId().fullPackedData());
    unsigned int nHits = channel.size();
    if(pmtId.ring() != 0) { // 8-inch PMTs
      for(unsigned int hitIdx = 0; hitIdx < nHits; hitIdx++)
        adTrig.timeAD.push_back(channel.time(hitIdx));
    }
  }
  
  adTrig.isFlasher = flasherTest();
  
  // Transform AD coordinate into global coordinate
  Site_t site = recAdTrig.detector().site();
  stringstream deSite;
  switch (site) {
    case kDayaBay:
      deSite << "/dd/Structure/AD/db-oav";
      deSite << adTrig.id.second;
      break;
    case kLingAo:
      deSite << "/dd/Structure/AD/la-oav";
      deSite << adTrig.id.second;
      break;
    case kFar:
      deSite << "/dd/Structure/AD/far-oav";
      deSite << adTrig.id.second;
      break;
    default:
      error() << "the site of this AD hit is not valid" << endreq;
      return;
  }
  DetectorElement* de = getDet<DetectorElement>(deSite.str());
  Gaudi::XYZPoint recPos = Gaudi::XYZPoint(adTrig.x,adTrig.y,adTrig.z);
  Gaudi::XYZPoint gblPos = de->geometry()->toGlobal(recPos);
  Gaudi::XYZPoint locPos = de->geometry()->toLocal(gblPos);
  
  // determine in which AD zone this event takes place
  const double rIav = 1500, rOav = 2000, rSsv = 2500;
  if(locPos.X()*locPos.X()+locPos.Y()*locPos.Y() < rIav*rIav &&
     fabs(locPos.Z()) < rIav) adTrig.zone = 1;
  else if(locPos.X()*locPos.X()+locPos.Y()*locPos.Y() < rOav*rOav &&
          fabs(locPos.Z()) < rOav) adTrig.zone = 2;
  else if(locPos.X()*locPos.X()+locPos.Y()*locPos.Y() < rSsv*rSsv &&
          fabs(locPos.Z()) < rSsv) adTrig.zone = 3;
  else adTrig.zone = 4;
  
  // translate to pool-centered coordinate
  locPos.SetX(locPos.X() + adLocalTranslation[site][adTrig.id.second].X());
  locPos.SetY(locPos.Y() + adLocalTranslation[site][adTrig.id.second].Y());
  locPos.SetZ(locPos.Z() + adLocalTranslation[site][adTrig.id.second].Z());
  
  int detIdx = (int)adTrig.id.second-1;
  m_gblAdCenter[detIdx] = de->geometry()->toGlobal(Gaudi::XYZPoint(0,0,0));
  Gaudi::XYZPoint disV(gblPos.X()-m_gblAdCenter[detIdx].X(),gblPos.Y()-m_gblAdCenter[detIdx].Y(),gblPos.Z()-m_gblAdCenter[detIdx].Z());
  double distance = sqrt(disV.X()*disV.X()+disV.Y()*disV.Y());
  adTrig.x = gblPos.X();
  adTrig.y = gblPos.Y();
  adTrig.z = gblPos.Z();
  adTrig.xl = locPos.X();
  adTrig.yl = locPos.Y();
  adTrig.zl = locPos.Z();
  debug() << adTrig.x << " " << adTrig.y << " " << adTrig.z << endreq;
  debug() << distance << endreq; /// Raduis 2.1m is a good choice.
  // end of coordinate transformation
  
  m_adMuonQue[(int)adTrig.id.second-1].push(adTrig);
  m_adMichQue[(int)adTrig.id.second-1].push(adTrig);
}


void MuonLookBack::processRpc()
{
  
  RecRpcHeader* recRpc = get<RecRpcHeader>("/Event/Rec/RpcSimple");
  if(!recRpc) {
    error() << "failed to get RPC reconstructed header" << endreq;
    return;
  }
  
  const RecRpcTrigger recRpcTrigger = recRpc->recTrigger();
  debug() << "number of clusters: " << recRpcTrigger.numCluster() << endreq;
  
  const vector<const RecRpcCluster*> clusters = recRpcTrigger.clusters();
  
  MyDataModel::RpcTrigger rpcTrig;
  rpcTrig.id.first = recRpcTrigger.triggerNumber();
  rpcTrig.id.second = recRpcTrigger.detector().detectorId();
  rpcTrig.ts = recRpcTrigger.triggerTime();
  rpcTrig.nRecPts = recRpcTrigger.numCluster();
  rpcTrig.nTracks = recRpcTrigger.numTrack();
  for(unsigned int i = 0; i < clusters.size(); i++)
  {
    rpcTrig.x = clusters[i]->position().x();
    rpcTrig.y = clusters[i]->position().y();
    rpcTrig.z = clusters[i]->position().z();
    debug() << rpcTrig.x << " " << rpcTrig.y << " " << rpcTrig.z << endreq;
  }
  
  Site_t site = recRpcTrigger.detector().site();
  
  string deSite;
  Gaudi::XYZPoint zeroOffset;
  switch (site) {
    case kDayaBay:
      deSite = "/dd/Structure/DayaBay/db-rock/db-rpc";
      zeroOffset.SetX(x_offset[1]);
      zeroOffset.SetY(y_offset[1]);
      zeroOffset.SetZ(z_offset[1]);
      break;
    case kLingAo:
      deSite = "/dd/Structure/DayaBay/la-rock/la-rpc";
      zeroOffset.SetX(x_offset[2]);
      zeroOffset.SetY(y_offset[2]);
      zeroOffset.SetZ(z_offset[2]);
      break;
    case kFar:
      deSite = "/dd/Structure/DayaBay/far-rock/far-rpc";
      zeroOffset.SetX(x_offset[3]);
      zeroOffset.SetY(y_offset[3]);
      zeroOffset.SetZ(z_offset[3]);
      break;
    default:
      error() << "the site of this RPC hit is not valid" << endreq;
      return;
  }
  
  DetectorElement* de = getDet<DetectorElement>(deSite);
  Gaudi::XYZPoint recPos = Gaudi::XYZPoint(rpcTrig.x,rpcTrig.y,rpcTrig.z);
  Gaudi::XYZPoint locPos = recPos;
  /// Jilei hardcoded the coordinate translation in the reconstruction algorithm.
  recPos = recPos - zeroOffset;
  Gaudi::XYZPoint gblPos = de->geometry()->toGlobal(recPos);
  for(unsigned int i = 0; i < clusters.size(); i++)
  {
    rpcTrig.x = gblPos.X();
    rpcTrig.y = gblPos.Y();
    rpcTrig.z = gblPos.Z();
    rpcTrig.xl = locPos.X();
    rpcTrig.yl = locPos.Y();
    rpcTrig.zl = locPos.Z();
    debug() << rpcTrig.x << " " << rpcTrig.y << " " << rpcTrig.z << endreq;
  }
  
    
  // deal with tracks
  rpcTrig.tr_xl = vector<float>(rpcTrig.nTracks*2);
  rpcTrig.tr_yl = vector<float>(rpcTrig.nTracks*2);
  rpcTrig.tr_zl = vector<float>(rpcTrig.nTracks*2);
  for(int i = 0; i < rpcTrig.nTracks; i++) {
    Gaudi::XYZPoint locStart = Gaudi::XYZPoint(clusters[1]->position().x(),clusters[1]->position().y(),clusters[1]->position().z());
    Gaudi::XYZPoint locEnd = Gaudi::XYZPoint(clusters[0]->position().x(),clusters[0]->position().y(),clusters[0]->position().z());
    rpcTrig.tr_xl[0] = locStart.X();
    rpcTrig.tr_yl[0] = locStart.Y();
    rpcTrig.tr_zl[0] = locStart.Z();
    rpcTrig.tr_xl[1] = locEnd.X();
    rpcTrig.tr_yl[1] = locEnd.Y();
    rpcTrig.tr_zl[1] = locEnd.Z();
  }
  
  m_rpcTrigQue.push(rpcTrig);
  /*
   * code snippet for retrieving the RPC mother detector element
  
  const CalibReadoutRpcCrate* rpcCrate = dynamic_cast<const CalibReadoutRpcCrate*>(m_crHdr->calibReadout());
  CalibReadoutRpcCrate::RpcChannelReadouts channels = rpcCrate->channelReadout();
  CalibReadoutRpcCrate::RpcChannelReadouts::iterator ci = channels.begin();
  IRpcGeomInfo *rpcGeomInfo;
  for(; ci!=channels.end(); ci++)
  {
    const CalibReadoutRpcChannel& channel = *ci;
    rpcGeomInfo = m_rpcGeomSvc->get(channel.rpcSensorId().fullPackedData());
  }
  
  info() << rpcGeomInfo->parentDetector().name() << endreq;
  */
}


void MuonLookBack::processWp()
{
  
  MyDataModel::WpTrigger wpTrig;
  
  wpTrig.id.first = m_crHdr->calibReadout()->triggerNumber();
  wpTrig.id.second = m_crHdr->calibReadout()->detector().detectorId();
  wpTrig.ts = m_crHdr->calibReadout()->triggerTime();
  
  const CalibReadoutPmtCrate* pmtCrate = dynamic_cast<const CalibReadoutPmtCrate*>(m_crHdr->calibReadout());
  CalibReadoutPmtCrate::PmtChannelReadouts channels = pmtCrate->channelReadout();
  wpTrig.nHits = channels.size();
  
  /// record the position of each hit PMT
  CalibReadoutPmtCrate::PmtChannelReadouts::const_iterator channelIter,
  channelDone = channels.end();
  for(channelIter=channels.begin();channelIter!=channelDone;++channelIter) {
    const CalibReadoutPmtChannel& channel = *channelIter;
    IPmtGeomInfo *pmtGeomInfo = m_pmtGeomSvc->get(channel.pmtSensorId().fullPackedData());
    wpTrig.pmtPositions.push_back(Gaudi::XYZPoint(pmtGeomInfo->globalPosition().x(),pmtGeomInfo->globalPosition().y(),pmtGeomInfo->globalPosition().z()));
    wpTrig.pmtLocalPositions.push_back(Gaudi::XYZPoint(pmtGeomInfo->localPosition().x(),pmtGeomInfo->localPosition().y(),pmtGeomInfo->localPosition().z()));
    
    /// record the time of the first TDC hit
    PoolPmtSensor pmtId(channel.pmtSensorId().fullPackedData());
    unsigned int nHits = channel.size();
    double earliestTime = 10000;
    /// for loop just in case the hits are not time ordered
    unsigned int earliestHitIdx = 0;
    for(unsigned int hitIdx=0; hitIdx<nHits; hitIdx++)
      if(channel.time(hitIdx) < earliestTime) {
        earliestTime = channel.time(hitIdx);
        earliestHitIdx = hitIdx;
      }
    wpTrig.firstHitTime.push_back(earliestTime);
    wpTrig.firstHitCharge.push_back(channel.charge(earliestHitIdx));
    
    /// for each PMT record the total charge
    wpTrig.pmtCharge.push_back(channel.sumCharge());
  }
  
  m_wpTrigQue[(int)wpTrig.id.second-5].push(wpTrig);
}


void MuonLookBack::formCoincidence(deque<MyDataModel::AdTrigger>& muonlist, queue<MyDataModel::AdTrigger>& michque)
{
  typedef pair<TimeStamp, TimeStamp> TIntv;
  /// slice time axis into intervals
  vector<TIntv > tIntervals;

  for(unsigned int i = 0; i < muonlist.size(); i++) {
    TIntv tInterval;
    TimeStamp stime, etime;
    stime = muonlist[i].ts;
    etime = muonlist[i].ts;
    stime.Add(m_windowStart*1e-6);
    etime.Add(m_windowEnd*1e-6);
    tInterval.first = stime;
    tInterval.second = etime;
    if(i != muonlist.size()-1) {
      TimeStamp nextstime = muonlist[i+1].ts;
      nextstime.Add(m_windowStart*1e-6);
      if(etime > nextstime) tInterval.second = nextstime;
    }
    
    /// make sure the start of the time window is earlier than the next muon time
    if(i != muonlist.size()-1)
      if(stime > muonlist[i+1].ts) continue;
    
    tIntervals.push_back(tInterval);
  }
  
  for(unsigned int i = 0; i < tIntervals.size(); i++)
  {
    MyDataModel::DecayMuon decayMuon;
    decayMuon.prompt = muonlist.front();
    while(!michque.empty()) {
      TimeStamp ts = michque.front().ts;
      if(ts <= tIntervals[i].first) michque.pop();
      else if(ts > tIntervals[i].first && ts < tIntervals[i].second) {
        if(i == tIntervals.size()-1 && michque.back().ts < tIntervals[i].second)
          break;
        if(michque.front().energyStatus == 1 && !(michque.front().isFlasher)
           && michque.front().positionStatus == 1)
           //&& michque.front().energy > 20 && michque.front().energy < 60)
           //&& muonlist.front().energy > 20 && muonlist.front().energy < 90)
          decayMuon.delayed.push_back(michque.front());
        michque.pop();
      }
      else {
        muonlist.pop_front();
        break;
      }
    }

    if(decayMuon.delayed.size())
      debug() << "decay muon found with " << decayMuon.delayed.size() << " Michel electron(s)" << endreq;
    /// store any muon events even if no delayed signals
    m_decayMuons.push(decayMuon);
  }
  debug() << muonlist.size() << " muons left in the queue" << endreq;
}


void MuonLookBack::bindWpMuon(MyDataModel::eventId& adid, int detId)
{
  string det;
  if(detId == 5) det = "IWS";
  if(detId == 6) det = "OWS";
  string tn = "triggerNumber_" + det;
  MyDataModel::eventId id(m_spall->getInt(tn), (DetectorId_t)detId);
  
  while (!m_wpTrigQue[detId-5].empty()) {
    if(id == m_wpTrigQue[detId-5].front().id) {
      debug() << det << " match found" << endreq;
      break;
    }
    m_wpTrigQue[detId-5].pop();
  }
  if(m_wpTrigQue[detId-5].empty()) {
    error() << "no " << det << " match found" << endreq;
    return;
  }
  
  if(m_muonLookup.find(adid) == m_muonLookup.end()) // doesn't exist
    m_muonLookup[adid] = MyDataModel::MuonVeto();
  if(detId == 5) m_muonLookup[adid].iwsTrig.push_back(m_wpTrigQue[detId-5].front());
  if(detId == 6) m_muonLookup[adid].owsTrig.push_back(m_wpTrigQue[detId-5].front());
}


void MuonLookBack::bindRpcMuon(MyDataModel::eventId& adid)
{
  string tn = "triggerNumber_RPC";
  MyDataModel::eventId id(m_spall->getInt(tn), (DetectorId_t)(7));

  while (!m_rpcTrigQue.empty()) {
    if(id == m_rpcTrigQue.front().id) {
      debug() << "RPC match found" << endreq;
      break;
    }
    m_rpcTrigQue.pop();
  }
  if(m_rpcTrigQue.empty()) {
    error() << "no RPC match found" << endreq;
    return;
  }
  
  if(m_muonLookup.find(adid) == m_muonLookup.end()) // doesn't exist
    m_muonLookup[adid] = MyDataModel::MuonVeto();
  m_muonLookup[adid].rpcTrig.push_back(m_rpcTrigQue.front());
}


void MuonLookBack::fillTree()
{

  MyDataModel::AdTrigger           muon = m_decayMuons.front().prompt;
  vector<MyDataModel::AdTrigger> michel = m_decayMuons.front().delayed;
  m_tv.site = muon.site;
  m_tv.detector = muon.id.second;
  /// trigger numbers signment
  m_tv.triggerNumberMu = muon.id.first;
  /// AD center global coordinate
  double x0 = m_gblAdCenter[m_tv.detector-1].X(),
         y0 = m_gblAdCenter[m_tv.detector-1].Y(),
         z0 = m_gblAdCenter[m_tv.detector-1].Z();
  m_tv.tAdMuSec = muon.ts.GetSec();
  m_tv.tAdMuNanoSec = muon.ts.GetNanoSec();
  m_tv.eMu = muon.energy;
  m_tv.nMi = michel.size();
  m_tv.xMu = muon.x;
  m_tv.yMu = muon.y;
  m_tv.zMu = muon.z;
  m_tv.xlMu = muon.xl;
  m_tv.ylMu = muon.yl;
  m_tv.zlMu = muon.zl;
  m_tv.rho2Mu = (muon.x-x0)*(muon.x-x0)+(muon.y-y0)*(muon.y-y0);
  m_tv.muTheta = 0.;
  m_tv.muPhi = 0.;
  m_tv.nNeutron = 0;
  m_tv.nNeutronTight = 0;
  m_tv.nNeutronTight2 = 0;
  m_tv.nAfterPulse = 0;
  /// number of officially tagged spallation neutron
  if(m_nNeutronOfficial.find(muon.id) != m_nNeutronOfficial.end())
    m_tv.nNeutronOfficial = m_nNeutronOfficial[muon.id];
  
  /// start filling Michel electron data
  for(int i = 0; i < m_tv.nMi; i++) {
    m_tv.triggerNumberMi[i] = michel[i].id.first;
    m_tv.tAdMiSec[i] = michel[i].ts.GetSec();
    m_tv.tAdMiNanoSec[i] = michel[i].ts.GetNanoSec();
    m_tv.eMi[i] = michel[i].energy;
    m_tv.dtMuMi[i] = michel[i].ts.CloneAndSubtract(muon.ts).GetSeconds();
    double dx = muon.x - michel[i].x;
    double dy = muon.y - michel[i].y;
    double dz = muon.z - michel[i].z;
    m_tv.dlMuMi[i] = sqrt(dx*dx+dy*dy+dz*dz);
    m_tv.xMi[i] = michel[i].x;
    m_tv.yMi[i] = michel[i].y;
    m_tv.zMi[i] = michel[i].z;
    m_tv.rho2Mi[i] = (m_tv.xMi[i]-x0)*(m_tv.xMi[i]-x0)+(m_tv.yMi[i]-y0)*(m_tv.yMi[i]-y0);
    m_tv.miZone[i] = michel[i].zone;
    
    /// properties that have to be found with a histogram
    TH1F hTdc("hTdc", "hTdc", 100, -1960, -520);
    for(unsigned int j = 0; j < michel[i].timeAD.size(); j++)
      hTdc.Fill(michel[i].timeAD[j]);
    m_tv.nMaxPmtMi[i] = (int)hTdc.GetMaximum();
    m_tv.tMaxPmtMi[i] = hTdc.GetBinCenter(hTdc.GetMaximumBin());
    debug() << "maximum number of PMTs: " << m_tv.nMaxPmtMi[i] << " ";
    debug() << "time: " << m_tv.tMaxPmtMi[i] << " ";
    debug() << "bin: " << hTdc.GetMaximumBin() << endreq;
    
    /// neutron criteria: 7 MeV < eMi < 9 MeV
    if(michel[i].energy > 7 && michel[i].energy < 9) {
      m_tv.nNeutron++;
      m_tv.dlNeutron[m_tv.nNeutron-1] = -1.;
    }
    
    if(michel[i].energy > 7 && michel[i].energy < 9 && m_tv.nMaxPmtMi[i] >= 50)
      m_tv.nNeutronTight++;
    
    /// after pulse criteria: nMaxPmtMi < 50
    if(m_tv.nMaxPmtMi[i] < 50) m_tv.nAfterPulse++;
    
    /// retrigger tag in the official production files
    m_tv.isRetrigger[i] = 0;
    if(m_retriggerSet.find(michel[i].id) != m_retriggerSet.end())
      m_tv.isRetrigger[i] = 1;
    
    /// yield of neutrons with official retrigger tag cut
    if(michel[i].energy > 7 && michel[i].energy < 9 && m_tv.isRetrigger[i] == 0)
      m_tv.nNeutronTight2++;
  }
  
  /// fill water pool information
  vector<MyDataModel::WpTrigger> iws = m_muonLookup[muon.id].iwsTrig;
  vector<MyDataModel::WpTrigger> ows = m_muonLookup[muon.id].owsTrig;
  m_tv.hitIws = iws.size();
  m_tv.nHitsIws = 0;
  m_tv.triggerNumberIws = 0;
  if(iws.size()) {
    m_tv.triggerNumberIws = iws[0].id.first;
    m_tv.nHitsIws = iws[0].nHits;
  }
  // IWS PMT positions
  for(int i = 0; i < m_tv.nHitsIws; i++) {
    m_tv.xIws[i] = iws[0].pmtPositions[i].X();
    m_tv.yIws[i] = iws[0].pmtPositions[i].Y();
    m_tv.zIws[i] = iws[0].pmtPositions[i].Z();
    m_tv.xlIws[i] = iws[0].pmtLocalPositions[i].X();
    m_tv.ylIws[i] = iws[0].pmtLocalPositions[i].Y();
    m_tv.zlIws[i] = iws[0].pmtLocalPositions[i].Z();
    m_tv.firstHitTimeIws[i] = iws[0].firstHitTime[i];
    m_tv.firstHitChargeIws[i] = iws[0].firstHitCharge[i];
    m_tv.totChargeIws[i] = iws[0].pmtCharge[i];
  }
  /// simple reconstrution using only bottom PMTs
  int nHitsIwsBot = 0;
  double totChargeIwsBot = 0;
  m_tv.xlIwsBot = 0;
  m_tv.ylIwsBot = 0;
  m_tv.zlIwsBot = 0;
  if(m_tv.hitIws) {
    for(int i = 0; i < m_tv.nHitsIws; i++)
      if(m_tv.zlIws[i] < -4000) {
        nHitsIwsBot++;
        totChargeIwsBot += m_tv.totChargeIws[i];
        m_tv.xlIwsBot += m_tv.totChargeIws[i]*m_tv.xlIws[i];
        m_tv.ylIwsBot += m_tv.totChargeIws[i]*m_tv.ylIws[i];
        m_tv.zlIwsBot += m_tv.totChargeIws[i]*m_tv.zlIws[i];
      }
    m_tv.xlIwsBot /= totChargeIwsBot;
    m_tv.ylIwsBot /= totChargeIwsBot;
    m_tv.zlIwsBot /= totChargeIwsBot;
  }
  
  m_tv.hitOws = ows.size();
  m_tv.nHitsOws = 0;
  m_tv.triggerNumberOws = 0;
  if(ows.size()) {
    m_tv.triggerNumberOws = ows[0].id.first;
    m_tv.nHitsOws = ows[0].nHits;
  }
  // OWS PMT positions
  for(int i = 0; i < m_tv.nHitsOws; i++) {
    m_tv.xOws[i] = ows[0].pmtPositions[i].X();
    m_tv.yOws[i] = ows[0].pmtPositions[i].Y();
    m_tv.zOws[i] = ows[0].pmtPositions[i].Z();
    m_tv.xlOws[i] = ows[0].pmtLocalPositions[i].X();
    m_tv.ylOws[i] = ows[0].pmtLocalPositions[i].Y();
    m_tv.zlOws[i] = ows[0].pmtLocalPositions[i].Z();
    m_tv.firstHitTimeOws[i] = ows[0].firstHitTime[i];
    m_tv.firstHitChargeOws[i] = ows[0].firstHitCharge[i];
    m_tv.totChargeOws[i] = ows[0].pmtCharge[i];
  }
  /// simple reconstrution using only bottom PMTs
  int nHitsOwsBot = 0;
  double totChargeOwsBot = 0;
  m_tv.xlOwsBot = 0;
  m_tv.ylOwsBot = 0;
  m_tv.zlOwsBot = 0;
  if(m_tv.hitOws) {
    for(int i = 0; i < m_tv.nHitsOws; i++)
      if(m_tv.zlOws[i] < -4000) {
        nHitsOwsBot++;
        totChargeOwsBot += m_tv.totChargeOws[i];
        m_tv.xlOwsBot += m_tv.totChargeOws[i]*m_tv.xlOws[i];
        m_tv.ylOwsBot += m_tv.totChargeOws[i]*m_tv.ylOws[i];
        m_tv.zlOwsBot += m_tv.totChargeOws[i]*m_tv.zlOws[i];
      }
    m_tv.xlOwsBot /= totChargeOwsBot;
    m_tv.ylOwsBot /= totChargeOwsBot;
    m_tv.zlOwsBot /= totChargeOwsBot;
  }
  
  /// fill RPC information
  vector<MyDataModel::RpcTrigger> rpc = m_muonLookup[muon.id].rpcTrig;
  m_tv.hitRpc = rpc.size();
  m_tv.nHitsRpc = 0;
  if(m_tv.hitRpc) {
    m_tv.triggerNumberRpc = rpc[0].id.first;
    m_tv.nHitsRpc = rpc[0].nRecPts;
    m_tv.xRpc = rpc[0].x;
    m_tv.yRpc = rpc[0].y;
    m_tv.zRpc = rpc[0].z;
    m_tv.xlRpc = rpc[0].xl;
    m_tv.ylRpc = rpc[0].yl;
    m_tv.zlRpc = rpc[0].zl;
    /// calculate muon angles
    Gaudi::XYZPoint mudir(rpc[0].x-muon.x, rpc[0].y-muon.y, rpc[0].z-muon.z);
    m_tv.muTheta = (double)mudir.Theta()*180/TMath::Pi();
    m_tv.muPhi = (double)mudir.Phi()*180/TMath::Pi();
    m_tv.nTracks = rpc[0].nTracks;
    if(m_tv.nTracks) {
      m_tv.tele_xl = rpc[0].tr_xl[0];
      m_tv.tele_yl = rpc[0].tr_yl[0];
      m_tv.tele_zl = rpc[0].tr_zl[0];
      m_tv.arr_xl = rpc[0].tr_xl[1];
      m_tv.arr_yl = rpc[0].tr_yl[1];
      m_tv.arr_zl = rpc[0].tr_zl[1];
    }
  }
  
  m_tv.nRpcMi = (int)(rpc.size()*michel.size());
  for(int i = 0; i < m_tv.nRpcMi; i++) {
    m_tv.dtRpcMi[i] = michel[i].ts.CloneAndSubtract(rpc[0].ts).GetSeconds();
    double dx = rpc[0].x - michel[i].x;
    double dy = rpc[0].y - michel[i].y;
    double dz = rpc[0].z - michel[i].z;
    m_tv.dlRpcMi[i] = sqrt(dx*dx+dy*dy+dz*dz);
    
    /// calculate the intersection of the track with OAV
    //Gaudi::XYZPoint intersection = getIntersection(i);
    //m_tv.xRim[i] = intersection.X();
    //m_tv.yRim[i] = intersection.Y();
    //m_tv.zRim[i] = intersection.Z();
    getIntersection(michel[i], m_tv.tRim[i], m_tv.tRimOut[i], m_tv.inPosMi[i], m_tv.outPosMi[i]);
    m_tv.dlRimMi[i] = m_tv.dlRpcMi[i]*(1-m_tv.tRim[i]);
    if(m_tv.tRim[i] > 0.) {
      m_tv.xRim[i] = m_tv.xRpc+m_tv.tRim[i]*(m_tv.xMi[i]-m_tv.xRpc);
      m_tv.yRim[i] = m_tv.yRpc+m_tv.tRim[i]*(m_tv.yMi[i]-m_tv.yRpc);
      m_tv.zRim[i] = m_tv.zRpc+m_tv.tRim[i]*(m_tv.zMi[i]-m_tv.zRpc);
    }
    else {
      m_tv.xRim[i] = x0;
      m_tv.yRim[i] = y0;
      m_tv.zRim[i] = z0;
    }
  }
  
  /// calculate info between RPC and AD reconstructed positions
  if(m_tv.hitRpc) {
    m_tv.dtRpcMu = muon.ts.CloneAndSubtract(rpc[0].ts).GetSeconds();
    double dx = rpc[0].x - muon.x;
    double dy = rpc[0].y - muon.y;
    double dz = rpc[0].z - muon.z;
    m_tv.dlRpcMu = sqrt(dx*dx+dy*dy+dz*dz);
    
    /// calculate the intersection of the track with OAV
    getIntersection(muon, m_tv.tRimMu, m_tv.tRimMuOut, m_tv.inPosMu, m_tv.outPosMu);
    m_tv.dlRimMu = m_tv.dlRpcMu*(1-m_tv.tRimMu);
    m_tv.dlMuRim = m_tv.dlRpcMu*(m_tv.tRimMuOut-1);
    if(m_tv.tRimMu > 0.) {
      m_tv.xRimMu = m_tv.xRpc+m_tv.tRimMu*(m_tv.xMu-m_tv.xRpc);
      m_tv.yRimMu = m_tv.yRpc+m_tv.tRimMu*(m_tv.yMu-m_tv.yRpc);
      m_tv.zRimMu = m_tv.zRpc+m_tv.tRimMu*(m_tv.zMu-m_tv.zRpc);
    }
    else {
      m_tv.xRimMu = x0;
      m_tv.yRimMu = y0;
      m_tv.zRimMu = z0;
    }
    
    /// calculate the outgoing point and its related properties
    if(m_tv.tRimMuOut > 0.) {
      m_tv.xRimMuOut = m_tv.xRpc+m_tv.tRimMuOut*(m_tv.xMu-m_tv.xRpc);
      m_tv.yRimMuOut = m_tv.yRpc+m_tv.tRimMuOut*(m_tv.yMu-m_tv.yRpc);
      m_tv.zRimMuOut = m_tv.zRpc+m_tv.tRimMuOut*(m_tv.zMu-m_tv.zRpc);
    }
    else {
      m_tv.xRimMuOut = x0;
      m_tv.yRimMuOut = y0;
      m_tv.zRimMuOut = z0;
    }
    
    debug() << m_tv.muTheta << " " << m_tv.muPhi << endreq;
    
    /// extrapolate the track to the bottom of the pool
    double tRatio = (m_tv.zlRpc-(-5000))/(m_tv.zlRpc-m_tv.zlMu);
    m_tv.xlBot = m_tv.xlRpc+tRatio*(m_tv.xlMu-m_tv.xlRpc);
    m_tv.ylBot = m_tv.ylRpc+tRatio*(m_tv.ylMu-m_tv.ylRpc);
    m_tv.zlBot = -5000;
    tRatio = (m_tv.zlRpc-(-5000))/(m_tv.zlRpc-m_tv.zlIwsBot);
    double tempx, tempy, tempz;
    tempx = m_tv.xlRpc+tRatio*(m_tv.xlIwsBot-m_tv.xlRpc);
    tempy = m_tv.ylRpc+tRatio*(m_tv.ylIwsBot-m_tv.ylRpc);
    tempz = -5000;
    m_tv.dIwsBot = sqrt((tempx-m_tv.xlBot)*(tempx-m_tv.xlBot) +
                        (tempy-m_tv.ylBot)*(tempy-m_tv.ylBot) +
                        (tempz-m_tv.zlBot)*(tempz-m_tv.zlBot));
    tRatio = (m_tv.zlRpc-(-5000))/(m_tv.zlRpc-m_tv.zlOwsBot);
    tempx = m_tv.xlRpc+tRatio*(m_tv.xlOwsBot-m_tv.xlRpc);
    tempy = m_tv.ylRpc+tRatio*(m_tv.ylOwsBot-m_tv.ylRpc);
    m_tv.dOwsBot = sqrt((tempx-m_tv.xlBot)*(tempx-m_tv.xlBot) +
                        (tempy-m_tv.ylBot)*(tempy-m_tv.ylBot) +
                        (tempz-m_tv.zlBot)*(tempz-m_tv.zlBot));
  }
  
  /// calculate neutron z distance along the muon track. Origin at the rim.
  /// calculate distance between the neutron and the muon track
  if(m_tv.hitRpc) {
    int j = 0;
    for(int i = 0; i < m_tv.nMi; i++)
      if(michel[i].energy > 7 && michel[i].energy < 9) {
        m_tv.dlNeutron[j] = neutronDistance2MuonTrack(rpc[0], muon, michel[i]);
        m_tv.zNeutron[j] = m_tv.dlRimMu + neutronDistanceAlongMuonTrack(rpc[0], muon, michel[i]);
        m_tv.zDepth[j] = 0;
        double muonTrackLength = m_tv.dlRimMu + m_tv.dlMuRim;
        if(muonTrackLength > 0.) m_tv.zDepth[j] = m_tv.zNeutron[j]/muonTrackLength;
        m_tv.nZone[j] = michel[j].zone;
        j++;
      }
  }
  
  m_tree->Fill();
}


void MuonLookBack::getIntersection(MyDataModel::AdTrigger& adEvt, double& tin, double& tout, int& inpos, int& outpos)
{
  
  tin = -1.;
  tout = -1.;
  inpos = 0;
  outpos = 0;
  
  const double rOav = 2000;
  const double halfH8Oav = 2000;
  
  Gaudi::XYZPoint adCenter = Gaudi::XYZPoint(m_gblAdCenter[m_tv.detector-1].X(),
                                             m_gblAdCenter[m_tv.detector-1].Y(),
                                             m_gblAdCenter[m_tv.detector-1].Z());

  /// first check if the AD event position is inside the OAV
  
  double rho2 = (adEvt.x-adCenter.X())*(adEvt.x-adCenter.X())+
                (adEvt.y-adCenter.Y())*(adEvt.y-adCenter.Y());
  
  if(rho2 >= rOav*rOav || fabs(adEvt.z-adCenter.Z()) >= halfH8Oav)
    return;
  
  /// second check if the RPC hit is inside the OAV cylinder
  double a = adEvt.x-m_tv.xRpc;
  double b = adEvt.y-m_tv.yRpc;
  double c = adEvt.z-m_tv.zRpc;
  double l = m_tv.xRpc-adCenter.X();
  double m = m_tv.yRpc-adCenter.Y();

  /// the quadratic discriminant
  double D = (a*l+b*m)*(a*l+b*m)-(a*a+b*b)*(l*l+m*m-rOav*rOav);
  if(l*l+m*m <= rOav*rOav) { // RPC hit inside the OAV cylinder
    tin = (adCenter.Z()+halfH8Oav-m_tv.zRpc)/(adEvt.z-m_tv.zRpc);
    inpos = 0;
  }
  else {                     // RPC hit outside the OAV cylinder
    if(D < 0) {
      return;
    }
    else {
      tin = (-(a*l+b*m)-sqrt(D))/(a*a+b*b);
      inpos = 1;
      if(fabs(m_tv.zRpc+tin*c-adCenter.Z()) > halfH8Oav) {
        tin = (adCenter.Z()+halfH8Oav-m_tv.zRpc)/(adEvt.z-m_tv.zRpc);
        inpos = 0;
      }
    }
  }
  
  /// t_out always corredponds to +D
  if(D > 0) {
    tout = (-(a*l+b*m)+sqrt(D))/(a*a+b*b);
    outpos = 1;
    if(fabs(m_tv.zRpc+tout*c-adCenter.Z()) > halfH8Oav) {
      tout = tout*(m_tv.zRpc-(adCenter.Z()-halfH8Oav))/(-tout*c);
      outpos = 0;
    }
  }
}


double MuonLookBack::neutronDistance2MuonTrack(const MyDataModel::RpcTrigger& pRpc, const MyDataModel::AdTrigger& pAd, const MyDataModel::AdTrigger& pn)
{
  Gaudi::XYZVector vnAd(pAd.x-pn.x, pAd.y-pn.y, pAd.z-pn.z);
  Gaudi::XYZVector vRpcAd(pAd.x-pRpc.x, pAd.y-pRpc.y, pAd.z-pRpc.z);
  double lnAd = sqrt(vnAd.Mag2());
  double lRpcAd = sqrt(vRpcAd.Mag2());
  if(lnAd == 0. || lRpcAd == 0.) return -2;
  double cosv1v2 = vnAd.Dot(vRpcAd)/lnAd/lRpcAd;
  double sinv1v2 = sqrt(1-cosv1v2*cosv1v2);
  return lnAd*sinv1v2;
}


double MuonLookBack::neutronDistanceAlongMuonTrack(const MyDataModel::RpcTrigger& pRpc, const MyDataModel::AdTrigger& pAd, const MyDataModel::AdTrigger& pn)
{
  Gaudi::XYZVector vnAd(pAd.x-pn.x, pAd.y-pn.y, pAd.z-pn.z);
  Gaudi::XYZVector vRpcAd(pAd.x-pRpc.x, pAd.y-pRpc.y, pAd.z-pRpc.z);
  double lnAd = sqrt(vnAd.Mag2());
  double lRpcAd = sqrt(vRpcAd.Mag2());
  if(lnAd == 0. || lRpcAd == 0.) return 0;
  double cosv1v2 = vnAd.Dot(vRpcAd)/lnAd/lRpcAd;
  return -lnAd*cosv1v2;
}


void MuonLookBack::fillIwsRecTree()
{
  // only deal with events with a RPC trigger
  if(!m_tv.hitRpc) return;
  // only deal with events with a IWS trigger
  if(!m_tv.hitIws) return;
  m_IwsVars.detector = m_tv.detector;
  m_IwsVars.xRpc = m_tv.xRpc;
  m_IwsVars.yRpc = m_tv.yRpc;
  m_IwsVars.zRpc = m_tv.zRpc;
  m_IwsVars.thetaAd = m_tv.muTheta;
  m_IwsVars.phiAd = m_tv.muPhi;
  
  double totCharge = 0, totCharge1400 = 0, totCharge1500 = 0;
  double weightedX = 0, weightedY = 0, weightedZ = 0;
  double weightedX1400 = 0, weightedY1400 = 0, weightedZ1400 = 0;
  double weightedX1500 = 0, weightedY1500 = 0, weightedZ1500 = 0;
  
  m_IwsVars.nHit1400 = 0;
  m_IwsVars.nHit1500 = 0;
  for(int i = 0; i < m_tv.nHitsIws; i++) {
    totCharge += m_tv.firstHitChargeIws[i];
    weightedX += m_tv.firstHitChargeIws[i]*m_tv.xIws[i];
    weightedY += m_tv.firstHitChargeIws[i]*m_tv.yIws[i];
    weightedZ += m_tv.firstHitChargeIws[i]*m_tv.zIws[i];
    if(m_tv.firstHitTimeIws[i] < -1400) {
      m_IwsVars.nHit1400++;
      totCharge1400 += m_tv.firstHitChargeIws[i];
      weightedX1400 += m_tv.firstHitChargeIws[i]*m_tv.xIws[i];
      weightedY1400 += m_tv.firstHitChargeIws[i]*m_tv.yIws[i];
      weightedZ1400 += m_tv.firstHitChargeIws[i]*m_tv.zIws[i];
    }
    if(m_tv.firstHitTimeIws[i] < -1500) {
      m_IwsVars.nHit1500++;
      totCharge1500 += m_tv.firstHitChargeIws[i];
      weightedX1500 += m_tv.firstHitChargeIws[i]*m_tv.xIws[i];
      weightedY1500 += m_tv.firstHitChargeIws[i]*m_tv.yIws[i];
      weightedZ1500 += m_tv.firstHitChargeIws[i]*m_tv.zIws[i];
    }
  }
  
  m_IwsVars.xWp = weightedX/totCharge;
  m_IwsVars.yWp = weightedY/totCharge;
  m_IwsVars.zWp = weightedZ/totCharge;
  if(m_IwsVars.nHit1400) {
    m_IwsVars.xWp1400 = weightedX1400/totCharge1400;
    m_IwsVars.yWp1400 = weightedY1400/totCharge1400;
    m_IwsVars.zWp1400 = weightedZ1400/totCharge1400;
  }
  if(m_IwsVars.nHit1500) {
    m_IwsVars.xWp1500 = weightedX1500/totCharge1500;
    m_IwsVars.yWp1500 = weightedY1500/totCharge1500;
    m_IwsVars.zWp1500 = weightedZ1500/totCharge1500;
  }
  
  Gaudi::XYZVector vTrackWp(m_IwsVars.xRpc-m_IwsVars.xWp, m_IwsVars.yRpc-m_IwsVars.yWp, m_IwsVars.zRpc-m_IwsVars.zWp);
  m_IwsVars.thetaWp = (double)vTrackWp.Theta()*180/TMath::Pi();
  m_IwsVars.phiWp = (double)vTrackWp.Phi()*180/TMath::Pi();
  
  /// calculate angle between the 2 tracks
  Gaudi::XYZVector vTrackAd(m_tv.xRpc-m_tv.xMu, m_tv.yRpc-m_tv.yMu, m_tv.zRpc-m_tv.zMu);
  double lTrackAd = sqrt(vTrackAd.Mag2());
  double lTrackWp = sqrt(vTrackWp.Mag2());
  double cosAdWp = vTrackWp.Dot(vTrackAd)/lTrackAd/lTrackWp;
  m_IwsVars.angleBetween = acos(cosAdWp)*180./TMath::Pi();
  
  if(m_IwsVars.nHit1400) {
    Gaudi::XYZVector vTrackWp1400(m_IwsVars.xRpc-m_IwsVars.xWp1400, m_IwsVars.yRpc-m_IwsVars.yWp1400, m_IwsVars.zRpc-m_IwsVars.zWp1400);
    double lTrackWp1400 = sqrt(vTrackWp1400.Mag2());
    double cosAdWp1400 = vTrackWp1400.Dot(vTrackAd)/lTrackAd/lTrackWp1400;
    m_IwsVars.angleBetween1400 = acos(cosAdWp1400)*180./TMath::Pi();
  }
  
  if(m_IwsVars.nHit1500) {
    Gaudi::XYZVector vTrackWp1500(m_IwsVars.xRpc-m_IwsVars.xWp1500, m_IwsVars.yRpc-m_IwsVars.yWp1500, m_IwsVars.zRpc-m_IwsVars.zWp1500);
    double lTrackWp1500 = sqrt(vTrackWp1500.Mag2());
    double cosAdWp1500 = vTrackWp1500.Dot(vTrackAd)/lTrackAd/lTrackWp1500;
    m_IwsVars.angleBetween1500 = acos(cosAdWp1500)*180./TMath::Pi();
  }
  
  m_trIwsRec->Fill();
}


void MuonLookBack::fillOwsRecTree()
{
  // only deal with events with a RPC trigger
  if(!m_tv.hitRpc) return;
  // only deal with events with a OWS trigger
  if(!m_tv.hitOws) return;
  m_OwsVars.detector = m_tv.detector;
  m_OwsVars.xRpc = m_tv.xRpc;
  m_OwsVars.yRpc = m_tv.yRpc;
  m_OwsVars.zRpc = m_tv.zRpc;
  m_OwsVars.thetaAd = m_tv.muTheta;
  m_OwsVars.phiAd = m_tv.muPhi;
  
  double totCharge = 0, totCharge1400 = 0, totCharge1500 = 0;
  double weightedX = 0, weightedY = 0, weightedZ = 0;
  double weightedX1400 = 0, weightedY1400 = 0, weightedZ1400 = 0;
  double weightedX1500 = 0, weightedY1500 = 0, weightedZ1500 = 0;
  
  m_OwsVars.nHit1400 = 0;
  m_OwsVars.nHit1500 = 0;
  for(int i = 0; i < m_tv.nHitsIws; i++) {
    totCharge += m_tv.firstHitChargeOws[i];
    weightedX += m_tv.firstHitChargeOws[i]*m_tv.xOws[i];
    weightedY += m_tv.firstHitChargeOws[i]*m_tv.yOws[i];
    weightedZ += m_tv.firstHitChargeOws[i]*m_tv.zOws[i];
    if(m_tv.firstHitTimeOws[i] < -1400) {
      m_OwsVars.nHit1400++;
      totCharge1400 += m_tv.firstHitChargeOws[i];
      weightedX1400 += m_tv.firstHitChargeOws[i]*m_tv.xOws[i];
      weightedY1400 += m_tv.firstHitChargeOws[i]*m_tv.yOws[i];
      weightedZ1400 += m_tv.firstHitChargeOws[i]*m_tv.zOws[i];
    }
    if(m_tv.firstHitTimeOws[i] < -1500) {
      m_OwsVars.nHit1500++;
      totCharge1500 += m_tv.firstHitChargeOws[i];
      weightedX1500 += m_tv.firstHitChargeOws[i]*m_tv.xOws[i];
      weightedY1500 += m_tv.firstHitChargeOws[i]*m_tv.yOws[i];
      weightedZ1500 += m_tv.firstHitChargeOws[i]*m_tv.zOws[i];
    }
  }
  
  m_OwsVars.xWp = weightedX/totCharge;
  m_OwsVars.yWp = weightedY/totCharge;
  m_OwsVars.zWp = weightedZ/totCharge;
  if(m_OwsVars.nHit1400) {
    m_OwsVars.xWp1400 = weightedX1400/totCharge1400;
    m_OwsVars.yWp1400 = weightedY1400/totCharge1400;
    m_OwsVars.zWp1400 = weightedZ1400/totCharge1400;
  }
  if(m_OwsVars.nHit1500) {
    m_OwsVars.xWp1500 = weightedX1500/totCharge1500;
    m_OwsVars.yWp1500 = weightedY1500/totCharge1500;
    m_OwsVars.zWp1500 = weightedZ1500/totCharge1500;
  }
  
  Gaudi::XYZVector vTrackWp(m_OwsVars.xRpc-m_OwsVars.xWp, m_OwsVars.yRpc-m_OwsVars.yWp, m_OwsVars.zRpc-m_OwsVars.zWp);
  m_OwsVars.thetaWp = (double)vTrackWp.Theta()*180/TMath::Pi();
  m_OwsVars.phiWp = (double)vTrackWp.Phi()*180/TMath::Pi();
  
  /// calculate angle between the 2 tracks
  Gaudi::XYZVector vTrackAd(m_tv.xRpc-m_tv.xMu, m_tv.yRpc-m_tv.yMu, m_tv.zRpc-m_tv.zMu);
  double lTrackAd = sqrt(vTrackAd.Mag2());
  double lTrackWp = sqrt(vTrackWp.Mag2());
  double cosAdWp = vTrackWp.Dot(vTrackAd)/lTrackAd/lTrackWp;
  m_OwsVars.angleBetween = acos(cosAdWp)*180./TMath::Pi();
  
  if(m_OwsVars.nHit1400) {
    Gaudi::XYZVector vTrackWp1400(m_OwsVars.xRpc-m_OwsVars.xWp1400, m_OwsVars.yRpc-m_OwsVars.yWp1400, m_OwsVars.zRpc-m_OwsVars.zWp1400);
    double lTrackWp1400 = sqrt(vTrackWp1400.Mag2());
    double cosAdWp1400 = vTrackWp1400.Dot(vTrackAd)/lTrackAd/lTrackWp1400;
    m_OwsVars.angleBetween1400 = acos(cosAdWp1400)*180./TMath::Pi();
  }
  
  if(m_OwsVars.nHit1500) {
    Gaudi::XYZVector vTrackWp1500(m_OwsVars.xRpc-m_OwsVars.xWp1500, m_OwsVars.yRpc-m_OwsVars.yWp1500, m_OwsVars.zRpc-m_OwsVars.zWp1500);
    double lTrackWp1500 = sqrt(vTrackWp1500.Mag2());
    double cosAdWp1500 = vTrackWp1500.Dot(vTrackAd)/lTrackAd/lTrackWp1500;
    m_OwsVars.angleBetween1500 = acos(cosAdWp1500)*180./TMath::Pi();
  }
  
  m_trOwsRec->Fill();
}


void MuonLookBack::fillTrackCompTree()
{
  if(!m_tv.nTracks) return;
  
  Gaudi::XYZVector teleRpc(m_tv.arr_xl-m_tv.tele_xl, m_tv.arr_yl-m_tv.tele_yl, m_tv.arr_zl-m_tv.tele_zl);
  Gaudi::XYZVector rpcAd(m_tv.xlMu-m_tv.arr_xl, m_tv.ylMu-m_tv.arr_yl, m_tv.zlMu-m_tv.arr_zl);
  
  m_TraComp.cosAdTele = getCosine(teleRpc, rpcAd);
  if(!std::isinf(m_TraComp.cosAdTele))
    m_trTraComp->Fill();
}


double MuonLookBack::getCosine(Gaudi::XYZVector v1, Gaudi::XYZVector v2)
{
  double l1 = sqrt(v1.Mag2());
  double l2 = sqrt(v2.Mag2());
  return v1.Dot(v2)/l1/l2;
}
