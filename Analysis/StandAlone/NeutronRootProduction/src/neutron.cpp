/*
 * ROOT based neutron analysis without resorting to NuWa.
 * 
 * June 2014 Shih-Kai.
 * 
 */


#include <string>
#include <tclap/CmdLine.h>
#include "TFile.h"
#include "TLeaf.h"
#include "TTree.h"
#include "TVector3.h"

/// NuWa headers
#include "PerBaseEvent/PerHeaderObject.h"



using namespace Site;
using namespace std;





class MuonComponents
{
public:
  MuonComponents() : isDoneRpc(false), isDoneOws(false) {}
  map<int, unsigned int> trigNums;
  TVector3 pRpc;
  TVector3 pOws;
  bool isDoneRpc;
  bool isDoneOws;
  
  bool isDoneMuon();
};

bool MuonComponents::isDoneMuon()
{
  return (isDoneRpc && isDoneOws);
}


typedef pair<unsigned int, int> TriggerId;



int main(int argc, char** argv)
{
  /// parsing command line arguments
  TCLAP::CmdLine cmd("Command description message", ' ', "0.9");
  TCLAP::ValueArg<string> trackNameArg("t", "trackname", "name of the track reconstruction algorithm used", false, "RpcOws", "string");
  TCLAP::ValueArg<double> cylRadArg("r", "radius", "radius of the fiducial cylinder in mm", false, 1000, "double");
  TCLAP::ValueArg<string> fPathNameArg("f", "filename", "input file pathname", false, "/eliza16/dayabay/data/exp/dayabay/2011/p14a/Neutrino/1224/recon.Neutrino.0021221.Physics.EH1-Merged.P14A-P._0001.root", "string");
  cmd.add(trackNameArg);
  cmd.add(cylRadArg);
  cmd.add(fPathNameArg);
  cmd.parse(argc, argv);

  
  /// container variable declaration
  map<TriggerId, int> tn2mn; /// trigger number to muon number
  map<int, MuonComponents> mn2Comp;  /// muon number to component trigger information
  

  /// open the input file
  TFile inTFile(fPathNameArg.getValue().c_str());
  
  /// get trees used in this analysis
  TTree* trSpall = (TTree*)inTFile.Get("/Event/Data/Physics/Spallation");
  TTree* trPool = (TTree*)inTFile.Get("/Event/Rec/PoolSimple");
  TTree* trRpc = (TTree*)inTFile.Get("/Event/Rec/RpcSimple");
  
  
  /// record the muon trigger components
  int nEntries = trSpall->GetEntries();
  for(int i = 0; i < nEntries; i++)
  {
    if(i > 1000) break;
    
    trSpall->GetEntry(i);
    
    MuonComponents curMu;
    TriggerId curId;
    
    bool hitAD1 = ((int)trSpall->GetLeaf("hitAD1")->GetValue() == 1);
    bool hitAD2 = ((int)trSpall->GetLeaf("hitAD2")->GetValue() == 1);
    bool hitAD3 = ((int)trSpall->GetLeaf("hitAD3")->GetValue() == 1);
    bool hitAD4 = ((int)trSpall->GetLeaf("hitAD4")->GetValue() == 1);
    bool hitOWS = ((int)trSpall->GetLeaf("hitOWS")->GetValue() == 1);
    bool hitRPC = ((int)trSpall->GetLeaf("hitRPC")->GetValue() == 1);
    
    if(hitAD1){
      curMu.trigNums[1] = (unsigned int)trSpall->GetLeaf("triggerNumber_AD1")->GetValue();
      curId.first = curMu.trigNums[1];
      curId.second = 1;
      tn2mn[curId] = i;
    }
    if(hitAD2){
      curMu.trigNums[2] = (unsigned int)trSpall->GetLeaf("triggerNumber_AD2")->GetValue();
      curId.first = curMu.trigNums[2];
      curId.second = 2;
      tn2mn[curId] = i;
    }
    if(hitAD3){
      curMu.trigNums[3] = (unsigned int)trSpall->GetLeaf("triggerNumber_AD3")->GetValue();
      curId.first = curMu.trigNums[3];
      curId.second = 3;
      tn2mn[curId] = i;
    }
    if(hitAD4){
      curMu.trigNums[4] = (unsigned int)trSpall->GetLeaf("triggerNumber_AD4")->GetValue();
      curId.first = curMu.trigNums[4];
      curId.second = 4;
      tn2mn[curId] = i;
    }
    if(hitOWS){
      curMu.trigNums[6] = (unsigned int)trSpall->GetLeaf("triggerNumber_OWS")->GetValue();
      curId.first = curMu.trigNums[6];
      curId.second = 6;
      tn2mn[curId] = i;
    }
    if(hitRPC){
      curMu.trigNums[7] = (unsigned int)trSpall->GetLeaf("triggerNumber_RPC")->GetValue();
      curId.first = curMu.trigNums[7];
      curId.second = 7;
      tn2mn[curId] = i;
    }
    
    /// only store muons with RPC and OWS points for tracking
    if(hitOWS && hitRPC) mn2Comp[i] = curMu;


  }
  
  
  /// Get water pool reconstructed point
  nEntries = trPool->GetEntries();
  for(int i = 0; i < nEntries; i++)
  {
    trPool->GetEntry(i);
    
    int detId = (int)trPool->GetLeaf("detector")->GetValue();
    if(detId != 6) continue;
    
    unsigned int trigNum = (unsigned int)trPool->GetLeaf("triggerNumber")->GetValue();
    TriggerId curId(trigNum, 6);
    
    map<TriggerId, int>::iterator it = tn2mn.find(curId);
    if(it != tn2mn.end()) cout << "muon number: " << tn2mn[curId] << endl;
  }
  
  
  
  return 0;
}
