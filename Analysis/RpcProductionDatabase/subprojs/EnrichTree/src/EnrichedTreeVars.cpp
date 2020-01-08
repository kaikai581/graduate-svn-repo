#include <algorithm> /// for set intersection
#include <iostream>
#include <cmath>
#include "EnrichedTreeVars.hpp"


using namespace std;



EnrichedTreeVars::EnrichedTreeVars()
{
  m_mRpcFromRot = new vector<bool>;
  m_mRpcRow = new vector<unsigned int>;
  m_mRpcColumn = new vector<unsigned int>;
  m_mRpcLayer = new vector<vector<unsigned int> >;
  m_mRpcStrip = new vector<vector<vector<unsigned int> > >;
  m_mNLayers = new vector<unsigned int>;
  m_mNStrips = new vector<unsigned int>;
  m_nStrips = new vector<vector<unsigned int> >;
  m_lStripCms = new vector<vector<double> >;
  m_mXCmsDist = new vector<double>;
  m_mYCmsDist = new vector<double>;
  m_mNClusters = new vector<vector<unsigned int> >;
  m_mClusterSize = new vector<vector<vector<unsigned int> > >;
  m_mMaxClusterSizeX = new vector<unsigned int>;
  m_mMaxClusterSizeY = new vector<unsigned int>;
  m_mClusterHits = new vector<vector<vector<vector<unsigned int> > > >;
  m_mNTracksX = new vector<unsigned int>;
  m_mNTracksY = new vector<unsigned int>;
  m_mTrackClusterIdxX1 = new vector<vector<unsigned int> >;
  m_mTrackClusterIdxY2 = new vector<vector<unsigned int> >;
  m_mTrackClusterIdxY3 = new vector<vector<unsigned int> >;
  m_mTrackClusterIdxX4 = new vector<vector<unsigned int> >;
  m_mNSpuriousX = new vector<unsigned int>;
  m_mNSpuriousY = new vector<unsigned int>;
  m_mSpuriousClusterIdxX1 = new vector<vector<unsigned int> >;
  m_mSpuriousClusterIdxY2 = new vector<vector<unsigned int> >;
  m_mSpuriousClusterIdxY3 = new vector<vector<unsigned int> >;
  m_mSpuriousClusterIdxX4 = new vector<vector<unsigned int> >;
  m_mDx = new vector<vector<int> >;
  m_mDy = new vector<vector<int> >;
  m_mNDx = new vector<unsigned int>;
  m_mNDy = new vector<unsigned int>;
  m_mDxMuon = new vector<vector<int> >;
  m_mDyMuon = new vector<vector<int> >;
  m_dSingleMuon = new vector<int>;
  m_singleStripId32 = new vector<int>;
}



EnrichedTreeVars::~EnrichedTreeVars()
{
  delete m_mRpcFromRot;
  delete m_mRpcRow;
  delete m_mRpcColumn;
  delete m_mRpcLayer;
  delete m_mRpcStrip;
  delete m_mNLayers;
  delete m_mNStrips;
  delete m_nStrips;
  delete m_lStripCms;
  delete m_mXCmsDist;
  delete m_mYCmsDist;
  delete m_mNClusters;
  delete m_mClusterSize;
  delete m_mMaxClusterSizeX;
  delete m_mMaxClusterSizeY;
  delete m_mClusterHits;
  delete m_mNTracksX;
  delete m_mNTracksY;
  delete m_mTrackClusterIdxX1;
  delete m_mTrackClusterIdxY2;
  delete m_mTrackClusterIdxY3;
  delete m_mTrackClusterIdxX4;
  delete m_mNSpuriousX;
  delete m_mNSpuriousY;
  delete m_mSpuriousClusterIdxX1;
  delete m_mSpuriousClusterIdxY2;
  delete m_mSpuriousClusterIdxY3;
  delete m_mSpuriousClusterIdxX4;
  delete m_mDx;
  delete m_mDy;
  delete m_mNDx;
  delete m_mNDy;
  delete m_mDxMuon;
  delete m_mDyMuon;
  delete m_dSingleMuon;
  delete m_singleStripId32;
}



void EnrichedTreeVars::getHitLayersAndReverseIndex()
{
  m_hasLayer.clear();
  m_layer2Idx.clear();
  for(unsigned int m = 0; m < m_nModules; m++)
  {
    vector<bool> hasLayer(4, false);
    vector<unsigned int> layer2Idx(4);
    for(unsigned int l = 0; l < m_mNLayers->at(m); l++)
    {
      layer2Idx[(*m_mRpcLayer)[m][l]-1] = l;
      hasLayer[(*m_mRpcLayer)[m][l]-1] = true;
    }
    m_hasLayer.push_back(hasLayer);
    m_layer2Idx.push_back(layer2Idx);
  }
}



void EnrichedTreeVars::getStripCms()
{
  m_lStripCms->clear();
  for(unsigned int m = 0; m < m_nModules; m++)
  {
    unsigned int nLayers = m_mNLayers->at(m);
    vector<double> cmsThisLayer(nLayers, 0.);
    for(unsigned int l = 0; l < nLayers; l++)
    {
      unsigned int nStrips = (*m_nStrips)[m][l];
      for(unsigned int s = 0; s < nStrips; s++)
        cmsThisLayer[l] += (*m_mRpcStrip)[m][l][s];
      cmsThisLayer[l] /= (double)nStrips;
    }
    m_lStripCms->push_back(cmsThisLayer);
  }
}



void EnrichedTreeVars::getCmsDist()
{
  m_mXCmsDist->clear();
  m_mYCmsDist->clear();
  
  for(unsigned int m = 0; m < m_nModules; m++)
  {

    /// if both x layers got hit
    if(m_hasLayer[m][0] && m_hasLayer[m][3])
      m_mXCmsDist->push_back(fabs((*m_lStripCms)[m][m_layer2Idx[m][0]]-(*m_lStripCms)[m][m_layer2Idx[m][3]]));
    else m_mXCmsDist->push_back(-1.);
    
    /// if both y layers got hit
    if(m_hasLayer[m][1] && m_hasLayer[m][2])
      m_mYCmsDist->push_back(fabs((*m_lStripCms)[m][m_layer2Idx[m][1]]-(*m_lStripCms)[m][m_layer2Idx[m][2]]));
    else m_mYCmsDist->push_back(-1.);
  }
    
}




/// data structure for cluster hits is a four dimensioanl array
/// four indices are [module index][layer index][cluster index][strip index]
/// and the value is strip ID
void EnrichedTreeVars::getClusterHits()
{
  m_mNClusters->clear();
  m_mClusterSize->clear();
  m_mMaxClusterSizeX->clear();
  m_mMaxClusterSizeY->clear();
  m_mClusterHits->clear();
  
  for(unsigned int m = 0; m < m_nModules; m++)
  {
    vector<vector<vector<unsigned int> > > clusterLayerByLayer;
    vector<vector<unsigned int> > clusterSizeLayerByLayer;
    m_mNClusters->push_back(vector<unsigned int>());
    
    /// store maximum size of the clusters for each layer
    unsigned int maxClusterSize[4] = {0};
    
    for(unsigned int l = 0; l < m_mNLayers->at(m); l++)
    {
      vector<vector<unsigned int> > clusterInOneLayer;
      vector<unsigned int> clusterSizeInOneLayer;
      vector<unsigned int> stripIdCluster;
      
      /// some events have strips unsorted for some unknown reason
      sort(m_mRpcStrip->at(m)[l].begin(), m_mRpcStrip->at(m)[l].end());
      
      for(unsigned int s = 0; s < m_nStrips->at(m)[l]; s++)
      {
        stripIdCluster.push_back(m_mRpcStrip->at(m)[l][s]);
        if(s < m_nStrips->at(m)[l]-1)
        {
          if(m_mRpcStrip->at(m)[l][s+1]-m_mRpcStrip->at(m)[l][s]!=1)
          {
            clusterInOneLayer.push_back(stripIdCluster);
            clusterSizeInOneLayer.push_back(stripIdCluster.size());
            stripIdCluster.clear();
          }
        }
        else /// last fired strip
        {
          clusterInOneLayer.push_back(stripIdCluster);
          clusterSizeInOneLayer.push_back(stripIdCluster.size());
          stripIdCluster.clear();
          clusterLayerByLayer.push_back(clusterInOneLayer);
        }
      }
      m_mNClusters->at(m).push_back(clusterInOneLayer.size());
      clusterSizeLayerByLayer.push_back(clusterSizeInOneLayer);
      
      /// after filling the clusters, maximum cluster size can be queried
      maxClusterSize[m_mRpcLayer->at(m)[l]-1] = *max_element(clusterSizeInOneLayer.begin(), clusterSizeInOneLayer.end());
    }
    m_mClusterHits->push_back(clusterLayerByLayer);
    m_mClusterSize->push_back(clusterSizeLayerByLayer);
    m_mMaxClusterSizeX->push_back(max(maxClusterSize[0], maxClusterSize[3]));
    m_mMaxClusterSizeY->push_back(max(maxClusterSize[1], maxClusterSize[2]));
    
    /// accumulate all cluster sizes in the dataset
    pair<unsigned int, unsigned int> modId(m_mRpcRow->at(m), m_mRpcColumn->at(m));
    map<pair<unsigned int, unsigned int>, vector<unsigned int> >::iterator it;
    it = m_clusterSizeDataSet.find(modId);
    /// first time this module's data show up
    if(it == m_clusterSizeDataSet.end())
      m_clusterSizeDataSet[modId] = vector<unsigned int>();
    for(unsigned int l = 0; l < clusterSizeLayerByLayer.size(); l++)
      for(unsigned int c = 0; c < clusterSizeLayerByLayer[l].size(); c++)
        m_clusterSizeDataSet.find(modId)->second.push_back(clusterSizeLayerByLayer[l][c]);
  }
}




/// For x clusters in different layers, check if their intersection of clusters
/// of strip IDs is nonempty. If yes, they form an x track. Otherwise, they form
/// spurious hits. Record corresponding cluster index.
/// Repeat for y clusters.

/// A spurious track is formed if the intersection of two clusters is empty.
void EnrichedTreeVars::getTracks()
{
  m_mNTracksX->clear();
  m_mNTracksY->clear();
  m_mTrackClusterIdxX1->clear();
  m_mTrackClusterIdxY2->clear();
  m_mTrackClusterIdxY3->clear();
  m_mTrackClusterIdxX4->clear();
  m_mNSpuriousX->clear();
  m_mNSpuriousY->clear();
  m_mSpuriousClusterIdxX1->clear();
  m_mSpuriousClusterIdxY2->clear();
  m_mSpuriousClusterIdxY3->clear();
  m_mSpuriousClusterIdxX4->clear();
  
  for(unsigned int m = 0; m < m_nModules; m++)
  {
    
    /// process x clusters
    if(m_hasLayer[m][0] && m_hasLayer[m][3])
    {
      vector<unsigned int>::iterator it;
      
      vector<vector<unsigned int> > ctop((*m_mClusterHits)[m][m_layer2Idx[m][3]]);
      vector<vector<unsigned int> > cbot((*m_mClusterHits)[m][m_layer2Idx[m][0]]);
      unsigned int ntop = ctop.size();
      unsigned int nbot = cbot.size();
      
      unsigned nTracksX = 0;
      
      /// store overlapping cluster index
      vector<unsigned int> topClusterIdxThisMod;
      vector<unsigned int> botClusterIdxThisMod;
      /// store spurious tracks
      vector<unsigned int> topSpuriousIdxThisMod;
      vector<unsigned int> botSpuriousIdxThisMod;
      for(unsigned int tt = 0; tt < ntop; tt++)
      {
        for(unsigned int bb = 0; bb < nbot; bb++)
        {
          /// each pair of cluster has at most 8 overlapping strips
          vector<unsigned int> vcommon(8);
          it = set_intersection(ctop[tt].begin(), ctop[tt].end(), cbot[bb].begin(), cbot[bb].end(), vcommon.begin());
          unsigned int ncommon = (int)(it-vcommon.begin());
          if(ncommon)
          {
            nTracksX++;
            
            topClusterIdxThisMod.push_back(tt);
            botClusterIdxThisMod.push_back(bb);
          }
          else
          {
            topSpuriousIdxThisMod.push_back(tt);
            botSpuriousIdxThisMod.push_back(bb);
          }
        }
      }
      
      /// store the index results to corresponding tree containers
      m_mNTracksX->push_back(nTracksX);
      m_mTrackClusterIdxX1->push_back(botClusterIdxThisMod);
      m_mTrackClusterIdxX4->push_back(topClusterIdxThisMod);
      m_mNSpuriousX->push_back(topSpuriousIdxThisMod.size());
      m_mSpuriousClusterIdxX1->push_back(botSpuriousIdxThisMod);
      m_mSpuriousClusterIdxX4->push_back(topSpuriousIdxThisMod);
    }
    else
    {
      m_mNTracksX->push_back(0);
      vector<unsigned int> emptyVec;
      m_mTrackClusterIdxX1->push_back(emptyVec);
      m_mTrackClusterIdxX4->push_back(emptyVec);
      m_mNSpuriousX->push_back(0);
      m_mSpuriousClusterIdxX1->push_back(emptyVec);
      m_mSpuriousClusterIdxX4->push_back(emptyVec);
    }
    
    
    /// process y clusters
    if(m_hasLayer[m][1] && m_hasLayer[m][2])
    {
      vector<unsigned int>::iterator it;
      
      vector<vector<unsigned int> > ctop((*m_mClusterHits)[m][m_layer2Idx[m][2]]);
      vector<vector<unsigned int> > cbot((*m_mClusterHits)[m][m_layer2Idx[m][1]]);
      unsigned int ntop = ctop.size();
      unsigned int nbot = cbot.size();
      
      unsigned nTracksY = 0;
      /// store overlapping cluster index
      vector<unsigned int> topClusterIdxThisMod;
      vector<unsigned int> botClusterIdxThisMod;
      /// store spurious tracks
      vector<unsigned int> topSpuriousIdxThisMod;
      vector<unsigned int> botSpuriousIdxThisMod;
      for(unsigned int tt = 0; tt < ntop; tt++)
      {
        for(unsigned int bb = 0; bb < nbot; bb++)
        {
          /// each pair of cluster has at most 8 overlapping strips
          vector<unsigned int> vcommon(8);
          it = set_intersection(ctop[tt].begin(), ctop[tt].end(), cbot[bb].begin(), cbot[bb].end(), vcommon.begin());
          unsigned int ncommon = (int)(it-vcommon.begin());
          if(ncommon)
          {
            nTracksY++;
            
            topClusterIdxThisMod.push_back(tt);
            botClusterIdxThisMod.push_back(bb);
          }
          else
          {
            topSpuriousIdxThisMod.push_back(tt);
            botSpuriousIdxThisMod.push_back(bb);
          }
        }
      }
      
      /// store the index results to corresponding tree containers
      m_mNTracksY->push_back(nTracksY);
      m_mTrackClusterIdxY2->push_back(botClusterIdxThisMod);
      m_mTrackClusterIdxY3->push_back(topClusterIdxThisMod);
      m_mNSpuriousY->push_back(topSpuriousIdxThisMod.size());
      m_mSpuriousClusterIdxY2->push_back(botSpuriousIdxThisMod);
      m_mSpuriousClusterIdxY3->push_back(topSpuriousIdxThisMod);
    }
    else
    {
      m_mNTracksY->push_back(0);
      vector<unsigned int> emptyVec;
      m_mTrackClusterIdxY2->push_back(emptyVec);
      m_mTrackClusterIdxY3->push_back(emptyVec);
      m_mNSpuriousY->push_back(0);
      m_mSpuriousClusterIdxY2->push_back(emptyVec);
      m_mSpuriousClusterIdxY3->push_back(emptyVec);
    }
  }
}




/// A vertical muon track is formed if the event has exactly one x and one y
/// track. In getting distance of all fired strips to the muon track, only those
/// tracks in which at least one of the x and y clusters has only one strip.
void EnrichedTreeVars::getDistance()
{
  m_mDx->clear();
  m_mDy->clear();
  m_mNDx->clear();
  m_mNDy->clear();
  m_mDxMuon->clear();
  m_mDyMuon->clear();
  m_dSingleMuon->clear();
  m_dSingleMuonH.clear();
  m_singleStripId32->clear();
  
  for(unsigned int m = 0; m < m_nModules; m++)
  {
    /// assign empty vector anyway
    vector<int> dxThisMod, dyThisMod, dxMuonThisMod, dyMuonThisMod;
    m_mDx->push_back(dxThisMod);
    m_mDy->push_back(dyThisMod);
    m_mDxMuon->push_back(dxMuonThisMod);
    m_mDyMuon->push_back(dyMuonThisMod);
    m_singleStripId32->push_back(-1); // -1 means nonapplicable
    m_dSingleMuon->push_back(-1); // -1 means nonapplicable
    

/* first fill variables with a speculated muon track and calculate the distance
 * of each strip to that track.
*/
    /// get sizes of the cluster intersection
    unsigned int nIntersectionX = 0, nIntersectionY = 0;
    unsigned int intersectionXId = 0, intersectionYId = 0;
    vector<unsigned int> distIncMuon;
    m_dSingleMuonH.push_back(distIncMuon);
    if(m_mNTracksX->at(m)==1)
    {
      vector<unsigned int>::iterator it;
      vector<unsigned int> vcommon(8);
      vector<vector<unsigned int> > ctop((*m_mClusterHits)[m][m_layer2Idx[m][3]]);
      vector<vector<unsigned int> > cbot((*m_mClusterHits)[m][m_layer2Idx[m][0]]);
      it = set_intersection(ctop[0].begin(), ctop[0].end(), cbot[0].begin(), cbot[0].end(), vcommon.begin());
      nIntersectionX = (unsigned int)(it-vcommon.begin());
      if(nIntersectionX == 1) intersectionXId = vcommon[0];
    }
    if(m_mNTracksY->at(m)==1)
    {
      vector<unsigned int>::iterator it;
      vector<unsigned int> vcommon(8);
      vector<vector<unsigned int> > ctop((*m_mClusterHits)[m][m_layer2Idx[m][2]]);
      vector<vector<unsigned int> > cbot((*m_mClusterHits)[m][m_layer2Idx[m][1]]);
      it = set_intersection(ctop[0].begin(), ctop[0].end(), cbot[0].begin(), cbot[0].end(), vcommon.begin());
      nIntersectionY = (unsigned int)(it-vcommon.begin());
      if(nIntersectionY == 1) intersectionYId = vcommon[0];
    }
    
    
    if(nIntersectionX==1 && nIntersectionY==1)
    {
      for(unsigned int l = 0; l < 4; l++)
      {
        /// y layers
        if(l%3)
        {
          for(unsigned int s = 0; s < m_mRpcStrip->at(m)[l].size(); s++)
            m_mDyMuon->at(m).push_back(abs((int)m_mRpcStrip->at(m)[l][s]-(int)intersectionYId));
        }
        /// x layers
        else
        {
          for(unsigned int s = 0; s < m_mRpcStrip->at(m)[l].size(); s++)
            m_mDxMuon->at(m).push_back(abs((int)m_mRpcStrip->at(m)[l][s]-(int)intersectionXId));
        }
      }
      
      
      /// 5 strips situation
      if(m_mNStrips->at(m) == 5)
      {
        pair<unsigned int, unsigned int> modId(m_mRpcRow->at(m), m_mRpcColumn->at(m));
        map<pair<unsigned int, unsigned int>, vector<unsigned int> >::iterator it;
        it = m_dSingleMuonH2.find(modId);
        /// first time this module's data show up
        if(it == m_dSingleMuonH2.end())
        {
          vector<unsigned int> emptyVec;
          vector<int> emptyVecSigned;
          m_dSingleMuonH2[modId] = emptyVec;
          m_dSingleMuonSigned[modId] = emptyVecSigned;
        }
        for(unsigned int l = 0; l < 4; l++)
        {
          if(m_nStrips->at(m)[l] == 2)
          {
            for(unsigned int s = 0; s < m_mRpcStrip->at(m)[l].size(); s++)
            {
              int stripId = m_mRpcStrip->at(m)[l][s];
              unsigned int layerId = m_mRpcLayer->at(m)[l];
              
              if(!(l%3)) // x layers
              {
                m_dSingleMuonH[m_dSingleMuonH.size()-1].push_back(abs(stripId-(int)intersectionXId));
                m_dSingleMuonH2.find(modId)->second.push_back(abs(stripId-(int)intersectionXId));
                m_dSingleMuonSigned.find(modId)->second.push_back(stripId-(int)intersectionXId);
                if((unsigned int)stripId != intersectionXId)
                {
                  m_dSingleMuon->at(m) = abs(stripId-(int)intersectionXId);
                  m_singleStripId32->at(m) = (layerId-1)*8+stripId-1;
                }
              }

              else // y layers
              {
                m_dSingleMuonH[m_dSingleMuonH.size()-1].push_back(abs(stripId-(int)intersectionYId));
                m_dSingleMuonH2.find(modId)->second.push_back(abs(stripId-(int)intersectionYId));
                m_dSingleMuonSigned.find(modId)->second.push_back(stripId-(int)intersectionYId);
                if((unsigned int)stripId != intersectionYId)
                {
                  m_dSingleMuon->at(m) = abs(stripId-(int)intersectionYId);
                  m_singleStripId32->at(m) = (layerId-1)*8+stripId-1;
                }
              }


            }
          }
        }
      }
    }
// end of the first variables calculation //////////////////////////////////////



// second variables: used the layer with only 1 hit as the seed and calculate
// strip difference of the other layer to that strip.
    int targetLayerIdxX = -1, targetLayerIdxY = -1;
    int normalX, normalY;
    /// only deal with situations with 1 x and 1 y track
    if(m_mNTracksX->at(m)==1 && m_mNTracksY->at(m)==1)
    {
      if(m_mClusterHits->at(m)[0][0].size()==1)
      {
        normalX = m_mClusterHits->at(m)[0][0][0];
        targetLayerIdxX = 3;
      }
      else if(m_mClusterHits->at(m)[3][0].size()==1)
      {
        normalX = m_mClusterHits->at(m)[3][0][0];
        targetLayerIdxX = 0;
      }
      if(m_mClusterHits->at(m)[1][0].size()==1)
      {
        normalY = m_mClusterHits->at(m)[1][0][0];
        targetLayerIdxY = 2;
      }
      else if(m_mClusterHits->at(m)[2][0].size()==1)
      {
        normalY = m_mClusterHits->at(m)[2][0][0];
        targetLayerIdxY = 1;
      }
      /// start getting dx and dy
      if(targetLayerIdxX >= 0)
        for(unsigned int s = 0; s < m_mRpcStrip->at(m)[targetLayerIdxX].size(); s++)
          m_mDx->at(m).push_back(abs((int)m_mRpcStrip->at(m)[targetLayerIdxX][s]-normalX));
      if(targetLayerIdxY >= 0)
        for(unsigned int s = 0; s < m_mRpcStrip->at(m)[targetLayerIdxY].size(); s++)
          m_mDy->at(m).push_back(abs((int)m_mRpcStrip->at(m)[targetLayerIdxY][s]-normalY));
    }
    
    
    /// Also deal with 3-fold. In this case, every layer can have at most one
    /// strip and the only x or y pair has to have strips lined up.
    //else if(m_mNLayers->at(m) == 3 && m_mNStrips->at(m) == 3)
    //{
      //if(m_mNTracksX->at(m)==1)
    //}
    
    m_mNDx->push_back(m_mDx->at(m).size());
    m_mNDy->push_back(m_mDy->at(m).size());
// end of second variables calculation /////////////////////////////////////////
  }
}
