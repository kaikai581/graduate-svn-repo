#ifndef MUONDATA_HPP
#define MUONDATA_HPP

#include <map>
#include <vector>
#include "Context/TimeStamp.h"
#include "Conventions/Detectors.h"
#include "Math/Point3D.h"
#include "Math/Vector3D.h"


typedef std::pair<ROOT::Math::XYZPoint, ROOT::Math::XYZVector> singleTrack;

class AdData
{
public:
  int       detector;
  int       triggerNumber;
  int       dtAdMuNanoSec;
  TimeStamp triggerTime;
  double energy;
  ROOT::Math::XYZPoint position;
};


class WpData
{
public:
  int detector;
  int triggerNumber;
};


class SingleMuonData
{
public:
  SingleMuonData(std::vector<DetectorId::DetectorId_t>);
  SingleMuonData() {};
  ~SingleMuonData() {};

public:
  std::map<DetectorId::DetectorId_t, int> m_triggerId;
  std::map<DetectorId::DetectorId_t, bool> m_detectorFinished;
  std::map<DetectorId::DetectorId_t, AdData> m_adMuon;
  std::map<DetectorId::DetectorId_t, std::vector<AdData> > m_adNeutron;
  std::vector<DetectorId::DetectorId_t> m_hitMap;

};


class SingleMuonProgress
{
public:
  std::map<DetectorId::DetectorId_t, bool> m_detectorFinished;
  
  SingleMuonProgress();
  bool muonFinished();
};


class SingleMuonBuffer
{
public:
  bool hitRpcArray, hitRpcTele;
  
  Site::Site_t site;
  std::vector<DetectorId::DetectorId_t> hitDet;
  std::map<DetectorId::DetectorId_t, double> eAd;
  std::map<DetectorId::DetectorId_t, std::vector<ROOT::Math::XYZPoint> > pDetRec;
  
  std::vector<singleTrack> tTeleRpc;
  std::vector<singleTrack> tAdRpc;
  std::vector<singleTrack> tComb;
  std::vector<double> rmsDistComb; /// RMS of the minimum value of the merit function
  std::vector<int> nPtsComb; // number of points used in the combined track
  // intersections of the AD-RPC track with OAV
  std::map<DetectorId::DetectorId_t, std::vector<std::pair<ROOT::Math::XYZPoint, ROOT::Math::XYZPoint> > > endsOavAR;
  // intersections of the combined track with OAV
  std::map<DetectorId::DetectorId_t, std::vector<std::pair<ROOT::Math::XYZPoint, ROOT::Math::XYZPoint> > > endsOavCb;
  // the angle OAV_center-combined_track-AdSimple
  std::map<DetectorId::DetectorId_t, std::vector<double> > angCTRs;
  
  /// member functions
  unsigned int nPoints(DetectorId::DetectorId_t);
};

#endif
