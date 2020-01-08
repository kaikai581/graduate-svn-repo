#include "MuonData.hpp"

using namespace DetectorId;
using namespace std;

MuonData :: MuonData(vector<DetectorId_t> hitMap)
{
  m_hitMap = hitMap;
  
  for(unsigned int i = 0; i < hitMap.size(); i++)
    m_detectorFinished[hitMap[i]] = false;
}


bool MuonData :: allDetectorsFinished()
{
  bool res = true;
  
  map<DetectorId_t, bool>::iterator it = m_detectorFinished.begin();
  for(; it != m_detectorFinished.end(); it++) res = res && it->second;
  
  return res;
}


bool MuonData :: includeAd()
{
  for(unsigned int i = 0; i < m_hitMap.size(); i++)
    if(m_hitMap[i] == kAD1 ||
       m_hitMap[i] == kAD2 ||
       m_hitMap[i] == kAD3 ||
       m_hitMap[i] == kAD4) return true;
  
  return false;
}
