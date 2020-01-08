#include <algorithm>

#include <iostream>

#include <map>

#include <utility>

#include <vector>

#include "Math/Point3D.h"

#include "Math/Vector3D.h"

#include "TFile.h"

#include "TTree.h"

#define MAXNMICHEL 100

using namespace std;

using namespace ROOT::Math;

void SingleNeutronInfo(int evtOffset = 2)
{
  
  if(evtOffset < 1) {
    cerr << "event offset has to be larger than 2" << endl;
    
    return;
  }
  
  TFile* f = new TFile("/disk1/data/2012/p12e/Neutrino/0101/neutron.Neutrino.0021358.Physics.EH1-Merged.P12E-P._0047.root");
  
  TTree* t = (TTree*)f->Get("dmucoin");
  
  /// container variables
  int nMi, detector;
  
  double eMu, eNeu[MAXNMICHEL];
  
  int triggerTimeSecNeu[MAXNMICHEL], triggerTimeNanoSecNeu[MAXNMICHEL];
  
  int triggerTimeSecMu, triggerTimeNanoSecMu;
  
  int triggerNumberMu, triggerNumberNeu[MAXNMICHEL];
  
  int triggerNumberIws, triggerNumberOws, triggerNumberRpc;
  
  double xNeu[MAXNMICHEL], yNeu[MAXNMICHEL], zNeu[MAXNMICHEL];
  
  double xMu, yMu, zMu;
  
  double xRpc, yRpc, zRpc;
  
  int nHitsIws, nHitsOws, nHitsRpc;
  
  double dlIav;
  
  int hitRpc, hitIws, hitOws;
  /// end of container variables
  
  t->SetBranchAddress("nMi", &nMi);
  
  t->SetBranchAddress("detector", &detector);
  
  t->SetBranchAddress("eMu", &eMu);
  
  t->SetBranchAddress("eMi", eNeu);
  
  t->SetBranchAddress("tAdMuSec", &triggerTimeSecMu);
  
  t->SetBranchAddress("tAdMuNanoSec", &triggerTimeNanoSecMu);
  
  t->SetBranchAddress("tAdMiSec", triggerTimeSecNeu);
  
  t->SetBranchAddress("tAdMiNanoSec", triggerTimeNanoSecNeu);
  
  t->SetBranchAddress("triggerNumberMi", triggerNumberNeu);
  
  t->SetBranchAddress("triggerNumberMu", &triggerNumberMu);
  
  t->SetBranchAddress("triggerNumberIws", &triggerNumberIws);
  
  t->SetBranchAddress("triggerNumberOws", &triggerNumberOws);
  
  t->SetBranchAddress("triggerNumberRpc", &triggerNumberRpc);
  
  t->SetBranchAddress("dlIav", &dlIav);
  
  t->SetBranchAddress("hitRpc", &hitRpc);
  
  t->SetBranchAddress("hitIws", &hitIws);
  
  t->SetBranchAddress("hitOws", &hitOws);
  
  t->SetBranchAddress("nHitsIws", &nHitsIws);
  
  t->SetBranchAddress("nHitsOws", &nHitsOws);
  
  t->SetBranchAddress("nHitsRpc", &nHitsRpc);
  
  t->SetBranchAddress("xMi", xNeu);
  
  t->SetBranchAddress("yMi", yNeu);
  
  t->SetBranchAddress("zMi", zNeu);
  
  t->SetBranchAddress("xMu", &xMu);
  
  t->SetBranchAddress("yMu", &yMu);
  
  t->SetBranchAddress("zMu", &zMu);
  
  t->SetBranchAddress("xRpc", &xRpc);
  
  t->SetBranchAddress("yRpc", &yRpc);
  
  t->SetBranchAddress("zRpc", &zRpc);
  
  int neu_cnt = 0;
  
  int nentry = t->GetEntries();
  
  int nMuBetween = 0;
  
  double totTrackLen = 0;
  
  map< int, vector<double> > trLenB4Neu;
  
  map<int, pair<int, int> > neuTime;
  
  for(int i = 0; i < nentry; i++)
  {
    t->GetEntry(i);
    
    if(detector != 1 || dlIav < 0) continue;
    
    if(trLenB4Neu.find(neu_cnt) == trLenB4Neu.end())
      trLenB4Neu[neu_cnt] = vector<double>();
    
    if(neuTime.find(neu_cnt) == neuTime.end())
      neuTime[neu_cnt] = pair<int, int>();
    
    if(nMi == 1) {
      
      if(eNeu[0] < 6 || eNeu[0] > 12) continue;
      
      cout << "nuetron ID: " << ++neu_cnt << endl;
      
      cout << "detector: " << detector << endl;
      
      cout << "muon trigger number: " << triggerNumberMu << endl;
      
      cout << "muon deposit energy: " << eMu << " MeV" << endl;
      
      cout << "muon trigger time sec: " << triggerTimeSecMu << endl;
      
      cout << "muon trigger time nanosec: " << triggerTimeNanoSecMu << endl;
      
      cout << "neutron trigger number: " << triggerNumberNeu[0] << endl;
      
      cout << "neutron energy: " << eNeu[0] << endl;
      
      cout << "number of neutron candidates: " << nMi << endl;
      
      cout << "neutron trigger time sec: " << triggerTimeSecNeu[0] << endl;
      
      cout << "neutron trigger time nanosec: " << triggerTimeNanoSecNeu[0] << endl;
      
      cout << "neutron time after the muon: " << (triggerTimeSecNeu[0]-triggerTimeSecMu)*1e9+(triggerTimeNanoSecNeu[0]-triggerTimeNanoSecMu) << endl;
      
      cout << "track length in IAV: " << dlIav << endl;
      
      cout << "hit IWS: " << hitIws;
      
      if(hitIws) {
        
        cout << " trigger number: " << triggerNumberIws;
        
        cout << " nhit: " << nHitsIws << endl;
        
      }
      else {
        
        cout << endl;
        
      }
      
      cout << "hit OWS: " << hitOws;
      
      if(hitOws) {
        
        cout << " trigger number: " << triggerNumberOws;
        
        cout << " nhit: " << nHitsOws << endl;
        
      }
      else {
        
        cout << endl;
        
      }
      
      cout << "hit RPC: " << hitRpc;
      
      if(hitRpc) {
        
        cout << " trigger number: " << triggerNumberRpc;
        
        cout << " nhit: " << nHitsRpc << endl;
        
      }
      else {
        
        cout << endl;
        
      }
      
      /// calculate neutron distance of closest approach to the track
      XYZPoint pRpc(xRpc, yRpc, zRpc);
    
      XYZPoint pMu(xMu, yMu, zMu);
      
      XYZVector dirV = (pMu-pRpc)/sqrt((pMu-pRpc).Mag2());
      
      XYZVector vNeu = XYZPoint(xNeu[0],yNeu[0],zNeu[0])-pRpc;
      
      double cos = vNeu.Dot(dirV)/sqrt(vNeu.Mag2());
      
      double clapp = sqrt(vNeu.Mag2())*sqrt(1-cos*cos);
      
      cout << "closest approach: " << clapp << endl;
      
      neuTime[neu_cnt].first = triggerTimeSecNeu[0];
      
      neuTime[neu_cnt].second = triggerTimeNanoSecNeu[0];
      
      cout << endl;
      
    }
    
    /// store each track langth before a neutron
    trLenB4Neu[neu_cnt].push_back(dlIav);
    
    if ( neu_cnt == 1 || neu_cnt == 2 ) {
      
      nMuBetween++;
      
      totTrackLen += dlIav;
      
    }
    
    if(neu_cnt >= evtOffset) break;
    
  }
  
  //cout << "number of muons between the two neutrons: " << nMuBetween << endl;
  
  //cout << "total track length: " << totTrackLen << endl;
  
  unsigned int idx = min(evtOffset, neu_cnt);
  
  cout << "number of muons since last neutron: " << trLenB4Neu[idx-1].size()-1 << endl;
  
  totTrackLen = 0;
  
  for(unsigned int i = 0; i < trLenB4Neu[idx-1].size(); i++)
    totTrackLen += trLenB4Neu[idx-1][i];
  
  cout << "total track length since last neutron: " << totTrackLen/10. << " cm" << endl;
  
  cout << "time since last neutron: " << (neuTime[idx].first-neuTime[idx-1].first)+(neuTime[idx].second-neuTime[idx-1].second)*1e-9 << " second" << endl;
  
  
  double trackLenAllNeu = 0;
  
  map< int, vector<double> >::iterator it = trLenB4Neu.begin();
  
  for(; it != trLenB4Neu.end(); it++)
  {
    for(unsigned int i = 0; i < it->second.size(); i++)
      trackLenAllNeu += it->second[i];
  }
  
  double firstTrLen = 0, lastTrLen = 0;
  
  for(unsigned int i = 0; i < trLenB4Neu[0].size(); i++)
    firstTrLen += trLenB4Neu[0][i];
  
  it--;
  
  for(unsigned int i = 0; i < it->second.size(); i++)
    lastTrLen += it->second[i];
  
  cout << endl << "total number of neutrons: " << neu_cnt-1 << endl;
  
  cout << "total track length: " << (trackLenAllNeu-firstTrLen-lastTrLen)/10. << endl;
}
