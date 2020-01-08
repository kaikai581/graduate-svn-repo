#ifndef MUONINDUCEDNEUTRONALG_HPP
#define MUONINDUCEDNEUTRONALG_HPP


#include "Context/TimeStamp.h"
#include "Conventions/Detectors.h"
#include "GaudiAlg/GaudiAlgorithm.h"
#include "Math/Point3D.h"
#include "Math/Vector3D.h"

#define MAXARRAYSIZE 10
#define MAXNINTEREVT 10000

class TFile;
class TTree;

typedef std::pair<DetectorId::DetectorId_t, int> triggerId;
enum RpcHitLocation_t {kArray, kTelescope};


class OneCylinder
{
public:
  ROOT::Math::XYZPoint pCenter;
  ROOT::Math::XYZPoint pup;
  ROOT::Math::XYZPoint pbot;
  double               tup;
  double               tbot;
  double               length;
};


class OneTrack
{
public:
  OneTrack() {};
  OneTrack(ROOT::Math::XYZPoint, ROOT::Math::XYZVector);
  /// points used for track reconstruction
  int nTrPts;
  std::map<DetectorId::DetectorId_t, ROOT::Math::XYZPoint> pPmtRecs;
  std::map<RpcHitLocation_t, ROOT::Math::XYZPoint> pRpcRecs;
  /// point to line distance
  std::map<DetectorId::DetectorId_t, double> dPmtRecs;
  std::map<RpcHitLocation_t, double> dRpcRecs;
  /// centroid and direction
  ROOT::Math::XYZPoint m_pCent;
  ROOT::Math::XYZVector m_vDir;
  
  double m_rmsDist;
  
  /// member function
  bool circleInsideIav(double, ROOT::Math::XYZVector);
  void connect2Points(ROOT::Math::XYZPoint, ROOT::Math::XYZPoint);
  double distance2Track(ROOT::Math::XYZPoint);
  void fitLeastSquaresLine();
  std::vector<std::pair<ROOT::Math::XYZPoint, ROOT::Math::XYZPoint> > getTrackOavIntersections(ROOT::Math::XYZPoint);
  std::vector<std::pair<ROOT::Math::XYZPoint, ROOT::Math::XYZPoint> > getTrackIavIntersections(ROOT::Math::XYZPoint);
  std::vector<OneCylinder> inscribeCylinder(ROOT::Math::XYZPoint, double);
  /// given any point, get the projected point on the track
  /// perpendicular to the track
  ROOT::Math::XYZPoint perpPoint(ROOT::Math::XYZPoint);
  double perpParameter(ROOT::Math::XYZPoint);
};


class OneMuonProgress
{
public:
  std::map<DetectorId::DetectorId_t, bool> m_detectorFinished;
  std::map<DetectorId::DetectorId_t, bool> m_interEventFinished;
  
  OneMuonProgress();
  bool muonFinished();
  bool interEventFinished();
};


class OneInterMuonBuffer
{
public:
  double               en;
  int                  tn;
  TimeStamp            ts;
  ROOT::Math::XYZPoint pRec;
};


class OneMuonBuffer
{
public:

  bool hitRpcArray, hitRpcTele;
  
  Site::Site_t site;
  std::vector<DetectorId::DetectorId_t> hitDet;
  std::map<DetectorId::DetectorId_t, double> eAd;
  std::map<DetectorId::DetectorId_t, int>    tn;
  std::map<DetectorId::DetectorId_t, TimeStamp> ts;
  /// reconstructed points of PMT based detectors
  std::map<DetectorId::DetectorId_t, ROOT::Math::XYZPoint> pPmtRecs;
  std::map<RpcHitLocation_t, ROOT::Math::XYZPoint> pRpcRecs;
  
  std::vector<OneTrack> trRpcOws;
  std::vector<int> nPtsComb; /// number of points used in the combined track
  /// intersections of the track with OAV
  std::map<DetectorId::DetectorId_t, std::vector<std::pair<ROOT::Math::XYZPoint, ROOT::Math::XYZPoint> > > endsOav;
  std::map<DetectorId::DetectorId_t, std::vector<std::pair<ROOT::Math::XYZPoint, ROOT::Math::XYZPoint> > > endsIav;
  /// the angle OAV_center-combined_track-AdSimple
  std::map<DetectorId::DetectorId_t, std::vector<double> > angCTRs;
  
  /// inter muon event data
  std::map<DetectorId::DetectorId_t, std::vector<OneInterMuonBuffer> > m_interEvents;
  std::map<DetectorId::DetectorId_t, std::vector<OneCylinder> > m_inscribedCylinder;
  
  /// member functions
  void makeTrack(double);
};


class MuonInducedNeutronTreeVars
{
public:
  int tMu_s;
  int tMu_ns;
  
  // hit condition and trigger numbers
  int hitAd1;
  int tnAd1[MAXARRAYSIZE];
  int hitAd2;
  int tnAd2[MAXARRAYSIZE];
  int hitAd3;
  int tnAd3[MAXARRAYSIZE];
  int hitAd4;
  int tnAd4[MAXARRAYSIZE];
  int hitIws;
  int tnIws[MAXARRAYSIZE];
  int hitOws;
  int tnOws[MAXARRAYSIZE];
  int hitRpc;
  int tnRpc[MAXARRAYSIZE];
  int hitRpcA; // array has hits
  int hitRpcT; // telescope has hits
  
  // number of hit detectors
  int nHitDets;
  
  // number of hit sensors
  int nHitsAd1;
  int nHitsAd2;
  int nHitsAd3;
  int nHitsAd4;
  int nHitsIws;
  int nHitsOws;
  int nHitsRpc;
  int nHitsRpcA; // number of cluster in RPC array
  int nHitsRpcT; // number of cluster in telescope RPC
  
  // number of valid reconstructed points
  int nRecAd1;
  int nRecAd2;
  int nRecAd3;
  int nRecAd4;
  int nRecIws;
  int nRecOws;
  int nRecRpcA;
  int nRecRpcT;
  
  // record AD muon deposited energy
  double eAd1[MAXARRAYSIZE];
  double eAd2[MAXARRAYSIZE];
  double eAd3[MAXARRAYSIZE];
  double eAd4[MAXARRAYSIZE];
  
  // record reconstructed points
  double xAd1[MAXARRAYSIZE];
  double xAd2[MAXARRAYSIZE];
  double xAd3[MAXARRAYSIZE];
  double xAd4[MAXARRAYSIZE];
  double xIws[MAXARRAYSIZE];
  double xOws[MAXARRAYSIZE];
  double xRpcA[MAXARRAYSIZE];
  double xRpcT[MAXARRAYSIZE];
  double yAd1[MAXARRAYSIZE];
  double yAd2[MAXARRAYSIZE];
  double yAd3[MAXARRAYSIZE];
  double yAd4[MAXARRAYSIZE];
  double yIws[MAXARRAYSIZE];
  double yOws[MAXARRAYSIZE];
  double yRpcA[MAXARRAYSIZE];
  double yRpcT[MAXARRAYSIZE];
  double zAd1[MAXARRAYSIZE];
  double zAd2[MAXARRAYSIZE];
  double zAd3[MAXARRAYSIZE];
  double zAd4[MAXARRAYSIZE];
  double zIws[MAXARRAYSIZE];
  double zOws[MAXARRAYSIZE];
  double zRpcA[MAXARRAYSIZE];
  double zRpcT[MAXARRAYSIZE];
  
  // if a combined track is able to form
  int nTracks;
  
  // muon track information
  // centroid of the reconstructed points: xCt[kind of track] Ct means centroid
  // direction angles
  
  int    nTrPts[MAXARRAYSIZE];
  double xCt[MAXARRAYSIZE];
  double yCt[MAXARRAYSIZE];
  double zCt[MAXARRAYSIZE];
  double theta[MAXARRAYSIZE];
  double phi[MAXARRAYSIZE];
  int    trIsFit;
  double rmsDist[MAXARRAYSIZE];
  
  /* inter muon event data starts */
  int    nIntEvtAd1;
  int    tnIntEvtAd1[MAXNINTEREVT];
  double eIntEvtAd1[MAXNINTEREVT];
  double dtIntEvtAd1[MAXNINTEREVT];
  double xIntEvtAd1[MAXNINTEREVT];
  double yIntEvtAd1[MAXNINTEREVT];
  double zIntEvtAd1[MAXNINTEREVT];
  int    nDistAd1;
  double dlIntEvt1[MAXNINTEREVT];
  bool   inCyl1[MAXNINTEREVT];
  
  int    nIntEvtAd2;
  int    tnIntEvtAd2[MAXNINTEREVT];
  double eIntEvtAd2[MAXNINTEREVT];
  double dtIntEvtAd2[MAXNINTEREVT];
  double xIntEvtAd2[MAXNINTEREVT];
  double yIntEvtAd2[MAXNINTEREVT];
  double zIntEvtAd2[MAXNINTEREVT];
  int    nDistAd2;
  double dlIntEvt2[MAXNINTEREVT];
  bool   inCyl2[MAXNINTEREVT];
  
  int    nIntEvtAd3;
  int    tnIntEvtAd3[MAXNINTEREVT];
  double eIntEvtAd3[MAXNINTEREVT];
  double dtIntEvtAd3[MAXNINTEREVT];
  double xIntEvtAd3[MAXNINTEREVT];
  double yIntEvtAd3[MAXNINTEREVT];
  double zIntEvtAd3[MAXNINTEREVT];
  int    nDistAd3;
  double dlIntEvt3[MAXNINTEREVT];
  bool   inCyl3[MAXNINTEREVT];
  
  int    nIntEvtAd4;
  int    tnIntEvtAd4[MAXNINTEREVT];
  double eIntEvtAd4[MAXNINTEREVT];
  double dtIntEvtAd4[MAXNINTEREVT];
  double xIntEvtAd4[MAXNINTEREVT];
  double yIntEvtAd4[MAXNINTEREVT];
  double zIntEvtAd4[MAXNINTEREVT];
  int    nDistAd4;
  double dlIntEvt4[MAXNINTEREVT];
  bool   inCyl4[MAXNINTEREVT];
  /* inter muon event data ends */
  
  // if the track goes throuth the OAV, record pertinent attributes
  int    passOav1;
  double xOav1In[MAXARRAYSIZE];
  double yOav1In[MAXARRAYSIZE];
  double zOav1In[MAXARRAYSIZE];
  double xOav1Out[MAXARRAYSIZE];
  double yOav1Out[MAXARRAYSIZE];
  double zOav1Out[MAXARRAYSIZE];
  double dlOav1[MAXARRAYSIZE];
  
  int    passOav2;
  double xOav2In[MAXARRAYSIZE];
  double yOav2In[MAXARRAYSIZE];
  double zOav2In[MAXARRAYSIZE];
  double xOav2Out[MAXARRAYSIZE];
  double yOav2Out[MAXARRAYSIZE];
  double zOav2Out[MAXARRAYSIZE];
  double dlOav2[MAXARRAYSIZE];
  
  int    passOav3;
  double xOav3In[MAXARRAYSIZE];
  double yOav3In[MAXARRAYSIZE];
  double zOav3In[MAXARRAYSIZE];
  double xOav3Out[MAXARRAYSIZE];
  double yOav3Out[MAXARRAYSIZE];
  double zOav3Out[MAXARRAYSIZE];
  double dlOav3[MAXARRAYSIZE];
  
  int    passOav4;
  double xOav4In[MAXARRAYSIZE];
  double yOav4In[MAXARRAYSIZE];
  double zOav4In[MAXARRAYSIZE];
  double xOav4Out[MAXARRAYSIZE];
  double yOav4Out[MAXARRAYSIZE];
  double zOav4Out[MAXARRAYSIZE];
  double dlOav4[MAXARRAYSIZE];
  
  // if the track goes throuth the IAV, record pertinent attributes
  int    passIav1;
  double xIav1In[MAXARRAYSIZE];
  double yIav1In[MAXARRAYSIZE];
  double zIav1In[MAXARRAYSIZE];
  double xIav1Out[MAXARRAYSIZE];
  double yIav1Out[MAXARRAYSIZE];
  double zIav1Out[MAXARRAYSIZE];
  double dlIav1[MAXARRAYSIZE];
  
  int    passIav2;
  double xIav2In[MAXARRAYSIZE];
  double yIav2In[MAXARRAYSIZE];
  double zIav2In[MAXARRAYSIZE];
  double xIav2Out[MAXARRAYSIZE];
  double yIav2Out[MAXARRAYSIZE];
  double zIav2Out[MAXARRAYSIZE];
  double dlIav2[MAXARRAYSIZE];
  
  int    passIav3;
  double xIav3In[MAXARRAYSIZE];
  double yIav3In[MAXARRAYSIZE];
  double zIav3In[MAXARRAYSIZE];
  double xIav3Out[MAXARRAYSIZE];
  double yIav3Out[MAXARRAYSIZE];
  double zIav3Out[MAXARRAYSIZE];
  double dlIav3[MAXARRAYSIZE];
  
  int    passIav4;
  double xIav4In[MAXARRAYSIZE];
  double yIav4In[MAXARRAYSIZE];
  double zIav4In[MAXARRAYSIZE];
  double xIav4Out[MAXARRAYSIZE];
  double yIav4Out[MAXARRAYSIZE];
  double zIav4Out[MAXARRAYSIZE];
  double dlIav4[MAXARRAYSIZE];
  
  int    inscribable1;
  double xCyl1Up[MAXARRAYSIZE];
  double yCyl1Up[MAXARRAYSIZE];
  double zCyl1Up[MAXARRAYSIZE];
  double xCyl1Bot[MAXARRAYSIZE];
  double yCyl1Bot[MAXARRAYSIZE];
  double zCyl1Bot[MAXARRAYSIZE];
  double tCyl1Up[MAXARRAYSIZE];
  double tCyl1Bot[MAXARRAYSIZE];
  
  int    inscribable2;
  double xCyl2Up[MAXARRAYSIZE];
  double yCyl2Up[MAXARRAYSIZE];
  double zCyl2Up[MAXARRAYSIZE];
  double xCyl2Bot[MAXARRAYSIZE];
  double yCyl2Bot[MAXARRAYSIZE];
  double zCyl2Bot[MAXARRAYSIZE];
  double tCyl2Up[MAXARRAYSIZE];
  double tCyl2Bot[MAXARRAYSIZE];
  
  int    inscribable3;
  double xCyl3Up[MAXARRAYSIZE];
  double yCyl3Up[MAXARRAYSIZE];
  double zCyl3Up[MAXARRAYSIZE];
  double xCyl3Bot[MAXARRAYSIZE];
  double yCyl3Bot[MAXARRAYSIZE];
  double zCyl3Bot[MAXARRAYSIZE];
  double tCyl3Up[MAXARRAYSIZE];
  double tCyl3Bot[MAXARRAYSIZE];
  
  int    inscribable4;
  double xCyl4Up[MAXARRAYSIZE];
  double yCyl4Up[MAXARRAYSIZE];
  double zCyl4Up[MAXARRAYSIZE];
  double xCyl4Bot[MAXARRAYSIZE];
  double yCyl4Bot[MAXARRAYSIZE];
  double zCyl4Bot[MAXARRAYSIZE];
  double tCyl4Up[MAXARRAYSIZE];
  double tCyl4Bot[MAXARRAYSIZE];
  
  // goodness of the AdSimple as a muon tracking point
  int    ad1AndCb;
  int    ad2AndCb;
  int    ad3AndCb;
  int    ad4AndCb;
  // the angle OAV_center-combined_track-AdSimple
  double angCTR1[MAXARRAYSIZE];
  double angCTR2[MAXARRAYSIZE];
  double angCTR3[MAXARRAYSIZE];
  double angCTR4[MAXARRAYSIZE];
};


class MuonInducedNeutronAlg : public GaudiAlgorithm
{
public:
  /// Constructor has to be in this form
  MuonInducedNeutronAlg(const std::string&, ISvcLocator*);
  virtual ~MuonInducedNeutronAlg(){};
  
  /// Three mandatory member functions of any algorithm
  StatusCode initialize();
  StatusCode execute();
  StatusCode finalize();
  
private:

  /// variables
  std::string m_infilename;
  std::string m_outfilename;
  double      m_rTrackCyl;
  // progress counter
  int exeCntr;
  
  TFile* m_infile;
  TTree* m_treeSpal;
  
  std::map<triggerId, std::string> m_muonLookupTable;
  std::map<std::string, OneMuonProgress> m_muonProgressTable;
  std::map<std::string, OneMuonBuffer> m_muonDataBuffer;
  std::map<DetectorId::DetectorId_t, std::string> m_lastMuonId;
  
  // output variables
  TFile* m_outfile;
  TTree* m_treeRO;
  MuonInducedNeutronTreeVars m_trvarRO;
  
  // geometry information
  std::map<Site::Site_t, std::map<DetectorId::DetectorId_t, std::string> > m_referenceDetector;
  std::map<Site::Site_t, ROOT::Math::XYZPoint> m_rpcRecOffset;
  std::map<Site::Site_t, std::map<DetectorId::DetectorId_t, ROOT::Math::XYZPoint> > m_oavCtr;
  std::map<Site::Site_t, std::map<DetectorId::DetectorId_t, ROOT::Math::XYZPoint> > m_iavCtr;
  
  /// methods
  void assignBranches(TTree*, MuonInducedNeutronTreeVars&);
  OneTrack connect2Points(ROOT::Math::XYZPoint, ROOT::Math::XYZPoint);
  void fillOutputTree(std::string, MuonInducedNeutronTreeVars&, TTree*);
  std::vector<double> getAngCTR(Site::Site_t, DetectorId::DetectorId_t, OneTrack, ROOT::Math::XYZPoint);
  std::string makeMuonId(int, int);
  bool isFlasher();
  bool wsPositionValid(ROOT::Math::XYZPoint);
};


#endif
