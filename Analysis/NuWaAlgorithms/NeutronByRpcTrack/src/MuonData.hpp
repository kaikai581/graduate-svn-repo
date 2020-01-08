#ifndef MUONDATA_HPP
#define MUONDATA_HPP

#include <map>
#include <vector>
#include "Context/TimeStamp.h"
#include "Conventions/Detectors.h"
#include "Math/Point3D.h"


class AdData
{
public:
  int       triggerNumber;
  int       dtAdMuNanoSec;
  TimeStamp triggerTime;
  double energy;
  ROOT::Math::XYZPoint position;
};


class MuonData
{
public:
  MuonData(std::vector<DetectorId::DetectorId_t>);
  MuonData() {};
  ~MuonData() {};
  bool allDetectorsFinished();
  bool includeAd();

public:
  std::map<DetectorId::DetectorId_t, int> m_triggerId;
  std::map<DetectorId::DetectorId_t, bool> m_detectorFinished;
  std::map<DetectorId::DetectorId_t, AdData> m_adMuon;
  std::map<DetectorId::DetectorId_t, std::vector<AdData> > m_adNeutron;
  std::vector<DetectorId::DetectorId_t> m_hitMap;

private:
  //std::vector<DetectorId::DetectorId_t> m_hitMap;

};


class MuonLookupStruct
{
public:
  std::string m_muonId;
  std::vector<std::pair<DetectorId::DetectorId_t, int> > m_hitMap;
};


#endif
