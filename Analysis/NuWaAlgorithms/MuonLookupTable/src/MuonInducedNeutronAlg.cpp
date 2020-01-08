#include "DetDesc/DetectorElement.h"
#include "DetDesc/IGeometryInfo.h"
#include "Event/RecHeader.h"
#include "Event/RecRpcHeader.h"
#include "Event/UserDataHeader.h"
#include "Math/Polynomial.h"
#include "MuonInducedNeutronAlg.hpp"
#include "TFile.h"
#include "TLeaf.h"
#include "TMath.h"
#include "TTree.h"

#include "gsl/gsl_blas.h"
#include "gsl/gsl_eigen.h"
#include "gsl/gsl_linalg.h"

#define RIAV 1500.

using namespace DayaBay;
using namespace DetectorId;
using namespace ROOT::Math;
using namespace Site;
using namespace std;

MuonInducedNeutronAlg::MuonInducedNeutronAlg(const string& name, ISvcLocator* svcloc) :
GaudiAlgorithm(name, svcloc), exeCntr(1)
{
  declareProperty("InFileName", m_infilename = "input.root", "input root file name");
  declareProperty("OutFileName", m_outfilename = "output.root", "output root file name");
  declareProperty("RTrackCyl", m_rTrackCyl = 1000., "radius of the track cylinder");
  
  /// assign the reference detector strings
  m_referenceDetector[kDayaBay][kAD1] = "/dd/Structure/DayaBay/db-rock/db-ows/db-curtain/db-iws/db-ade1/db-sst1/db-oil1";
  m_referenceDetector[kDayaBay][kAD2] = "/dd/Structure/DayaBay/db-rock/db-ows/db-curtain/db-iws/db-ade2/db-sst2/db-oil2";
  m_referenceDetector[kDayaBay][kIWS] = "/dd/Structure/Pool/db-ows";
  m_referenceDetector[kDayaBay][kOWS] = "/dd/Structure/Pool/db-ows";
  m_referenceDetector[kDayaBay][kRPC] = "/dd/Structure/DayaBay/db-rock/db-rpc";
  
  m_referenceDetector[kLingAo][kAD1] = "/dd/Structure/DayaBay/la-rock/la-ows/la-curtain/la-iws/la-ade1/la-sst1/la-oil1";
  m_referenceDetector[kLingAo][kAD2] = "/dd/Structure/DayaBay/la-rock/la-ows/la-curtain/la-iws/la-ade2/la-sst2/la-oil2";
  m_referenceDetector[kLingAo][kIWS] = "/dd/Structure/Pool/la-ows";
  m_referenceDetector[kLingAo][kOWS] = "/dd/Structure/Pool/la-ows";
  m_referenceDetector[kLingAo][kRPC] = "/dd/Structure/DayaBay/la-rock/la-rpc";
  
  m_referenceDetector[kFar][kAD1] = "/dd/Structure/DayaBay/far-rock/far-ows/far-curtain/far-iws/far-ade1/far-sst1/far-oil1";
  m_referenceDetector[kFar][kAD2] = "/dd/Structure/DayaBay/far-rock/far-ows/far-curtain/far-iws/far-ade2/far-sst2/far-oil2";
  m_referenceDetector[kFar][kAD3] = "/dd/Structure/DayaBay/far-rock/far-ows/far-curtain/far-iws/far-ade3/far-sst3/far-oil3";
  m_referenceDetector[kFar][kAD4] = "/dd/Structure/DayaBay/far-rock/far-ows/far-curtain/far-iws/far-ade4/far-sst4/far-oil4";
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
  
  /// IAV center location
  m_iavCtr[kDayaBay][kAD1] = XYZPoint(-18079.5, -799699, -7107.5);
  m_iavCtr[kDayaBay][kAD2] = XYZPoint(-14960.5, -804521, -7107.5);
  m_iavCtr[kLingAo][kAD1] = XYZPoint(472323, 60325.2, -3727.5);
  m_iavCtr[kLingAo][kAD2] = XYZPoint(471297, 65974.8, -3727.5);
  m_iavCtr[kFar][kAD1] = XYZPoint(-406758, 812082, -2257.5);
  m_iavCtr[kFar][kAD2] = XYZPoint(-411758, 809258, -2257.5);
  m_iavCtr[kFar][kAD3] = XYZPoint(-409582, 817082, -2257.5);
  m_iavCtr[kFar][kAD4] = XYZPoint(-414582, 814258, -2257.5);
  
  /// initialize last AD muon ID
  m_lastMuonId[kAD1] = string("0");
  m_lastMuonId[kAD2] = string("0");
  m_lastMuonId[kAD3] = string("0");
  m_lastMuonId[kAD4] = string("0");
}


void MuonInducedNeutronAlg::assignBranches(TTree* tr, MuonInducedNeutronTreeVars& tv)
{
  tr->Branch("tMu_s",&tv.tMu_s,"tMu_s/I");
  tr->Branch("tMu_ns",&tv.tMu_ns,"tMu_ns/I");
  
  tr->Branch("hitAd1",&tv.hitAd1,"hitAd1/I");
  tr->Branch("tnAd1",tv.tnAd1,"tnAd1[hitAd1]/I");
  tr->Branch("hitAd2",&tv.hitAd2,"hitAd2/I");
  tr->Branch("tnAd2",tv.tnAd2,"tnAd2[hitAd2]/I");
  tr->Branch("hitAd3",&tv.hitAd3,"hitAd3/I");
  tr->Branch("tnAd3",tv.tnAd3,"tnAd3[hitAd3]/I");
  tr->Branch("hitAd4",&tv.hitAd4,"hitAd4/I");
  tr->Branch("tnAd4",tv.tnAd4,"tnAd4[hitAd4]/I");
  tr->Branch("hitIws",&tv.hitIws,"hitIws/I");
  tr->Branch("tnIws",tv.tnIws,"tnIws[hitIws]/I");
  tr->Branch("hitOws",&tv.hitOws,"hitOws/I");
  tr->Branch("tnOws",tv.tnOws,"tnOws[hitOws]/I");
  tr->Branch("hitRpc",&tv.hitRpc,"hitRpc/I");
  tr->Branch("tnRpc",tv.tnRpc,"tnRpc[hitRpc]/I");
  tr->Branch("hitRpcA",&tv.hitRpcA,"hitRpcA/I");
  tr->Branch("hitRpcT",&tv.hitRpcT,"hitRpcT/I");
  
  tr->Branch("nRecAd1",&tv.nRecAd1,"nRecAd1/I");
  tr->Branch("nRecAd2",&tv.nRecAd2,"nRecAd2/I");
  tr->Branch("nRecAd3",&tv.nRecAd3,"nRecAd3/I");
  tr->Branch("nRecAd4",&tv.nRecAd4,"nRecAd4/I");
  tr->Branch("nRecIws",&tv.nRecIws,"nRecIws/I");
  tr->Branch("nRecOws",&tv.nRecOws,"nRecOws/I");
  tr->Branch("nRecRpcA",&tv.nRecRpcA,"nRecRpcA/I");
  tr->Branch("nRecRpcT",&tv.nRecRpcT,"nRecRpcT/I");
  
  tr->Branch("nHitDets",&tv.nHitDets,"nHitDets/I");
  
  tr->Branch("eAd1",tv.eAd1,"eAd1[nRecAd1]/D");
  tr->Branch("eAd2",tv.eAd2,"eAd2[nRecAd2]/D");
  tr->Branch("eAd3",tv.eAd3,"eAd3[nRecAd3]/D");
  tr->Branch("eAd4",tv.eAd4,"eAd4[nRecAd4]/D");
  
  tr->Branch("xAd1",tv.xAd1,"xAd1[nRecAd1]/D");
  tr->Branch("yAd1",tv.yAd1,"yAd1[nRecAd1]/D");
  tr->Branch("zAd1",tv.zAd1,"zAd1[nRecAd1]/D");
  tr->Branch("xAd2",tv.xAd2,"xAd2[nRecAd2]/D");
  tr->Branch("yAd2",tv.yAd2,"yAd2[nRecAd2]/D");
  tr->Branch("zAd2",tv.zAd2,"zAd2[nRecAd2]/D");
  tr->Branch("xAd3",tv.xAd3,"xAd3[nRecAd3]/D");
  tr->Branch("yAd3",tv.yAd3,"yAd3[nRecAd3]/D");
  tr->Branch("zAd3",tv.zAd3,"zAd3[nRecAd3]/D");
  tr->Branch("xAd4",tv.xAd4,"xAd4[nRecAd4]/D");
  tr->Branch("yAd4",tv.yAd4,"yAd4[nRecAd4]/D");
  tr->Branch("zAd4",tv.zAd4,"zAd4[nRecAd4]/D");
  tr->Branch("xIws",tv.xIws,"xIws[nRecIws]/D");
  tr->Branch("yIws",tv.yIws,"yIws[nRecIws]/D");
  tr->Branch("zIws",tv.zIws,"zIws[nRecIws]/D");
  tr->Branch("xOws",tv.xOws,"xOws[nRecOws]/D");
  tr->Branch("yOws",tv.yOws,"yOws[nRecOws]/D");
  tr->Branch("zOws",tv.zOws,"zOws[nRecOws]/D");
  tr->Branch("xRpcA",tv.xRpcA,"xRpcA[nRecRpcA]/D");
  tr->Branch("yRpcA",tv.yRpcA,"yRpcA[nRecRpcA]/D");
  tr->Branch("zRpcA",tv.zRpcA,"zRpcA[nRecRpcA]/D");
  tr->Branch("xRpcT",tv.xRpcT,"xRpcT[nRecRpcT]/D");
  tr->Branch("yRpcT",tv.yRpcT,"yRpcT[nRecRpcT]/D");
  tr->Branch("zRpcT",tv.zRpcT,"zRpcT[nRecRpcT]/D");
  
  tr->Branch("nTracks",&tv.nTracks,"nTracks/I");
  tr->Branch("nTrPts",tv.nTrPts,"nTrPts[nTracks]/I");
  tr->Branch("xCt",tv.xCt,"xCt[nTracks]/D");
  tr->Branch("yCt",tv.yCt,"yCt[nTracks]/D");
  tr->Branch("zCt",tv.zCt,"zCt[nTracks]/D");
  tr->Branch("theta",tv.theta,"theta[nTracks]/D");
  tr->Branch("phi",tv.phi,"phi[nTracks]/D");
  tr->Branch("trIsFit",&tv.trIsFit,"trIsFit/I");
  tr->Branch("rmsDist",tv.rmsDist,"rmsDist[trIsFit]/D");
  
  tr->Branch("passOav1",&tv.passOav1,"passOav1/I");
  tr->Branch("xOav1In",tv.xOav1In,"xOav1In[passOav1]/D");
  tr->Branch("yOav1In",tv.yOav1In,"yOav1In[passOav1]/D");
  tr->Branch("zOav1In",tv.zOav1In,"zOav1In[passOav1]/D");
  tr->Branch("xOav1Out",tv.xOav1Out,"xOav1Out[passOav1]/D");
  tr->Branch("yOav1Out",tv.yOav1Out,"yOav1Out[passOav1]/D");
  tr->Branch("zOav1Out",tv.zOav1Out,"zOav1Out[passOav1]/D");
  tr->Branch("dlOav1",tv.dlOav1,"dlOav1[passOav1]/D");
  tr->Branch("passOav2",&tv.passOav2,"passOav2/I");
  tr->Branch("xOav2In",tv.xOav2In,"xOav2In[passOav2]/D");
  tr->Branch("yOav2In",tv.yOav2In,"yOav2In[passOav2]/D");
  tr->Branch("zOav2In",tv.zOav2In,"zOav2In[passOav2]/D");
  tr->Branch("xOav2Out",tv.xOav2Out,"xOav2Out[passOav2]/D");
  tr->Branch("yOav2Out",tv.yOav2Out,"yOav2Out[passOav2]/D");
  tr->Branch("zOav2Out",tv.zOav2Out,"zOav2Out[passOav2]/D");
  tr->Branch("dlOav2",tv.dlOav2,"dlOav2[passOav2]/D");
  tr->Branch("passOav3",&tv.passOav3,"passOav3/I");
  tr->Branch("xOav3In",tv.xOav3In,"xOav3In[passOav3]/D");
  tr->Branch("yOav3In",tv.yOav3In,"yOav3In[passOav3]/D");
  tr->Branch("zOav3In",tv.zOav3In,"zOav3In[passOav3]/D");
  tr->Branch("xOav3Out",tv.xOav3Out,"xOav3Out[passOav3]/D");
  tr->Branch("yOav3Out",tv.yOav3Out,"yOav3Out[passOav3]/D");
  tr->Branch("zOav3Out",tv.zOav3Out,"zOav3Out[passOav3]/D");
  tr->Branch("dlOav3",tv.dlOav3,"dlOav3[passOav3]/D");
  tr->Branch("passOav4",&tv.passOav4,"passOav4/I");
  tr->Branch("xOav4In",tv.xOav4In,"xOav4In[passOav4]/D");
  tr->Branch("yOav4In",tv.yOav4In,"yOav4In[passOav4]/D");
  tr->Branch("zOav4In",tv.zOav4In,"zOav4In[passOav4]/D");
  tr->Branch("xOav4Out",tv.xOav4Out,"xOav4Out[passOav4]/D");
  tr->Branch("yOav4Out",tv.yOav4Out,"yOav4Out[passOav4]/D");
  tr->Branch("zOav4Out",tv.zOav4Out,"zOav4Out[passOav4]/D");
  tr->Branch("dlOav4",tv.dlOav4,"dlOav4[passOav4]/D");
  
  tr->Branch("passIav1",&tv.passIav1,"passIav1/I");
  tr->Branch("xIav1In",tv.xIav1In,"xIav1In[passIav1]/D");
  tr->Branch("yIav1In",tv.yIav1In,"yIav1In[passIav1]/D");
  tr->Branch("zIav1In",tv.zIav1In,"zIav1In[passIav1]/D");
  tr->Branch("xIav1Out",tv.xIav1Out,"xIav1Out[passIav1]/D");
  tr->Branch("yIav1Out",tv.yIav1Out,"yIav1Out[passIav1]/D");
  tr->Branch("zIav1Out",tv.zIav1Out,"zIav1Out[passIav1]/D");
  tr->Branch("dlIav1",tv.dlIav1,"dlIav1[passIav1]/D");
  tr->Branch("passIav2",&tv.passIav2,"passIav2/I");
  tr->Branch("xIav2In",tv.xIav2In,"xIav2In[passIav2]/D");
  tr->Branch("yIav2In",tv.yIav2In,"yIav2In[passIav2]/D");
  tr->Branch("zIav2In",tv.zIav2In,"zIav2In[passIav2]/D");
  tr->Branch("xIav2Out",tv.xIav2Out,"xIav2Out[passIav2]/D");
  tr->Branch("yIav2Out",tv.yIav2Out,"yIav2Out[passIav2]/D");
  tr->Branch("zIav2Out",tv.zIav2Out,"zIav2Out[passIav2]/D");
  tr->Branch("dlIav2",tv.dlIav2,"dlIav2[passIav2]/D");
  tr->Branch("passIav3",&tv.passIav3,"passIav3/I");
  tr->Branch("xIav3In",tv.xIav3In,"xIav3In[passIav3]/D");
  tr->Branch("yIav3In",tv.yIav3In,"yIav3In[passIav3]/D");
  tr->Branch("zIav3In",tv.zIav3In,"zIav3In[passIav3]/D");
  tr->Branch("xIav3Out",tv.xIav3Out,"xIav3Out[passIav3]/D");
  tr->Branch("yIav3Out",tv.yIav3Out,"yIav3Out[passIav3]/D");
  tr->Branch("zIav3Out",tv.zIav3Out,"zIav3Out[passIav3]/D");
  tr->Branch("dlIav3",tv.dlIav3,"dlIav3[passIav3]/D");
  tr->Branch("passIav4",&tv.passIav4,"passIav4/I");
  tr->Branch("xIav4In",tv.xIav4In,"xIav4In[passIav4]/D");
  tr->Branch("yIav4In",tv.yIav4In,"yIav4In[passIav4]/D");
  tr->Branch("zIav4In",tv.zIav4In,"zIav4In[passIav4]/D");
  tr->Branch("xIav4Out",tv.xIav4Out,"xIav4Out[passIav4]/D");
  tr->Branch("yIav4Out",tv.yIav4Out,"yIav4Out[passIav4]/D");
  tr->Branch("zIav4Out",tv.zIav4Out,"zIav4Out[passIav4]/D");
  tr->Branch("dlIav4",tv.dlIav4,"dlIav4[passIav4]/D");
  
  tr->Branch("inscribable1",&tv.inscribable1,"inscribable1/I");
  tr->Branch("xCyl1Up",tv.xCyl1Up,"xCyl1Up[inscribable1]/D");
  tr->Branch("yCyl1Up",tv.yCyl1Up,"yCyl1Up[inscribable1]/D");
  tr->Branch("zCyl1Up",tv.zCyl1Up,"zCyl1Up[inscribable1]/D");
  tr->Branch("xCyl1Bot",tv.xCyl1Bot,"xCyl1Bot[inscribable1]/D");
  tr->Branch("yCyl1Bot",tv.yCyl1Bot,"yCyl1Bot[inscribable1]/D");
  tr->Branch("zCyl1Bot",tv.zCyl1Bot,"zCyl1Bot[inscribable1]/D");
  tr->Branch("tCyl1Up",tv.tCyl1Up,"tCyl1Up[inscribable1]/D");
  tr->Branch("tCyl1Bot",tv.tCyl1Bot,"tCyl1Bot[inscribable1]/D");
  
  tr->Branch("inscribable2",&tv.inscribable2,"inscribable2/I");
  tr->Branch("xCyl2Up",tv.xCyl2Up,"xCyl2Up[inscribable2]/D");
  tr->Branch("yCyl2Up",tv.yCyl2Up,"yCyl2Up[inscribable2]/D");
  tr->Branch("zCyl2Up",tv.zCyl2Up,"zCyl2Up[inscribable2]/D");
  tr->Branch("xCyl2Bot",tv.xCyl2Bot,"xCyl2Bot[inscribable2]/D");
  tr->Branch("yCyl2Bot",tv.yCyl2Bot,"yCyl2Bot[inscribable2]/D");
  tr->Branch("zCyl2Bot",tv.zCyl2Bot,"zCyl2Bot[inscribable2]/D");
  tr->Branch("tCyl2Up",tv.tCyl2Up,"tCyl2Up[inscribable2]/D");
  tr->Branch("tCyl2Bot",tv.tCyl2Bot,"tCyl2Bot[inscribable2]/D");
  
  tr->Branch("inscribable3",&tv.inscribable3,"inscribable3/I");
  tr->Branch("xCyl3Up",tv.xCyl3Up,"xCyl3Up[inscribable3]/D");
  tr->Branch("yCyl3Up",tv.yCyl3Up,"yCyl3Up[inscribable3]/D");
  tr->Branch("zCyl3Up",tv.zCyl3Up,"zCyl3Up[inscribable3]/D");
  tr->Branch("xCyl3Bot",tv.xCyl3Bot,"xCyl3Bot[inscribable3]/D");
  tr->Branch("yCyl3Bot",tv.yCyl3Bot,"yCyl3Bot[inscribable3]/D");
  tr->Branch("zCyl3Bot",tv.zCyl3Bot,"zCyl3Bot[inscribable3]/D");
  tr->Branch("tCyl3Up",tv.tCyl3Up,"tCyl3Up[inscribable3]/D");
  tr->Branch("tCyl3Bot",tv.tCyl3Bot,"tCyl3Bot[inscribable3]/D");
  
  tr->Branch("inscribable4",&tv.inscribable4,"inscribable4/I");
  tr->Branch("xCyl4Up",tv.xCyl4Up,"xCyl4Up[inscribable4]/D");
  tr->Branch("yCyl4Up",tv.yCyl4Up,"yCyl4Up[inscribable4]/D");
  tr->Branch("zCyl4Up",tv.zCyl4Up,"zCyl4Up[inscribable4]/D");
  tr->Branch("xCyl4Bot",tv.xCyl4Bot,"xCyl4Bot[inscribable4]/D");
  tr->Branch("yCyl4Bot",tv.yCyl4Bot,"yCyl4Bot[inscribable4]/D");
  tr->Branch("zCyl4Bot",tv.zCyl4Bot,"zCyl4Bot[inscribable4]/D");
  tr->Branch("tCyl4Up",tv.tCyl4Up,"tCyl4Up[inscribable4]/D");
  tr->Branch("tCyl4Bot",tv.tCyl4Bot,"tCyl4Bot[inscribable4]/D");
  
  tr->Branch("nIntEvtAd1",&tv.nIntEvtAd1,"nIntEvtAd1/I");
  tr->Branch("tnIntEvtAd1",tv.tnIntEvtAd1,"tnIntEvtAd1[nIntEvtAd1]/I");
  tr->Branch("eIntEvtAd1",tv.eIntEvtAd1,"eIntEvtAd1[nIntEvtAd1]/D");
  tr->Branch("dtIntEvtAd1",tv.dtIntEvtAd1,"dtIntEvtAd1[nIntEvtAd1]/D");
  tr->Branch("xIntEvtAd1",tv.xIntEvtAd1,"xIntEvtAd1[nIntEvtAd1]/D");
  tr->Branch("yIntEvtAd1",tv.yIntEvtAd1,"yIntEvtAd1[nIntEvtAd1]/D");
  tr->Branch("zIntEvtAd1",tv.zIntEvtAd1,"zIntEvtAd1[nIntEvtAd1]/D");
  tr->Branch("inCyl1",tv.inCyl1,"inCyl1[nIntEvtAd1]/O");
  tr->Branch("nDistAd1",&tv.nDistAd1,"nDistAd1/I");
  tr->Branch("dlIntEvt1",tv.dlIntEvt1,"dlIntEvt1[nDistAd1]/D");
  
  tr->Branch("nIntEvtAd2",&tv.nIntEvtAd2,"nIntEvtAd2/I");
  tr->Branch("tnIntEvtAd2",tv.tnIntEvtAd2,"tnIntEvtAd2[nIntEvtAd2]/I");
  tr->Branch("eIntEvtAd2",tv.eIntEvtAd2,"eIntEvtAd2[nIntEvtAd2]/D");
  tr->Branch("dtIntEvtAd2",tv.dtIntEvtAd2,"dtIntEvtAd2[nIntEvtAd2]/D");
  tr->Branch("xIntEvtAd2",tv.xIntEvtAd2,"xIntEvtAd2[nIntEvtAd2]/D");
  tr->Branch("yIntEvtAd2",tv.yIntEvtAd2,"yIntEvtAd2[nIntEvtAd2]/D");
  tr->Branch("zIntEvtAd2",tv.zIntEvtAd2,"zIntEvtAd2[nIntEvtAd2]/D");
  tr->Branch("inCyl2",tv.inCyl2,"inCyl2[nIntEvtAd2]/O");
  tr->Branch("nDistAd2",&tv.nDistAd2,"nDistAd2/I");
  tr->Branch("dlIntEvt2",tv.dlIntEvt2,"dlIntEvt2[nDistAd2]/D");
  
  tr->Branch("nIntEvtAd3",&tv.nIntEvtAd3,"nIntEvtAd3/I");
  tr->Branch("tnIntEvtAd3",tv.tnIntEvtAd3,"tnIntEvtAd3[nIntEvtAd3]/I");
  tr->Branch("eIntEvtAd3",tv.eIntEvtAd3,"eIntEvtAd3[nIntEvtAd3]/D");
  tr->Branch("dtIntEvtAd3",tv.dtIntEvtAd3,"dtIntEvtAd3[nIntEvtAd3]/D");
  tr->Branch("xIntEvtAd3",tv.xIntEvtAd3,"xIntEvtAd3[nIntEvtAd3]/D");
  tr->Branch("yIntEvtAd3",tv.yIntEvtAd3,"yIntEvtAd3[nIntEvtAd3]/D");
  tr->Branch("zIntEvtAd3",tv.zIntEvtAd3,"zIntEvtAd3[nIntEvtAd3]/D");
  tr->Branch("inCyl3",tv.inCyl3,"inCyl3[nIntEvtAd3]/O");
  tr->Branch("nDistAd3",&tv.nDistAd3,"nDistAd3/I");
  tr->Branch("dlIntEvt3",tv.dlIntEvt3,"dlIntEvt3[nDistAd3]/D");
  
  tr->Branch("nIntEvtAd4",&tv.nIntEvtAd4,"nIntEvtAd4/I");
  tr->Branch("tnIntEvtAd4",tv.tnIntEvtAd4,"tnIntEvtAd4[nIntEvtAd4]/I");
  tr->Branch("eIntEvtAd4",tv.eIntEvtAd4,"eIntEvtAd4[nIntEvtAd4]/D");
  tr->Branch("dtIntEvtAd4",tv.dtIntEvtAd4,"dtIntEvtAd4[nIntEvtAd4]/D");
  tr->Branch("xIntEvtAd4",tv.xIntEvtAd4,"xIntEvtAd4[nIntEvtAd4]/D");
  tr->Branch("yIntEvtAd4",tv.yIntEvtAd4,"yIntEvtAd4[nIntEvtAd4]/D");
  tr->Branch("zIntEvtAd4",tv.zIntEvtAd4,"zIntEvtAd4[nIntEvtAd4]/D");
  tr->Branch("inCyl4",tv.inCyl4,"inCyl4[nIntEvtAd4]/O");
  tr->Branch("nDistAd4",&tv.nDistAd4,"nDistAd4/I");
  tr->Branch("dlIntEvt4",tv.dlIntEvt4,"dlIntEvt4[nDistAd4]/D");
}


StatusCode MuonInducedNeutronAlg::execute()
{
  /// process AD events
  RecHeader* recHeader = get<RecHeader>("/Event/Rec/AdSimple");
  const RecTrigger& recAdTrig = recHeader->recTrigger();
  //if(recAdTrig.triggerNumber() == 3571674 || recAdTrig.triggerNumber() == 3571673 || recAdTrig.triggerNumber() == 3571675)
    //info() << recAdTrig.triggerNumber() << " " << recAdTrig.detector() << " " << isFlasher() << " " << recAdTrig.energyStatus() << " " << recAdTrig.positionStatus() << endreq;
  if(recAdTrig.detector().isAD())
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
        m_muonDataBuffer[muId] = OneMuonBuffer();
      
      m_muonDataBuffer[muId].site = recAdTrig.detector().site();
      m_muonDataBuffer[muId].eAd[detId] = recAdTrig.energy();
      m_muonDataBuffer[muId].tn[detId] = recAdTrig.triggerNumber();
      m_muonDataBuffer[muId].ts[detId] = recAdTrig.triggerTime();
      DetectorElement* de = getDet<DetectorElement>(m_referenceDetector[recAdTrig.detector().site()][detId]);
      XYZPoint gblPos = de->geometry()->toGlobal(XYZPoint(recAdTrig.position().x(),recAdTrig.position().y(),recAdTrig.position().z()));
      m_muonDataBuffer[muId].pPmtRecs[detId] = gblPos;
      m_muonDataBuffer[muId].hitDet.push_back(detId);
      
      m_muonLookupTable.erase(curTrigId);
      m_muonProgressTable[muId].m_detectorFinished[detId] = true;
      /// set the last muon as finished and replace the last muon ID
      if(m_lastMuonId[detId] != "0")
        m_muonProgressTable[m_lastMuonId[detId]].m_interEventFinished[detId] = true;
      m_lastMuonId[detId] = muId;
    }
    else if(m_lastMuonId[detId] != "0")
    {
      /// ensure the reconstruction quality
      if(!isFlasher() && recAdTrig.energyStatus() == 1 && recAdTrig.positionStatus() == 1)
      {
        OneInterMuonBuffer im;
        im.en = recAdTrig.energy();
        im.tn = recAdTrig.triggerNumber();
        im.ts = recAdTrig.triggerTime();
        //if(recAdTrig.triggerNumber() > 3571114 && recAdTrig.triggerNumber() < 3571120 && detId == kAD3)
          //info() << "3571114: " << m_lastMuonId[detId] << " " << recAdTrig.triggerNumber() << endreq;
        //if(recAdTrig.triggerNumber() > 3571674 && recAdTrig.triggerNumber() < 3571680 && detId == kAD3)
          //info() << "3571674: " << m_lastMuonId[detId] << " " << recAdTrig.triggerNumber() << endreq;
        im.pRec = getDet<DetectorElement>(m_referenceDetector[recAdTrig.detector().site()][detId])->geometry()->toGlobal(XYZPoint(recAdTrig.position().x(),recAdTrig.position().y(),recAdTrig.position().z()));
        /// store intermuon events to the corresponding muon
        m_muonDataBuffer[m_lastMuonId[detId]].m_interEvents[detId].push_back(im);
      }
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
        m_muonDataBuffer[muId] = OneMuonBuffer();
      
      m_muonDataBuffer[muId].site = recPoolTrig.detector().site();
      m_muonDataBuffer[muId].tn[detId] = recPoolTrig.triggerNumber();
      DetectorElement* de = getDet<DetectorElement>(m_referenceDetector[recPoolTrig.detector().site()][detId]);
      /// NOTE: the unit used here is in METER! Convert to MILLIMETER before use!
      XYZPoint recPos(recPoolTrig.position().x()*1000.,recPoolTrig.position().y()*1000.,recPoolTrig.position().z()*1000.);
      
      if(wsPositionValid(recPos))
      {
        XYZPoint gblPos = de->geometry()->toGlobal(recPos);
        m_muonDataBuffer[muId].pPmtRecs[detId] = gblPos;
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
        m_muonDataBuffer[muId] = OneMuonBuffer();
      /// initialize
      m_muonDataBuffer[muId].hitRpcArray = false;
      m_muonDataBuffer[muId].hitRpcTele = false;
      
      m_muonDataBuffer[muId].site = recRpcTrigger.detector().site();
      m_muonDataBuffer[muId].tn[detId] = recRpcTrigger.triggerNumber();
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
        
        m_muonDataBuffer[muId].pRpcRecs[kArray] = de->geometry()->toGlobal(pArr);
        m_muonDataBuffer[muId].pRpcRecs[kTelescope] = de->geometry()->toGlobal(pTele);
        m_muonDataBuffer[muId].hitRpcArray = true;
        m_muonDataBuffer[muId].hitRpcTele = true;
      }
      // second, ordinary array hit
      else if(recRpcTrigger.numCluster() == 1)
      {
        XYZPoint pArr(recRpcTrigger.getCluster(0)->position().x(),recRpcTrigger.getCluster(0)->position().y(),recRpcTrigger.getCluster(0)->position().z());
        pArr = pArr - m_rpcRecOffset[recRpcTrigger.detector().site()];
        
        m_muonDataBuffer[muId].pRpcRecs[kArray] = de->geometry()->toGlobal(pArr);
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
  map<string, OneMuonBuffer>::iterator muit = m_muonDataBuffer.begin();
  vector<string> finishedMuId;
  for(; muit != m_muonDataBuffer.end(); muit++)
  {
    string muId = muit->first;
    if(m_muonProgressTable[muId].muonFinished() && m_muonProgressTable[muId].interEventFinished())
    {
      //info() << "muon " << muId << " finished" << endreq;
      /// make tracks out of the points
      m_muonDataBuffer[muId].makeTrack(m_rTrackCyl);
      fillOutputTree(muId, m_trvarRO, m_treeRO);
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


void MuonInducedNeutronAlg::fillOutputTree(string muId, MuonInducedNeutronTreeVars& tv, TTree* tr)
{
  /* start initializing tree variables */
  tv.hitAd1 = 0;
  tv.hitAd2 = 0;
  tv.hitAd3 = 0;
  tv.hitAd4 = 0;
  tv.hitIws = 0;
  tv.hitOws = 0;
  tv.hitRpc = 0;
  tv.hitRpcA = 0;
  tv.hitRpcT = 0;
  
  tv.nHitsAd1 = 0;
  tv.nHitsAd2 = 0;
  tv.nHitsAd3 = 0;
  tv.nHitsAd4 = 0;
  tv.nHitsIws = 0;
  tv.nHitsOws = 0;
  tv.nHitsRpc = 0;
  tv.nHitsRpcA = 0;
  tv.nHitsRpcT = 0;
  
  tv.nIntEvtAd1 = 0;
  tv.nIntEvtAd2 = 0;
  tv.nIntEvtAd3 = 0;
  tv.nIntEvtAd4 = 0;
  
  tv.nDistAd1 = 0;
  tv.nDistAd2 = 0;
  tv.nDistAd3 = 0;
  tv.nDistAd4 = 0;
  
  tv.inscribable1 = 0;
  tv.inscribable2 = 0;
  tv.inscribable3 = 0;
  tv.inscribable4 = 0;
  /* end initializing tree variables */
  
  tv.tMu_s = atoi(muId.substr(0,muId.length()-9).c_str());
  tv.tMu_ns = atoi(muId.substr(muId.length()-9,9).c_str());
  
  OneMuonBuffer curmu = m_muonDataBuffer[muId];
  if(find(curmu.hitDet.begin(), curmu.hitDet.end(), kAD1) != curmu.hitDet.end())
  {
    tv.hitAd1 = 1;
    tv.tnAd1[0] = curmu.tn[kAD1];
  }
  if(find(curmu.hitDet.begin(), curmu.hitDet.end(), kAD2) != curmu.hitDet.end())
  {
    tv.hitAd2 = 1;
    tv.tnAd2[0] = curmu.tn[kAD2];
  }
  if(find(curmu.hitDet.begin(), curmu.hitDet.end(), kAD3) != curmu.hitDet.end())
  {
    tv.hitAd3 = 1;
    tv.tnAd3[0] = curmu.tn[kAD3];
  }
  if(find(curmu.hitDet.begin(), curmu.hitDet.end(), kAD4) != curmu.hitDet.end())
  {
    tv.hitAd4 = 1;
    tv.tnAd4[0] = curmu.tn[kAD4];
  }
  if(find(curmu.hitDet.begin(), curmu.hitDet.end(), kIWS) != curmu.hitDet.end())
  {
    tv.hitIws = 1;
    tv.tnIws[0] = curmu.tn[kIWS];
  }
  if(find(curmu.hitDet.begin(), curmu.hitDet.end(), kOWS) != curmu.hitDet.end())
  {
    tv.hitOws = 1;
    tv.tnOws[0] = curmu.tn[kOWS];
  }
  if(find(curmu.hitDet.begin(), curmu.hitDet.end(), kRPC) != curmu.hitDet.end())
  {
    tv.hitRpc = 1;
    tv.tnRpc[0] = curmu.tn[kRPC];
  }
  tv.hitRpcA = (int)curmu.hitRpcArray;
  tv.hitRpcT = (int)curmu.hitRpcTele;
  
  tv.nHitDets = (int)tv.hitAd1 + (int)tv.hitAd2 + (int)tv.hitAd3 + (int)tv.hitAd4 + (int)tv.hitIws + (int)tv.hitOws + (int)tv.hitRpc;
  
  tv.nRecAd1 = (int)(curmu.pPmtRecs.find(kAD1) != curmu.pPmtRecs.end());
  tv.nRecAd2 = (int)(curmu.pPmtRecs.find(kAD2) != curmu.pPmtRecs.end());
  tv.nRecAd3 = (int)(curmu.pPmtRecs.find(kAD3) != curmu.pPmtRecs.end());
  tv.nRecAd4 = (int)(curmu.pPmtRecs.find(kAD4) != curmu.pPmtRecs.end());
  tv.nRecIws = (int)(curmu.pPmtRecs.find(kIWS) != curmu.pPmtRecs.end());
  tv.nRecOws = (int)(curmu.pPmtRecs.find(kOWS) != curmu.pPmtRecs.end());
  tv.nRecRpcA = (int)curmu.hitRpcArray;
  tv.nRecRpcT = (int)curmu.hitRpcTele;
  
  /* start recording reconstructed points */
  for(int i = 0; i < tv.nRecAd1; i++)
  {
    tv.eAd1[i] = curmu.eAd[kAD1];
    tv.xAd1[i] = curmu.pPmtRecs[kAD1].X();
    tv.yAd1[i] = curmu.pPmtRecs[kAD1].Y();
    tv.zAd1[i] = curmu.pPmtRecs[kAD1].Z();
  }
  for(int i = 0; i < tv.nRecAd2; i++)
  {
    tv.eAd2[i] = curmu.eAd[kAD2];
    tv.xAd2[i] = curmu.pPmtRecs[kAD2].X();
    tv.yAd2[i] = curmu.pPmtRecs[kAD2].Y();
    tv.zAd2[i] = curmu.pPmtRecs[kAD2].Z();
  }
  for(int i = 0; i < tv.nRecAd3; i++)
  {
    tv.eAd3[i] = curmu.eAd[kAD3];
    tv.xAd3[i] = curmu.pPmtRecs[kAD3].X();
    tv.yAd3[i] = curmu.pPmtRecs[kAD3].Y();
    tv.zAd3[i] = curmu.pPmtRecs[kAD3].Z();
  }
  for(int i = 0; i < tv.nRecAd4; i++)
  {
    tv.eAd4[i] = curmu.eAd[kAD4];
    tv.xAd4[i] = curmu.pPmtRecs[kAD4].X();
    tv.yAd4[i] = curmu.pPmtRecs[kAD4].Y();
    tv.zAd4[i] = curmu.pPmtRecs[kAD4].Z();
  }
  for(int i = 0; i < tv.nRecIws; i++)
  {
    tv.xIws[i] = curmu.pPmtRecs[kIWS].X();
    tv.yIws[i] = curmu.pPmtRecs[kIWS].Y();
    tv.zIws[i] = curmu.pPmtRecs[kIWS].Z();
  }
  for(int i = 0; i < tv.nRecOws; i++)
  {
    tv.xOws[i] = curmu.pPmtRecs[kOWS].X();
    tv.yOws[i] = curmu.pPmtRecs[kOWS].Y();
    tv.zOws[i] = curmu.pPmtRecs[kOWS].Z();
  }
  if(curmu.hitRpcArray && curmu.hitRpcTele)
  {
    tv.xRpcA[0] = curmu.pRpcRecs[kArray].X();
    tv.yRpcA[0] = curmu.pRpcRecs[kArray].Y();
    tv.zRpcA[0] = curmu.pRpcRecs[kArray].Z();
    tv.xRpcT[0] = curmu.pRpcRecs[kTelescope].X();
    tv.yRpcT[0] = curmu.pRpcRecs[kTelescope].Y();
    tv.zRpcT[0] = curmu.pRpcRecs[kTelescope].Z();
  }
  else if(curmu.hitRpcArray && !curmu.hitRpcTele)
  {
    tv.xRpcA[0] = curmu.pRpcRecs[kArray].X();
    tv.yRpcA[0] = curmu.pRpcRecs[kArray].Y();
    tv.zRpcA[0] = curmu.pRpcRecs[kArray].Z();
  }
  else if(!curmu.hitRpcArray && curmu.hitRpcTele)
  {
    tv.xRpcT[0] = curmu.pRpcRecs[kTelescope].X();
    tv.yRpcT[0] = curmu.pRpcRecs[kTelescope].Y();
    tv.zRpcT[0] = curmu.pRpcRecs[kTelescope].Z();
  }
  /* end recording reconstructed points */
  
  tv.nTracks = (int)curmu.trRpcOws.size();
  for(unsigned int i = 0; i < curmu.trRpcOws.size(); i++)
  {
    tv.nTrPts[i] = curmu.trRpcOws[i].nTrPts;
    XYZPoint centroid = curmu.trRpcOws[i].m_pCent;
    tv.xCt[i] = centroid.X();
    tv.yCt[i] = centroid.Y();
    tv.zCt[i] = centroid.Z();
    XYZVector dir = curmu.trRpcOws[i].m_vDir;
    tv.theta[i] = dir.Theta();
    tv.phi[i] = dir.Phi();
  }
  tv.trIsFit = 0;
  if(curmu.trRpcOws.size())
  {
    tv.trIsFit = (int)(curmu.trRpcOws[0].nTrPts > 2);
    if(tv.trIsFit) tv.rmsDist[0] = curmu.trRpcOws[0].m_rmsDist;
  }
  tv.nIntEvtAd1 = curmu.m_interEvents[kAD1].size();
  tv.nDistAd1 = tv.nIntEvtAd1*curmu.trRpcOws.size();
  for(unsigned int i = 0; i < curmu.m_interEvents[kAD1].size(); i++)
  {
    tv.tnIntEvtAd1[i] = curmu.m_interEvents[kAD1][i].tn;
    tv.eIntEvtAd1[i] = curmu.m_interEvents[kAD1][i].en;
    tv.dtIntEvtAd1[i] = (curmu.m_interEvents[kAD1][i].ts.CloneAndSubtract(curmu.ts[kAD1])).GetSeconds();
    tv.xIntEvtAd1[i] = curmu.m_interEvents[kAD1][i].pRec.X();
    tv.yIntEvtAd1[i] = curmu.m_interEvents[kAD1][i].pRec.Y();
    tv.zIntEvtAd1[i] = curmu.m_interEvents[kAD1][i].pRec.Z();
    if(curmu.trRpcOws.size())
      tv.dlIntEvt1[i] = curmu.trRpcOws[0].distance2Track(curmu.m_interEvents[kAD1][i].pRec);
    
    tv.inCyl1[i] = false;
    if(curmu.m_inscribedCylinder[kAD1].size() && curmu.trRpcOws.size())
    {
      double t = curmu.trRpcOws[0].perpParameter(curmu.m_interEvents[kAD1][i].pRec);
      if(t<curmu.m_inscribedCylinder[kAD1][0].tup && t>curmu.m_inscribedCylinder[kAD1][0].tbot && tv.dlIntEvt1[i] < m_rTrackCyl) tv.inCyl1[i] = true;
    }
  }
  tv.nIntEvtAd2 = curmu.m_interEvents[kAD2].size();
  tv.nDistAd2 = tv.nIntEvtAd2*curmu.trRpcOws.size();
  for(unsigned int i = 0; i < curmu.m_interEvents[kAD2].size(); i++)
  {
    tv.tnIntEvtAd2[i] = curmu.m_interEvents[kAD2][i].tn;
    tv.eIntEvtAd2[i] = curmu.m_interEvents[kAD2][i].en;
    tv.dtIntEvtAd2[i] = (curmu.m_interEvents[kAD2][i].ts.CloneAndSubtract(curmu.ts[kAD2])).GetSeconds();
    tv.xIntEvtAd2[i] = curmu.m_interEvents[kAD2][i].pRec.X();
    tv.yIntEvtAd2[i] = curmu.m_interEvents[kAD2][i].pRec.Y();
    tv.zIntEvtAd2[i] = curmu.m_interEvents[kAD2][i].pRec.Z();
    if(curmu.trRpcOws.size())
      tv.dlIntEvt2[i] = curmu.trRpcOws[0].distance2Track(curmu.m_interEvents[kAD2][i].pRec);
    
    tv.inCyl2[i] = false;
    if(curmu.m_inscribedCylinder[kAD2].size() && curmu.trRpcOws.size())
    {
      double t = curmu.trRpcOws[0].perpParameter(curmu.m_interEvents[kAD2][i].pRec);
      if(t<curmu.m_inscribedCylinder[kAD2][0].tup && t>curmu.m_inscribedCylinder[kAD2][0].tbot && tv.dlIntEvt2[i] < m_rTrackCyl) tv.inCyl2[i] = true;
    }
  }
  tv.nIntEvtAd3 = curmu.m_interEvents[kAD3].size();
  tv.nDistAd3 = tv.nIntEvtAd3*curmu.trRpcOws.size();
  for(unsigned int i = 0; i < curmu.m_interEvents[kAD3].size(); i++)
  {
    tv.tnIntEvtAd3[i] = curmu.m_interEvents[kAD3][i].tn;
    tv.eIntEvtAd3[i] = curmu.m_interEvents[kAD3][i].en;
    tv.dtIntEvtAd3[i] = (curmu.m_interEvents[kAD3][i].ts.CloneAndSubtract(curmu.ts[kAD3])).GetSeconds();
    tv.xIntEvtAd3[i] = curmu.m_interEvents[kAD3][i].pRec.X();
    tv.yIntEvtAd3[i] = curmu.m_interEvents[kAD3][i].pRec.Y();
    tv.zIntEvtAd3[i] = curmu.m_interEvents[kAD3][i].pRec.Z();
    //if(i == 0) info() << muId << endreq;
    //if(curmu.m_interEvents[kAD3][i].tn >= 3571674 && curmu.m_interEvents[kAD3][i].tn <= 3571975)
      //info() << curmu.m_interEvents[kAD3][i].tn << endreq;
    if(curmu.trRpcOws.size())
      tv.dlIntEvt3[i] = curmu.trRpcOws[0].distance2Track(curmu.m_interEvents[kAD3][i].pRec);
    
    tv.inCyl3[i] = false;
    if(curmu.m_inscribedCylinder[kAD3].size() && curmu.trRpcOws.size())
    {
      double t = curmu.trRpcOws[0].perpParameter(curmu.m_interEvents[kAD3][i].pRec);
      if(t<curmu.m_inscribedCylinder[kAD3][0].tup && t>curmu.m_inscribedCylinder[kAD3][0].tbot && tv.dlIntEvt3[i] < m_rTrackCyl) tv.inCyl3[i] = true;
    }
  }
  tv.nIntEvtAd4 = curmu.m_interEvents[kAD4].size();
  tv.nDistAd4 = tv.nIntEvtAd4*curmu.trRpcOws.size();
  for(unsigned int i = 0; i < curmu.m_interEvents[kAD4].size(); i++)
  {
    tv.tnIntEvtAd4[i] = curmu.m_interEvents[kAD4][i].tn;
    tv.eIntEvtAd4[i] = curmu.m_interEvents[kAD4][i].en;
    tv.dtIntEvtAd4[i] = (curmu.m_interEvents[kAD4][i].ts.CloneAndSubtract(curmu.ts[kAD4])).GetSeconds();
    tv.xIntEvtAd4[i] = curmu.m_interEvents[kAD4][i].pRec.X();
    tv.yIntEvtAd4[i] = curmu.m_interEvents[kAD4][i].pRec.Y();
    tv.zIntEvtAd4[i] = curmu.m_interEvents[kAD4][i].pRec.Z();
    if(curmu.trRpcOws.size())
      tv.dlIntEvt4[i] = curmu.trRpcOws[0].distance2Track(curmu.m_interEvents[kAD4][i].pRec);
    
    tv.inCyl4[i] = false;
    if(curmu.m_inscribedCylinder[kAD4].size() && curmu.trRpcOws.size())
    {
      double t = curmu.trRpcOws[0].perpParameter(curmu.m_interEvents[kAD4][i].pRec);
      if(t<curmu.m_inscribedCylinder[kAD4][0].tup && t>curmu.m_inscribedCylinder[kAD4][0].tbot && tv.dlIntEvt4[i] < m_rTrackCyl) tv.inCyl4[i] = true;
    }
  }
  
  tv.passOav1 = curmu.endsOav[kAD1].size();
  if(tv.passOav1)
  {
    tv.xOav1In[0] = curmu.endsOav[kAD1][0].first.X();
    tv.yOav1In[0] = curmu.endsOav[kAD1][0].first.Y();
    tv.zOav1In[0] = curmu.endsOav[kAD1][0].first.Z();
    tv.xOav1Out[0] = curmu.endsOav[kAD1][0].second.X();
    tv.yOav1Out[0] = curmu.endsOav[kAD1][0].second.Y();
    tv.zOav1Out[0] = curmu.endsOav[kAD1][0].second.Z();
    tv.dlOav1[0] = sqrt((curmu.endsOav[kAD1][0].first-curmu.endsOav[kAD1][0].second).Mag2());
  }
  tv.passOav2 = curmu.endsOav[kAD2].size();
  if(tv.passOav2)
  {
    tv.xOav2In[0] = curmu.endsOav[kAD2][0].first.X();
    tv.yOav2In[0] = curmu.endsOav[kAD2][0].first.Y();
    tv.zOav2In[0] = curmu.endsOav[kAD2][0].first.Z();
    tv.xOav2Out[0] = curmu.endsOav[kAD2][0].second.X();
    tv.yOav2Out[0] = curmu.endsOav[kAD2][0].second.Y();
    tv.zOav2Out[0] = curmu.endsOav[kAD2][0].second.Z();
    tv.dlOav2[0] = sqrt((curmu.endsOav[kAD2][0].first-curmu.endsOav[kAD2][0].second).Mag2());
  }
  tv.passOav3 = curmu.endsOav[kAD3].size();
  if(tv.passOav3)
  {
    tv.xOav3In[0] = curmu.endsOav[kAD3][0].first.X();
    tv.yOav3In[0] = curmu.endsOav[kAD3][0].first.Y();
    tv.zOav3In[0] = curmu.endsOav[kAD3][0].first.Z();
    tv.xOav3Out[0] = curmu.endsOav[kAD3][0].second.X();
    tv.yOav3Out[0] = curmu.endsOav[kAD3][0].second.Y();
    tv.zOav3Out[0] = curmu.endsOav[kAD3][0].second.Z();
    tv.dlOav3[0] = sqrt((curmu.endsOav[kAD3][0].first-curmu.endsOav[kAD3][0].second).Mag2());
  }
  tv.passOav4 = curmu.endsOav[kAD4].size();
  if(tv.passOav4)
  {
    tv.xOav4In[0] = curmu.endsOav[kAD4][0].first.X();
    tv.yOav4In[0] = curmu.endsOav[kAD4][0].first.Y();
    tv.zOav4In[0] = curmu.endsOav[kAD4][0].first.Z();
    tv.xOav4Out[0] = curmu.endsOav[kAD4][0].second.X();
    tv.yOav4Out[0] = curmu.endsOav[kAD4][0].second.Y();
    tv.zOav4Out[0] = curmu.endsOav[kAD4][0].second.Z();
    tv.dlOav4[0] = sqrt((curmu.endsOav[kAD4][0].first-curmu.endsOav[kAD4][0].second).Mag2());
  }
  
  tv.passIav1 = curmu.endsIav[kAD1].size();
  if(tv.passIav1)
  {
    tv.xIav1In[0] = curmu.endsIav[kAD1][0].first.X();
    tv.yIav1In[0] = curmu.endsIav[kAD1][0].first.Y();
    tv.zIav1In[0] = curmu.endsIav[kAD1][0].first.Z();
    tv.xIav1Out[0] = curmu.endsIav[kAD1][0].second.X();
    tv.yIav1Out[0] = curmu.endsIav[kAD1][0].second.Y();
    tv.zIav1Out[0] = curmu.endsIav[kAD1][0].second.Z();
    tv.dlIav1[0] = sqrt((curmu.endsIav[kAD1][0].first-curmu.endsIav[kAD1][0].second).Mag2());
  }
  tv.passIav2 = curmu.endsIav[kAD2].size();
  if(tv.passIav2)
  {
    tv.xIav2In[0] = curmu.endsIav[kAD2][0].first.X();
    tv.yIav2In[0] = curmu.endsIav[kAD2][0].first.Y();
    tv.zIav2In[0] = curmu.endsIav[kAD2][0].first.Z();
    tv.xIav2Out[0] = curmu.endsIav[kAD2][0].second.X();
    tv.yIav2Out[0] = curmu.endsIav[kAD2][0].second.Y();
    tv.zIav2Out[0] = curmu.endsIav[kAD2][0].second.Z();
    tv.dlIav2[0] = sqrt((curmu.endsIav[kAD2][0].first-curmu.endsIav[kAD2][0].second).Mag2());
  }
  tv.passIav3 = curmu.endsIav[kAD3].size();
  if(tv.passIav3)
  {
    tv.xIav3In[0] = curmu.endsIav[kAD3][0].first.X();
    tv.yIav3In[0] = curmu.endsIav[kAD3][0].first.Y();
    tv.zIav3In[0] = curmu.endsIav[kAD3][0].first.Z();
    tv.xIav3Out[0] = curmu.endsIav[kAD3][0].second.X();
    tv.yIav3Out[0] = curmu.endsIav[kAD3][0].second.Y();
    tv.zIav3Out[0] = curmu.endsIav[kAD3][0].second.Z();
    tv.dlIav3[0] = sqrt((curmu.endsIav[kAD3][0].first-curmu.endsIav[kAD3][0].second).Mag2());
  }
  tv.passIav4 = curmu.endsIav[kAD4].size();
  if(tv.passIav4)
  {
    tv.xIav4In[0] = curmu.endsIav[kAD4][0].first.X();
    tv.yIav4In[0] = curmu.endsIav[kAD4][0].first.Y();
    tv.zIav4In[0] = curmu.endsIav[kAD4][0].first.Z();
    tv.xIav4Out[0] = curmu.endsIav[kAD4][0].second.X();
    tv.yIav4Out[0] = curmu.endsIav[kAD4][0].second.Y();
    tv.zIav4Out[0] = curmu.endsIav[kAD4][0].second.Z();
    tv.dlIav4[0] = sqrt((curmu.endsIav[kAD4][0].first-curmu.endsIav[kAD4][0].second).Mag2());
  }
  
  /// inscribable cylinder information
  if(curmu.m_inscribedCylinder[kAD1].size())
  {
    tv.inscribable1 = 1;
    tv.xCyl1Up[0] = curmu.m_inscribedCylinder[kAD1][0].pup.X();
    tv.yCyl1Up[0] = curmu.m_inscribedCylinder[kAD1][0].pup.Y();
    tv.zCyl1Up[0] = curmu.m_inscribedCylinder[kAD1][0].pup.Z();
    tv.xCyl1Bot[0] = curmu.m_inscribedCylinder[kAD1][0].pbot.X();
    tv.yCyl1Bot[0] = curmu.m_inscribedCylinder[kAD1][0].pbot.Y();
    tv.zCyl1Bot[0] = curmu.m_inscribedCylinder[kAD1][0].pbot.Z();
    tv.tCyl1Up[0] = curmu.m_inscribedCylinder[kAD1][0].tup;
    tv.tCyl1Bot[0] = curmu.m_inscribedCylinder[kAD1][0].tbot;
  }
  if(curmu.m_inscribedCylinder[kAD2].size())
  {
    tv.inscribable2 = 1;
    tv.xCyl2Up[0] = curmu.m_inscribedCylinder[kAD2][0].pup.X();
    tv.yCyl2Up[0] = curmu.m_inscribedCylinder[kAD2][0].pup.Y();
    tv.zCyl2Up[0] = curmu.m_inscribedCylinder[kAD2][0].pup.Z();
    tv.xCyl2Bot[0] = curmu.m_inscribedCylinder[kAD2][0].pbot.X();
    tv.yCyl2Bot[0] = curmu.m_inscribedCylinder[kAD2][0].pbot.Y();
    tv.zCyl2Bot[0] = curmu.m_inscribedCylinder[kAD2][0].pbot.Z();
    tv.tCyl2Up[0] = curmu.m_inscribedCylinder[kAD2][0].tup;
    tv.tCyl2Bot[0] = curmu.m_inscribedCylinder[kAD2][0].tbot;
  }
  if(curmu.m_inscribedCylinder[kAD3].size())
  {
    tv.inscribable3 = 1;
    tv.xCyl3Up[0] = curmu.m_inscribedCylinder[kAD3][0].pup.X();
    tv.yCyl3Up[0] = curmu.m_inscribedCylinder[kAD3][0].pup.Y();
    tv.zCyl3Up[0] = curmu.m_inscribedCylinder[kAD3][0].pup.Z();
    tv.xCyl3Bot[0] = curmu.m_inscribedCylinder[kAD3][0].pbot.X();
    tv.yCyl3Bot[0] = curmu.m_inscribedCylinder[kAD3][0].pbot.Y();
    tv.zCyl3Bot[0] = curmu.m_inscribedCylinder[kAD3][0].pbot.Z();
    tv.tCyl3Up[0] = curmu.m_inscribedCylinder[kAD3][0].tup;
    tv.tCyl3Bot[0] = curmu.m_inscribedCylinder[kAD3][0].tbot;
  }
  if(curmu.m_inscribedCylinder[kAD4].size())
  {
    tv.inscribable4 = 1;
    tv.xCyl4Up[0] = curmu.m_inscribedCylinder[kAD4][0].pup.X();
    tv.yCyl4Up[0] = curmu.m_inscribedCylinder[kAD4][0].pup.Y();
    tv.zCyl4Up[0] = curmu.m_inscribedCylinder[kAD4][0].pup.Z();
    tv.xCyl4Bot[0] = curmu.m_inscribedCylinder[kAD4][0].pbot.X();
    tv.yCyl4Bot[0] = curmu.m_inscribedCylinder[kAD4][0].pbot.Y();
    tv.zCyl4Bot[0] = curmu.m_inscribedCylinder[kAD4][0].pbot.Z();
    tv.tCyl4Up[0] = curmu.m_inscribedCylinder[kAD4][0].tup;
    tv.tCyl4Bot[0] = curmu.m_inscribedCylinder[kAD4][0].tbot;
  }
  tr->Fill();
}


StatusCode MuonInducedNeutronAlg::finalize()
{
  m_outfile->Write();
  m_outfile->Close();
  delete m_outfile;
  
  return StatusCode::SUCCESS;
}


vector<double> MuonInducedNeutronAlg::getAngCTR(Site_t site, DetectorId_t det, OneTrack tr, XYZPoint pRec)
{
  vector<double> resAng;
  
  if(site != kFar && (det == kAD3 || det == kAD4)) return resAng;
  
  XYZPoint pTr = tr.m_pCent;
  /// translate the coordinate origin to the center of the OAV
  pTr = pTr - m_oavCtr[site][det];
  pRec = pRec - m_oavCtr[site][det];
  XYZVector vTr = tr.m_vDir;
  
  /// signed length of pTr along vTr
  double lTrPara = pTr.Dot(vTr);
  XYZVector pPerp = (XYZVector)pTr - lTrPara*vTr;
  XYZVector vPerpRec = (XYZVector)pRec - pPerp;
  vPerpRec = vPerpRec - vPerpRec.Dot(vTr)*vTr;
  
  resAng.push_back(-(vPerpRec.Unit()).Dot(pPerp.Unit()));
  
  return resAng;
}


StatusCode MuonInducedNeutronAlg::initialize()
{
  m_infile = new TFile(m_infilename.c_str());
  m_treeSpal = (TTree*)m_infile->Get("/Event/Data/Physics/Spallation");
  
  
  for(int ent = 0; ent < m_treeSpal->GetEntries(); ent++)
  //for(int ent = 0; ent < 1000; ent++)
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
    m_muonProgressTable[muId] = OneMuonProgress();
    if(hitAD1 == 1)
    {
      m_muonLookupTable[triggerId(kAD1,triggerNumberAD1)] = muId;
      m_muonProgressTable[muId].m_detectorFinished[kAD1] = false;
      m_muonProgressTable[muId].m_interEventFinished[kAD1] = false;
    }
    if(hitAD2 == 1)
    {
      m_muonLookupTable[triggerId(kAD2,triggerNumberAD2)] = muId;
      m_muonProgressTable[muId].m_detectorFinished[kAD2] = false;
      m_muonProgressTable[muId].m_interEventFinished[kAD2] = false;
    }
    if(hitAD3 == 1)
    {
      m_muonLookupTable[triggerId(kAD3,triggerNumberAD3)] = muId;
      m_muonProgressTable[muId].m_detectorFinished[kAD3] = false;
      m_muonProgressTable[muId].m_interEventFinished[kAD3] = false;
    }
    if(hitAD4 == 1)
    {
      m_muonLookupTable[triggerId(kAD4,triggerNumberAD4)] = muId;
      m_muonProgressTable[muId].m_detectorFinished[kAD4] = false;
      m_muonProgressTable[muId].m_interEventFinished[kAD4] = false;
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
  m_outfile->mkdir("RPC-OWS");
  m_outfile->cd("RPC-OWS");
  m_treeRO = new TTree("mun", "muon induced neutrons");
  /// branch assignment
  assignBranches(m_treeRO, m_trvarRO);
  
  //int countAD3 = 0;
  //map<triggerId, string>::iterator itad3 = m_muonLookupTable.begin();
  //for(; itad3 != m_muonLookupTable.end(); itad3++)
  //{
    //if(itad3->first.first == kAD3)
    //{
      //countAD3++;
      //if(itad3->first.second >= 3571674)
        //info() << "muon id: " << itad3->second << endreq;
    //}
  //}
  //info() << countAD3 << endreq;
  return StatusCode::SUCCESS;
}


bool MuonInducedNeutronAlg::isFlasher()
{
  UserDataHeader* calibStats = get<UserDataHeader>("/Event/Data/CalibStats");
  
  float Quadrant = calibStats->getFloat("Quadrant");
  float MaxQ     = calibStats->getFloat("MaxQ");
  float flasherScale = Quadrant*Quadrant+(MaxQ/.45)*(MaxQ/.45);
  
  if(flasherScale < 1.) return false;
  
  return true;
}


string MuonInducedNeutronAlg::makeMuonId(int s, int ns)
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


//void MuonInducedNeutronAlg::makeTrack(string muId)
//{
  //int nPtsAdTot = m_muonDataBuffer[muId].nPoints(kAD1) + m_muonDataBuffer[muId].nPoints(kAD2) + m_muonDataBuffer[muId].nPoints(kAD3) + m_muonDataBuffer[muId].nPoints(kAD4);
  //int nPtsIws = m_muonDataBuffer[muId].nPoints(kIWS);
  //int nPtsOws = m_muonDataBuffer[muId].nPoints(kOWS);
  //int nPtsRpc = m_muonDataBuffer[muId].nPoints(kRPC);
  
  /// tele-RPC track
  //if(nPtsRpc == 2)
  //{
    //XYZPoint pArr = m_muonDataBuffer[muId].pRpcRecs[kArray];
    //XYZPoint pTele = m_muonDataBuffer[muId].pRpcRecs[kTelescope];
    
    //m_muonDataBuffer[muId].tTeleRpc.push_back(connect2Points(pArr, pTele));
  //}
  
  /// AD-RPC track
  //if(nPtsAdTot == 1 && nPtsRpc == 1)
  //{
    //XYZPoint pAd;
    //if(m_muonDataBuffer[muId].nPoints(kAD1) == 1)
      //pAd = m_muonDataBuffer[muId].pPmtRecs[kAD1];
    //if(m_muonDataBuffer[muId].nPoints(kAD2) == 1)
      //pAd = m_muonDataBuffer[muId].pPmtRecs[kAD2];
    //if(m_muonDataBuffer[muId].nPoints(kAD3) == 1)
      //pAd = m_muonDataBuffer[muId].pPmtRecs[kAD3];
    //if(m_muonDataBuffer[muId].nPoints(kAD4) == 1)
      //pAd = m_muonDataBuffer[muId].pPmtRecs[kAD4];
    //XYZPoint pRpc;
    //if(m_muonDataBuffer[muId].pRpcRecs.find(kTelescope) != m_muonDataBuffer[muId].pRpcRecs.end())
      //pRpc = m_muonDataBuffer[muId].pRpcRecs[kTelescope];
    //if(m_muonDataBuffer[muId].pRpcRecs.find(kArray) != m_muonDataBuffer[muId].pRpcRecs.end())
      //pRpc = m_muonDataBuffer[muId].pRpcRecs[kArray];
    
    //m_muonDataBuffer[muId].tAdRpc.push_back(connect2Points(pAd, pRpc));
  //}
  //if(m_muonDataBuffer[muId].tAdRpc.size())
  //{
    //Site_t site = m_muonDataBuffer[muId].site;
    //m_muonDataBuffer[muId].endsOavAR[kAD1] = getTrackOavIntersections(site, kAD1, m_muonDataBuffer[muId].tAdRpc[0]);
    //m_muonDataBuffer[muId].endsOavAR[kAD2] = getTrackOavIntersections(site, kAD2, m_muonDataBuffer[muId].tAdRpc[0]);
    //m_muonDataBuffer[muId].endsOavAR[kAD3] = getTrackOavIntersections(site, kAD3, m_muonDataBuffer[muId].tAdRpc[0]);
    //m_muonDataBuffer[muId].endsOavAR[kAD4] = getTrackOavIntersections(site, kAD4, m_muonDataBuffer[muId].tAdRpc[0]);
  //}

  /// combined track
  //if(nPtsIws + nPtsOws + nPtsRpc >= 2) fitLeastSquaresLine(muId);
  //if(m_muonDataBuffer[muId].tComb.size())
  //{
    //Site_t site = m_muonDataBuffer[muId].site;
    //m_muonDataBuffer[muId].endsOavCb[kAD1] = getTrackOavIntersections(site, kAD1, m_muonDataBuffer[muId].tComb[0]);
    //m_muonDataBuffer[muId].endsOavCb[kAD2] = getTrackOavIntersections(site, kAD2, m_muonDataBuffer[muId].tComb[0]);
    //m_muonDataBuffer[muId].endsOavCb[kAD3] = getTrackOavIntersections(site, kAD3, m_muonDataBuffer[muId].tComb[0]);
    //m_muonDataBuffer[muId].endsOavCb[kAD4] = getTrackOavIntersections(site, kAD4, m_muonDataBuffer[muId].tComb[0]);
    
    ///// if there is AdSimple
    //if(m_muonDataBuffer[muId].pPmtRecs.find(kAD1) != m_muonDataBuffer[muId].pPmtRecs.end())
      //m_muonDataBuffer[muId].angCTRs[kAD1] = getAngCTR(site, kAD1, m_muonDataBuffer[muId].tComb[0], m_muonDataBuffer[muId].pPmtRecs[kAD1]);
    //if(m_muonDataBuffer[muId].pPmtRecs.find(kAD2) != m_muonDataBuffer[muId].pPmtRecs.end())
      //m_muonDataBuffer[muId].angCTRs[kAD2] = getAngCTR(site, kAD2, m_muonDataBuffer[muId].tComb[0], m_muonDataBuffer[muId].pPmtRecs[kAD2]);
    //if(m_muonDataBuffer[muId].pPmtRecs.find(kAD3) != m_muonDataBuffer[muId].pPmtRecs.end())
      //m_muonDataBuffer[muId].angCTRs[kAD3] = getAngCTR(site, kAD3, m_muonDataBuffer[muId].tComb[0], m_muonDataBuffer[muId].pPmtRecs[kAD3]);
    //if(m_muonDataBuffer[muId].pPmtRecs.find(kAD4) != m_muonDataBuffer[muId].pPmtRecs.end())
      //m_muonDataBuffer[muId].angCTRs[kAD4] = getAngCTR(site, kAD4, m_muonDataBuffer[muId].tComb[0], m_muonDataBuffer[muId].pPmtRecs[kAD4]);
  //}
  //vector<XYZPoint> pCloud;
  
  ///// RPC-OWS track
  //map<RpcHitLocation_t, XYZPoint>::iterator itrpc = m_muonDataBuffer[muId].pRpcRecs.begin();
  //for(; itrpc != m_muonDataBuffer[muId].pRpcRecs.end(); itrpc++)
    //pCloud.push_back(itrpc->second);
  //if(m_muonDataBuffer[muId].pPmtRecs.find(kOWS) != m_muonDataBuffer[muId].pPmtRecs.end())
    //pCloud.push_back(m_muonDataBuffer[muId].pPmtRecs[kOWS]);
  //info() << "total # of points: " << pCloud.size() << endreq;
//}


/// If PoolSimple fails to reconstruct a position, the default value is
/// assumed, which is (0.,0.,0.) in floating point!
bool MuonInducedNeutronAlg::wsPositionValid(XYZPoint p)
{
  if(sqrt(p.Mag2()) > 1e-10) return true;
  return false;
}


OneMuonProgress::OneMuonProgress()
{
  m_detectorFinished[kAD1] = true;
  m_detectorFinished[kAD2] = true;
  m_detectorFinished[kAD3] = true;
  m_detectorFinished[kAD4] = true;
  m_detectorFinished[kIWS] = true;
  m_detectorFinished[kOWS] = true;
  m_detectorFinished[kRPC] = true;
}


bool OneMuonProgress::interEventFinished()
{
  bool result = true;
  
  map<DetectorId_t, bool>::iterator it = m_interEventFinished.begin();
  for(; it != m_interEventFinished.end(); it++) result = (result && it->second);
  
  return result;
}


bool OneMuonProgress::muonFinished()
{
  map<DetectorId_t, bool>::iterator it = m_detectorFinished.begin();
  
  bool result = true;
  for(; it != m_detectorFinished.end(); it++)
    result = (result && it->second);
  
  return result;
}


void OneMuonBuffer::makeTrack(double rins)
{
  /// OAV center location
  map<Site_t, map<DetectorId_t, XYZPoint> > oavCtr;
  oavCtr[kDayaBay][kAD1] = XYZPoint(-18079.5, -799699, -7141.5);
  oavCtr[kDayaBay][kAD2] = XYZPoint(-14960.5, -804521, -7141.5);
  oavCtr[kLingAo][kAD1] = XYZPoint(472323, 60325.2, -3761.5);
  oavCtr[kLingAo][kAD2] = XYZPoint(471297, 65974.8, -3761.5);
  oavCtr[kFar][kAD1] = XYZPoint(-406758, 812082, -2291.5);
  oavCtr[kFar][kAD2] = XYZPoint(-411758, 809258, -2291.5);
  oavCtr[kFar][kAD3] = XYZPoint(-409582, 817082, -2291.5);
  oavCtr[kFar][kAD4] = XYZPoint(-414582, 814258, -2291.5);
  
  /// IAV center location
  map<Site_t, map<DetectorId_t, XYZPoint> > iavCtr;
  iavCtr[kDayaBay][kAD1] = XYZPoint(-18079.5, -799699, -7107.5);
  iavCtr[kDayaBay][kAD2] = XYZPoint(-14960.5, -804521, -7107.5);
  iavCtr[kLingAo][kAD1] = XYZPoint(472323, 60325.2, -3727.5);
  iavCtr[kLingAo][kAD2] = XYZPoint(471297, 65974.8, -3727.5);
  iavCtr[kFar][kAD1] = XYZPoint(-406758, 812082, -2257.5);
  iavCtr[kFar][kAD2] = XYZPoint(-411758, 809258, -2257.5);
  iavCtr[kFar][kAD3] = XYZPoint(-409582, 817082, -2257.5);
  iavCtr[kFar][kAD4] = XYZPoint(-414582, 814258, -2257.5);
  
  /// RPC-OWS track
  int nRpc = (int)pRpcRecs.size();

  if(nRpc && pPmtRecs.find(kOWS) != pPmtRecs.end())
  {
    OneTrack tr;
    tr.pRpcRecs = pRpcRecs;
    tr.pPmtRecs = pPmtRecs;
    
    tr.nTrPts = nRpc + 1;
    
    if(nRpc == 1) tr.connect2Points(pRpcRecs.begin()->second, pPmtRecs[kOWS]);
    if(nRpc == 2) tr.fitLeastSquaresLine();
    
    trRpcOws.push_back(tr);
  }
  
  if(trRpcOws.size())
  {
    endsOav[kAD1] = trRpcOws[0].getTrackOavIntersections(oavCtr[site][kAD1]);
    endsIav[kAD1] = trRpcOws[0].getTrackIavIntersections(iavCtr[site][kAD1]);
    endsOav[kAD2] = trRpcOws[0].getTrackOavIntersections(oavCtr[site][kAD2]);
    endsIav[kAD2] = trRpcOws[0].getTrackIavIntersections(iavCtr[site][kAD2]);
    
    if(endsIav[kAD1].size())
      m_inscribedCylinder[kAD1] = trRpcOws[0].inscribeCylinder(iavCtr[site][kAD1], rins);
    if(endsIav[kAD2].size())
      m_inscribedCylinder[kAD2] = trRpcOws[0].inscribeCylinder(iavCtr[site][kAD2], rins);
    if(site > 2)
    {
      endsOav[kAD3] = trRpcOws[0].getTrackOavIntersections(oavCtr[site][kAD3]);
      endsIav[kAD3] = trRpcOws[0].getTrackIavIntersections(iavCtr[site][kAD3]);
      endsOav[kAD4] = trRpcOws[0].getTrackOavIntersections(oavCtr[site][kAD4]);
      endsIav[kAD4] = trRpcOws[0].getTrackIavIntersections(iavCtr[site][kAD4]);
      
      if(endsIav[kAD3].size())
        m_inscribedCylinder[kAD3] = trRpcOws[0].inscribeCylinder(iavCtr[site][kAD3], rins);
      if(endsIav[kAD4].size())
        m_inscribedCylinder[kAD4] = trRpcOws[0].inscribeCylinder(iavCtr[site][kAD4], rins);
    }
  }
}


OneTrack::OneTrack(XYZPoint p, XYZVector v)
{
  m_pCent = p;
  m_vDir = v;
}


bool OneTrack::circleInsideIav(double zTrackCoord, XYZVector pXYI)
{
  int nstep = 360;
  
  AxisAngle rotTrack(XYZVector(0,0,1).Cross(m_vDir),(m_vDir).Theta());
  
  Transform3D track2Iav(rotTrack, pXYI);
  
  for(int i = 0; i < nstep; i++)
  {
    double th = i*TMath::TwoPi()/nstep;
    XYZPoint pCircleTrackCoord(1000.*cos(th), 1000.*sin(th), zTrackCoord);
    
    double thisZ = track2Iav(pCircleTrackCoord).Z();
    
    if(thisZ > RIAV || thisZ < -RIAV) return false;
  }
  
  return true;
}


void OneTrack::connect2Points(XYZPoint ps, XYZPoint pe)
{
  XYZPoint centroid;
  XYZVector direction;
  
  centroid = (ps+(XYZVector)pe)/2.;
  direction = (pe-ps).Unit();
  
  /// direction vector always points to the sky
  if(direction.Z() < 0) direction *= -1.;

  m_pCent = centroid;
  m_vDir = direction;
}


double OneTrack::distance2Track(XYZPoint p)
{
  XYZVector vPC = p - m_pCent;
  double lPC = sqrt(vPC.Mag2());
  double ang = acos(vPC.Unit().Dot(m_vDir));
  
  return lPC * sin(ang);
}


void OneTrack::fitLeastSquaresLine()
{
  vector<XYZPoint> pCloud;
  
  map<DetectorId_t, XYZPoint>::iterator itpmt = pPmtRecs.begin();
  for(; itpmt != pPmtRecs.end(); itpmt++)
    pCloud.push_back(itpmt->second);
  map<RpcHitLocation_t, XYZPoint>::iterator itrpc = pRpcRecs.begin();
  for(; itrpc != pRpcRecs.end(); itrpc++)
    pCloud.push_back(itrpc->second);
  
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
    
    //debug() << "point disp:(" << gsl_matrix_get(vDir,0,0) << ",";
    //debug() << gsl_matrix_get(vDir,1,0) << ",";
    //debug() << gsl_matrix_get(vDir,2,0) << ")" << endreq;
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
  
  //debug() << "resultant matrix:" << endreq;
  //for(int i = 0; i < 3; i++)
  //{
    //for(int j = 0; j < 3; j++)
      //debug() << gsl_matrix_get(mTotRes,i,j) << " ";
    //debug() << endreq;
  //}
  
  double eval_min = gsl_vector_get (eval, 0);
  gsl_vector_view evec_min = gsl_matrix_column (evec, 0);
  
  /// store results
  m_rmsDist = sqrt(fabs(eval_min)/pCloud.size());
  XYZVector direction(gsl_matrix_get(evec, 0, 0),gsl_matrix_get(evec, 1, 0),gsl_matrix_get(evec, 2, 0));
  if(direction.Z() < 0) direction *= -1.;
  m_pCent = pAvg;
  m_vDir = direction;
  
  //debug() << "RMS distance: " << rmsDist << endreq;
  //debug() << "direction: (" << gsl_matrix_get(evec, 0, 0) << ",";
  //debug() << gsl_matrix_get(evec, 1, 0) << ",";
  //debug() << gsl_matrix_get(evec, 2, 0) << ")" << endreq;
  
  /// free memory
  gsl_matrix_free(vDir);
  gsl_matrix_free(mOnePtRes);
  gsl_matrix_free(mTotRes);
  gsl_matrix_free(mTotResWithConst);
  gsl_vector_free(eval);
  gsl_matrix_free(evec);
}


vector<pair<XYZPoint, XYZPoint> > OneTrack::getTrackIavIntersections(XYZPoint iavCtr)
{
  vector<pair<XYZPoint, XYZPoint> > pEnds;

  XYZPoint pTr = m_pCent;
  /// translate the coordinate origin to the center of the OAV
  pTr = pTr - iavCtr;
  XYZVector vTr = m_vDir;
  
  XYZVector u1(0,0,1);
  double a21 = u1.Dot(vTr);
  double b1 = pTr.Dot(u1), b2 = pTr.Dot(vTr);
  double t1 = (-b1+a21*b2)/(-1+a21*a21);
  double t2 = (b2-a21*b1)/(-1+a21*a21);
  
  // closest points on the IAV cylinder
  XYZVector cpZ = t1*u1;
  // closest point on the track
  XYZVector cpTr = (XYZVector)pTr + t2*vTr;
  
  double skewDist = sqrt((cpZ-cpTr).Mag2());
  
  if(skewDist > 1500) return pEnds;
  
  double tup = sqrt(1500*1500-skewDist*skewDist)/sin(vTr.Theta());
  
  XYZVector pup = cpTr + tup*vTr;
  XYZVector pbot = cpTr - tup*vTr;
  if(cpTr.Z() > 1500)
  {
    if(pbot.Z() > 1500) return pEnds;
    pup = cpTr - (1500-cpTr.Z())/(pbot-cpTr).Z()*tup*vTr;
  }
  else if(cpTr.Z() < -1500)
  {
    if(pup.Z() < -1500) return pEnds;
    pbot = cpTr + (-1500-cpTr.Z())/(pup-cpTr).Z()*tup*vTr;
  }

  if(pup.Z() > 1500+1e-3)
    pup = cpTr + (1500-cpTr.Z())/(pup-cpTr).Z()*tup*vTr;
  if(pbot.Z() < -1500-1e-3)
    pbot = cpTr - (-1500-cpTr.Z())/(pbot-cpTr).Z()*tup*vTr;
  
  pEnds.push_back(pair<XYZPoint, XYZPoint>(iavCtr+pup, iavCtr+pbot));
  return pEnds;
}


vector<pair<XYZPoint, XYZPoint> > OneTrack::getTrackOavIntersections(XYZPoint oavCtr)
{
  vector<pair<XYZPoint, XYZPoint> > pEnds;

  XYZPoint pTr = m_pCent;
  /// translate the coordinate origin to the center of the OAV
  pTr = pTr - oavCtr;
  XYZVector vTr = m_vDir;
  
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
  
  if(skewDist > 2000) return pEnds;
  
  double tup = sqrt(2000*2000-skewDist*skewDist)/sin(vTr.Theta());

  XYZVector pup = cpTr + tup*vTr;
  XYZVector pbot = cpTr - tup*vTr;
  if(cpTr.Z() > 2000)
  {
    if(pbot.Z() > 2000) return pEnds;
    pup = cpTr - (2000-cpTr.Z())/(pbot-cpTr).Z()*tup*vTr;
  }
  else if(cpTr.Z() < -2000)
  {
    if(pup.Z() < -2000) return pEnds;
    pbot = cpTr + (-2000-cpTr.Z())/(pup-cpTr).Z()*tup*vTr;
  }
      
  if(pup.Z() > 2000+1e-3)
    pup = cpTr + (2000-cpTr.Z())/(pup-cpTr).Z()*tup*vTr;
  if(pbot.Z() < -2000-1e-3)
    pbot = cpTr - (-2000-cpTr.Z())/(pbot-cpTr).Z()*tup*vTr;
  
  pEnds.push_back(pair<XYZPoint, XYZPoint>(oavCtr+pup, oavCtr+pbot));
  return pEnds;
}


//vector<OneCylinder> OneTrack::inscribeCylinder(XYZPoint iavctr)
//{
  ///* any vector or point with suffix I means IAV coordinate */
  ///* any vector or point with suffix T means track coordinate */
  //OneCylinder cyl;
  
  ///// translate coordinate to the IAV center of the current AD
  //XYZVector pTrack = m_pCent - iavctr;
  
  ///// translation vector: track intersection with xy plane
  //XYZVector pXYI = pTrack - pTrack.Z()/m_vDir.Z()*m_vDir;
  
  ///// axis along which the IAV is rotated by the angle between the track and the IAV
  //XYZVector pivot = -XYZVector(0,0,1).Cross(m_vDir).Unit();
  
  ///// transformation of rotation along pivot by the angle between the track and the IAV
  //AxisAngle rotateIav(pivot, m_vDir.Theta());
  
  ///// the translation of the IAV in the track coordinate
  //XYZVector translateIav = -(rotateIav*pXYI);
  
  ///// extract the rotation matrix elements
  //XYZVector e1(1,0,0);
  //XYZVector e2(0,1,0);
  //XYZVector e3(0,0,1);
  
  ///// transformation of rotating the track away from z axis
  //AxisAngle rotateTrack = rotateIav.Inverse();
  
  ///// equation coefficiencts
  //double a11 = e1.Dot(rotateTrack*e1);
  //double a12 = e1.Dot(rotateTrack*e2);
  //double a13 = e1.Dot(rotateTrack*e3);
  //double a21 = e2.Dot(rotateTrack*e1);
  //double a22 = e2.Dot(rotateTrack*e2);
  //double a23 = e2.Dot(rotateTrack*e3);
  //double s1 = (rotateTrack*(-translateIav)).X();
  //double s2 = (rotateTrack*(-translateIav)).Y();
  
  ///// number of steps on the solution curve
  //int nstep = 360;
  
  ///// if the discriminant is ever negative, a closed loop is formed
  ///// and a smaller cylinder can not be inscribed
  //bool twoContour = true;
  
  ///// find the minimum of the upper curve and
  ///// the maximum of the lower curve and the corresponding theta values
  //double zupminTrackCoord, zlowmaxTrackCoord;
  //double phiUpTrackCoord, phiLowTrackCoord;
  
  //bool firstRealSolution = true;
  
  ///// for the equation, see the group meeting report on Jan. 31, 2013
  //for(int i = 0; i < nstep; i++)
  //{
    
    //double x = -TMath::Pi()+TMath::TwoPi()/nstep*i;
    //double R = RADIUSIAV;
    //double r = 1000.;
    //double x1 = r*cos(x), x2 = r*sin(x);
    //double a2 = a13*a13+a23*a23;
    //double rcoor1 = a11*x1+a12*x2+s1, rcoor2 = a21*x1+a22*x2+s2;
    //double a1 = 2*(a13*rcoor1+a23*rcoor2);
    //double a0 = rcoor1*rcoor1+rcoor2*rcoor2-R*R;
    //double D = a1*a1-4*a0*a2;
    
    //if(D > 0) {
      
      //double yup = 1./2/a2*(-a1+sqrt(D));
      //double ylow = 1./2/a2*(-a1-sqrt(D));
      
      //if(firstRealSolution) /// initialize
      //{
        //zupminTrackCoord = yup;
        //zlowmaxTrackCoord = ylow;
        
        //phiUpTrackCoord = x;
        //phiLowTrackCoord = x;
        
        //firstRealSolution = false;
      //}
      
      //else {
        //if(yup < zupminTrackCoord) {
          //zupminTrackCoord = yup;
          //phiUpTrackCoord = x;
        //}
        
        //if(ylow > zlowmaxTrackCoord) {
          //zlowmaxTrackCoord = ylow;
          //phiLowTrackCoord = x;
        //}
      //}
    //}
    //else twoContour = false;
  //} /* end of for(int i = 0; i < nstep; i++) loop */
  
  ///// now consider the intersection is on the top/bottom IAV plane
  //XYZPoint pTrackTopIavCoord, pTrackDownIavCoord;
  
  //pTrackTopIavCoord = pTrack - (pTrack.Z()-RADIUSIAV)/m_vDir.Z()*m_vDir;
  //pTrackDownIavCoord = pTrack - (pTrack.Z()+RADIUSIAV)/m_vDir.Z()*m_vDir;
  
  //XYZVector vPerp = e3.Cross(-m_vDir).Unit();
  //XYZVector vAlong = vPerp.Cross(e3);
  //double tiltAng = m_vDir.Theta();
  
  //XYZPoint pTangentTopIavCoord, pTangentBotIavCoord;
  
  //pTangentTopIavCoord = pTrackTopIavCoord+(1000./cos(tiltAng))*vAlong;
  //pTangentBotIavCoord = pTrackDownIavCoord-(1000./cos(tiltAng))*vAlong;
  
  ///// check if zupmin is lower than the z of the top IAV plane
  ///// and zlowmax is higher than the z of the bottom IAV plane
  //XYZPoint pUpMinTrackCoord(1000.*cos(phiUpTrackCoord), 1000.*sin(phiUpTrackCoord), zupminTrackCoord);
  //XYZPoint pLowMaxTrackCoord(1000.*cos(phiLowTrackCoord), 1000.*sin(phiLowTrackCoord), zlowmaxTrackCoord);
  
  ///// define the isometric transformation
  //Transform3D track2Iav(rotateTrack, pXYI);
  //Transform3D iav2Track(rotateIav, translateIav);
  
  //XYZPoint pUpMinIavCoord = track2Iav(pUpMinTrackCoord);
  //XYZPoint pLowMaxIavCoord = track2Iav(pLowMaxTrackCoord);
  
  ///// get top and bottom tangent points in track coordinate system
  //XYZPoint pTangentTopTrackCoord = iav2Track(pTangentTopIavCoord);
  //XYZPoint pTangentBotTrackCoord = iav2Track(pTangentBotIavCoord);
  
  
  ///// record the tangent point of the inscribing cylinder in IAV coordinate
  //XYZPoint pInsCylTopIavCoord, pInsCylBotIavCoord;
  ///// the z value of the upper and lower planes of the inscribing cylinder
  ///// in track coordinate
  //double zInsCylTopTrackCoord, zInsCylBotTrackCoord;
  //double inscribable = false;
  //if(twoContour)
  //{
    //bool topTangentInside = (sqrt(pTangentTopIavCoord.X()*pTangentTopIavCoord.X()+pTangentTopIavCoord.Y()*pTangentTopIavCoord.Y()) < RADIUSIAV);
    
    //bool botTangentInside = (sqrt(pTangentBotIavCoord.X()*pTangentBotIavCoord.X()+pTangentBotIavCoord.Y()*pTangentBotIavCoord.Y()) < RADIUSIAV);
    
    //bool topLoopInside = (pUpMinIavCoord.Z() < RADIUSIAV);
    //bool botLoopInside = (pLowMaxIavCoord.Z() > -RADIUSIAV);
    
    //bool topInscribable, botInscribable;
    
    //bool upCircleInsideIav = circleInsideIav(zupminTrackCoord, pXYI);
    //bool lowCircleInsideIav = circleInsideIav(zlowmaxTrackCoord, pXYI);
    
    ///// deal with top
    //if(topTangentInside && topLoopInside) {
      //pInsCylTopIavCoord = (pTangentTopTrackCoord.Z() < pUpMinTrackCoord.Z()) ? pTangentTopIavCoord : pUpMinIavCoord;
      ////topInscribable = (upCircleInsideIav ? true : false);
      //topInscribable = true;
    //}
    
    //if(topTangentInside && !topLoopInside) {
      //pInsCylTopIavCoord = pTangentTopIavCoord;
      //topInscribable = true;
    //}
    
    //if(!topTangentInside && topLoopInside) {
      //pInsCylTopIavCoord = pUpMinIavCoord;
      ////topInscribable = (upCircleInsideIav ? true : false);
      //topInscribable = true;
    //}
    
    //if(!topTangentInside && !topLoopInside) topInscribable = false;
    
    ///// deal with bottom
    //if(botTangentInside && botLoopInside) {
      //pInsCylBotIavCoord = (pTangentBotTrackCoord.Z() > pLowMaxTrackCoord.Z()) ? pTangentBotIavCoord : pLowMaxIavCoord;
      ////botInscribable = (lowCircleInsideIav ? true : false);
      //botInscribable = true;
    //}
    
    //if(botTangentInside && !botLoopInside) {
      //pInsCylBotIavCoord = pTangentBotIavCoord;
      //botInscribable = true;
    //}
    
    //if(!botTangentInside && botLoopInside) {
      //pInsCylBotIavCoord = pLowMaxIavCoord;
      ////botInscribable = (lowCircleInsideIav ? true : false);
      //botInscribable = true;
    //}
    
    //if(!botTangentInside && !botLoopInside) botInscribable = false;
    
    //zInsCylTopTrackCoord = iav2Track(pInsCylTopIavCoord).Z();
    //zInsCylBotTrackCoord = iav2Track(pInsCylBotIavCoord).Z();
    
    //inscribable = topInscribable && botInscribable && (zInsCylTopTrackCoord > zInsCylBotTrackCoord);
    
  //} /* end of if(twoContour) */
  //cout << "inscribing cylinder... " << inscribable << endl;
  
  ///// store results
  //vector<OneCylinder> res;
  
  //if(inscribable)
  //{
    //cyl.pup = pInsCylTopIavCoord + (XYZVector)iavctr;
    //cyl.pbot = pInsCylBotIavCoord + (XYZVector)iavctr;
    //cyl.tup = perpParameter(cyl.pup);
    //cyl.tbot = perpParameter(cyl.pbot);
    
    //res.push_back(cyl);
  //}
  
  //return res;
//}


vector<OneCylinder> OneTrack::inscribeCylinder(XYZPoint iavctr, double rins)
{
  vector<OneCylinder> res;
  OneCylinder cyl;
  
  /// translate coordinate to the IAV center of the current AD
  XYZVector pTrack = m_pCent - iavctr;
  
  XYZVector e1(1,0,0);
  XYZVector e2(0,1,0);
  XYZVector e3(0,0,1);
  
  AxisAngle rotateTrack(e3.Cross(m_vDir), m_vDir.Theta());
  AxisAngle rotateIav = rotateTrack.Inverse();
  
  /// matrix elements of the inverse rotation matrix
  double a11 = e1.Dot(rotateIav*e1);
  double a12 = e1.Dot(rotateIav*e2);
  double a13 = e1.Dot(rotateIav*e3);
  double a21 = e2.Dot(rotateIav*e1);
  double a22 = e2.Dot(rotateIav*e2);
  double a23 = e2.Dot(rotateIav*e3);
  
  
  /// containers
  vector<pair<XYZVector, XYZVector> > positionCurvePoints;
  vector<pair<int, int> > regionAndNRoots;
  set<int> regionName;
  
  int nsteps = 360;
  
  /// partition phi into regions: closed loop regions and empty regions
  for(int i = 0; i < nsteps; i++)
  {
    double x = -TMath::Pi()+TMath::TwoPi()/nsteps*i;
    double R = RIAV;
    double r = rins;
    double x1 = R*cos(x)-pTrack.X();
    double x2 = R*sin(x)-pTrack.Y();
    
    double a2 = a13*a13+a23*a23;
    double b1 = a11*x1+a12*x2;
    double b2 = a21*x1+a22*x2;
    double a1 = 2*(a13*b1+a23*b2);
    double a0 = b1*b1+b2*b2-r*r;
    
    vector<double> roots;
    double D = a1*a1-4*a2*a0;
    if(D > 0)
    {
      roots.push_back((-a1+sqrt(D))/2./a2);
      roots.push_back((-a1-sqrt(D))/2./a2);
    }
    
    double x31, x32;
    if(roots.size() == 2)
    {
      x31 = roots[0]+pTrack.Z();
      x32 = roots[1]+pTrack.Z();
      x1 += pTrack.X();
      x2 += pTrack.Y();
      XYZVector v1(x1,x2,x31);
      XYZVector v2(x1,x2,x32);
      positionCurvePoints.push_back(pair<XYZVector, XYZVector>(v1, v2));
    }
    else
    {
      XYZVector vz(0,0,0);
      positionCurvePoints.push_back(pair<XYZVector, XYZVector>(vz, vz));
    }
    
    if( i == 0 )
    {
      regionAndNRoots.push_back(pair<int, int>(0,roots.size()));
    }
    else
    {
      int lastRegion = regionAndNRoots[i-1].first;
      int lastNRoots = regionAndNRoots[i-1].second;
      int curRegion = lastRegion;
      int curNRoots = roots.size();
      if(((curNRoots > 0) && (lastNRoots == 0)) ||
         ((curNRoots == 0) && (lastNRoots > 0))) curRegion++;
      regionAndNRoots.push_back(pair<int, int>(curRegion,curNRoots));
    }
    regionName.insert(regionAndNRoots[i].first);
  }
  
  
  /// wrap the last region to the first region due to phi periodicity
  map<int, vector<XYZVector> > posPartCurve;
  if(regionName.size() > 1)
  {
    for(unsigned int i = 0; i < regionAndNRoots.size(); i++)
    {
      regionAndNRoots[i].first = regionAndNRoots[i].first%(regionName.size()-1);
    }
    
    /// map from region to the group of curve points
    for(unsigned int i = 0; i < regionAndNRoots.size(); i++)
    {
      int curRegion = regionAndNRoots[i].first;
      int curNRoots = regionAndNRoots[i].second;
      if(curNRoots == 2)
      {
        posPartCurve[curRegion].push_back(positionCurvePoints[i].first);
        posPartCurve[curRegion].push_back(positionCurvePoints[i].second);
      }
    }
  }
  
  
  /// container map from region to the indices of the extremities
  map<int, pair<XYZVector, XYZVector> > extP;
  map<int, pair<unsigned int,unsigned int> > extInd;
  map<int, pair<double, double> > extT;
  for(map<int, vector<XYZVector> >::iterator it = posPartCurve.begin(); it != posPartCurve.end(); it++)
  {
    for(unsigned int i = 0; i < it->second.size(); i++)
    {
      if(i == 0)
      {
        extInd[it->first] = pair<int,int>(0,0);
        extP[it->first] = pair<XYZVector, XYZVector>(it->second[0], it->second[0]);
        extT[it->first].first = perpParameter(it->second[0]+iavctr);
        extT[it->first].second = extT[it->first].first;
      }
      else
      {
        double z = perpParameter(it->second[i]+iavctr);
        if(z > extT[it->first].first)
        {
          extT[it->first].first = z;
          extInd[it->first].first = i;
          extP[it->first].first = it->second[i];
        }
        if(z < extT[it->first].second)
        {
          extT[it->first].second = z;
          extInd[it->first].second = i;
          extP[it->first].second = it->second[i];
        }
      }
    }
  }
  
  //if(extT.size() == 2)
  //{
    //map<int, pair<double, double> >::iterator itt = extT.begin();
    //for(; itt != extT.end(); itt++)
    //{
      //cout << itt->second.first << " ";
      //cout << itt->second.second << " ";
    //}
    //cout << endl;
  //}
  
  /// container for the answer
  if(extT.size() == 2)
  {
    map<int, pair<double, double> >::iterator itup = extT.begin();
    int reg1 = itup->first;
    itup++;
    int reg2 = itup->first;
    if(extT[reg1].first < extT[reg2].first)
    {
      int regtemp = reg2;
      reg2 = reg1;
      reg1 = regtemp;
    }
    
    /// unit vectors parallel to the up and bottom plane of the IAV
    double th = m_vDir.Theta();
    XYZVector paraup = (-m_vDir-cos(th)*-e3).Unit();
    XYZVector parabot = (m_vDir-cos(th)*e3).Unit();
    
    double tIavUp = -(pTrack.Z()-RIAV)/m_vDir.Z();
    XYZVector pIavUp = pTrack + tIavUp*m_vDir;
    tIavUp -= rins*tan(th);
    pIavUp = pIavUp + rins/cos(th)*paraup;
    
    double tIavBot = (-RIAV-pTrack.Z())/m_vDir.Z();
    XYZVector pIavBot = pTrack + tIavBot*m_vDir;
    tIavBot += rins*tan(th);
    pIavBot = pIavBot + rins/cos(th)*parabot;
    
    cyl.pup = (XYZPoint)(extP[reg1].second+iavctr);
    cyl.tup = extT[reg1].second;
    if(tIavUp < cyl.tup)
    {
      cyl.tup = tIavUp;
      cyl.pup = (XYZPoint)(pIavUp+iavctr);
    }
    
    cyl.pbot = (XYZPoint)(extP[reg2].first+iavctr);
    cyl.tbot = extT[reg2].first;
    if(tIavBot > cyl.tbot)
    {
      cyl.tbot = tIavBot;
      cyl.pbot = (XYZPoint)(pIavBot+iavctr);
    }
    if(cyl.tup > cyl.tbot)
    {
      res.push_back(cyl);
    }
  }
  
  return res;
}


double OneTrack::perpParameter(XYZPoint p)
{
  XYZVector vCP = p - m_pCent;
  double cos = vCP.Unit().Dot(m_vDir);
  
  return sqrt(vCP.Mag2())*cos;
}


XYZPoint OneTrack::perpPoint(XYZPoint p)
{
  XYZVector vCP = p - m_pCent;
  double cos = vCP.Unit().Dot(m_vDir);
  
  return m_pCent + sqrt(vCP.Mag2())*cos*m_vDir;
}
