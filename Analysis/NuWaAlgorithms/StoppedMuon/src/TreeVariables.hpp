// second try: store Michel candidates in a variable length array
/*
 * For the first attempt, record only the Michel event closest in time to the
 * muon hit and discard all the others.
 */

#ifndef TREEVARIABLES_HPP
#define TREEVARIABLES_HPP
#define MAXNMICHEL 100
#define MAXNPMT    1000


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
  bool         inscribable;
  double       cylLen; // length of each virtual cylinder
  double       zInsCylTopTrackCoord;
  double       zInsCylBotTrackCoord;
  double       xInsCylTopIavCoord, xInsCylBotIavCoord;
  double       yInsCylTopIavCoord, yInsCylBotIavCoord;
  double       zInsCylTopIavCoord, zInsCylBotIavCoord;
  
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
  double       xlMi[MAXNMICHEL];
  double       ylMi[MAXNMICHEL];
  double       zlMi[MAXNMICHEL];
  double       rho2Mi[MAXNMICHEL];
  int          triggerNumberMi[MAXNMICHEL];
  int          nMaxPmtMi[MAXNMICHEL]; // maximum number of PMTs getting hit in a time bin inside the readout window
  double       tMaxPmtMi[MAXNMICHEL]; // the time when the above happens
  int          isRetrigger[MAXNMICHEL]; // retrigger tag in the official production file
  int          miZone[MAXNMICHEL];    // zone 1: GdLS 2: LS 3: MO
  int          inCyl[MAXNMICHEL]; // whether the event is inside a virtual cylinder drawn around the muon track
  double       zTrack[MAXNMICHEL]; // z coordinate of the event with respect to the center of the inscribing cylinder
  double       rhoTrack[MAXNMICHEL]; // closest approach of the event to the axis of the inscribing cylinder
  double       phiTrack[MAXNMICHEL]; // azimuthal angle of the event to the axis of the inscribing cylinder
  
  // background events: (1000+start,1000+end) microseconds after a muon
  int          nBkg;
  double       eBkg[MAXNMICHEL];
  
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
  
  // muon track info
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
  double       dlIav; // muon track length in IAV
  double       xIavIn;
  double       yIavIn;
  double       zIavIn;
  double       xIavOut;
  double       yIavOut;
  double       zIavOut;
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




#endif
