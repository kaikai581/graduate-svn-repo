// second try: store Michel candidates in a variable length array
/*
 * For the first attempt, record only the Michel event closest in time to the
 * muon hit and discard all the others.
 */

#ifndef TREEVARIABLES_HPP
#define TREEVARIABLES_HPP
#define MAXNMICHEL 100
#define MAXNPMT    1000
#define MAXNREC    1


class TreeVariables
{
public:
  int          site;
  int          detector;
  
  // AD muon data
  int          tAdMuSec;
  int          tAdMuNanoSec;
  double       eMu;
  double       xMu;
  double       yMu;
  double       zMu;
  double       xlMu; // local coordinate
  double       ylMu;
  double       zlMu;
  double       rho2Mu; // (x-x0)^2 + (y-y0)^2
  // connect a line from AD reconstruction point to RPC reconstruction point
  // ask the theta and phi angles of this vector
  double       muTheta;
  double       muPhi;
  int          triggerNumberMu;
  
  // AD Michel electron data
  int          nMi;
  int          tAdMiSec[MAXNMICHEL];
  int          tAdMiNanoSec[MAXNMICHEL];
  double       eMi[MAXNMICHEL];
  double       dtMuMi[MAXNMICHEL];
  double       dlMuMi[MAXNMICHEL];
  double       xMi[MAXNMICHEL];
  double       yMi[MAXNMICHEL];
  double       zMi[MAXNMICHEL];
  double       rho2Mi[MAXNMICHEL];
  int          triggerNumberMi[MAXNMICHEL];
  int          nMaxPmtMi[MAXNMICHEL]; // maximum number of PMTs getting hit in a time bin inside the readout window
  double       tMaxPmtMi[MAXNMICHEL]; // the time when the above happens
  int          isRetrigger[MAXNMICHEL]; // retrigger tag in the official production file
  int          miZone[MAXNMICHEL];    // zone 1: GdLS 2: LS 3: MO
  
  // WP muon data
  int          hitIws;
  int          nHitsIws;
  int          triggerNumberIws;
  double       xIws[MAXNPMT];
  double       yIws[MAXNPMT];
  double       zIws[MAXNPMT];
  double       xlIws[MAXNPMT]; // local coordinate
  double       ylIws[MAXNPMT];
  double       zlIws[MAXNPMT];
  double       firstHitTimeIws[MAXNPMT]; // the earliest TDC hit time of a PMT
  double       firstHitChargeIws[MAXNPMT]; // the earliest TDC hit charge of a PMT
  double       totChargeIws[MAXNPMT]; // total charge of a PMT
  int          hitOws;
  int          nHitsOws;
  int          triggerNumberOws;
  double       xOws[MAXNPMT];
  double       yOws[MAXNPMT];
  double       zOws[MAXNPMT];
  double       xlOws[MAXNPMT]; // local coordinate
  double       ylOws[MAXNPMT];
  double       zlOws[MAXNPMT];
  double       firstHitTimeOws[MAXNPMT]; // the earliest TDC hit time of a PMT
  double       firstHitChargeOws[MAXNPMT]; // the earliest TDC hit charge of a PMT
  double       totChargeOws[MAXNPMT]; // total charge of a PMT
  double       xlIwsBot; // bottom only PMT reconstruction
  double       ylIwsBot;
  double       zlIwsBot;
  double       xlOwsBot; // bottom only PMT reconstruction
  double       ylOwsBot;
  double       zlOwsBot;
  
  // RPC muon data
  int          hitRpc;
  int          nHitsRpc;
  int          nRpcMi; // number of RPC-Michel pairs
  double       dtRpcMi[MAXNMICHEL];
  double       dlRpcMi[MAXNMICHEL];
  double       xRpc;
  double       yRpc;
  double       zRpc;
  int          triggerNumberRpc;
  double       xlRpc; // local coordinate
  double       ylRpc;
  double       zlRpc;
  /// track info
  int          nTracks;
  double       tele_xl;
  double       tele_yl;
  double       tele_zl;
  double       arr_xl;
  double       arr_yl;
  double       arr_zl;
  
  // muon track info from RPC array + AD
  // straight line equation: p = r + t*(a-r)
  // where p is a point in the line, r is the RPC position and
  // a is the AD position.
  double       tRim[MAXNMICHEL];
  double       xRim[MAXNMICHEL];
  double       yRim[MAXNMICHEL];
  double       zRim[MAXNMICHEL];
  double       tRimOut[MAXNMICHEL];
  double       dlRimMi[MAXNMICHEL]; // distance from the intersection to the electrons
  int          inPosMi[MAXNMICHEL];
  int          outPosMi[MAXNMICHEL];
  
  // muon track info
  // between RPC and muon reconstructed positions
  double       dtRpcMu;
  double       dlRpcMu;
  double       dlRimMu;
  double       dlMuRim;
  double       tRimMu;
  double       xRimMu;
  double       yRimMu;
  double       zRimMu;
  double       tRimMuOut;
  double       xRimMuOut;
  double       yRimMuOut;
  double       zRimMuOut;
  /// position of the intersection of the muon track and the OAV
  /// 0 means through the lid or bottom
  /// 1 means through the rim
  int          inPosMu;
  int          outPosMu;
  double       xlBot; // extrapolation of track to the bottom of the pool
  double       ylBot;
  double       zlBot;
  double       dIwsBot; // if projecting WS reconstructed point to the bottom,
  double       dOwsBot; // what is the distance between the RPC-WS and RPC-AD intersection with the bottom of the pool
  
  // general accounting
  int          nNeutron; // number of neutron candidates in the after-muon window
  int          nNeutronOfficial; // number of neutrons directly taken from the production files
  int          nNeutronTight; // same as nNeutron but with retrigger cut
  int          nNeutronTight2; // same as nNeutron but with official retrigger cut
  int          nAfterPulse; // number of after pulse candidates in the after-muon window
  
  // neutron properties
  double       dlNeutron[MAXNMICHEL]; // Distance from the neutron to the muon track. Array size: nNeutron
  double       zNeutron[MAXNMICHEL];  // Distance along the track. Origin at the incident point.
  double       zDepth[MAXNMICHEL];    // zNeutron/(muon track length)
  int          nZone[MAXNMICHEL];     // zone 1: GdLS 2: LS 3: MO
};


class SingleWsRecTreeVars
{
public:
  double xRpc;
  double yRpc;
  double zRpc;
  double xWp;
  double yWp;
  double zWp;
  int    nHit1400;
  int    nHit1500;
  double xWp1400; // TDC cut on hits < -1400 ns
  double yWp1400;
  double zWp1400;
  double xWp1500; // TDC cut on hits < -1500 ns
  double yWp1500;
  double zWp1500;
  int    detector; // denote which AD is hit
  double thetaAd;
  double phiAd;
  double thetaWp;
  double phiWp;
  double angleBetween; // angle between 2 tracks
  double angleBetween1400; // angle between 2 tracks with 1400 TDC cut
  double angleBetween1500; // angle between 2 tracks with 1500 TDC cut
};


/// compare RPC-AD reconstruction with RPC telescope-array reconstruction
class TrackComp
{
public:
  double cosAdTele; // cos theta between RPC-AD and RPC-Telescope tracks
};


#endif
