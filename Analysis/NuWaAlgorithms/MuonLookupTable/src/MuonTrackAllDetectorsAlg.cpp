#include "DetDesc/DetectorElement.h"
#include "DetDesc/IGeometryInfo.h"
#include "Event/RecHeader.h"
#include "Event/RecRpcHeader.h"
#include "Event/UserDataHeader.h"
#include "MuonTrackAllDetectorsAlg.hpp"
#include "TFile.h"
#include "TLeaf.h"
#include "TMath.h"
#include "TTree.h"

#include "gsl/gsl_blas.h"
#include "gsl/gsl_eigen.h"
#include "gsl/gsl_linalg.h"

using namespace DayaBay;
using namespace DetectorId;
using namespace ROOT::Math;
using namespace Site;
using namespace std;

MuonTrackAllDetectorsAlg::MuonTrackAllDetectorsAlg(const string& name, ISvcLocator* svcloc) :
GaudiAlgorithm(name, svcloc), exeCntr(1)
{
  declareProperty("InFileName", m_infilename = "input.root", "input root file name");
  declareProperty("OutFileName", m_outfilename = "output.root", "output root file name");
  
  /// assign the reference detector strings
  m_referenceDetector[kDayaBay][kAD1] = "/dd/Structure/DayaBay/db-rock/db-ows/db-curtain/db-iws/db-ade1/db-sst1/db-oil1";
  m_referenceDetector[kDayaBay][kAD2] = "/dd/Structure/DayaBay/db-rock/db-ows/db-curtain/db-iws/db-ade2/db-sst2/db-oil2";
  //m_referenceDetector[kDayaBay][kIWS] = "/dd/Structure/DayaBay/db-rock/db-ows/db-curtain/db-iws";
  //m_referenceDetector[kDayaBay][kOWS] = "/dd/Structure/DayaBay/db-rock/db-ows";
  m_referenceDetector[kDayaBay][kIWS] = "/dd/Structure/Pool/db-ows";
  m_referenceDetector[kDayaBay][kOWS] = "/dd/Structure/Pool/db-ows";
  m_referenceDetector[kDayaBay][kRPC] = "/dd/Structure/DayaBay/db-rock/db-rpc";
  
  m_referenceDetector[kLingAo][kAD1] = "/dd/Structure/DayaBay/la-rock/la-ows/la-curtain/la-iws/la-ade1/la-sst1/la-oil1";
  m_referenceDetector[kLingAo][kAD2] = "/dd/Structure/DayaBay/la-rock/la-ows/la-curtain/la-iws/la-ade2/la-sst2/la-oil2";
  //m_referenceDetector[kLingAo][kIWS] = "/dd/Structure/DayaBay/la-rock/la-ows/la-curtain/la-iws";
  //m_referenceDetector[kLingAo][kOWS] = "/dd/Structure/DayaBay/la-rock/la-ows";
  m_referenceDetector[kLingAo][kIWS] = "/dd/Structure/Pool/la-ows";
  m_referenceDetector[kLingAo][kOWS] = "/dd/Structure/Pool/la-ows";
  m_referenceDetector[kLingAo][kRPC] = "/dd/Structure/DayaBay/la-rock/la-rpc";
  
  m_referenceDetector[kFar][kAD1] = "/dd/Structure/DayaBay/far-rock/far-ows/far-curtain/far-iws/far-ade1/far-sst1/far-oil1";
  m_referenceDetector[kFar][kAD2] = "/dd/Structure/DayaBay/far-rock/far-ows/far-curtain/far-iws/far-ade2/far-sst2/far-oil2";
  m_referenceDetector[kFar][kAD3] = "/dd/Structure/DayaBay/far-rock/far-ows/far-curtain/far-iws/far-ade3/far-sst3/far-oil3";
  m_referenceDetector[kFar][kAD4] = "/dd/Structure/DayaBay/far-rock/far-ows/far-curtain/far-iws/far-ade4/far-sst4/far-oil4";
  //m_referenceDetector[kFar][kIWS] = "/dd/Structure/DayaBay/far-rock/far-ows/far-curtain/far-iws";
  //m_referenceDetector[kFar][kOWS] = "/dd/Structure/DayaBay/far-rock/far-ows";
  m_referenceDetector[kFar][kIWS] = "/dd/Structure/Pool/far-ows";
  m_referenceDetector[kFar][kOWS] = "/dd/Structure/Pool/far-ows";
  m_referenceDetector[kFar][kRPC] = "/dd/Structure/DayaBay/far-rock/far-rpc";
  
  /// assign RPC offsets
  m_rpcRecOffset[kDayaBay] = XYZPoint(2500.,-500.,12500.);
  m_rpcRecOffset[kLingAo] = XYZPoint(2500.,-500.,12500.);
  m_rpcRecOffset[kFar] = XYZPoint(5650.,500.,12500.);
  
  /// OAV center location
  m_oavCtr[kDayaBay][kAD1] = XYZPoint(-18079.5, -799699, -7141.5);
  m_oavCtr[kDayaBay][kAD2] = XYZPoint(-14960.5, -804521, -7141.5);
  m_oavCtr[kLingAo][kAD1] = XYZPoint(472323, 60325.2, -3761.5);
  m_oavCtr[kLingAo][kAD2] = XYZPoint(471297, 65974.8, -3761.5);
  m_oavCtr[kFar][kAD1] = XYZPoint(-406758, 812082, -2291.5);
  m_oavCtr[kFar][kAD2] = XYZPoint(-411758, 809258, -2291.5);
  m_oavCtr[kFar][kAD3] = XYZPoint(-409582, 817082, -2291.5);
  m_oavCtr[kFar][kAD4] = XYZPoint(-414582, 814258, -2291.5);
}


singleTrack MuonTrackAllDetectorsAlg::connect2Points(XYZPoint ps, XYZPoint pe)
{
  XYZPoint centroid;
  XYZVector direction;
  
  centroid = (ps+(XYZVector)pe)/2.;
  direction = (pe-ps).Unit();
  
  /// direction vector always points to the sky
  if(direction.Z() < 0) direction *= -1.;
  
  return singleTrack(centroid, direction);
}


StatusCode MuonTrackAllDetectorsAlg::execute()
{
  /// process AD events
  RecHeader* recHeader = get<RecHeader>("/Event/Rec/AdSimple");
  const RecTrigger& recAdTrig = recHeader->recTrigger();
  
  if(recAdTrig.detector().isAD() && !isFlasher() &&
     recAdTrig.energyStatus() == 1 && recAdTrig.positionStatus() == 1)
  {
    DetectorId_t detId = recAdTrig.detector().detectorId();
    int trigNum = recAdTrig.triggerNumber();
    triggerId curTrigId(detId, trigNum);
    
    /// if this event is a muon
    if(m_muonLookupTable.find(curTrigId) != m_muonLookupTable.end())
    {
      debug() << "AD muon found" << endreq;
      string muId = m_muonLookupTable[curTrigId];
      
      if(m_muonDataBuffer.find(muId) == m_muonDataBuffer.end())
        m_muonDataBuffer[muId] = SingleMuonBuffer();
      
      m_muonDataBuffer[muId].site = recAdTrig.detector().site();
      m_muonDataBuffer[muId].eAd[detId] = recAdTrig.energy();
      DetectorElement* de = getDet<DetectorElement>(m_referenceDetector[recAdTrig.detector().site()][detId]);
      XYZPoint gblPos = de->geometry()->toGlobal(XYZPoint(recAdTrig.position().x(),recAdTrig.position().y(),recAdTrig.position().z()));
      m_muonDataBuffer[muId].pDetRec[detId].push_back(gblPos);
      m_muonDataBuffer[muId].hitDet.push_back(detId);
      
      m_muonLookupTable.erase(curTrigId);
      m_muonProgressTable[muId].m_detectorFinished[detId] = true;
    }
  }
  
  /// process WS events
  recHeader = get<RecHeader>("/Event/Rec/PoolSimple");
  const RecTrigger& recPoolTrig = recHeader->recTrigger();
  
  if(recPoolTrig.detector().isWaterShield())
  {
    DetectorId_t detId = recPoolTrig.detector().detectorId();
    int trigNum = recPoolTrig.triggerNumber();
    triggerId curTrigId(detId, trigNum);
    
    /// if this event is a muon
    if(m_muonLookupTable.find(curTrigId) != m_muonLookupTable.end())
    {
      debug() << "WS muon found" << endreq;
      string muId = m_muonLookupTable[curTrigId];
      
      if(m_muonDataBuffer.find(muId) == m_muonDataBuffer.end())
        m_muonDataBuffer[muId] = SingleMuonBuffer();
      
      m_muonDataBuffer[muId].site = recPoolTrig.detector().site();
      DetectorElement* de = getDet<DetectorElement>(m_referenceDetector[recPoolTrig.detector().site()][detId]);
      /// NOTE: the unit used here is in METER! Convert to MILLIMETER before use!
      XYZPoint recPos(recPoolTrig.position().x()*1000.,recPoolTrig.position().y()*1000.,recPoolTrig.position().z()*1000.);
      
      if(wsPositionValid(recPos))
      {
        XYZPoint gblPos = de->geometry()->toGlobal(recPos);
        m_muonDataBuffer[muId].pDetRec[detId].push_back(gblPos);
      }
      m_muonDataBuffer[muId].hitDet.push_back(detId);
      
      m_muonLookupTable.erase(curTrigId);
      m_muonProgressTable[muId].m_detectorFinished[detId] = true;
    }
  }
  
  
  /// process RPC events
  RecRpcHeader* recRpc = get<RecRpcHeader>("/Event/Rec/RpcSimple");
  const RecRpcTrigger recRpcTrigger = recRpc->recTrigger();
  if(recRpcTrigger.detector().isRPC())
  {
    DetectorId_t detId = recRpcTrigger.detector().detectorId();
    int trigNum = recRpcTrigger.triggerNumber();
    triggerId curTrigId(detId, trigNum);
    
    /// if this event is a muon
    if(m_muonLookupTable.find(curTrigId) != m_muonLookupTable.end())
    {
      debug() << "RPC muon found" << endreq;
      string muId = m_muonLookupTable[curTrigId];
      
      if(m_muonDataBuffer.find(muId) == m_muonDataBuffer.end())
        m_muonDataBuffer[muId] = SingleMuonBuffer();
      /// initialize
      m_muonDataBuffer[muId].hitRpcArray = false;
      m_muonDataBuffer[muId].hitRpcTele = false;
      
      m_muonDataBuffer[muId].site = recRpcTrigger.detector().site();
      DetectorElement* de = getDet<DetectorElement>(m_referenceDetector[recRpcTrigger.detector().site()][detId]);
      
      /// different RPC hit conditions
      // first, if a track is formed
      if(recRpcTrigger.numTrack())
      {
        /// start is always telescope and end is always array
        XYZPoint pTele(recRpcTrigger.getTrack(0).start()->position().x(),recRpcTrigger.getTrack(0).start()->position().y(),recRpcTrigger.getTrack(0).start()->position().z());
        XYZPoint pArr(recRpcTrigger.getTrack(0).end()->position().x(),recRpcTrigger.getTrack(0).end()->position().y(),recRpcTrigger.getTrack(0).end()->position().z());
        pTele = pTele - m_rpcRecOffset[recRpcTrigger.detector().site()];
        pArr = pArr - m_rpcRecOffset[recRpcTrigger.detector().site()];
        
        m_muonDataBuffer[muId].pDetRec[detId].push_back(de->geometry()->toGlobal(pArr));
        m_muonDataBuffer[muId].pDetRec[detId].push_back(de->geometry()->toGlobal(pTele));
        m_muonDataBuffer[muId].hitRpcArray = true;
        m_muonDataBuffer[muId].hitRpcTele = true;
      }
      // second, ordinary array hit
      else if(recRpcTrigger.numCluster() == 1)
      {
        XYZPoint pArr(recRpcTrigger.getCluster(0)->position().x(),recRpcTrigger.getCluster(0)->position().y(),recRpcTrigger.getCluster(0)->position().z());
        pArr = pArr - m_rpcRecOffset[recRpcTrigger.detector().site()];
        
        m_muonDataBuffer[muId].pDetRec[detId].push_back(de->geometry()->toGlobal(pArr));
        if(recRpcTrigger.getCluster(0)->type() == RecRpcCluster::kNormalRPC)
          m_muonDataBuffer[muId].hitRpcArray = true;
        if(recRpcTrigger.getCluster(0)->type() == RecRpcCluster::kTelescopeRPC)
          m_muonDataBuffer[muId].hitRpcTele = true;
      }
      
      m_muonDataBuffer[muId].hitDet.push_back(detId);
      
      m_muonLookupTable.erase(curTrigId);
      m_muonProgressTable[muId].m_detectorFinished[detId] = true;
    }
  }
  
  /// if a muon is finished, record it
  map<string, SingleMuonBuffer>::iterator muit = m_muonDataBuffer.begin();
  vector<string> finishedMuId;
  for(; muit != m_muonDataBuffer.end(); muit++)
  {
    string muId = muit->first;
    if(m_muonProgressTable[muId].muonFinished())
    {
      info() << "muon " << muId << " finished" << endreq;
      /// make tracks out of the points
      makeTrack(muId);
      fillOutputTree(muId);
      finishedMuId.push_back(muId);
      m_muonProgressTable.erase(muId);
    }
  }
  /// clear finished muon data buffer
  for(unsigned int i = 0; i < finishedMuId.size(); i++)
    m_muonDataBuffer.erase(finishedMuId[i]);
  
  /// output progress
  if(exeCntr%1000 == 0)
    info() << exeCntr << " events are processed" << endreq;
  exeCntr++;
  
  /// quit earlier if no pending muons
  /// doesn't work... event loop still goes on by returning SUCCESS...
  //if(m_muonProgressTable.size() == 0 || m_muonLookupTable.size() == 0)
    //return StatusCode::SUCCESS;
  
  return StatusCode::SUCCESS;
}


void MuonTrackAllDetectorsAlg::fillOutputTree(string muId)
{
  /* start initializing tree variables */
  m_trvar.hitAd1 = false;
  m_trvar.hitAd2 = false;
  m_trvar.hitAd3 = false;
  m_trvar.hitAd4 = false;
  m_trvar.hitIws = false;
  m_trvar.hitOws = false;
  m_trvar.hitRpc = false;
  m_trvar.hitRpcA = false;
  m_trvar.hitRpcT = false;
  
  m_trvar.nHitsAd1 = 0;
  m_trvar.nHitsAd2 = 0;
  m_trvar.nHitsAd3 = 0;
  m_trvar.nHitsAd4 = 0;
  m_trvar.nHitsIws = 0;
  m_trvar.nHitsOws = 0;
  m_trvar.nHitsRpc = 0;
  m_trvar.nHitsRpcA = 0;
  m_trvar.nHitsRpcT = 0;
  /* end initializing tree variables */
  
  m_trvar.tMu_s = atoi(muId.substr(0,muId.length()-9).c_str());
  m_trvar.tMu_ns = atoi(muId.substr(muId.length()-9,9).c_str());
  
  SingleMuonBuffer curmu = m_muonDataBuffer[muId];
  if(find(curmu.hitDet.begin(), curmu.hitDet.end(), kAD1) != curmu.hitDet.end())
    m_trvar.hitAd1 = true;
  if(find(curmu.hitDet.begin(), curmu.hitDet.end(), kAD2) != curmu.hitDet.end())
    m_trvar.hitAd2 = true;
  if(find(curmu.hitDet.begin(), curmu.hitDet.end(), kAD3) != curmu.hitDet.end())
    m_trvar.hitAd3 = true;
  if(find(curmu.hitDet.begin(), curmu.hitDet.end(), kAD4) != curmu.hitDet.end())
    m_trvar.hitAd4 = true;
  if(find(curmu.hitDet.begin(), curmu.hitDet.end(), kIWS) != curmu.hitDet.end())
    m_trvar.hitIws = true;
  if(find(curmu.hitDet.begin(), curmu.hitDet.end(), kOWS) != curmu.hitDet.end())
    m_trvar.hitOws = true;
  if(find(curmu.hitDet.begin(), curmu.hitDet.end(), kRPC) != curmu.hitDet.end())
    m_trvar.hitRpc = true;
  m_trvar.hitRpcA = curmu.hitRpcArray;
  m_trvar.hitRpcT = curmu.hitRpcTele;
  
  m_trvar.nHitDets = (int)m_trvar.hitAd1 + (int)m_trvar.hitAd2 + (int)m_trvar.hitAd3 + (int)m_trvar.hitAd4 + (int)m_trvar.hitIws + (int)m_trvar.hitOws + (int)m_trvar.hitRpc;
  
  m_trvar.nRecAd1 = curmu.pDetRec[kAD1].size();
  m_trvar.nRecAd2 = curmu.pDetRec[kAD2].size();
  m_trvar.nRecAd3 = curmu.pDetRec[kAD3].size();
  m_trvar.nRecAd4 = curmu.pDetRec[kAD4].size();
  m_trvar.nRecIws = curmu.pDetRec[kIWS].size();
  m_trvar.nRecOws = curmu.pDetRec[kOWS].size();
  m_trvar.nRecRpcA = (int)curmu.hitRpcArray;
  m_trvar.nRecRpcT = (int)curmu.hitRpcTele;
  
  /* start recording reconstructed points */
  for(unsigned int i = 0; i < curmu.pDetRec[kAD1].size(); i++)
  {
    m_trvar.eAd1[i] = curmu.eAd[kAD1];
    m_trvar.xAd1[i] = curmu.pDetRec[kAD1][i].X();
    m_trvar.yAd1[i] = curmu.pDetRec[kAD1][i].Y();
    m_trvar.zAd1[i] = curmu.pDetRec[kAD1][i].Z();
  }
  for(unsigned int i = 0; i < curmu.pDetRec[kAD2].size(); i++)
  {
    m_trvar.eAd2[i] = curmu.eAd[kAD2];
    m_trvar.xAd2[i] = curmu.pDetRec[kAD2][i].X();
    m_trvar.yAd2[i] = curmu.pDetRec[kAD2][i].Y();
    m_trvar.zAd2[i] = curmu.pDetRec[kAD2][i].Z();
  }
  for(unsigned int i = 0; i < curmu.pDetRec[kAD3].size(); i++)
  {
    m_trvar.eAd3[i] = curmu.eAd[kAD3];
    m_trvar.xAd3[i] = curmu.pDetRec[kAD3][i].X();
    m_trvar.yAd3[i] = curmu.pDetRec[kAD3][i].Y();
    m_trvar.zAd3[i] = curmu.pDetRec[kAD3][i].Z();
  }
  for(unsigned int i = 0; i < curmu.pDetRec[kAD4].size(); i++)
  {
    m_trvar.eAd4[i] = curmu.eAd[kAD4];
    m_trvar.xAd4[i] = curmu.pDetRec[kAD4][i].X();
    m_trvar.yAd4[i] = curmu.pDetRec[kAD4][i].Y();
    m_trvar.zAd4[i] = curmu.pDetRec[kAD4][i].Z();
  }
  for(unsigned int i = 0; i < curmu.pDetRec[kIWS].size(); i++)
  {
    m_trvar.xIws[i] = curmu.pDetRec[kIWS][i].X();
    m_trvar.yIws[i] = curmu.pDetRec[kIWS][i].Y();
    m_trvar.zIws[i] = curmu.pDetRec[kIWS][i].Z();
  }
  for(unsigned int i = 0; i < curmu.pDetRec[kOWS].size(); i++)
  {
    m_trvar.xOws[i] = curmu.pDetRec[kOWS][i].X();
    m_trvar.yOws[i] = curmu.pDetRec[kOWS][i].Y();
    m_trvar.zOws[i] = curmu.pDetRec[kOWS][i].Z();
  }
  if(curmu.hitRpcArray && curmu.hitRpcTele)
  {
    m_trvar.xRpcA[0] = curmu.pDetRec[kRPC][0].X();
    m_trvar.yRpcA[0] = curmu.pDetRec[kRPC][0].Y();
    m_trvar.zRpcA[0] = curmu.pDetRec[kRPC][0].Z();
    m_trvar.xRpcT[0] = curmu.pDetRec[kRPC][1].X();
    m_trvar.yRpcT[0] = curmu.pDetRec[kRPC][1].Y();
    m_trvar.zRpcT[0] = curmu.pDetRec[kRPC][1].Z();
  }
  else if(curmu.hitRpcArray && !curmu.hitRpcTele)
  {
    m_trvar.xRpcA[0] = curmu.pDetRec[kRPC][0].X();
    m_trvar.yRpcA[0] = curmu.pDetRec[kRPC][0].Y();
    m_trvar.zRpcA[0] = curmu.pDetRec[kRPC][0].Z();
  }
  else if(!curmu.hitRpcArray && curmu.hitRpcTele)
  {
    m_trvar.xRpcT[0] = curmu.pDetRec[kRPC][0].X();
    m_trvar.yRpcT[0] = curmu.pDetRec[kRPC][0].Y();
    m_trvar.zRpcT[0] = curmu.pDetRec[kRPC][0].Z();
  }
  /* end recording reconstructed points */
  
  m_trvar.nAdRpc = curmu.tAdRpc.size();
  m_trvar.nTelRpc = curmu.tTeleRpc.size();
  m_trvar.nComb = curmu.tComb.size();
  
  for(unsigned int i = 0; i < curmu.tAdRpc.size(); i++)
  {
    XYZPoint centroid = curmu.tAdRpc[i].first;
    m_trvar.xCtAR[i] = centroid.X();
    m_trvar.yCtAR[i] = centroid.Y();
    m_trvar.zCtAR[i] = centroid.Z();
    XYZVector dir = curmu.tAdRpc[i].second;
    m_trvar.thetaAR[i] = dir.Theta();
    m_trvar.phiAR[i] = dir.Phi();
  }
  
  for(unsigned int i = 0; i < curmu.tTeleRpc.size(); i++)
  {
    XYZPoint centroid = curmu.tTeleRpc[i].first;
    m_trvar.xCtTR[i] = centroid.X();
    m_trvar.yCtTR[i] = centroid.Y();
    m_trvar.zCtTR[i] = centroid.Z();
    XYZVector dir = curmu.tTeleRpc[i].second;
    m_trvar.thetaTR[i] = dir.Theta();
    m_trvar.phiTR[i] = dir.Phi();
  }
  
  for(unsigned int i = 0; i < curmu.tComb.size(); i++)
  {
    m_trvar.nPtsCb[i] = curmu.nPtsComb[i];
    XYZPoint centroid = curmu.tComb[i].first;
    m_trvar.xCtCb[i] = centroid.X();
    m_trvar.yCtCb[i] = centroid.Y();
    m_trvar.zCtCb[i] = centroid.Z();
    XYZVector dir = curmu.tComb[i].second;
    m_trvar.thetaCb[i] = dir.Theta();
    m_trvar.phiCb[i] = dir.Phi();
    m_trvar.rmsDistCb[i] = curmu.rmsDistComb[i];
  }
  
  /* store OAV related track properties */
  // AD-RPC track
  m_trvar.passOav1AR = curmu.endsOavAR[kAD1].size();
  for(unsigned int i = 0; i < curmu.endsOavAR[kAD1].size(); i++)
  {
    m_trvar.xOav1InAR[i] = curmu.endsOavAR[kAD1][i].first.X();
    m_trvar.yOav1InAR[i] = curmu.endsOavAR[kAD1][i].first.Y();
    m_trvar.zOav1InAR[i] = curmu.endsOavAR[kAD1][i].first.Z();
    m_trvar.xOav1OutAR[i] = curmu.endsOavAR[kAD1][i].second.X();
    m_trvar.yOav1OutAR[i] = curmu.endsOavAR[kAD1][i].second.Y();
    m_trvar.zOav1OutAR[i] = curmu.endsOavAR[kAD1][i].second.Z();
    m_trvar.dlOav1AR[i] = sqrt((curmu.endsOavAR[kAD1][i].first-curmu.endsOavAR[kAD1][i].second).Mag2());
  }
  m_trvar.passOav2AR = curmu.endsOavAR[kAD2].size();
  for(unsigned int i = 0; i < curmu.endsOavAR[kAD2].size(); i++)
  {
    m_trvar.xOav2InAR[i] = curmu.endsOavAR[kAD2][i].first.X();
    m_trvar.yOav2InAR[i] = curmu.endsOavAR[kAD2][i].first.Y();
    m_trvar.zOav2InAR[i] = curmu.endsOavAR[kAD2][i].first.Z();
    m_trvar.xOav2OutAR[i] = curmu.endsOavAR[kAD2][i].second.X();
    m_trvar.yOav2OutAR[i] = curmu.endsOavAR[kAD2][i].second.Y();
    m_trvar.zOav2OutAR[i] = curmu.endsOavAR[kAD2][i].second.Z();
    m_trvar.dlOav2AR[i] = sqrt((curmu.endsOavAR[kAD2][i].first-curmu.endsOavAR[kAD2][i].second).Mag2());
  }
  m_trvar.passOav3AR = curmu.endsOavAR[kAD3].size();
  for(unsigned int i = 0; i < curmu.endsOavAR[kAD3].size(); i++)
  {
    m_trvar.xOav3InAR[i] = curmu.endsOavAR[kAD3][i].first.X();
    m_trvar.yOav3InAR[i] = curmu.endsOavAR[kAD3][i].first.Y();
    m_trvar.zOav3InAR[i] = curmu.endsOavAR[kAD3][i].first.Z();
    m_trvar.xOav3OutAR[i] = curmu.endsOavAR[kAD3][i].second.X();
    m_trvar.yOav3OutAR[i] = curmu.endsOavAR[kAD3][i].second.Y();
    m_trvar.zOav3OutAR[i] = curmu.endsOavAR[kAD3][i].second.Z();
    m_trvar.dlOav3AR[i] = sqrt((curmu.endsOavAR[kAD3][i].first-curmu.endsOavAR[kAD3][i].second).Mag2());
  }
  m_trvar.passOav4AR = curmu.endsOavAR[kAD4].size();
  for(unsigned int i = 0; i < curmu.endsOavAR[kAD4].size(); i++)
  {
    m_trvar.xOav4InAR[i] = curmu.endsOavAR[kAD4][i].first.X();
    m_trvar.yOav4InAR[i] = curmu.endsOavAR[kAD4][i].first.Y();
    m_trvar.zOav4InAR[i] = curmu.endsOavAR[kAD4][i].first.Z();
    m_trvar.xOav4OutAR[i] = curmu.endsOavAR[kAD4][i].second.X();
    m_trvar.yOav4OutAR[i] = curmu.endsOavAR[kAD4][i].second.Y();
    m_trvar.zOav4OutAR[i] = curmu.endsOavAR[kAD4][i].second.Z();
    m_trvar.dlOav4AR[i] = sqrt((curmu.endsOavAR[kAD4][i].first-curmu.endsOavAR[kAD4][i].second).Mag2());
  }
  // combined track
  m_trvar.passOav1Cb = curmu.endsOavCb[kAD1].size();
  for(unsigned int i = 0; i < curmu.endsOavCb[kAD1].size(); i++)
  {
    m_trvar.xOav1InCb[i] = curmu.endsOavCb[kAD1][i].first.X();
    m_trvar.yOav1InCb[i] = curmu.endsOavCb[kAD1][i].first.Y();
    m_trvar.zOav1InCb[i] = curmu.endsOavCb[kAD1][i].first.Z();
    m_trvar.xOav1OutCb[i] = curmu.endsOavCb[kAD1][i].second.X();
    m_trvar.yOav1OutCb[i] = curmu.endsOavCb[kAD1][i].second.Y();
    m_trvar.zOav1OutCb[i] = curmu.endsOavCb[kAD1][i].second.Z();
    m_trvar.dlOav1Cb[i] = sqrt((curmu.endsOavCb[kAD1][i].first-curmu.endsOavCb[kAD1][i].second).Mag2());
  }
  m_trvar.passOav2Cb = curmu.endsOavCb[kAD2].size();
  for(unsigned int i = 0; i < curmu.endsOavCb[kAD2].size(); i++)
  {
    m_trvar.xOav2InCb[i] = curmu.endsOavCb[kAD2][i].first.X();
    m_trvar.yOav2InCb[i] = curmu.endsOavCb[kAD2][i].first.Y();
    m_trvar.zOav2InCb[i] = curmu.endsOavCb[kAD2][i].first.Z();
    m_trvar.xOav2OutCb[i] = curmu.endsOavCb[kAD2][i].second.X();
    m_trvar.yOav2OutCb[i] = curmu.endsOavCb[kAD2][i].second.Y();
    m_trvar.zOav2OutCb[i] = curmu.endsOavCb[kAD2][i].second.Z();
    m_trvar.dlOav2Cb[i] = sqrt((curmu.endsOavCb[kAD2][i].first-curmu.endsOavCb[kAD2][i].second).Mag2());
  }
  m_trvar.passOav3Cb = curmu.endsOavCb[kAD3].size();
  for(unsigned int i = 0; i < curmu.endsOavCb[kAD3].size(); i++)
  {
    m_trvar.xOav3InCb[i] = curmu.endsOavCb[kAD3][i].first.X();
    m_trvar.yOav3InCb[i] = curmu.endsOavCb[kAD3][i].first.Y();
    m_trvar.zOav3InCb[i] = curmu.endsOavCb[kAD3][i].first.Z();
    m_trvar.xOav3OutCb[i] = curmu.endsOavCb[kAD3][i].second.X();
    m_trvar.yOav3OutCb[i] = curmu.endsOavCb[kAD3][i].second.Y();
    m_trvar.zOav3OutCb[i] = curmu.endsOavCb[kAD3][i].second.Z();
    m_trvar.dlOav3Cb[i] = sqrt((curmu.endsOavCb[kAD3][i].first-curmu.endsOavCb[kAD3][i].second).Mag2());
  }
  m_trvar.passOav4Cb = curmu.endsOavCb[kAD4].size();
  for(unsigned int i = 0; i < curmu.endsOavCb[kAD4].size(); i++)
  {
    m_trvar.xOav4InCb[i] = curmu.endsOavCb[kAD4][i].first.X();
    m_trvar.yOav4InCb[i] = curmu.endsOavCb[kAD4][i].first.Y();
    m_trvar.zOav4InCb[i] = curmu.endsOavCb[kAD4][i].first.Z();
    m_trvar.xOav4OutCb[i] = curmu.endsOavCb[kAD4][i].second.X();
    m_trvar.yOav4OutCb[i] = curmu.endsOavCb[kAD4][i].second.Y();
    m_trvar.zOav4OutCb[i] = curmu.endsOavCb[kAD4][i].second.Z();
    m_trvar.dlOav4Cb[i] = sqrt((curmu.endsOavCb[kAD4][i].first-curmu.endsOavCb[kAD4][i].second).Mag2());
  }
  
  m_trvar.ad1AndCb = curmu.angCTRs[kAD1].size();
  for(unsigned int i = 0; i < curmu.angCTRs[kAD1].size(); i++)
    m_trvar.angCTR1[i] = curmu.angCTRs[kAD1][i];
  m_trvar.ad2AndCb = curmu.angCTRs[kAD2].size();
  for(unsigned int i = 0; i < curmu.angCTRs[kAD2].size(); i++)
    m_trvar.angCTR2[i] = curmu.angCTRs[kAD2][i];
  m_trvar.ad3AndCb = curmu.angCTRs[kAD3].size();
  for(unsigned int i = 0; i < curmu.angCTRs[kAD3].size(); i++)
    m_trvar.angCTR3[i] = curmu.angCTRs[kAD3][i];
  m_trvar.ad4AndCb = curmu.angCTRs[kAD4].size();
  for(unsigned int i = 0; i < curmu.angCTRs[kAD4].size(); i++)
    m_trvar.angCTR4[i] = curmu.angCTRs[kAD4][i];
  
  m_outtree->Fill();
}


StatusCode MuonTrackAllDetectorsAlg::finalize()
{
  m_outfile->Write();
  m_outfile->Close();
  delete m_outfile;
  
  return StatusCode::SUCCESS;
}


void MuonTrackAllDetectorsAlg::fitLeastSquaresLine(string muId)
{
  vector<XYZPoint> pCloud;
  
  for(unsigned int i = 0; i < m_muonDataBuffer[muId].pDetRec[kIWS].size(); i++)
    pCloud.push_back(m_muonDataBuffer[muId].pDetRec[kIWS][i]);
  
  for(unsigned int i = 0; i < m_muonDataBuffer[muId].pDetRec[kOWS].size(); i++)
    pCloud.push_back(m_muonDataBuffer[muId].pDetRec[kOWS][i]);
  
  for(unsigned int i = 0; i < m_muonDataBuffer[muId].pDetRec[kRPC].size(); i++)
    pCloud.push_back(m_muonDataBuffer[muId].pDetRec[kRPC][i]);
  
  /// find the centroid of the cloud of points
  XYZPoint pAvg(0,0,0);
  for(unsigned int i = 0; i < pCloud.size(); i++)
    pAvg = pAvg + (XYZVector)pCloud[i];
  pAvg = pAvg/(double)pCloud.size();
  
  /// form displacement vectors of points from the centroid
  gsl_matrix* mTotRes = gsl_matrix_calloc(3,3);
  gsl_matrix* mOnePtRes = gsl_matrix_alloc(3,3);
  gsl_matrix* vDir = gsl_matrix_alloc(3,1);
  /// constant term independent of the direction
  double sumDispMag2 = 0.;
  /// total matrix including the const amplitudes of the displacement vectors
  gsl_matrix* mTotResWithConst = gsl_matrix_alloc(3,3);
  gsl_matrix_set_identity(mTotResWithConst);
  for(unsigned int i = 0; i < pCloud.size(); i++)
  {
    XYZVector vDisp = pCloud[i] - pAvg;
    sumDispMag2 += vDisp.Dot(vDisp);
    
    gsl_matrix_set(vDir,0,0,vDisp.X());
    gsl_matrix_set(vDir,1,0,vDisp.Y());
    gsl_matrix_set(vDir,2,0,vDisp.Z());

    gsl_blas_dgemm(CblasNoTrans,CblasTrans,1.,vDir,vDir,0.,mOnePtRes);
    gsl_matrix_add(mTotRes,mOnePtRes);
    
    debug() << "point disp:(" << gsl_matrix_get(vDir,0,0) << ",";
    debug() << gsl_matrix_get(vDir,1,0) << ",";
    debug() << gsl_matrix_get(vDir,2,0) << ")" << endreq;
  }
  
  gsl_matrix_scale(mTotResWithConst, sumDispMag2);
  gsl_matrix_sub(mTotResWithConst, mTotRes);
  
  /// start eigenvalues and eigenvectors search
  gsl_vector *eval = gsl_vector_alloc(3);
  gsl_matrix *evec = gsl_matrix_alloc(3,3);
  
  gsl_eigen_symmv_workspace* w = gsl_eigen_symmv_alloc(3);
  gsl_eigen_symmv(mTotResWithConst, eval, evec, w);
  gsl_eigen_symmv_free(w);
  gsl_eigen_symmv_sort(eval, evec, GSL_EIGEN_SORT_ABS_ASC);
  /// end eigenvalues and eigenvectors search
  
  debug() << "resultant matrix:" << endreq;
  for(int i = 0; i < 3; i++)
  {
    for(int j = 0; j < 3; j++)
      debug() << gsl_matrix_get(mTotRes,i,j) << " ";
    debug() << endreq;
  }
  
  double eval_min = gsl_vector_get (eval, 0);
  gsl_vector_view evec_min = gsl_matrix_column (evec, 0);
  
  /// store results
  double rmsDist = sqrt(fabs(eval_min)/pCloud.size());
  XYZVector direction(gsl_matrix_get(evec, 0, 0),gsl_matrix_get(evec, 1, 0),gsl_matrix_get(evec, 2, 0));
  if(direction.Z() < 0) direction *= -1.;
  m_muonDataBuffer[muId].nPtsComb.push_back(pCloud.size());
  m_muonDataBuffer[muId].rmsDistComb.push_back(rmsDist);
  m_muonDataBuffer[muId].tComb.push_back(singleTrack(pAvg, direction));
  
  debug() << "RMS distance: " << rmsDist << endreq;
  debug() << "direction: (" << gsl_matrix_get(evec, 0, 0) << ",";
  debug() << gsl_matrix_get(evec, 1, 0) << ",";
  debug() << gsl_matrix_get(evec, 2, 0) << ")" << endreq;
  
  /// free memory
  gsl_matrix_free(vDir);
  gsl_matrix_free(mOnePtRes);
  gsl_matrix_free(mTotRes);
  gsl_matrix_free(mTotResWithConst);
  gsl_vector_free(eval);
  gsl_matrix_free(evec);
}


vector<double> MuonTrackAllDetectorsAlg::getAngCTR(Site_t site, DetectorId_t det, singleTrack tr, XYZPoint pRec)
{
  vector<double> resAng;
  
  if(site != kFar && (det == kAD3 || det == kAD4)) return resAng;
  
  XYZPoint pTr = tr.first;
  /// translate the coordinate origin to the center of the OAV
  pTr = pTr - m_oavCtr[site][det];
  pRec = pRec - m_oavCtr[site][det];
  XYZVector vTr = tr.second;
  
  /// signed length of pTr along vTr
  double lTrPara = pTr.Dot(vTr);
  XYZVector pPerp = (XYZVector)pTr - lTrPara*vTr;
  XYZVector vPerpRec = (XYZVector)pRec - pPerp;
  vPerpRec = vPerpRec - vPerpRec.Dot(vTr)*vTr;
  
  resAng.push_back(-(vPerpRec.Unit()).Dot(pPerp.Unit()));
  
  return resAng;
}


vector<pair<XYZPoint, XYZPoint> > MuonTrackAllDetectorsAlg::getTrackOavIntersections(Site_t site, DetectorId_t det, singleTrack tr)
{
  vector<pair<XYZPoint, XYZPoint> > pEnds;
  
  if(site != kFar && (det == kAD3 || det == kAD4)) return pEnds;

  XYZPoint pTr = tr.first;
  /// translate the coordinate origin to the center of the OAV
  pTr = pTr - m_oavCtr[site][det];
  XYZVector vTr = tr.second;
  
  XYZVector u1(0,0,1);
  double a21 = u1.Dot(vTr);
  double b1 = pTr.Dot(u1), b2 = pTr.Dot(vTr);
  double t1 = (-b1+a21*b2)/(-1+a21*a21);
  double t2 = (b2-a21*b1)/(-1+a21*a21);
  
  // closest points on the OAV cylinder
  XYZVector cpZ = t1*u1;
  // closest point on the track
  XYZVector cpTr = (XYZVector)pTr + t2*vTr;
  
  double skewDist = sqrt((cpZ-cpTr).Mag2());
  
  if(skewDist > 2000 || cpTr.Z() > 2000 || cpTr.Z() < -2000) return pEnds;
  
  double tup = sqrt(2000*2000-skewDist*skewDist)/sin(vTr.Theta());
  
  XYZVector pup = cpTr + tup*vTr;
  XYZVector pbot = cpTr - tup*vTr;
  
  if(pup.Z() > 2000)
    pup = cpTr + (2000-cpTr.Z())/(pup-cpTr).Z()*tup*vTr;
  if(pbot.Z() < -2000)
    pbot = cpTr - (-2000-cpTr.Z())/(pbot-cpTr).Z()*tup*vTr;
  
  //info() << "p2: (" << pTr.X() << "," << pTr.Y() << "," << pTr.Z() << ")" << endreq;
  //info() << "u2: (" << vTr.X() << "," << vTr.Y() << "," << vTr.Z() << ")" << endreq;
  //info() << "distance: " << skewDist << endreq;
  info() << "pup: (" << pup.X() << "," << pup.Y() << "," << pup.Z() << ")" << endreq;
  info() << "pbot: (" << pbot.X() << "," << pbot.Y() << "," << pbot.Z() << ")" << endreq;
  
  pEnds.push_back(pair<XYZPoint, XYZPoint>(m_oavCtr[site][det]+pup, m_oavCtr[site][det]+pbot));
  return pEnds;
}


StatusCode MuonTrackAllDetectorsAlg::initialize()
{
  m_infile = new TFile(m_infilename.c_str());
  m_treeSpal = (TTree*)m_infile->Get("/Event/Data/Physics/Spallation");
  
  
  for(int ent = 0; ent < m_treeSpal->GetEntries(); ent++)
  //for(int ent = 0; ent < 200; ent++)
  {
    m_treeSpal->GetEntry(ent);
    string muId = makeMuonId((int)m_treeSpal->GetLeaf("tMu_s")->GetValue(), (int)m_treeSpal->GetLeaf("tMu_ns")->GetValue());
    
    int hitAD1 = (int)m_treeSpal->GetLeaf("hitAD1")->GetValue();
    int hitAD2 = (int)m_treeSpal->GetLeaf("hitAD2")->GetValue();
    int hitAD3 = (int)m_treeSpal->GetLeaf("hitAD3")->GetValue();
    int hitAD4 = (int)m_treeSpal->GetLeaf("hitAD4")->GetValue();
    int hitIWS = (int)m_treeSpal->GetLeaf("hitIWS")->GetValue();
    int hitOWS = (int)m_treeSpal->GetLeaf("hitOWS")->GetValue();
    int hitRPC = (int)m_treeSpal->GetLeaf("hitRPC")->GetValue();
    int triggerNumberAD1 = (int)m_treeSpal->GetLeaf("triggerNumber_AD1")->GetValue();
    int triggerNumberAD2 = (int)m_treeSpal->GetLeaf("triggerNumber_AD2")->GetValue();
    int triggerNumberAD3 = (int)m_treeSpal->GetLeaf("triggerNumber_AD3")->GetValue();
    int triggerNumberAD4 = (int)m_treeSpal->GetLeaf("triggerNumber_AD4")->GetValue();
    int triggerNumberIWS = (int)m_treeSpal->GetLeaf("triggerNumber_IWS")->GetValue();
    int triggerNumberOWS = (int)m_treeSpal->GetLeaf("triggerNumber_OWS")->GetValue();
    int triggerNumberRPC = (int)m_treeSpal->GetLeaf("triggerNumber_RPC")->GetValue();
    
    /// store this muon for recording its hit pattern
    m_muonProgressTable[muId] = SingleMuonProgress();
    if(hitAD1 == 1)
    {
      m_muonLookupTable[triggerId(kAD1,triggerNumberAD1)] = muId;
      m_muonProgressTable[muId].m_detectorFinished[kAD1] = false;
    }
    if(hitAD2 == 1)
    {
      m_muonLookupTable[triggerId(kAD2,triggerNumberAD2)] = muId;
      m_muonProgressTable[muId].m_detectorFinished[kAD2] = false;
    }
    if(hitAD3 == 1)
    {
      m_muonLookupTable[triggerId(kAD3,triggerNumberAD3)] = muId;
      m_muonProgressTable[muId].m_detectorFinished[kAD3] = false;
    }
    if(hitAD4 == 1)
    {
      m_muonLookupTable[triggerId(kAD4,triggerNumberAD4)] = muId;
      m_muonProgressTable[muId].m_detectorFinished[kAD4] = false;
    }
    if(hitIWS == 1)
    {
      m_muonLookupTable[triggerId(kIWS,triggerNumberIWS)] = muId;
      m_muonProgressTable[muId].m_detectorFinished[kIWS] = false;
    }
    if(hitOWS == 1)
    {
      m_muonLookupTable[triggerId(kOWS,triggerNumberOWS)] = muId;
      m_muonProgressTable[muId].m_detectorFinished[kOWS] = false;
    }
    if(hitRPC == 1)
    {
      m_muonLookupTable[triggerId(kRPC,triggerNumberRPC)] = muId;
      m_muonProgressTable[muId].m_detectorFinished[kRPC] = false;
    }
    
    if(!((ent+1)%10000))
      info() << ent+1 << " muons booked" << endreq;
  }
  
  m_infile->Close();
  delete m_infile;
  
  /// set up output file and tree
  m_outfile = new TFile(m_outfilename.c_str(), "RECREATE");
  m_outtree = new TTree("mutrack", "muon track reconstruction");
  /// branch assignment
  m_outtree->Branch("tMu_s",&m_trvar.tMu_s,"tMu_s/I");
  m_outtree->Branch("tMu_ns",&m_trvar.tMu_ns,"tMu_ns/I");
  
  m_outtree->Branch("hitAd1",&m_trvar.hitAd1,"hitAd1/O");
  m_outtree->Branch("hitAd2",&m_trvar.hitAd2,"hitAd2/O");
  m_outtree->Branch("hitAd3",&m_trvar.hitAd3,"hitAd3/O");
  m_outtree->Branch("hitAd4",&m_trvar.hitAd4,"hitAd4/O");
  m_outtree->Branch("hitIws",&m_trvar.hitIws,"hitIws/O");
  m_outtree->Branch("hitOws",&m_trvar.hitOws,"hitOws/O");
  m_outtree->Branch("hitRpc",&m_trvar.hitRpc,"hitRpc/O");
  m_outtree->Branch("hitRpcA",&m_trvar.hitRpcA,"hitRpcA/O");
  m_outtree->Branch("hitRpcT",&m_trvar.hitRpcT,"hitRpcT/O");
  
  m_outtree->Branch("nHitDets",&m_trvar.nHitDets,"nHitDets/I");
  
  m_outtree->Branch("nHitsAd1",&m_trvar.nHitsAd1,"nHitsAd1/I");
  m_outtree->Branch("nHitsAd2",&m_trvar.nHitsAd2,"nHitsAd2/I");
  m_outtree->Branch("nHitsAd3",&m_trvar.nHitsAd3,"nHitsAd3/I");
  m_outtree->Branch("nHitsAd4",&m_trvar.nHitsAd4,"nHitsAd4/I");
  m_outtree->Branch("nHitsIws",&m_trvar.nHitsIws,"nHitsIws/I");
  m_outtree->Branch("nHitsOws",&m_trvar.nHitsOws,"nHitsOws/I");
  m_outtree->Branch("nHitsRpc",&m_trvar.nHitsRpc,"nHitsRpc/I");
  m_outtree->Branch("nHitsRpcA",&m_trvar.nHitsRpcA,"nHitsRpcA/I");
  m_outtree->Branch("nHitsRpcT",&m_trvar.nHitsRpcT,"nHitsRpcT/I");
  
  m_outtree->Branch("nRecAd1",&m_trvar.nRecAd1,"nRecAd1/I");
  m_outtree->Branch("nRecAd2",&m_trvar.nRecAd2,"nRecAd2/I");
  m_outtree->Branch("nRecAd3",&m_trvar.nRecAd3,"nRecAd3/I");
  m_outtree->Branch("nRecAd4",&m_trvar.nRecAd4,"nRecAd4/I");
  m_outtree->Branch("nRecIws",&m_trvar.nRecIws,"nRecIws/I");
  m_outtree->Branch("nRecOws",&m_trvar.nRecOws,"nRecOws/I");
  m_outtree->Branch("nRecRpcA",&m_trvar.nRecRpcA,"nRecRpcA/I");
  m_outtree->Branch("nRecRpcT",&m_trvar.nRecRpcT,"nRecRpcT/I");
  
  m_outtree->Branch("eAd1",m_trvar.eAd1,"eAd1[nRecAd1]/D");
  m_outtree->Branch("eAd2",m_trvar.eAd2,"eAd2[nRecAd2]/D");
  m_outtree->Branch("eAd3",m_trvar.eAd3,"eAd3[nRecAd3]/D");
  m_outtree->Branch("eAd4",m_trvar.eAd4,"eAd4[nRecAd4]/D");
  
  m_outtree->Branch("xAd1",m_trvar.xAd1,"xAd1[nRecAd1]/D");
  m_outtree->Branch("yAd1",m_trvar.yAd1,"yAd1[nRecAd1]/D");
  m_outtree->Branch("zAd1",m_trvar.zAd1,"zAd1[nRecAd1]/D");
  m_outtree->Branch("xAd2",m_trvar.xAd2,"xAd2[nRecAd2]/D");
  m_outtree->Branch("yAd2",m_trvar.yAd2,"yAd2[nRecAd2]/D");
  m_outtree->Branch("zAd2",m_trvar.zAd2,"zAd2[nRecAd2]/D");
  m_outtree->Branch("xAd3",m_trvar.xAd3,"xAd3[nRecAd3]/D");
  m_outtree->Branch("yAd3",m_trvar.yAd3,"yAd3[nRecAd3]/D");
  m_outtree->Branch("zAd3",m_trvar.zAd3,"zAd3[nRecAd3]/D");
  m_outtree->Branch("xAd4",m_trvar.xAd4,"xAd4[nRecAd4]/D");
  m_outtree->Branch("yAd4",m_trvar.yAd4,"yAd4[nRecAd4]/D");
  m_outtree->Branch("zAd4",m_trvar.zAd4,"zAd4[nRecAd4]/D");
  m_outtree->Branch("xIws",m_trvar.xIws,"xIws[nRecIws]/D");
  m_outtree->Branch("yIws",m_trvar.yIws,"yIws[nRecIws]/D");
  m_outtree->Branch("zIws",m_trvar.zIws,"zIws[nRecIws]/D");
  m_outtree->Branch("xOws",m_trvar.xOws,"xOws[nRecOws]/D");
  m_outtree->Branch("yOws",m_trvar.yOws,"yOws[nRecOws]/D");
  m_outtree->Branch("zOws",m_trvar.zOws,"zOws[nRecOws]/D");
  m_outtree->Branch("xRpcA",m_trvar.xRpcA,"xRpcA[nRecRpcA]/D");
  m_outtree->Branch("yRpcA",m_trvar.yRpcA,"yRpcA[nRecRpcA]/D");
  m_outtree->Branch("zRpcA",m_trvar.zRpcA,"zRpcA[nRecRpcA]/D");
  m_outtree->Branch("xRpcT",m_trvar.xRpcT,"xRpcT[nRecRpcT]/D");
  m_outtree->Branch("yRpcT",m_trvar.yRpcT,"yRpcT[nRecRpcT]/D");
  m_outtree->Branch("zRpcT",m_trvar.zRpcT,"zRpcT[nRecRpcT]/D");
  
  m_outtree->Branch("nAdRpc",&m_trvar.nAdRpc,"nAdRpc/I");
  m_outtree->Branch("nTelRpc",&m_trvar.nTelRpc,"nTelRpc/I");
  m_outtree->Branch("nComb",&m_trvar.nComb,"nComb/I");
  
  m_outtree->Branch("xCtAR",m_trvar.xCtAR,"xCtAR[nAdRpc]/D");
  m_outtree->Branch("yCtAR",m_trvar.yCtAR,"yCtAR[nAdRpc]/D");
  m_outtree->Branch("zCtAR",m_trvar.zCtAR,"zCtAR[nAdRpc]/D");
  m_outtree->Branch("thetaAR",m_trvar.thetaAR,"thetaAR[nAdRpc]/D");
  m_outtree->Branch("phiAR",m_trvar.phiAR,"phiAR[nAdRpc]/D");
  
  m_outtree->Branch("xCtTR",m_trvar.xCtTR,"xCtTR[nTelRpc]/D");
  m_outtree->Branch("yCtTR",m_trvar.yCtTR,"yCtTR[nTelRpc]/D");
  m_outtree->Branch("zCtTR",m_trvar.zCtTR,"zCtTR[nTelRpc]/D");
  m_outtree->Branch("thetaTR",m_trvar.thetaTR,"thetaTR[nTelRpc]/D");
  m_outtree->Branch("phiTR",m_trvar.phiTR,"phiTR[nTelRpc]/D");
  
  m_outtree->Branch("nPtsCb",m_trvar.nPtsCb,"nPtsCb[nComb]/I");
  m_outtree->Branch("xCtCb",m_trvar.xCtCb,"xCtCb[nComb]/D");
  m_outtree->Branch("yCtCb",m_trvar.yCtCb,"yCtCb[nComb]/D");
  m_outtree->Branch("zCtCb",m_trvar.zCtCb,"zCtCb[nComb]/D");
  m_outtree->Branch("thetaCb",m_trvar.thetaCb,"thetaCb[nComb]/D");
  m_outtree->Branch("phiCb",m_trvar.phiCb,"phiCb[nComb]/D");
  m_outtree->Branch("rmsDistCb",m_trvar.rmsDistCb,"rmsDistCb[nComb]/D");
  
  m_outtree->Branch("passOav1AR",&m_trvar.passOav1AR,"passOav1AR/I");
  m_outtree->Branch("xOav1InAR",m_trvar.xOav1InAR,"xOav1InAR[passOav1AR]/D");
  m_outtree->Branch("yOav1InAR",m_trvar.yOav1InAR,"yOav1InAR[passOav1AR]/D");
  m_outtree->Branch("zOav1InAR",m_trvar.zOav1InAR,"zOav1InAR[passOav1AR]/D");
  m_outtree->Branch("xOav1OutAR",m_trvar.xOav1OutAR,"xOav1OutAR[passOav1AR]/D");
  m_outtree->Branch("yOav1OutAR",m_trvar.yOav1OutAR,"yOav1OutAR[passOav1AR]/D");
  m_outtree->Branch("zOav1OutAR",m_trvar.zOav1OutAR,"zOav1OutAR[passOav1AR]/D");
  m_outtree->Branch("dlOav1AR",m_trvar.dlOav1AR,"dlOav1AR[passOav1AR]/D");
  m_outtree->Branch("passOav2AR",&m_trvar.passOav2AR,"passOav2AR/I");
  m_outtree->Branch("xOav2InAR",m_trvar.xOav2InAR,"xOav2InAR[passOav2AR]/D");
  m_outtree->Branch("yOav2InAR",m_trvar.yOav2InAR,"yOav2InAR[passOav2AR]/D");
  m_outtree->Branch("zOav2InAR",m_trvar.zOav2InAR,"zOav2InAR[passOav2AR]/D");
  m_outtree->Branch("xOav2OutAR",m_trvar.xOav2OutAR,"xOav2OutAR[passOav2AR]/D");
  m_outtree->Branch("yOav2OutAR",m_trvar.yOav2OutAR,"yOav2OutAR[passOav2AR]/D");
  m_outtree->Branch("zOav2OutAR",m_trvar.zOav2OutAR,"zOav2OutAR[passOav2AR]/D");
  m_outtree->Branch("dlOav2AR",m_trvar.dlOav2AR,"dlOav2AR[passOav2AR]/D");
  m_outtree->Branch("passOav3AR",&m_trvar.passOav3AR,"passOav3AR/I");
  m_outtree->Branch("xOav3InAR",m_trvar.xOav3InAR,"xOav3InAR[passOav3AR]/D");
  m_outtree->Branch("yOav3InAR",m_trvar.yOav3InAR,"yOav3InAR[passOav3AR]/D");
  m_outtree->Branch("zOav3InAR",m_trvar.zOav3InAR,"zOav3InAR[passOav3AR]/D");
  m_outtree->Branch("xOav3OutAR",m_trvar.xOav3OutAR,"xOav3OutAR[passOav3AR]/D");
  m_outtree->Branch("yOav3OutAR",m_trvar.yOav3OutAR,"yOav3OutAR[passOav3AR]/D");
  m_outtree->Branch("zOav3OutAR",m_trvar.zOav3OutAR,"zOav3OutAR[passOav3AR]/D");
  m_outtree->Branch("dlOav3AR",m_trvar.dlOav3AR,"dlOav3AR[passOav3AR]/D");
  m_outtree->Branch("passOav4AR",&m_trvar.passOav4AR,"passOav4AR/I");
  m_outtree->Branch("xOav4InAR",m_trvar.xOav4InAR,"xOav4InAR[passOav4AR]/D");
  m_outtree->Branch("yOav4InAR",m_trvar.yOav4InAR,"yOav4InAR[passOav4AR]/D");
  m_outtree->Branch("zOav4InAR",m_trvar.zOav4InAR,"zOav4InAR[passOav4AR]/D");
  m_outtree->Branch("xOav4OutAR",m_trvar.xOav4OutAR,"xOav4OutAR[passOav4AR]/D");
  m_outtree->Branch("yOav4OutAR",m_trvar.yOav4OutAR,"yOav4OutAR[passOav4AR]/D");
  m_outtree->Branch("zOav4OutAR",m_trvar.zOav4OutAR,"zOav4OutAR[passOav4AR]/D");
  m_outtree->Branch("dlOav4AR",m_trvar.dlOav4AR,"dlOav4AR[passOav4AR]/D");
  
  m_outtree->Branch("passOav1Cb",&m_trvar.passOav1Cb,"passOav1Cb/I");
  m_outtree->Branch("xOav1InCb",m_trvar.xOav1InCb,"xOav1InCb[passOav1Cb]/D");
  m_outtree->Branch("yOav1InCb",m_trvar.yOav1InCb,"yOav1InCb[passOav1Cb]/D");
  m_outtree->Branch("zOav1InCb",m_trvar.zOav1InCb,"zOav1InCb[passOav1Cb]/D");
  m_outtree->Branch("xOav1OutCb",m_trvar.xOav1OutCb,"xOav1OutCb[passOav1Cb]/D");
  m_outtree->Branch("yOav1OutCb",m_trvar.yOav1OutCb,"yOav1OutCb[passOav1Cb]/D");
  m_outtree->Branch("zOav1OutCb",m_trvar.zOav1OutCb,"zOav1OutCb[passOav1Cb]/D");
  m_outtree->Branch("dlOav1Cb",m_trvar.dlOav1Cb,"dlOav1Cb[passOav1Cb]/D");
  m_outtree->Branch("passOav2Cb",&m_trvar.passOav2Cb,"passOav2Cb/I");
  m_outtree->Branch("xOav2InCb",m_trvar.xOav2InCb,"xOav2InCb[passOav2Cb]/D");
  m_outtree->Branch("yOav2InCb",m_trvar.yOav2InCb,"yOav2InCb[passOav2Cb]/D");
  m_outtree->Branch("zOav2InCb",m_trvar.zOav2InCb,"zOav2InCb[passOav2Cb]/D");
  m_outtree->Branch("xOav2OutCb",m_trvar.xOav2OutCb,"xOav2OutCb[passOav2Cb]/D");
  m_outtree->Branch("yOav2OutCb",m_trvar.yOav2OutCb,"yOav2OutCb[passOav2Cb]/D");
  m_outtree->Branch("zOav2OutCb",m_trvar.zOav2OutCb,"zOav2OutCb[passOav2Cb]/D");
  m_outtree->Branch("dlOav2Cb",m_trvar.dlOav2Cb,"dlOav2Cb[passOav2Cb]/D");
  m_outtree->Branch("passOav3Cb",&m_trvar.passOav3Cb,"passOav3Cb/I");
  m_outtree->Branch("xOav3InCb",m_trvar.xOav3InCb,"xOav3InCb[passOav3Cb]/D");
  m_outtree->Branch("yOav3InCb",m_trvar.yOav3InCb,"yOav3InCb[passOav3Cb]/D");
  m_outtree->Branch("zOav3InCb",m_trvar.zOav3InCb,"zOav3InCb[passOav3Cb]/D");
  m_outtree->Branch("xOav3OutCb",m_trvar.xOav3OutCb,"xOav3OutCb[passOav3Cb]/D");
  m_outtree->Branch("yOav3OutCb",m_trvar.yOav3OutCb,"yOav3OutCb[passOav3Cb]/D");
  m_outtree->Branch("zOav3OutCb",m_trvar.zOav3OutCb,"zOav3OutCb[passOav3Cb]/D");
  m_outtree->Branch("dlOav3Cb",m_trvar.dlOav3Cb,"dlOav3Cb[passOav3Cb]/D");
  m_outtree->Branch("passOav4Cb",&m_trvar.passOav4Cb,"passOav4Cb/I");
  m_outtree->Branch("xOav4InCb",m_trvar.xOav4InCb,"xOav4InCb[passOav4Cb]/D");
  m_outtree->Branch("yOav4InCb",m_trvar.yOav4InCb,"yOav4InCb[passOav4Cb]/D");
  m_outtree->Branch("zOav4InCb",m_trvar.zOav4InCb,"zOav4InCb[passOav4Cb]/D");
  m_outtree->Branch("xOav4OutCb",m_trvar.xOav4OutCb,"xOav4OutCb[passOav4Cb]/D");
  m_outtree->Branch("yOav4OutCb",m_trvar.yOav4OutCb,"yOav4OutCb[passOav4Cb]/D");
  m_outtree->Branch("zOav4OutCb",m_trvar.zOav4OutCb,"zOav4OutCb[passOav4Cb]/D");
  m_outtree->Branch("dlOav4Cb",m_trvar.dlOav4Cb,"dlOav4Cb[passOav4Cb]/D");
  m_outtree->Branch("ad1AndCb",&m_trvar.ad1AndCb,"ad1AndCb/I");
  m_outtree->Branch("angCTR1",m_trvar.angCTR1,"angCTR1[ad1AndCb]/D");
  m_outtree->Branch("ad2AndCb",&m_trvar.ad2AndCb,"ad2AndCb/I");
  m_outtree->Branch("angCTR2",m_trvar.angCTR2,"angCTR2[ad2AndCb]/D");
  m_outtree->Branch("ad3AndCb",&m_trvar.ad3AndCb,"ad3AndCb/I");
  m_outtree->Branch("angCTR3",m_trvar.angCTR3,"angCTR3[ad3AndCb]/D");
  m_outtree->Branch("ad4AndCb",&m_trvar.ad4AndCb,"ad4AndCb/I");
  m_outtree->Branch("angCTR4",m_trvar.angCTR4,"angCTR4[ad4AndCb]/D");
  
  
  return StatusCode::SUCCESS;
}


bool MuonTrackAllDetectorsAlg::isFlasher()
{
  UserDataHeader* calibStats = get<UserDataHeader>("/Event/Data/CalibStats");
  
  float Quadrant = calibStats->getFloat("Quadrant");
  float MaxQ     = calibStats->getFloat("MaxQ");
  float flasherScale = Quadrant*Quadrant+(MaxQ/.45)*(MaxQ/.45);
  
  if(flasherScale < 1.) return false;
  
  return true;
}


string MuonTrackAllDetectorsAlg::makeMuonId(int s, int ns)
{
  stringstream muonId;
  muonId << s;
  /// pad zeroes in the ns part
  unsigned int ndigits;
  ndigits = ns > 0 ? (unsigned int) log10 ((double) ns) + 1 : 1;
  for(int i = 0; i < 9-(int)ndigits; i++) muonId << 0;
  muonId << ns;
  
  return muonId.str();
}


void MuonTrackAllDetectorsAlg::makeTrack(string muId)
{
  int nPtsAdTot = m_muonDataBuffer[muId].nPoints(kAD1) + m_muonDataBuffer[muId].nPoints(kAD2) + m_muonDataBuffer[muId].nPoints(kAD3) + m_muonDataBuffer[muId].nPoints(kAD4);
  int nPtsIws = m_muonDataBuffer[muId].nPoints(kIWS);
  int nPtsOws = m_muonDataBuffer[muId].nPoints(kOWS);
  int nPtsRpc = m_muonDataBuffer[muId].nPoints(kRPC);
  
  /// tele-RPC track
  if(nPtsRpc == 2)
  {
    XYZPoint pArr = m_muonDataBuffer[muId].pDetRec[kRPC][0];
    XYZPoint pTele = m_muonDataBuffer[muId].pDetRec[kRPC][1];
    
    m_muonDataBuffer[muId].tTeleRpc.push_back(connect2Points(pArr, pTele));
  }
  
  /// AD-RPC track
  if(nPtsAdTot == 1 && nPtsRpc == 1)
  {
    XYZPoint pAd;
    if(m_muonDataBuffer[muId].nPoints(kAD1) == 1)
      pAd = m_muonDataBuffer[muId].pDetRec[kAD1][0];
    if(m_muonDataBuffer[muId].nPoints(kAD2) == 1)
      pAd = m_muonDataBuffer[muId].pDetRec[kAD2][0];
    if(m_muonDataBuffer[muId].nPoints(kAD3) == 1)
      pAd = m_muonDataBuffer[muId].pDetRec[kAD3][0];
    if(m_muonDataBuffer[muId].nPoints(kAD4) == 1)
      pAd = m_muonDataBuffer[muId].pDetRec[kAD4][0];
    XYZPoint pRpc = m_muonDataBuffer[muId].pDetRec[kRPC][0];
    
    m_muonDataBuffer[muId].tAdRpc.push_back(connect2Points(pAd, pRpc));
  }
  if(m_muonDataBuffer[muId].tAdRpc.size())
  {
    Site_t site = m_muonDataBuffer[muId].site;
    m_muonDataBuffer[muId].endsOavAR[kAD1] = getTrackOavIntersections(site, kAD1, m_muonDataBuffer[muId].tAdRpc[0]);
    m_muonDataBuffer[muId].endsOavAR[kAD2] = getTrackOavIntersections(site, kAD2, m_muonDataBuffer[muId].tAdRpc[0]);
    m_muonDataBuffer[muId].endsOavAR[kAD3] = getTrackOavIntersections(site, kAD3, m_muonDataBuffer[muId].tAdRpc[0]);
    m_muonDataBuffer[muId].endsOavAR[kAD4] = getTrackOavIntersections(site, kAD4, m_muonDataBuffer[muId].tAdRpc[0]);
  }

  /// combined track
  if(nPtsIws + nPtsOws + nPtsRpc >= 2) fitLeastSquaresLine(muId);
  if(m_muonDataBuffer[muId].tComb.size())
  {
    Site_t site = m_muonDataBuffer[muId].site;
    m_muonDataBuffer[muId].endsOavCb[kAD1] = getTrackOavIntersections(site, kAD1, m_muonDataBuffer[muId].tComb[0]);
    m_muonDataBuffer[muId].endsOavCb[kAD2] = getTrackOavIntersections(site, kAD2, m_muonDataBuffer[muId].tComb[0]);
    m_muonDataBuffer[muId].endsOavCb[kAD3] = getTrackOavIntersections(site, kAD3, m_muonDataBuffer[muId].tComb[0]);
    m_muonDataBuffer[muId].endsOavCb[kAD4] = getTrackOavIntersections(site, kAD4, m_muonDataBuffer[muId].tComb[0]);
    
    /// if there is AdSimple
    if(m_muonDataBuffer[muId].pDetRec[kAD1].size())
      m_muonDataBuffer[muId].angCTRs[kAD1] = getAngCTR(site, kAD1, m_muonDataBuffer[muId].tComb[0], m_muonDataBuffer[muId].pDetRec[kAD1][0]);
    if(m_muonDataBuffer[muId].pDetRec[kAD2].size())
      m_muonDataBuffer[muId].angCTRs[kAD2] = getAngCTR(site, kAD2, m_muonDataBuffer[muId].tComb[0], m_muonDataBuffer[muId].pDetRec[kAD2][0]);
    if(m_muonDataBuffer[muId].pDetRec[kAD3].size())
      m_muonDataBuffer[muId].angCTRs[kAD3] = getAngCTR(site, kAD3, m_muonDataBuffer[muId].tComb[0], m_muonDataBuffer[muId].pDetRec[kAD3][0]);
    if(m_muonDataBuffer[muId].pDetRec[kAD4].size())
      m_muonDataBuffer[muId].angCTRs[kAD4] = getAngCTR(site, kAD4, m_muonDataBuffer[muId].tComb[0], m_muonDataBuffer[muId].pDetRec[kAD4][0]);
  }
}


/// If PoolSimple fails to reconstruct a position, the default value is
/// assumed, which is (0.,0.,0.) in floating point!
bool MuonTrackAllDetectorsAlg::wsPositionValid(XYZPoint p)
{
  if(sqrt(p.Mag2()) > 1e-10) return true;
  return false;
}
