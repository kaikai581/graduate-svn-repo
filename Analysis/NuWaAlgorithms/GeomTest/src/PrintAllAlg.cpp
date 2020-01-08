#include <fstream>
#include <sstream>
#include "CLHEP/Vector/LorentzVector.h"
#include "Conventions/DetectorId.h"
#include "Conventions/SimFlag.h"
#include "DataSvc/ICableSvc.h"
#include "DetDesc/IDetectorElement.h"
#include "DetDesc/IGeometryInfo.h"
#include "DetHelpers/IPmtGeomInfoSvc.h"
#include "DetHelpers/IRpcGeomInfoSvc.h"
#include "PrintAllAlg.hpp"

using namespace DayaBay;
using namespace DetectorId;
using namespace ROOT::Math;
using namespace SimFlag;
using namespace Site;
using namespace std;

PrintAllAlg::PrintAllAlg(const string& name, ISvcLocator* svcloc) :
GaudiAlgorithm(name, svcloc)
{
}


StatusCode PrintAllAlg::execute()
{
  debug() << "execute()" << endreq;
  
  return StatusCode::SUCCESS;
}


StatusCode PrintAllAlg::finalize()
{
  debug() << "finalize()" << endreq;
  
  return StatusCode::SUCCESS;
}


StatusCode PrintAllAlg::initialize()
{
  debug() << "initialize()" << endreq;
  
  /// Get PmtGeomInfo Service
  m_pmtGeomSvc = svc<IPmtGeomInfoSvc>("PmtGeomInfoSvc", true);
  if(!m_pmtGeomSvc) {
    error() << "Can't initialize PMT geometry service." << endreq;
    return StatusCode::FAILURE;
  }
  /// Get RpcGeomInfo Service
  m_rpcGeomSvc = svc<IRpcGeomInfoSvc>("RpcGeomInfoSvc", true);
  if(!m_rpcGeomSvc) {
    error() << "Can't initialize Rpc geometry service." << endreq;
    return StatusCode::FAILURE;
  }
  /// get cable serivce
  /// Do not forget to specify --dbconf=offline_db in conjunction with
  /// CableSvc service.
  m_cableSvc = svc<ICableSvc>("CableSvc", true);
  
  Site_t halls[] = {kDayaBay, kLingAo, kFar};
  map<Site_t, vector<DetectorId_t> > siteDet;
  for(int i = 0; i < 3; i++)
  {
    siteDet[halls[i]].push_back(kAD1);
    siteDet[halls[i]].push_back(kAD2);
    if(halls[i] == kFar)
    {
      siteDet[halls[i]].push_back(kAD3);
      siteDet[halls[i]].push_back(kAD4);
    }
    siteDet[halls[i]].push_back(kIWS);
    siteDet[halls[i]].push_back(kOWS);
    siteDet[halls[i]].push_back(kRPC);
  }
  
  map<Site_t, vector<DetectorId_t> >::iterator itsite = siteDet.begin();
  /// Record the parent detector of each sensor.
  map<Detector, string> detPaths;
  for(; itsite != siteDet.end(); itsite++)
  {
    Site_t cursite = itsite->first;
    info() << "processing site " << (int)cursite << endreq;
    vector<DetectorId_t> dets = itsite->second;
    
    /// For the first time entering the detector loop,
    /// overwrite the output file.
    /// Otherwise append the output file.
    bool firstAdSite = true, firstWsSite = true;
    bool firstTimeSite = true;
    
    for(unsigned int i = 0; i < dets.size(); i++)
    {
      Detector curdet(cursite, dets[i]);
      
      Context context(cursite, kData, TimeStamp(2012,1,1,0,0,0), dets[i]);
      ServiceMode svcMode(context, 0);
      
      if(curdet.isAD())
      {
        vector<AdPmtSensor> pmtSensors = m_cableSvc->adPmtSensors(svcMode);
        info() << "number of AD PMTs: " << pmtSensors.size() << endreq;
        
        /// get the mother detector
        detPaths[curdet] = m_pmtGeomSvc->get(pmtSensors[0].fullPackedData())->parentDetector().name();
        XYZPoint detO = m_pmtGeomSvc->get(pmtSensors[0].fullPackedData())->parentDetector().geometry()->toGlobal(XYZPoint(0,0,0));
        XYZVector dirX = m_pmtGeomSvc->get(pmtSensors[0].fullPackedData())->parentDetector().geometry()->toGlobal(XYZVector(1,0,0));
        stringstream ofnAdOrigin;
        ofnAdOrigin << "DetectorOrigins" << (int)cursite;
        ofstream ofAdOrigin;
        if(firstTimeSite)
        {
          firstTimeSite = false;
          ofAdOrigin.open(ofnAdOrigin.str().c_str());
        }
        else
          ofAdOrigin.open(ofnAdOrigin.str().c_str(), fstream::app);
        ofAdOrigin << (int)cursite << " ";
        ofAdOrigin << (int)dets[i] << " ";
        ofAdOrigin << detO.X() << " " << detO.Y() << " " << detO.Z() << " ";
        ofAdOrigin << dirX.X() << " " << dirX.Y() << " " << dirX.Z() << endl;
        ofAdOrigin.close();
        
        /// output PMT data
        stringstream ofnAdPmt;
        ofnAdPmt << "AdPmt" << (int)cursite;
        ofstream ofAdPmt;
        if(firstAdSite)
        {
          firstAdSite = false;
          ofAdPmt.open(ofnAdPmt.str().c_str());
        }
        else
          ofAdPmt.open(ofnAdPmt.str().c_str(), fstream::app);
        
        for(unsigned int j = 0; j < pmtSensors.size(); j++)
        {
          IPmtGeomInfo* geoinfo = m_pmtGeomSvc->get(pmtSensors[j].fullPackedData());
          if(!geoinfo) continue;
          ofAdPmt << (int)cursite << " ";
          ofAdPmt << (int)dets[i] << " ";
          ofAdPmt << pmtSensors[j].ring() << " " << pmtSensors[j].column() << " ";
          ofAdPmt << geoinfo->globalPosition()[0] << " ";
          ofAdPmt << geoinfo->globalPosition()[1] << " ";
          ofAdPmt << geoinfo->globalPosition()[2] << " ";
          ofAdPmt << geoinfo->globalDirection()[0] << " ";
          ofAdPmt << geoinfo->globalDirection()[1] << " ";
          ofAdPmt << geoinfo->globalDirection()[2] << endl;
        }
        ofAdPmt.close();
      }
      
      if(curdet.isWaterShield())
      {
        vector<PoolPmtSensor> pmtSensors = m_cableSvc->poolPmtSensors(svcMode);
        info() << "number of Pool PMTs: " << pmtSensors.size() << endreq;
        
        /// get the mother detector
        detPaths[curdet] = m_pmtGeomSvc->get(pmtSensors[0].fullPackedData())->parentDetector().name();
        XYZPoint detO = m_pmtGeomSvc->get(pmtSensors[0].fullPackedData())->parentDetector().geometry()->toGlobal(XYZPoint(0,0,0));
        XYZVector dirX = m_pmtGeomSvc->get(pmtSensors[0].fullPackedData())->parentDetector().geometry()->toGlobal(XYZVector(1,0,0));
        stringstream ofnWsOrigin;
        ofnWsOrigin << "DetectorOrigins" << (int)cursite;
        ofstream ofWsOrigin;
        if(firstTimeSite)
        {
          firstTimeSite = false;
          ofWsOrigin.open(ofnWsOrigin.str().c_str());
        }
        else
          ofWsOrigin.open(ofnWsOrigin.str().c_str(), fstream::app);
        ofWsOrigin << (int)cursite << " ";
        ofWsOrigin << (int)dets[i] << " ";
        ofWsOrigin << detO.X() << " " << detO.Y() << " " << detO.Z() << " ";
        ofWsOrigin << dirX.X() << " " << dirX.Y() << " " << dirX.Z() << endl;
        ofWsOrigin.close();
        
        /// output PMT data
        stringstream ofnWsPmt;
        ofnWsPmt << "WsPmt" << (int)cursite;
        ofstream ofWsPmt;
        if(firstWsSite)
        {
          firstWsSite = false;
          ofWsPmt.open(ofnWsPmt.str().c_str());
        }
        else
          ofWsPmt.open(ofnWsPmt.str().c_str(), fstream::app);
        
        for(unsigned int j = 0; j < pmtSensors.size(); j++)
        {
          IPmtGeomInfo* geoinfo = m_pmtGeomSvc->get(pmtSensors[j].fullPackedData());
          ofWsPmt << (int)cursite << " ";
          ofWsPmt << (int)dets[i] << " ";
          ofWsPmt << pmtSensors[j].wallNumber() << " " << pmtSensors[j].wallSpot() << " " << pmtSensors[j].inwardFacing() << " ";
          ofWsPmt << geoinfo->globalPosition()[0] << " ";
          ofWsPmt << geoinfo->globalPosition()[1] << " ";
          ofWsPmt << geoinfo->globalPosition()[2] << " ";
          ofWsPmt << geoinfo->globalDirection()[0] << " ";
          ofWsPmt << geoinfo->globalDirection()[1] << " ";
          ofWsPmt << geoinfo->globalDirection()[2] << endl;
        }
        ofWsPmt.close();
      }
      
      if(curdet.isRPC())
      {
        vector<RpcSensor> rpcSensors = m_cableSvc->rpcSensors(svcMode);
        info() << "number of RPC strips: " << rpcSensors.size() << endreq;
        //CLHEP::Hep3Vector stripGlo = m_rpcGeomSvc->get(rpcSensors[0].fullPackedData())->globalPosition();
        //Gaudi::XYZPoint stripOGlo = m_rpcGeomSvc->get(rpcSensors[0].fullPackedData())->parentDetector().geometry()->toGlobal(Gaudi::XYZPoint(0,0,0));
        //info() << stripGlo[0]-stripOGlo.X() << " " << stripGlo[1]-stripOGlo.Y() << " " << stripGlo[2]-stripOGlo.Z() << endreq;
        /// get the mother detector
        detPaths[curdet] = m_rpcGeomSvc->get(rpcSensors[0].fullPackedData())->parentDetector().name();
        XYZPoint detO = m_rpcGeomSvc->get(rpcSensors[0].fullPackedData())->parentDetector().geometry()->toGlobal(XYZPoint(0,0,0));
        XYZVector dirX = m_rpcGeomSvc->get(rpcSensors[0].fullPackedData())->parentDetector().geometry()->toGlobal(XYZVector(1,0,0));
        stringstream ofnRpcOrigin;
        ofnRpcOrigin << "DetectorOrigins" << (int)cursite;
        ofstream ofRpcOrigin;
        if(firstTimeSite)
        {
          firstTimeSite = false;
          ofRpcOrigin.open(ofnRpcOrigin.str().c_str());
        }
        else
          ofRpcOrigin.open(ofnRpcOrigin.str().c_str(), fstream::app);
        ofRpcOrigin << (int)cursite << " ";
        ofRpcOrigin << (int)dets[i] << " ";
        ofRpcOrigin << detO.X() << " " << detO.Y() << " " << detO.Z() << " ";
        ofRpcOrigin << dirX.X() << " " << dirX.Y() << " " << dirX.Z() << endl;
        ofRpcOrigin.close();
        
        /// output strip data
        stringstream ofnRpcStrip;
        ofnRpcStrip << "RpcStrip" << (int)cursite;
        ofstream ofRpcStrip;
        ofRpcStrip.open(ofnRpcStrip.str().c_str());
        
        for(unsigned int j = 0; j < rpcSensors.size(); j++)
        {
          IRpcGeomInfo* geoinfo = m_rpcGeomSvc->get(rpcSensors[j].fullPackedData());
          ofRpcStrip << (int)cursite << " ";
          ofRpcStrip << (int)dets[i] << " ";
          ofRpcStrip << rpcSensors[j].panelRow() << " " << rpcSensors[j].panelColumn() << " " << rpcSensors[j].layer() << " " << rpcSensors[j].strip() << " ";
          ofRpcStrip << geoinfo->globalPosition()[0] << " ";
          ofRpcStrip << geoinfo->globalPosition()[1] << " ";
          ofRpcStrip << geoinfo->globalPosition()[2] << " ";
          ofRpcStrip << geoinfo->globalDirection()[0] << " ";
          ofRpcStrip << geoinfo->globalDirection()[1] << " ";
          ofRpcStrip << geoinfo->globalDirection()[2] << endl;
        }
        ofRpcStrip.close();
      }
    }
  }
  
  map<Detector, string>::iterator itDetStr = detPaths.begin();
  for(; itDetStr != detPaths.end(); itDetStr++)
    info() << itDetStr->second << endreq;
  
  return StatusCode::SUCCESS;
}
