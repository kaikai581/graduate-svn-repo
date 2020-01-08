#ifndef MYDATAMODEL_HPP
#define MYDATAMODEL_HPP


#include "Conventions/DetectorId.h"
#include "Context/TimeStamp.h"


namespace MyDataModel
{
  /// first is trigger number, second is detectorId
  typedef std::pair<unsigned int, DetectorId::DetectorId_t> eventId;
  
  
  class AdTrigger
  {
  public:
    int          site;
    eventId      id;
    TimeStamp    ts;
    float        energy;
    int          energyStatus;
    unsigned int nHits;
    std::vector<double> timeAD;
    bool         isFlasher;
    float        x;
    float        y;
    float        z;
    int          positionStatus;
    float        xl; // local coordinates
    float        yl;
    float        zl;
    int          zone; // zone 1: GdLS 2: LS 3: MO
  };
  
  
  class WpTrigger
  {
  public:
    eventId      id;
    TimeStamp    ts;
    unsigned int nHits;
    std::vector<Gaudi::XYZPoint> pmtPositions;
    std::vector<Gaudi::XYZPoint> pmtLocalPositions;
    std::vector<double>          firstHitTime; // the earliest TDC hit time of a PMT
    std::vector<double>          firstHitCharge; // the earliest TDC hit charge of a PMT
    std::vector<double>          pmtCharge;      // total charge of a PMT in a trigger
  };
  
  
  class RpcTrigger
  {
  public:
    eventId      id;
    TimeStamp    ts;
    unsigned int nRecPts;
    float        x;
    float        y;
    float        z;
    float        xl; // local coordinates
    float        yl;
    float        zl;
    int          nTracks;
    std::vector<float> tr_xl; // track information
    std::vector<float> tr_yl;
    std::vector<float> tr_zl;
  };
  
  
  class DecayMuon
  {
  public:
    AdTrigger              prompt;
    std::vector<AdTrigger> delayed;
  };
  
  
  class MuonVeto
  {
  public:
    std::vector<WpTrigger>  iwsTrig;
    std::vector<WpTrigger>  owsTrig;
    std::vector<RpcTrigger> rpcTrig;
  };
  
  
  /// look for muon information in other detectors with AD muon as the key
  typedef std::map<eventId, MuonVeto> MuonLookup;
}


#endif