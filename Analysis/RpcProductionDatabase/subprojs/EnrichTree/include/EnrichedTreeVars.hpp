#ifndef ENRICHEDTREEVARS_HPP
#define ENRICHEDTREEVARS_HPP
#include <map>
#include <vector>


class EnrichedTreeVars
{
public:
  /// variables for holding old tree variables
  unsigned int m_triggerTimeSec;
  unsigned int m_triggerTimeNanoSec;
  std::vector<bool>* m_mRpcFromRot;
  std::vector<unsigned int>* m_mRpcRow;
  std::vector<unsigned int>* m_mRpcColumn;
  std::vector<std::vector<unsigned int> >* m_mRpcLayer;
  std::vector<std::vector<std::vector<unsigned int> > >* m_mRpcStrip;
  unsigned int m_nModules;
  std::vector<unsigned int>* m_mNLayers;
  std::vector<unsigned int>* m_mNStrips;
  std::vector<std::vector<unsigned int> >* m_nStrips;
  
  /// new variables
  std::vector<std::vector<bool> > m_hasLayer;
  std::vector<std::vector<unsigned int> > m_layer2Idx;
  std::vector<std::vector<double> >* m_lStripCms;
  std::vector<double>*               m_mXCmsDist;
  std::vector<double>*               m_mYCmsDist;
  
  /// consecutive strips form cluster hits
  std::vector<std::vector<unsigned int> >* m_mNClusters;
  std::vector<std::vector<std::vector<unsigned int> > >* m_mClusterSize;
  std::vector<unsigned int>* m_mMaxClusterSizeX;
  std::vector<unsigned int>* m_mMaxClusterSizeY;
  std::vector<std::vector<std::vector<std::vector<unsigned int> > > >*
                                     m_mClusterHits;


  /// A track is formed if the intersection of two x or y cluster hits
  /// is non-empty.
  /// mTrackX1, mTrackY2, mTrackY3, mTrackX4 have dimension number of tracks
  /// and store the index of the cluster hits
  std::vector<unsigned int>* m_mNTracksX;
  std::vector<unsigned int>* m_mNTracksY;
  std::vector<std::vector<unsigned int> >* m_mTrackClusterIdxX1;
  std::vector<std::vector<unsigned int> >* m_mTrackClusterIdxY2;
  std::vector<std::vector<unsigned int> >* m_mTrackClusterIdxY3;
  std::vector<std::vector<unsigned int> >* m_mTrackClusterIdxX4;
  
  std::vector<unsigned int>* m_mNSpuriousX;
  std::vector<unsigned int>* m_mNSpuriousY;
  std::vector<std::vector<unsigned int> >* m_mSpuriousClusterIdxX1;
  std::vector<std::vector<unsigned int> >* m_mSpuriousClusterIdxY2;
  std::vector<std::vector<unsigned int> >* m_mSpuriousClusterIdxY3;
  std::vector<std::vector<unsigned int> >* m_mSpuriousClusterIdxX4;
  
  /// normal track container
  //std::vector<std::vector<unsigned int> > m_mNormalTrack;
  /// strip distance to the muon track
  std::vector<std::vector<int> >* m_mDx;
  std::vector<std::vector<int> >* m_mDy;
  /// number of dx and dy calculated
  std::vector<unsigned int>* m_mNDx;
  std::vector<unsigned int>* m_mNDy;
  /// distance to the speculated muon track
  std::vector<std::vector<int> >* m_mDxMuon;
  std::vector<std::vector<int> >* m_mDyMuon;
  /// 5 strips events. their distance to the speculated muon track
  /// and their real position in [0..31] representation
  std::vector<int>* m_dSingleMuon;
  std::vector<std::vector<unsigned int> > m_dSingleMuonH; // same variable for a histogram
  std::map<std::pair<unsigned int, unsigned int>, std::vector<unsigned int> >
    m_dSingleMuonH2;
  std::map<std::pair<unsigned int, unsigned int>, std::vector<int> >
    m_dSingleMuonSigned;
  std::vector<int>* m_singleStripId32;
  /// container for storing cluster size for all events in a dataset
  std::map<std::pair<unsigned int, unsigned int>, std::vector<unsigned int> >
    m_clusterSizeDataSet;
  
  
  EnrichedTreeVars();
  ~EnrichedTreeVars();
  
  void getHitLayersAndReverseIndex();
  void getStripCms();
  void getCmsDist();
  void getClusterHits();
  void getTracks();
  void getDistance();
};

#endif
