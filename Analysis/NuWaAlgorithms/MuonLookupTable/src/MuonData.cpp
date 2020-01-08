#include "MuonData.hpp"

using namespace DetectorId;
using namespace std;

SingleMuonData :: SingleMuonData(vector<DetectorId_t> hitMap)
{
  m_hitMap = hitMap;
  
  for(unsigned int i = 0; i < hitMap.size(); i++)
    m_detectorFinished[hitMap[i]] = false;
}


SingleMuonProgress::SingleMuonProgress()
{
  m_detectorFinished[kAD1] = true;
  m_detectorFinished[kAD2] = true;
  m_detectorFinished[kAD3] = true;
  m_detectorFinished[kAD4] = true;
  m_detectorFinished[kIWS] = true;
  m_detectorFinished[kOWS] = true;
  m_detectorFinished[kRPC] = true;
}


bool SingleMuonProgress::muonFinished()
{
  map<DetectorId_t, bool>::iterator it = m_detectorFinished.begin();
  
  bool result = true;
  for(; it != m_detectorFinished.end(); it++)
    result = (result && it->second);
  
  return result;
}


unsigned int SingleMuonBuffer::nPoints(DetectorId_t detId)
{
  return pDetRec[detId].size();
}
