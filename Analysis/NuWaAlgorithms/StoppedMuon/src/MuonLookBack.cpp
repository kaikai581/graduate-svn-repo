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
using namespace ROOT::Math;
using namespace Site;
using namespace std;
using namespace DayaBay;


MuonLookBack::MuonLookBack(const string& name, ISvcLocator* svcloc) :
GaudiAlgorithm(name, svcloc), exeCntr(1), m_totCylLen(0)
{
  declareProperty("FileName", m_rootfilename = "output.root", "root file name");
  declareProperty("WindowStart", m_windowStart = 0.5, "start of the time window after a muon in microsecond");
  declareProperty("WindowEnd", m_windowEnd = 20, "end of the time window after a muon in microsecond");
  declareProperty("TrackCylinderRadius", m_trackCylinderRadius = 1000, "the inscribing cylinder coaxial with the muon track");
  
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
  m_tree->Branch("nBkg", &m_tv.nBkg, "nBkg/I");
  m_tree->Branch("hitIws", &m_tv.hitIws, "hitIws/I");
  m_tree->Branch("nHitsIws", &m_tv.nHitsIws, "nHitsIws/I");
  m_tree->Branch("hitOws", &m_tv.hitOws, "hitOws/I");
  m_tree->Branch("nHitsOws", &m_tv.nHitsOws, "nHitsOws/I");
  m_tree->Branch("hitRpc", &m_tv.hitRpc, "hitRpc/I");
  m_tree->Branch("nHitsRpc", &m_tv.nHitsRpc, "nHitsRpc/I");
  m_tree->Branch("nRpcMi", &m_tv.nRpcMi, "nRpcMi/I");
  m_tree->Branch("muTheta", &m_tv.muTheta, "muTheta/D");
  m_tree->Branch("muPhi", &m_tv.muPhi, "muPhi/D");
  m_tree->Branch("inscribable", &m_tv.inscribable, "inscribable/O");
  m_tree->Branch("cylLen", &m_tv.cylLen, "cylLen/D");
  m_tree->Branch("zInsCylTopTrackCoord", &m_tv.zInsCylTopTrackCoord, "zInsCylTopTrackCoord/D");
  m_tree->Branch("zInsCylBotTrackCoord", &m_tv.zInsCylBotTrackCoord, "zInsCylBotTrackCoord/D");
  m_tree->Branch("xInsCylTopIavCoord", &m_tv.xInsCylTopIavCoord, "xInsCylTopIavCoord/D");
  m_tree->Branch("yInsCylTopIavCoord", &m_tv.yInsCylTopIavCoord, "yInsCylTopIavCoord/D");
  m_tree->Branch("zInsCylTopIavCoord", &m_tv.zInsCylTopIavCoord, "zInsCylTopIavCoord/D");
  m_tree->Branch("xInsCylBotIavCoord", &m_tv.xInsCylBotIavCoord, "xInsCylBotIavCoord/D");
  m_tree->Branch("yInsCylBotIavCoord", &m_tv.yInsCylBotIavCoord, "yInsCylBotIavCoord/D");
  m_tree->Branch("zInsCylBotIavCoord", &m_tv.zInsCylBotIavCoord, "zInsCylBotIavCoord/D");
  // variable arrays
  m_tree->Branch("triggerNumberMi", m_tv.triggerNumberMi, "triggerNumberMi[nMi]/I");
  m_tree->Branch("isRetrigger", m_tv.isRetrigger, "isRetrigger[nMi]/I");
  m_tree->Branch("inCyl", m_tv.inCyl, "inCyl[nMi]/I");
  m_tree->Branch("zTrack", m_tv.zTrack, "zTrack[nMi]/D");
  m_tree->Branch("rhoTrack", m_tv.rhoTrack, "rhoTrack[nMi]/D");
  m_tree->Branch("phiTrack", m_tv.phiTrack, "phiTrack[nMi]/D");
  m_tree->Branch("nMaxPmtMi", m_tv.nMaxPmtMi, "nMaxPmtMi[nMi]/I");
  m_tree->Branch("tMaxPmtMi", m_tv.tMaxPmtMi, "tMaxPmtMi[nMi]/D");
  m_tree->Branch("tAdMiSec", m_tv.tAdMiSec, "tAdMiSec[nMi]/I");
  m_tree->Branch("tAdMiNanoSec", m_tv.tAdMiNanoSec, "tAdMiNanoSec[nMi]/I");
  m_tree->Branch("eMi", m_tv.eMi, "eMi[nMi]/D");
  m_tree->Branch("eBkg", m_tv.eBkg, "eBkg[nBkg]/D");
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
  m_tree->Branch("xlMi", m_tv.xlMi, "xlMi[nMi]/D");
  m_tree->Branch("ylMi", m_tv.ylMi, "ylMi[nMi]/D");
  m_tree->Branch("zlMi", m_tv.zlMi, "zlMi[nMi]/D");
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
  m_tree->Branch("dlIav", &m_tv.dlIav, "dlIav/D");
  m_tree->Branch("xIavIn", &m_tv.xIavIn, "xIavIn/D");
  m_tree->Branch("yIavIn", &m_tv.yIavIn, "yIavIn/D");
  m_tree->Branch("zIavIn", &m_tv.zIavIn, "zIavIn/D");
  m_tree->Branch("xIavOut", &m_tv.xIavOut, "xIavOut/D");
  m_tree->Branch("yIavOut", &m_tv.yIavOut, "yIavOut/D");
  m_tree->Branch("zIavOut", &m_tv.zIavOut, "zIavOut/D");
  // water pool PMT coordinates
  m_tree->Branch("xIws", m_tv.xIws, "xIws[nHitsIws]/D");
  m_tree->Branch("yIws", m_tv.yIws, "yIws[nHitsIws]/D");
  m_tree->Branch("zIws", m_tv.zIws, "zIws[nHitsIws]/D");
  m_tree->Branch("xlIws", m_tv.xlIws, "xlIws[nHitsIws]/D");
  m_tree->Branch("ylIws", m_tv.ylIws, "ylIws[nHitsIws]/D");
  m_tree->Branch("zlIws", m_tv.zlIws, "zlIws[nHitsIws]/D");
  m_tree->Branch("firstHitTimeIws", m_tv.firstHitTimeIws, "firstHitTimeIws[nHitsIws]/D");
  m_tree->Branch("xOws", m_tv.xOws, "xOws[nHitsOws]/D");
  m_tree->Branch("yOws", m_tv.yOws, "yOws[nHitsOws]/D");
  m_tree->Branch("zOws", m_tv.zOws, "zOws[nHitsOws]/D");
  m_tree->Branch("xlOws", m_tv.xlOws, "xlOws[nHitsOws]/D");
  m_tree->Branch("ylOws", m_tv.ylOws, "ylOws[nHitsOws]/D");
  m_tree->Branch("zlOws", m_tv.zlOws, "zlOws[nHitsOws]/D");
  m_tree->Branch("firstHitTimeOws", m_tv.firstHitTimeOws, "firstHitTimeOws[nHitsOws]/D");
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
  
  // store the window start time and end time
  TParameter<float> wstart("windowstart", m_windowStart);
  TParameter<float> wend("windowend", m_windowEnd);
  TParameter<float> trCylRad("trackCylinderRadius", m_trackCylinderRadius);
  wstart.Write();
  wend.Write();
  trCylRad.Write();
  
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
    //for(unsigned int kk = 0; kk < m_muonPrompt[i].size(); kk++)
      //if(m_muonPrompt[i][kk].id.first == 1251)
      //{
        //info() << "AD muon 1251 detector " << m_muonPrompt[i][kk].id.second << " found" << endreq;
        //info() << " time stamp: " << m_muonPrompt[i][kk].ts.AsString() << endreq;
        //info() << "loop " << exeCntr << endreq;
      //}
    unsigned int listSize = m_muonPrompt[i].size();
    if(listSize)
    {
      MyDataModel::AdTrigger lastMuon = m_muonPrompt[i][listSize-1];
      m_muonPrompt[i].clear();
      m_muonPrompt[i].push_back(lastMuon);
    }
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
  //while(m_decayMuons.size() > 1) {
    fillTree();
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
  
  /// bookkeeping the total length of the virtual cylinders
  TParameter<float> totCylLen("totCylLen", m_totCylLen);
  m_rootfile->cd();
  totCylLen.Write();
  
  m_rootfile->Write();
  delete m_tree;
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
    for(unsigned int hitIdx=0; hitIdx<nHits; hitIdx++)
      if(channel.time(hitIdx) < earliestTime) earliestTime = channel.time(hitIdx);
    wpTrig.firstHitTime.push_back(earliestTime);
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
  
  /// before working on the Michel electron, make a copy for background events
  queue<MyDataModel::AdTrigger> bkgque(michque);
  
  for(unsigned int i = 0; i < tIntervals.size(); i++)
  {
    MyDataModel::DecayMuon decayMuon;
    decayMuon.prompt = muonlist.front();
    
    /// background events in the window (1000+start,1000+end)
    while(!bkgque.empty()) {
      TimeStamp ts = bkgque.front().ts;
      TimeStamp bkgWinStart = tIntervals[i].first;
      TimeStamp bkgWinEnd = tIntervals[i].second;
      bkgWinStart.Add(m_windowEnd*1e-6);
      bkgWinEnd.Add(m_windowEnd*1e-6);
      if(ts <= bkgWinStart) bkgque.pop();
      else if(ts > bkgWinStart && ts < bkgWinEnd) {
        if(i == tIntervals.size()-1 && bkgque.back().ts < bkgWinEnd)
          break;
        if(bkgque.front().energyStatus == 1 && !(bkgque.front().isFlasher)
           && bkgque.front().positionStatus == 1)
          decayMuon.background.push_back(bkgque.front());
        bkgque.pop();
      }
      else break;
    }
    
    while(!michque.empty()) {
      TimeStamp ts = michque.front().ts;
      if(ts <= tIntervals[i].first)
      {
        if(michque.front().id.first == 1257)
        {
          info() << "1257 found. first if" << " " << i << ". detector " << michque.front().id.second << endreq;
          info() << "loop " << exeCntr << endreq;
        }
        michque.pop();
      }
      else if(ts > tIntervals[i].first && ts < tIntervals[i].second) {
        if(i == tIntervals.size()-1 && michque.back().ts < tIntervals[i].second)
          break;
        if(michque.front().energyStatus == 1 && !(michque.front().isFlasher)
           && michque.front().positionStatus == 1)
           //&& michque.front().energy > 20 && michque.front().energy < 60)
           //&& muonlist.front().energy > 20 && muonlist.front().energy < 90)
          decayMuon.delayed.push_back(michque.front());
        if(michque.front().id.first == 1257)
        {
          info() << "1257 found. third if" << " " << i << ". detector " << michque.front().id.second << endreq;
          info() << "loop " << exeCntr << endreq;
        }
        if(i != tIntervals.size()-1)
          michque.pop();
        else break;
      }
      else {
        /// debug code...
        if(muonlist.front().id.first == 1251)
        {
          info() << "AD muon 1251 found";
          info() << " time stamp: " << muonlist.front().ts.AsString() << endreq;
          info() << "loop " << exeCntr << endreq;
        }
        if(muonlist.size() > 1) /// newly added
          muonlist.pop_front();
        break;
      }
    }

    if(decayMuon.delayed.size())
    {
      debug() << "decay muon found with " << decayMuon.delayed.size() << " Michel electron(s)" << endreq;
      /// 3/28/2013
      /// it seems I lose some inter muon events by comparing with Yung-Shun
      /// print more information to check
      info() << "AD" << decayMuon.prompt.id.second << endreq;
      info() << "muon data:" << endreq;
      info() << decayMuon.prompt.id.first << endreq;
      info() << "intermuon data:" << endreq;
      for(unsigned ni = 0; ni < decayMuon.delayed.size(); ni++)
        info() << decayMuon.delayed[ni].id.first << endreq;
    }
    /// store any muon events even if no delayed signals
    if(i < tIntervals.size()-1)
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


/// given a circle with center at (0,0,zTrackCoord) in track coordinate, 
/// find if this circle is completely contained in the IAV
bool MuonLookBack::circleInsideIav(TrackInfo& ti, double zTrackCoord)
{
  int nstep = 360;
  
  AxisAngle rotTrack(ROOT::Math::XYZVector(0,0,1).Cross(-ti.dirVec),(-ti.dirVec).Theta());
  
  Transform3D track2Iav(rotTrack, (XYZVector)ti.pXYPlane);
  
  for(int i = 0; i < nstep; i++)
  {
    double th = i*TMath::TwoPi()/nstep;
    XYZPoint pCircleTrackCoord(m_trackCylinderRadius*cos(th), m_trackCylinderRadius*sin(th), zTrackCoord);
    
    double thisZ = track2Iav(pCircleTrackCoord).Z();
    
    if(thisZ > RADIUSIAV || thisZ < -RADIUSIAV) return false;
  }
  
  return true;
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
  vector<MyDataModel::AdTrigger>    bkg = m_decayMuons.front().background;
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
  m_tv.nBkg = bkg.size();
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
  m_tv.dlIav = -1;
  /// number of officially tagged spallation neutron
  if(m_nNeutronOfficial.find(muon.id) != m_nNeutronOfficial.end())
    m_tv.nNeutronOfficial = m_nNeutronOfficial[muon.id];
  
  /// start filling background data
  for(int i = 0; i < m_tv.nBkg; i++) {
    m_tv.eBkg[i] = bkg[i].energy;
  }
  
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
    m_tv.xlMi[i] = michel[i].xl;
    m_tv.ylMi[i] = michel[i].yl;
    m_tv.zlMi[i] = michel[i].zl;
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
    
    /// calculate the intersection of the track with IAV
    m_tv.dlIav = getIavTrackLength(muon);
    
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
  
  /// calculate length of the virtual cylinders
  m_tv.cylLen = getCylLen();
  //if(m_tv.cylLen > 0) m_totCylLen += m_tv.cylLen;
  /// check if the ad event is inside the virtual cylinder
  for(int i = 0; i < m_tv.nMi; i++) {
    m_tv.inCyl[i] = 0;
    if(m_tv.hitRpc)
      m_tv.inCyl[i] = isInCyl(rpc[0], muon, michel[i]);
  }
  
  
  /* get the inscribing cylinder analytically if a track exists */
  
  /// initialize related variables
  m_tv.inscribable = false;
  m_tv.cylLen = -1;
  for(int ii = 0; ii < m_tv.nMi; ii++)
  {
    m_tv.inCyl[ii] = 0;
    m_tv.zTrack[ii] = 2000;
    m_tv.rhoTrack[ii] = -1;
    m_tv.phiTrack[ii] = -3*TMath::Pi();
  }
  
  TrackInfo trackInfo;
  //if(m_tv.nHitsRpc == 1)
  if(m_tv.hitRpc)
  {
    
    /// get AD center as the origin
    XYZPoint adCenter = Gaudi::XYZPoint(m_gblAdCenter[m_tv.detector-1].X(),m_gblAdCenter[m_tv.detector-1].Y(),m_gblAdCenter[m_tv.detector-1].Z());
    
    /// RPC point coordinate
    trackInfo.pRpc = XYZPoint(m_tv.xRpc,m_tv.yRpc,m_tv.zRpc);
    trackInfo.pRpc = trackInfo.pRpc - adCenter;
    
    /// AD muon point coordinate
    XYZPoint pMu(m_tv.xMu, m_tv.yMu, m_tv.zMu);
    pMu = pMu - adCenter;
    
    /// direction vector from RPC to AD muon
    trackInfo.dirVec = pMu - trackInfo.pRpc;
    trackInfo.dirVec = trackInfo.dirVec/sqrt(trackInfo.dirVec.Mag2());
    
    /// intersection point of track and xy plane
    double scale = -trackInfo.pRpc.Z()/trackInfo.dirVec.Z();
    trackInfo.pXYPlane = trackInfo.pRpc + scale*trackInfo.dirVec;
    
    /// start finding the tangent points of the small cylinder to IAV
    XYZVector target = -trackInfo.dirVec;
    
    XYZVector iavRotationAxis = -XYZVector(0,0,1).Cross(target);
    
    AxisAngle rotateIav(iavRotationAxis, target.Theta());
    AxisAngle rotateTrack = rotateIav.Inverse();
    
    XYZVector translateIav = -(rotateIav*((XYZVector)trackInfo.pXYPlane));
    
    /// extract the rotation matrix elements
    XYZVector e1(1,0,0);
    XYZVector e2(0,1,0);
    XYZVector e3(0,0,1);
    
    double a11 = e1.Dot(rotateTrack*e1);
    double a12 = e1.Dot(rotateTrack*e2);
    double a13 = e1.Dot(rotateTrack*e3);
    double a21 = e2.Dot(rotateTrack*e1);
    double a22 = e2.Dot(rotateTrack*e2);
    double a23 = e2.Dot(rotateTrack*e3);
    double s1 = (rotateTrack*(-translateIav)).X();
    double s2 = (rotateTrack*(-translateIav)).Y();
    
    /// draw the solution curve
    int nstep = 360;
    
    /// if the discriminant is ever negative, a closed loop is formed
    /// and a small cylinder can not be inscribed
    trackInfo.twoContour = true;
    
    /// find the minimum of the upper curve and
    /// the maximum of the lower curve and the corresponding theta values
    double zupminTrackCoord, zlowmaxTrackCoord;
    double phiUpTrackCoord, phiLowTrackCoord;
    
    bool firstRealSolution = true;
    
    /// for the equation, see the group meeting report on Jan. 31, 2013
    for(int i = 0; i < nstep; i++)
    {
      
      double x = -TMath::Pi()+TMath::TwoPi()/nstep*i;
      double R = RADIUSIAV;
      double r = m_trackCylinderRadius;
      double x1 = r*cos(x), x2 = r*sin(x);
      double a2 = a13*a13+a23*a23;
      double rcoor1 = a11*x1+a12*x2+s1, rcoor2 = a21*x1+a22*x2+s2;
      double a1 = 2*(a13*rcoor1+a23*rcoor2);
      double a0 = rcoor1*rcoor1+rcoor2*rcoor2-R*R;
      double D = a1*a1-4*a0*a2;
      
      if(D > 0) {
        
        double yup = 1./2/a2*(-a1+sqrt(D));
        double ylow = 1./2/a2*(-a1-sqrt(D));
        
        if(firstRealSolution) /// initialize
        {
          zupminTrackCoord = yup;
          zlowmaxTrackCoord = ylow;
          
          phiUpTrackCoord = x;
          phiLowTrackCoord = x;
          
          firstRealSolution = false;
        }
        
        else {
          if(yup < zupminTrackCoord) {
            zupminTrackCoord = yup;
            phiUpTrackCoord = x;
          }
          
          if(ylow > zlowmaxTrackCoord) {
            zlowmaxTrackCoord = ylow;
            phiLowTrackCoord = x;
          }
        }
      }
      
      else trackInfo.twoContour = false;
    } /* end of for(int i = 0; i < nstep; i++) loop */
    
    /// now consider the intersection is on the top/bottom IAV plane
    XYZPoint pTrackTopIavCoord, pTrackDownIavCoord;
    
    pTrackTopIavCoord = trackInfo.pRpc - (trackInfo.pRpc.Z()-RADIUSIAV)/trackInfo.dirVec.Z()*trackInfo.dirVec;
    pTrackDownIavCoord = trackInfo.pRpc - (trackInfo.pRpc.Z()+RADIUSIAV)/trackInfo.dirVec.Z()*trackInfo.dirVec;
    
    XYZVector vPerp = e3.Cross(trackInfo.dirVec);
    
    /// normalize the perpendicular vector
    vPerp /= sqrt(vPerp.Mag2());
    XYZVector vAlong = vPerp.Cross(e3);
    double tiltAng = (-trackInfo.dirVec).Theta();
    
    XYZPoint pTangentTopIavCoord, pTangentBotIavCoord;
    
    pTangentTopIavCoord = pTrackTopIavCoord+(m_trackCylinderRadius/cos(tiltAng))*vAlong;
    pTangentBotIavCoord = pTrackDownIavCoord-(m_trackCylinderRadius/cos(tiltAng))*vAlong;
    
    /// check if zupmin is lower than the z of the top IAV plane
    /// and zlowmax is higher than the z of the bottom IAV plane
    XYZPoint pUpMinTrackCoord(m_trackCylinderRadius*cos(phiUpTrackCoord), m_trackCylinderRadius*sin(phiUpTrackCoord), zupminTrackCoord);
    XYZPoint pLowMaxTrackCoord(m_trackCylinderRadius*cos(phiLowTrackCoord), m_trackCylinderRadius*sin(phiLowTrackCoord), zlowmaxTrackCoord);
    
    /// define the isometric transformation
    Transform3D track2Iav(rotateTrack, (XYZVector)trackInfo.pXYPlane);
    Transform3D iav2Track(rotateIav, translateIav);
    
    XYZPoint pUpMinIavCoord = track2Iav(pUpMinTrackCoord);
    XYZPoint pLowMaxIavCoord = track2Iav(pLowMaxTrackCoord);
    
    /// get top and bottom tangent points in track coordinate system
    XYZPoint pTangentTopTrackCoord = iav2Track(pTangentTopIavCoord);
    XYZPoint pTangentBotTrackCoord = iav2Track(pTangentBotIavCoord);
    
    if(trackInfo.twoContour)
    {
      bool topTangentInside = (sqrt(pTangentTopIavCoord.X()*pTangentTopIavCoord.X()+pTangentTopIavCoord.Y()*pTangentTopIavCoord.Y()) < RADIUSIAV);
      
      bool botTangentInside = (sqrt(pTangentBotIavCoord.X()*pTangentBotIavCoord.X()+pTangentBotIavCoord.Y()*pTangentBotIavCoord.Y()) < RADIUSIAV);
      
      bool topLoopInside = (pUpMinIavCoord.Z() < RADIUSIAV);
      bool botLoopInside = (pLowMaxIavCoord.Z() > -RADIUSIAV);
      
      bool topInscribable, botInscribable;
      
      bool upCircleInsideIav = circleInsideIav(trackInfo, zupminTrackCoord);
      bool lowCircleInsideIav = circleInsideIav(trackInfo, zlowmaxTrackCoord);
      
      /// deal with top
      if(topTangentInside && topLoopInside) {
        trackInfo.pInsCylTopIavCoord = (pTangentTopTrackCoord.Z() < pUpMinTrackCoord.Z()) ? pTangentTopIavCoord : pUpMinIavCoord;
        topInscribable = (upCircleInsideIav ? true : false);
      }
      
      if(topTangentInside && !topLoopInside) {
        trackInfo.pInsCylTopIavCoord = pTangentTopIavCoord;
        topInscribable = true;
      }
      
      if(!topTangentInside && topLoopInside) {
        trackInfo.pInsCylTopIavCoord = pUpMinIavCoord;
        topInscribable = (upCircleInsideIav ? true : false);
      }
      
      if(!topTangentInside && !topLoopInside) topInscribable = false;
      
      /// deal with bottom
      if(botTangentInside && botLoopInside) {
        trackInfo.pInsCylBotIavCoord = (pTangentBotTrackCoord.Z() > pLowMaxTrackCoord.Z()) ? pTangentBotIavCoord : pLowMaxIavCoord;
        botInscribable = (lowCircleInsideIav ? true : false);
      }
      
      if(botTangentInside && !botLoopInside) {
        trackInfo.pInsCylBotIavCoord = pTangentBotIavCoord;
        botInscribable = true;
      }
      
      if(!botTangentInside && botLoopInside) {
        trackInfo.pInsCylBotIavCoord = pLowMaxIavCoord;
        botInscribable = (lowCircleInsideIav ? true : false);
      }
      
      if(!botTangentInside && !botLoopInside) botInscribable = false;
      
      trackInfo.zInsCylTopTrackCoord = iav2Track(trackInfo.pInsCylTopIavCoord).Z();
      trackInfo.zInsCylBotTrackCoord = iav2Track(trackInfo.pInsCylBotIavCoord).Z();
      
      trackInfo.inscribable = topInscribable && botInscribable && (trackInfo.zInsCylTopTrackCoord > trackInfo.zInsCylBotTrackCoord);
      
    } /* end of if(trackInfo.twoContour) */
    
    /// write the tree variables
    m_tv.inscribable = trackInfo.inscribable;
    if(trackInfo.inscribable)
    {
      info() << "an inscribable track found" << endreq;
      m_tv.cylLen = trackInfo.zInsCylTopTrackCoord - trackInfo.zInsCylBotTrackCoord;
      
      m_tv.zInsCylTopTrackCoord = trackInfo.zInsCylTopTrackCoord;
      m_tv.zInsCylBotTrackCoord = trackInfo.zInsCylBotTrackCoord;
      
      m_tv.xInsCylTopIavCoord = trackInfo.pInsCylTopIavCoord.X();
      m_tv.yInsCylTopIavCoord = trackInfo.pInsCylTopIavCoord.Y();
      m_tv.zInsCylTopIavCoord = trackInfo.pInsCylTopIavCoord.Z();
      m_tv.xInsCylBotIavCoord = trackInfo.pInsCylBotIavCoord.X();
      m_tv.yInsCylBotIavCoord = trackInfo.pInsCylBotIavCoord.Y();
      m_tv.zInsCylBotIavCoord = trackInfo.pInsCylBotIavCoord.Z();
      
      m_totCylLen += m_tv.cylLen;
    
      for(int ii = 0; ii < m_tv.nMi; ii++)
      {
        XYZPoint pIavCoord(m_tv.xMi[ii],m_tv.yMi[ii],m_tv.zMi[ii]);
        pIavCoord = pIavCoord - adCenter;
        
        XYZPoint pTrackCoord = iav2Track(pIavCoord);
    
        m_tv.rhoTrack[ii] = sqrt(pTrackCoord.X()*pTrackCoord.X()+pTrackCoord.Y()*pTrackCoord.Y());
        m_tv.phiTrack[ii] = pTrackCoord.Phi();
        bool inCylinder = (m_tv.rhoTrack[ii] < m_trackCylinderRadius);
        bool betweenPlanes = (pTrackCoord.Z() < trackInfo.zInsCylTopTrackCoord && pTrackCoord.Z() > trackInfo.zInsCylBotTrackCoord);
        
        m_tv.inCyl[ii] = (int)(inCylinder && betweenPlanes);
        
        double zCenter = (trackInfo.zInsCylTopTrackCoord+trackInfo.zInsCylBotTrackCoord)/2;
        m_tv.zTrack[ii] = pTrackCoord.Z() - zCenter;
      }
    }
    
    /* find the incoming and outgoing points of the track on IAV */
    /// set initial values for intersection type
    trackInfo.tCylinderIn = -1;
    trackInfo.tCylinderOut = -1;
    trackInfo.tIavIn = -1;
    trackInfo.tIavOut = -1;
    trackInfo.trackIntersectType = track::kNone;
    
    /// solve the equation for line parameter t
    
    /// t^2 coefficient
    double a2 = trackInfo.dirVec.X()*trackInfo.dirVec.X()+trackInfo.dirVec.Y()*trackInfo.dirVec.Y();
    
    /// t^1 coefficient
    double a1 = 2*(trackInfo.pRpc.X()*trackInfo.dirVec.X()+trackInfo.pRpc.Y()*trackInfo.dirVec.Y());
    
    /// t^0 coefficient
    double a0 = trackInfo.pRpc.X()*trackInfo.pRpc.X()+trackInfo.pRpc.Y()*trackInfo.pRpc.Y();
    
    a0 -= RADIUSIAV*RADIUSIAV;
    
    /// discriminant of the t quadratic equation
    double disc = a1*a1-4*a2*a0;
    
    if(disc > 0) {
      trackInfo.tCylinderIn = 1./2/a2*(-a1-sqrt(disc));
      trackInfo.tCylinderOut = 1./2/a2*(-a1+sqrt(disc));
      
      ROOT::Math::XYZPoint pin = trackInfo.pRpc+trackInfo.tCylinderIn*trackInfo.dirVec;
      ROOT::Math::XYZPoint pout = trackInfo.pRpc+trackInfo.tCylinderOut*trackInfo.dirVec;
      
      enum IntersectZone {
        kAbove,
        kIn,
        kBelow
      };
      
      IntersectZone zoneIn, zoneOut;
      
      if(pin.Z() >= RADIUSIAV) zoneIn = kAbove;
      else if(pin.Z() < RADIUSIAV && pin.Z() >= -RADIUSIAV) zoneIn = kIn;
      else zoneIn = kBelow;
      
      if(pout.Z() >= RADIUSIAV) zoneOut = kAbove;
      else if(pout.Z() < RADIUSIAV && pout.Z() >= -RADIUSIAV) zoneOut = kIn;
      else zoneOut = kBelow;
      
      if(zoneIn == kAbove && zoneOut == kIn) {
        trackInfo.pointIntersectTypeIn = point::kTop;
        trackInfo.pointIntersectTypeOut = point::kSide;
        trackInfo.trackIntersectType = track::kTopSide;
        
        trackInfo.tIavIn = -(trackInfo.pRpc.Z()-RADIUSIAV)/trackInfo.dirVec.Z();
        trackInfo.tIavOut = trackInfo.tCylinderOut;
      }
      else if(zoneIn == kAbove && zoneOut == kBelow) {
        trackInfo.pointIntersectTypeIn = point::kTop;
        trackInfo.pointIntersectTypeOut = point::kBot;
        trackInfo.trackIntersectType = track::kTopBot;
        
        trackInfo.tIavIn = -(trackInfo.pRpc.Z()-RADIUSIAV)/trackInfo.dirVec.Z();
        trackInfo.tIavOut = -(trackInfo.pRpc.Z()+RADIUSIAV)/trackInfo.dirVec.Z();
      }
      else if(zoneIn == kIn && zoneOut == kIn) {
        trackInfo.pointIntersectTypeIn = point::kSide;
        trackInfo.pointIntersectTypeOut = point::kSide;
        trackInfo.trackIntersectType = track::kSideSide;
        
        trackInfo.tIavIn = trackInfo.tCylinderIn;
        
        trackInfo.tIavOut = trackInfo.tCylinderOut;
      }
      else if(zoneIn == kIn && zoneOut == kBelow) {
        trackInfo.pointIntersectTypeIn = point::kSide;
        trackInfo.pointIntersectTypeOut = point::kBot;
        trackInfo.trackIntersectType = track::kSideBot;
        
        trackInfo.tIavIn = trackInfo.tCylinderIn;
        trackInfo.tIavOut = -(trackInfo.pRpc.Z()+RADIUSIAV)/trackInfo.dirVec.Z();
      }
      else {
        trackInfo.pointIntersectTypeIn = point::kNone;
        trackInfo.pointIntersectTypeOut = point::kNone;
        trackInfo.trackIntersectType = track::kNone;
      }
      
      if(trackInfo.trackIntersectType != track::kNone)
      {
        ROOT::Math::XYZPoint pIavIn = trackInfo.pRpc+trackInfo.tIavIn*trackInfo.dirVec;
        ROOT::Math::XYZPoint pIavOut = trackInfo.pRpc+trackInfo.tIavOut*trackInfo.dirVec;
        
        trackInfo.trackLength = sqrt((pIavIn-pIavOut).Mag2());
        
        m_tv.xIavIn = pIavIn.X()+adCenter.X();
        m_tv.yIavIn = pIavIn.Y()+adCenter.Y();
        m_tv.zIavIn = pIavIn.Z()+adCenter.Z();
        m_tv.xIavOut = pIavOut.X()+adCenter.X();
        m_tv.yIavOut = pIavOut.Y()+adCenter.Y();
        m_tv.zIavOut = pIavOut.Z()+adCenter.Z();
      }
    }
    
  } /* end of if(nHitsRpc == 1) */
  
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


double MuonLookBack::getIavTrackLength(MyDataModel::AdTrigger& adEvt)
{
  double result = -3.;
  
  const double rIav = 1500;
  const double halfH8Iav = 1500;
  
  Gaudi::XYZPoint adCenter = Gaudi::XYZPoint(m_gblAdCenter[m_tv.detector-1].X(),
                                             m_gblAdCenter[m_tv.detector-1].Y(),
                                             m_gblAdCenter[m_tv.detector-1].Z());

  double a = adEvt.x-m_tv.xRpc;
  double b = adEvt.y-m_tv.yRpc;
  double c = adEvt.z-m_tv.zRpc;
  double l = m_tv.xRpc-adCenter.X();
  double m = m_tv.yRpc-adCenter.Y();
  
  /// the quadratic discriminant
  double D = (a*l+b*m)*(a*l+b*m)-(a*a+b*b)*(l*l+m*m-rIav*rIav);
  
  double tin = -1000, tout = -1000;
  
  /// It's possible the two intersection points are both larger or smaller
  /// than the AD z axis.
  // Since, I didn't require the AD muon to be reconstructed inside the IAV!
  double zin_old = 0, zout_old = 0;
  //if(l*l+m*m <= rIav*rIav) // RPC hit inside the IAV cylinder
    //tin = (adCenter.Z()+halfH8Iav-m_tv.zRpc)/(adEvt.z-m_tv.zRpc);
  //else {                     // RPC hit outside the IAV cylinder
    //if(D < 0) {
      //return -2.;
    //}
    //else {
      //tin = (-(a*l+b*m)-sqrt(D))/(a*a+b*b);
      //zin_old = m_tv.zRpc+tin*c;
      //if(fabs(m_tv.zRpc+tin*c-adCenter.Z()) > halfH8Iav)
        //tin = (adCenter.Z()+halfH8Iav-m_tv.zRpc)/(adEvt.z-m_tv.zRpc);
    //}
  //}
  
  if(D < 0) return -2.;
  
  /// from now on D >= 0
  tin = (-(a*l+b*m)-sqrt(D))/(a*a+b*b);
  zin_old = m_tv.zRpc+tin*c;
  if(fabs(m_tv.zRpc+tin*c-adCenter.Z()) > halfH8Iav)
    tin = (adCenter.Z()+halfH8Iav-m_tv.zRpc)/(adEvt.z-m_tv.zRpc);
  if(l*l+m*m <= rIav*rIav) // RPC hit inside the IAV cylinder
    tin = (adCenter.Z()+halfH8Iav-m_tv.zRpc)/(adEvt.z-m_tv.zRpc);
  
  /// t_out always corredponds to +D
  tout = (-(a*l+b*m)+sqrt(D))/(a*a+b*b);
  zout_old = m_tv.zRpc+tout*c;
  if(fabs(m_tv.zRpc+tout*c-adCenter.Z()) > halfH8Iav)
    tout = tout*(m_tv.zRpc-(adCenter.Z()-halfH8Iav))/(-tout*c);
  
  double upperplane = adCenter.Z()+halfH8Iav;
  double lowerplane = adCenter.Z()-halfH8Iav;
  if(zin_old > upperplane && zout_old > upperplane) return -3;
  if(zin_old < lowerplane && zout_old < lowerplane) return -3;
  
  if(tin > -1000. && tout > -1000.)
    result = (tout-tin)*sqrt(a*a+b*b+c*c);
  
  /// calculate the IAV in and out point by the way...
  m_pInIav.SetX(m_tv.xRpc+tin*a);
  m_pInIav.SetY(m_tv.yRpc+tin*b);
  m_pInIav.SetZ(m_tv.zRpc+tin*c);
  m_pOutIav.SetX(m_tv.xRpc+tout*a);
  m_pOutIav.SetY(m_tv.yRpc+tout*b);
  m_pOutIav.SetZ(m_tv.zRpc+tout*c);
  
  double res2 = sqrt((m_pInIav.X()-m_pOutIav.X())*(m_pInIav.X()-m_pOutIav.X())
                    +(m_pInIav.Y()-m_pOutIav.Y())*(m_pInIav.Y()-m_pOutIav.Y())
                    +(m_pInIav.Z()-m_pOutIav.Z())*(m_pInIav.Z()-m_pOutIav.Z()));
  
  /// debug information
  if(fabs(result-res2) > 10.) {
    info() << "IAV track lengths by different ways differs by ";
    info() << fabs(result-res2) << " mm" << endreq;
  }

  /// debug information
  if(res2 > 4242) {
    info() << a << " " << b << " " << c << " " << l << " " << m << endreq;
    info() << "discriminant " << D << endreq;
    info() << "t_in " << tin << " t_out " << tout << endreq;
    info() << "AD center: " << adCenter.X() << " ";
    info() << adCenter.Y() << " ";
    info() << adCenter.Z() << endreq;
    info() << "zin old " << zin_old << " zout old " << zout_old << endreq;
    info() << "IAV in coord: " << m_pInIav.X() << " ";
    info() << m_pInIav.Y() << " ";
    info() << m_pInIav.Z() << endreq;
    info() << "IAV out coord: " << m_pOutIav.X() << " ";
    info() << m_pOutIav.Y() << " ";
    info() << m_pOutIav.Z() << endreq;
  }
  
  return result;
}


double MuonLookBack::getCylLen()
{
  /// if RPC is not triggered, doesn't apply
  if(!m_tv.hitRpc) return -1;
  /// if IAV is not hit, doesn't apply
  if(m_tv.dlIav < 0) return -1;
  
  const double r0 = 500;
  
  Gaudi::XYZPoint adCenter = Gaudi::XYZPoint(m_gblAdCenter[m_tv.detector-1].X(),
                                             m_gblAdCenter[m_tv.detector-1].Y(),
                                             m_gblAdCenter[m_tv.detector-1].Z());

  double dx = m_pInIav.X() - adCenter.X();
  double dy = m_pInIav.Y() - adCenter.Y();
  /// if in or out IAV point is outside of the cylinder, doesn't apply
  if(dx*dx+dy*dy > r0*r0) return -1;
  
  dx = m_pOutIav.X() - adCenter.X();
  dy = m_pOutIav.Y() - adCenter.Y();
  if(dx*dx+dy*dy > r0*r0) return -1;
  
  //info() << "cylinder candidate found" << endreq;
  double res = m_tv.dlIav - 1000*tan(m_tv.muTheta)*2;
  
  if(res > 0) return res;
  
  return -1;
}


int MuonLookBack::isInCyl(const MyDataModel::RpcTrigger& rpcEvt, const MyDataModel::AdTrigger& adMu, const MyDataModel::AdTrigger& adEvt)
{
  /// if RPC is not triggered, doesn't apply
  if(!m_tv.hitRpc) return 0;
  /// if IAV is not hit, doesn't apply
  if(m_tv.dlIav < 0) return 0;
  
  const double r0 = 500;
  
  Gaudi::XYZPoint adCenter = Gaudi::XYZPoint(m_gblAdCenter[m_tv.detector-1].X(),
                                             m_gblAdCenter[m_tv.detector-1].Y(),
                                             m_gblAdCenter[m_tv.detector-1].Z());

  double dx = m_pInIav.X() - adCenter.X();
  double dy = m_pInIav.Y() - adCenter.Y();
  /// if in or out IAV point is outside of the cylinder, doesn't apply
  if(dx*dx+dy*dy > r0*r0) return 0;
  
  dx = m_pOutIav.X() - adCenter.X();
  dy = m_pOutIav.Y() - adCenter.Y();
  if(dx*dx+dy*dy > r0*r0) return 0;
  
  /// make sure this is a good neutron
  double transNeutron = neutronDistance2MuonTrack(rpcEvt, adMu, adEvt);
  if(transNeutron < 0 || transNeutron > 1000) return 0;
  double tanNeutron = neutronDistanceAlongMuonTrack(rpcEvt, adMu, adEvt);
  if(tanNeutron < 0) return 0;
  
  double tanDistInIav = tanNeutron-(2000-1500)/cos(m_tv.muTheta);
  if(tanDistInIav > 1000*tan(m_tv.muTheta) && tanDistInIav < m_tv.dlIav - 1000*tan(m_tv.muTheta))
    return 1;
  
  return 0;
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
  if(lnAd == 0. || lRpcAd == 0.) return -2;
  double cosv1v2 = vnAd.Dot(vRpcAd)/lnAd/lRpcAd;
  return -lnAd*cosv1v2;
}
