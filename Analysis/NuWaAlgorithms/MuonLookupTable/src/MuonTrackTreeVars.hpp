#ifndef MUONTRACKTREEVARS_HPP
#define MUONTRACKTREEVARS_HPP

#define MAXARRAYSIZE 10

class MuonTrackTreeVars
{
public:
  int tMu_s;
  int tMu_ns;
  
  // hit condition
  bool hitAd1;
  bool hitAd2;
  bool hitAd3;
  bool hitAd4;
  bool hitIws;
  bool hitOws;
  bool hitRpc;
  bool hitRpcA; // array has hits
  bool hitRpcT; // telescope has hits
  
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
  int nAdRpc;
  int nTelRpc;
  int nComb; // combined track, i.e. fitting a line with all available points
  
  // muon track information
  // centroid of the reconstructed points: xCt[kind of track] Ct means centroid
  // direction angles
  double xCtAR[MAXARRAYSIZE];
  double yCtAR[MAXARRAYSIZE];
  double zCtAR[MAXARRAYSIZE];
  double thetaAR[MAXARRAYSIZE];
  double phiAR[MAXARRAYSIZE];
  
  double xCtTR[MAXARRAYSIZE];
  double yCtTR[MAXARRAYSIZE];
  double zCtTR[MAXARRAYSIZE];
  double thetaTR[MAXARRAYSIZE];
  double phiTR[MAXARRAYSIZE];
  
  int    nPtsCb[MAXARRAYSIZE];
  double xCtCb[MAXARRAYSIZE];
  double yCtCb[MAXARRAYSIZE];
  double zCtCb[MAXARRAYSIZE];
  double thetaCb[MAXARRAYSIZE];
  double phiCb[MAXARRAYSIZE];
  double rmsDistCb[MAXARRAYSIZE];
  
  // if the AD-RPC track goes throuth the OAV, record pertinent attributes
  int    passOav1AR;
  double xOav1InAR[MAXARRAYSIZE];
  double yOav1InAR[MAXARRAYSIZE];
  double zOav1InAR[MAXARRAYSIZE];
  double xOav1OutAR[MAXARRAYSIZE];
  double yOav1OutAR[MAXARRAYSIZE];
  double zOav1OutAR[MAXARRAYSIZE];
  double dlOav1AR[MAXARRAYSIZE];
  int    passOav2AR;
  double xOav2InAR[MAXARRAYSIZE];
  double yOav2InAR[MAXARRAYSIZE];
  double zOav2InAR[MAXARRAYSIZE];
  double xOav2OutAR[MAXARRAYSIZE];
  double yOav2OutAR[MAXARRAYSIZE];
  double zOav2OutAR[MAXARRAYSIZE];
  double dlOav2AR[MAXARRAYSIZE];
  int    passOav3AR;
  double xOav3InAR[MAXARRAYSIZE];
  double yOav3InAR[MAXARRAYSIZE];
  double zOav3InAR[MAXARRAYSIZE];
  double xOav3OutAR[MAXARRAYSIZE];
  double yOav3OutAR[MAXARRAYSIZE];
  double zOav3OutAR[MAXARRAYSIZE];
  double dlOav3AR[MAXARRAYSIZE];
  int    passOav4AR;
  double xOav4InAR[MAXARRAYSIZE];
  double yOav4InAR[MAXARRAYSIZE];
  double zOav4InAR[MAXARRAYSIZE];
  double xOav4OutAR[MAXARRAYSIZE];
  double yOav4OutAR[MAXARRAYSIZE];
  double zOav4OutAR[MAXARRAYSIZE];
  double dlOav4AR[MAXARRAYSIZE];
  
  // if the combined track goes throuth the OAV, record pertinent attributes
  int    passOav1Cb;
  double xOav1InCb[MAXARRAYSIZE];
  double yOav1InCb[MAXARRAYSIZE];
  double zOav1InCb[MAXARRAYSIZE];
  double xOav1OutCb[MAXARRAYSIZE];
  double yOav1OutCb[MAXARRAYSIZE];
  double zOav1OutCb[MAXARRAYSIZE];
  double dlOav1Cb[MAXARRAYSIZE];
  int    passOav2Cb;
  double xOav2InCb[MAXARRAYSIZE];
  double yOav2InCb[MAXARRAYSIZE];
  double zOav2InCb[MAXARRAYSIZE];
  double xOav2OutCb[MAXARRAYSIZE];
  double yOav2OutCb[MAXARRAYSIZE];
  double zOav2OutCb[MAXARRAYSIZE];
  double dlOav2Cb[MAXARRAYSIZE];
  int    passOav3Cb;
  double xOav3InCb[MAXARRAYSIZE];
  double yOav3InCb[MAXARRAYSIZE];
  double zOav3InCb[MAXARRAYSIZE];
  double xOav3OutCb[MAXARRAYSIZE];
  double yOav3OutCb[MAXARRAYSIZE];
  double zOav3OutCb[MAXARRAYSIZE];
  double dlOav3Cb[MAXARRAYSIZE];
  int    passOav4Cb;
  double xOav4InCb[MAXARRAYSIZE];
  double yOav4InCb[MAXARRAYSIZE];
  double zOav4InCb[MAXARRAYSIZE];
  double xOav4OutCb[MAXARRAYSIZE];
  double yOav4OutCb[MAXARRAYSIZE];
  double zOav4OutCb[MAXARRAYSIZE];
  double dlOav4Cb[MAXARRAYSIZE];
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




#endif
