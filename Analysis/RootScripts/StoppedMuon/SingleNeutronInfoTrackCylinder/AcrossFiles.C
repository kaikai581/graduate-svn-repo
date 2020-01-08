#include <algorithm>
#include <iomanip>
#include <iostream>
#include <map>
#include <utility>
#include <vector>
#include "Math/Point3D.h"
#include "Math/Vector3D.h"
#include "TChain.h"
#include "TTree.h"

#define MAXNMICHEL 100

using namespace std;
using namespace ROOT::Math;

void AcrossFiles(int evtOffset = 2)
{
  
  if(evtOffset < 1) {
    cerr << "event offset has to be larger than 2" << endl;
    return;
  }
  
  TChain* ch = new TChain("dmucoin");
  ch->Add("/global/project/projectdirs/dayabay/scratch/sklin/RootFile/StoppedMuonsNuWa/2012/p12e/Neutrino/0726/*EH1*.root");
  
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
  
  bool inscribable;
  int inCyl[MAXNMICHEL];
  double cylLen;
  double zTrack[MAXNMICHEL], rhoTrack[MAXNMICHEL];

  /// end of container variables
  
  ch->SetBranchAddress("inscribable", &inscribable);
  ch->SetBranchAddress("inCyl", inCyl);
  ch->SetBranchAddress("cylLen", &cylLen);
  ch->SetBranchAddress("zTrack", zTrack);
  ch->SetBranchAddress("rhoTrack", rhoTrack);
  
  ch->SetBranchAddress("nMi", &nMi);
  ch->SetBranchAddress("detector", &detector);
  ch->SetBranchAddress("eMu", &eMu);
  ch->SetBranchAddress("eMi", eNeu);
  ch->SetBranchAddress("tAdMuSec", &triggerTimeSecMu);
  ch->SetBranchAddress("tAdMuNanoSec", &triggerTimeNanoSecMu);
  ch->SetBranchAddress("tAdMiSec", triggerTimeSecNeu);
  ch->SetBranchAddress("tAdMiNanoSec", triggerTimeNanoSecNeu);
  ch->SetBranchAddress("triggerNumberMi", triggerNumberNeu);
  ch->SetBranchAddress("triggerNumberMu", &triggerNumberMu);
  ch->SetBranchAddress("triggerNumberIws", &triggerNumberIws);
  ch->SetBranchAddress("triggerNumberOws", &triggerNumberOws);
  ch->SetBranchAddress("triggerNumberRpc", &triggerNumberRpc);
  ch->SetBranchAddress("dlIav", &dlIav);
  ch->SetBranchAddress("hitRpc", &hitRpc);
  ch->SetBranchAddress("hitIws", &hitIws);
  ch->SetBranchAddress("hitOws", &hitOws);
  ch->SetBranchAddress("nHitsIws", &nHitsIws);
  ch->SetBranchAddress("nHitsOws", &nHitsOws);
  ch->SetBranchAddress("nHitsRpc", &nHitsRpc);
  
  ch->SetBranchAddress("xMi", xNeu);
  ch->SetBranchAddress("yMi", yNeu);
  ch->SetBranchAddress("zMi", zNeu);
  ch->SetBranchAddress("xMu", &xMu);
  ch->SetBranchAddress("yMu", &yMu);
  ch->SetBranchAddress("zMu", &zMu);
  ch->SetBranchAddress("xRpc", &xRpc);
  ch->SetBranchAddress("yRpc", &yRpc);
  ch->SetBranchAddress("zRpc", &zRpc);
  
  /// results containers
  int neu_cnt = 0;
  int nentry = ch->GetEntries();
  
  double totTrackLen = 0;
  double totCylLenSinceLastNeu = 0;
  
  map< int, vector<double> > trLenB4Neu;
  map< int, vector<double> > cylLenB4Neu;
  
  map<int, pair<int, int> > neuTime;
  
  for(int i = 0; i < nentry; i++)
  {
    ch->GetEntry(i);
    
    if(detector != 1 || dlIav < 0 || !inscribable) continue;
    
    if(trLenB4Neu.find(neu_cnt) == trLenB4Neu.end())
      trLenB4Neu[neu_cnt] = vector<double>();
    
    if(cylLenB4Neu.find(neu_cnt) == cylLenB4Neu.end())
      cylLenB4Neu[neu_cnt] = vector<double>();
    
    if(neuTime.find(neu_cnt) == neuTime.end())
      neuTime[neu_cnt] = pair<int, int>();
    
    if(nMi == 1 && inCyl[0] == 1) {
      if(eNeu[0] < 6 || eNeu[0] > 12) continue;
      
      ++neu_cnt;
      
      //cout << setw(35) << "nuetron ID: " << setw(15) << neu_cnt << endl;
      cout << setw(33) << "inducing muon data" << setw(50) << "neutron data" << endl;
      
      cout << setw(35) << "detector: " << setw(15) << detector;
      cout << setw(35) << "number of neutron candidates: " << setw(15) << nMi << endl;
      
      cout << setw(35) << "muon trigger number: " << setw(15) << triggerNumberMu;
      cout << setw(35) << "neutron trigger number: " << setw(15) << triggerNumberNeu[0] << endl;
      
      cout << setw(35) << "muon deposit energy: " << setw(11) << eMu << setw(4) << " MeV";
      cout << setw(35) << "neutron energy: " << setw(11) << eNeu[0] << setw(4) << " MeV" << endl;
      
      cout << setw(35) << "muon trigger time sec: " << setw(15) << triggerTimeSecMu;
      cout << setw(35) << "neutron trigger time sec: " << setw(15) << triggerTimeSecNeu[0] << endl;
      
      cout << setw(35) << "muon trigger time nanosec: " << setw(15) << triggerTimeNanoSecMu;
      cout << setw(35) << "neutron trigger time nanosec: " << setw(15) << triggerTimeNanoSecNeu[0] << endl;
      
      cout << setw(35) << "track length in IAV: " << setw(12) << dlIav << setw(3) << " mm";
      cout << setw(35) << "neutron time after the muon: " << setw(12) << (triggerTimeSecNeu[0]-triggerTimeSecMu)*1e9+(triggerTimeNanoSecNeu[0]-triggerTimeNanoSecMu)/1000. << " us" << endl;
      
      cout << setw(35) << "hit IWS: " << setw(15) << hitIws;
      
      /// calculate neutron distance of closest approach to the track
      XYZPoint pRpc(xRpc, yRpc, zRpc);
      XYZPoint pMu(xMu, yMu, zMu);
      XYZVector dirV = (pMu-pRpc)/sqrt((pMu-pRpc).Mag2());
      XYZVector vNeu = XYZPoint(xNeu[0],yNeu[0],zNeu[0])-pRpc;
      double cos = vNeu.Dot(dirV)/sqrt(vNeu.Mag2());
      double clapp = sqrt(vNeu.Mag2())*sqrt(1-cos*cos);
      
      cout << setw(35) << "closest approach: " << setw(12) << clapp << setw(3) << " mm" << endl;
      
      if(hitIws) {
        cout << setw(35) << "IWS trigger number: " << setw(15) << triggerNumberIws;
        cout << setw(35) << "cylinder length: " << setw(12) << cylLen << setw(3) << " mm" << endl;
        cout << setw(35) << "IWS nhit: " << setw(15) << nHitsIws;
        cout << setw(35) << "z w.r.t. cylinder: " << setw(12) << zTrack[0] << setw(3) << " mm" << endl;
      }
      else cout << setw(70) << "cylinder length: " << setw(12) << cylLen << setw(3) << " mm" << endl;
      
      cout << setw(35) << "hit OWS: " << setw(15) << hitOws;
      cout << setw(35) << "rho w.r.t. cylinder: " << setw(12) << rhoTrack[0] << setw(3) << " mm" << endl;
      
      if(hitOws) {
        cout << setw(35) << "OWS trigger number: " << setw(15) << triggerNumberOws;
        cout << setw(35) << "scaled z: " << setw(15) << zTrack[0]/cylLen << endl;
        cout << setw(35) << "OWS nhit: " << setw(15) << nHitsOws << endl;
      }
      else cout << endl;
      
      cout << setw(35) << "hit RPC: " << setw(15) << hitRpc << endl;
      
      if(hitRpc) {
        cout << setw(35) << "RPC trigger number: " << setw(15) << triggerNumberRpc << endl;
        cout << setw(35) << "RPC nhit: " << setw(15) << nHitsRpc << endl;
      }
      else cout << endl;
      
      neuTime[neu_cnt].first = triggerTimeSecNeu[0];
      
      neuTime[neu_cnt].second = triggerTimeNanoSecNeu[0];
      
      cout << endl;
      
    }
    
    /// store each track langth before a neutron
    trLenB4Neu[neu_cnt].push_back(dlIav);
    /// store each cylinder langth before a neutron
    if(inscribable) cylLenB4Neu[neu_cnt].push_back(cylLen);
    
    if(neu_cnt >= evtOffset) break;
  } /* end of event loop */
  
  unsigned int idx = min(evtOffset, neu_cnt);
  
  cout << setw(68) << "miscellaneous data" << endl;
  cout << setw(70) << "number of inscribable muons since last neutron: " << trLenB4Neu[idx-1].size()-1 << endl;
  
  for(unsigned int i = 0; i < trLenB4Neu[idx-1].size(); i++)
    totTrackLen += trLenB4Neu[idx-1][i];
  for(unsigned int i = 0; i < cylLenB4Neu[idx-1].size(); i++)
    totCylLenSinceLastNeu += cylLenB4Neu[idx-1][i];
  
  cout << setw(70) << "total track length since last neutron: " << totTrackLen/10. << " cm" << endl;
  cout << setw(70) << "total cylinder length since last neutron: " << totCylLenSinceLastNeu/10. << " cm" << endl;
  cout << setw(70) << "time since last neutron: " << (neuTime[idx].first-neuTime[idx-1].first)+(neuTime[idx].second-neuTime[idx-1].second)*1e-9 << " second" << endl;
  
  double cylLenAllNeu = 0;
  
  map< int, vector<double> >::iterator it = cylLenB4Neu.begin();
  
  for(; it != cylLenB4Neu.end(); it++)
  {
    for(unsigned int i = 0; i < it->second.size(); i++)
      cylLenAllNeu += it->second[i];
  }
  
  double firstCylLen = 0, lastCylLen = 0;
  
  for(unsigned int i = 0; i < cylLenB4Neu[0].size(); i++)
    firstCylLen += cylLenB4Neu[0][i];
  
  it--;
  
  for(unsigned int i = 0; i < it->second.size(); i++)
    lastCylLen += it->second[i];
  
  cout << endl << setw(70) << "total number of neutrons: " << neu_cnt-1 << endl;
  
  cout << setw(70) << "total cylinder length: " << (cylLenAllNeu-firstCylLen-lastCylLen)/10. << " cm" << endl;
}
