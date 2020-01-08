#include <fstream>
#include <sstream>
#include "CLHEP/Vector/LorentzVector.h"
#include "Context/Context.h"
#include "Context/ServiceMode.h"
#include "Conventions/DetectorId.h"
#include "Conventions/Site.h"
#include "Conventions/SimFlag.h"
#include "DataSvc/ICableSvc.h"
#include "DetDesc/DetectorElement.h"
#include "DetDesc/IDetectorElement.h"
#include "DetDesc/IGeometryInfo.h"
#include "DetHelpers/IPmtGeomInfoSvc.h"
#include "PrintAdPmtInLocalAlg.hpp"


using namespace CLHEP;
using namespace DayaBay;
using namespace DetectorId;
using namespace ROOT::Math;
using namespace SimFlag;
using namespace Site;
using namespace std;


PrintAdPmtInLocalAlg::PrintAdPmtInLocalAlg(const string& name, ISvcLocator* svcloc) :
GaudiAlgorithm(name, svcloc)
{
}


StatusCode PrintAdPmtInLocalAlg::execute()
{
  debug() << "execute()" << endreq;
  
  return StatusCode::SUCCESS;
}


StatusCode PrintAdPmtInLocalAlg::finalize()
{
  debug() << "finalize()" << endreq;
  
  return StatusCode::SUCCESS;
}


StatusCode PrintAdPmtInLocalAlg::initialize()
{
  debug() << "initialize()" << endreq;
  
  /// Get PmtGeomInfo Service
  m_pmtGeomSvc = svc<IPmtGeomInfoSvc>("PmtGeomInfoSvc", true);
  if(!m_pmtGeomSvc) {
    error() << "Can't initialize PMT geometry service." << endreq;
    return StatusCode::FAILURE;
  }
  /// get cable serivce
  /// Do not forget to specify --dbconf=offline_db in conjunction with
  /// CableSvc service.
  m_cableSvc = svc<ICableSvc>("CableSvc", true);
  
  Context context(kDayaBay, kData, TimeStamp(2012,1,1,0,0,0), kAD1);
  ServiceMode svcMode(context, 0);
  
  vector<AdPmtSensor> pmtSensors = m_cableSvc->adPmtSensors(svcMode);
  info() << "number of PMTs: " << pmtSensors.size() << endreq;
  
  //const Hep3Vector& pmtPos = m_pmtGeomSvc->get(pmtSensors[6].fullPackedData())->globalPosition();
  //info() << pmtPos.x() << " " << pmtPos[1] << " " << pmtPos[2] << endreq;
  ofstream ofpmtlocal("PmtLocalCoordinate.txt");
  for(unsigned int ii = 0; ii < pmtSensors.size(); ii++)
  {
    const Hep3Vector& pmtPosl = m_pmtGeomSvc->get(pmtSensors[ii].fullPackedData())->localPosition();
    info() << "(" << pmtSensors[ii].ring() << "," << pmtSensors[ii].column() << ")" << endreq;
    ofpmtlocal << pmtSensors[ii].ring() << " " << pmtSensors[ii].column() << " ";
    info() << pmtPosl.x() << " " << pmtPosl.y() << " " << pmtPosl.z() << endreq;
    ofpmtlocal << pmtPosl.x() << " " << pmtPosl.y() << " " << pmtPosl.z() << endl;
  }
  ofpmtlocal.close();
  
  /*DetectorElement* de = getDet<DetectorElement>("/dd/Structure/DayaBay/db-rock/db-ows/db-curtain/db-iws/db-ade1/db-sst1/db-oil1");
  Gaudi::XYZPoint oilOrigin = de->geometry()->toGlobal(Gaudi::XYZPoint(0,0,0));
  info() << oilOrigin.X() << " " << oilOrigin.Y() << " " << oilOrigin.Z() << endreq;
  
  Gaudi::XYZPoint pmt1 = de->geometry()->toGlobal(Gaudi::XYZPoint(-2304.61,-303.408,-1750));
  info() << pmt1.X() << " " << pmt1.Y() << " " << pmt1.Z() << endreq;
  
  /// print OAV and IAV center
  map<Site_t, map<DetectorId_t, string> > pathIav;
  map<Site_t, map<DetectorId_t, string> > pathOav;
  
  pathIav[kDayaBay][kAD1] = "/dd/Structure/DayaBay/db-rock/db-ows/db-curtain/db-iws/db-ade1/db-sst1/db-oil1/db-oav1/db-lso1/db-iav1";
  pathIav[kDayaBay][kAD2] = "/dd/Structure/DayaBay/db-rock/db-ows/db-curtain/db-iws/db-ade2/db-sst2/db-oil2/db-oav2/db-lso2/db-iav2";
  pathIav[kLingAo][kAD1] = "/dd/Structure/DayaBay/la-rock/la-ows/la-curtain/la-iws/la-ade1/la-sst1/la-oil1/la-oav1/la-lso1/la-iav1";
  pathIav[kLingAo][kAD2] = "/dd/Structure/DayaBay/la-rock/la-ows/la-curtain/la-iws/la-ade2/la-sst2/la-oil2/la-oav2/la-lso2/la-iav2";
  pathIav[kFar][kAD1] = "/dd/Structure/DayaBay/far-rock/far-ows/far-curtain/far-iws/far-ade1/far-sst1/far-oil1/far-oav1/far-lso1/far-iav1";
  pathIav[kFar][kAD2] = "/dd/Structure/DayaBay/far-rock/far-ows/far-curtain/far-iws/far-ade2/far-sst2/far-oil2/far-oav2/far-lso2/far-iav2";
  pathIav[kFar][kAD3] = "/dd/Structure/DayaBay/far-rock/far-ows/far-curtain/far-iws/far-ade3/far-sst3/far-oil3/far-oav3/far-lso3/far-iav3";
  pathIav[kFar][kAD4] = "/dd/Structure/DayaBay/far-rock/far-ows/far-curtain/far-iws/far-ade4/far-sst4/far-oil4/far-oav4/far-lso4/far-iav4";
  
  pathOav[kDayaBay][kAD1] = "/dd/Structure/DayaBay/db-rock/db-ows/db-curtain/db-iws/db-ade1/db-sst1/db-oil1/db-oav1";
  pathOav[kDayaBay][kAD2] = "/dd/Structure/DayaBay/db-rock/db-ows/db-curtain/db-iws/db-ade2/db-sst2/db-oil2/db-oav2";
  pathOav[kLingAo][kAD1] = "/dd/Structure/DayaBay/la-rock/la-ows/la-curtain/la-iws/la-ade1/la-sst1/la-oil1/la-oav1";
  pathOav[kLingAo][kAD2] = "/dd/Structure/DayaBay/la-rock/la-ows/la-curtain/la-iws/la-ade2/la-sst2/la-oil2/la-oav2";
  pathOav[kFar][kAD1] = "/dd/Structure/DayaBay/far-rock/far-ows/far-curtain/far-iws/far-ade1/far-sst1/far-oil1/far-oav1";
  pathOav[kFar][kAD2] = "/dd/Structure/DayaBay/far-rock/far-ows/far-curtain/far-iws/far-ade2/far-sst2/far-oil2/far-oav2";
  pathOav[kFar][kAD3] = "/dd/Structure/DayaBay/far-rock/far-ows/far-curtain/far-iws/far-ade3/far-sst3/far-oil3/far-oav3";
  pathOav[kFar][kAD4] = "/dd/Structure/DayaBay/far-rock/far-ows/far-curtain/far-iws/far-ade4/far-sst4/far-oil4/far-oav4";
  
  Site_t sites[] = {kDayaBay, kLingAo, kFar};
  DetectorId_t ads[] = {kAD1, kAD2, kAD3, kAD4};
  
  for(int i = 0; i < 3; i++)
  {
    stringstream fniav, fnoav;
    fniav << "IAV" << (int)sites[i];
    fnoav << "OAV" << (int)sites[i];
    ofstream ofiav(fniav.str().c_str());
    ofstream ofoav(fnoav.str().c_str());
    for(int j = 0; j < 4; j++)
    {
      if(pathIav[sites[i]].find(ads[j]) != pathIav[sites[i]].end())
      {
        de = getDet<DetectorElement>(pathIav[sites[i]][ads[j]].c_str());
        XYZPoint Origin = de->geometry()->toGlobal(XYZPoint(0,0,0));
        info() << "IAV center location: (";
        info() << Origin.X() << "," << Origin.Y() << "," << Origin.Z() << ")" << endreq;
        XYZVector xAxis = de->geometry()->toGlobal(XYZVector(1,0,0));
        info() << "IAV x axis direction: (";
        info() << xAxis.X() << "," << xAxis.Y() << "," << xAxis.Z() << ")" << endreq;
        ofiav << (int)sites[i] << " ";
        ofiav << (int)ads[j] << " ";
        ofiav << Origin.X() << " " << Origin.Y() << " " << Origin.Z() << " ";
        ofiav << xAxis.X() << " " << xAxis.Y() << " " << xAxis.Z() << endl;
        
        de = getDet<DetectorElement>(pathOav[sites[i]][ads[j]].c_str());
        Origin = de->geometry()->toGlobal(XYZPoint(0,0,0));
        info() << "OAV center location: (";
        info() << Origin.X() << "," << Origin.Y() << "," << Origin.Z() << ")" << endreq;
        xAxis = de->geometry()->toGlobal(XYZVector(1,0,0));
        info() << "OAV x axis direction: (";
        info() << xAxis.X() << "," << xAxis.Y() << "," << xAxis.Z() << ")" << endreq;
        ofoav << (int)sites[i] << " ";
        ofoav << (int)ads[j] << " ";
        ofoav << Origin.X() << " " << Origin.Y() << " " << Origin.Z() << " ";
        ofoav << xAxis.X() << " " << xAxis.Y() << " " << xAxis.Z() << endl;
      }
    }
    ofiav.close();
    ofoav.close();
  }*/
  
  return StatusCode::SUCCESS;
}
