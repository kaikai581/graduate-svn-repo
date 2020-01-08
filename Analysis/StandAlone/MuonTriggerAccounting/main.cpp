/*
 *  On 10/15/2013
 *  Dr. Whitehead asked me about the proportion of muons triggering only
 *  RPC but not any other detectors. Also among these muons what is the
 *  proportion triggering RPC+telescope. Therefore write this app for this
 *  accounting assignment.
 * 
 *  Shih-Kai, Oct. 2013
*/

#include <algorithm>
#include <fstream>
#include <iostream>
#include <set>
#include <tclap/CmdLine.h>
#include "TApplication.h"
#include "TFile.h"
#include "TLeaf.h"
#include "TTree.h"

using namespace std;


bool includeRpcArray(set<pair<int,int> >&, int);
bool includeRpcTelescope(set<pair<int,int> >&, int);


int main(int argc, char** argv)
{
  /// parsing command line arguments
  TCLAP::CmdLine cmd("Command description message", ' ', "0.9");
  TCLAP::ValueArg<string> fnArg("i", "input", "input file pathname", false, "input.root", "string");
  cmd.add(fnArg);
  cmd.parse(argc, argv);
  
  
  /// check file existance
  ifstream inf(fnArg.getValue().c_str());
  if(!inf.good())
  {
    inf.close();
    cerr << "input file doesn't exist" << endl;
    return -1;
  }
  inf.close();
  
  
  /// mandatory line to solve some weird problem
  TApplication theApp("muonacc", &argc, argv);
  
  /// pull out the spallation tree
  TFile rootinf(fnArg.getValue().c_str());
  TTree* trspal = (TTree*)rootinf.Get("/Event/Data/Physics/Spallation");
  int nent = trspal->GetEntries();
  cout << "number of muons: " << nent << endl;
  
  
  /// record RPC trigger numbers for each muon
  vector<int> listRpcNum;
  for(int ent = 0; ent < nent; ent++)
  {
    trspal->GetEntry(ent);
    bool hitAD1 = (trspal->GetLeaf("hitAD1")->GetValue()+1)/2;
    bool hitAD2 = (trspal->GetLeaf("hitAD2")->GetValue()+1)/2;
    bool hitAD3 = (trspal->GetLeaf("hitAD3")->GetValue()+1)/2;
    bool hitAD4 = (trspal->GetLeaf("hitAD4")->GetValue()+1)/2;
    bool hitAD = (hitAD1 || hitAD2 || hitAD3 || hitAD4);
    bool hitIWS = (trspal->GetLeaf("hitIWS")->GetValue()+1)/2;
    bool hitOWS = (trspal->GetLeaf("hitOWS")->GetValue()+1)/2;
    bool hitWS = (hitIWS || hitOWS);
    bool hitRPC = (trspal->GetLeaf("hitRPC")->GetValue()+1)/2;
    if(hitRPC && !hitAD && !hitWS) listRpcNum.push_back(trspal->GetLeaf("triggerNumber_RPC")->GetValue());
    //if(hitRPC) listRpcNum.push_back(trspal->GetLeaf("triggerNumber_RPC")->GetValue());
  }
  cout << "number of muons triggering only RPC: " << listRpcNum.size() << endl;

  
  /// check the RPC hits in the CalibReadoutHeader about the hit pattern
  /// i.e. hitting RPC array and/or RPC telescope
  TTree* trcalib = (TTree*)rootinf.Get("/Event/CalibReadout/CalibReadoutHeader");
  nent = trcalib->GetEntries();
  map<int,int> numHitPattern;
  numHitPattern[1] = 0;
  numHitPattern[2] = 0;
  numHitPattern[3] = 0;
  for(int ent = 0; ent < nent; ent++)
  {
    //if(ent >= 30) break;
    
    /// if no more RPCs in the list, just wrap up
    if(!listRpcNum.size()) break;
    trcalib->GetEntry(ent);
    int detid = trcalib->GetLeaf("detector")->GetValue();
    
    /// only process RPC
    if(detid != 7) continue;
    
    /// if this trigger is not tagged as muon, do nothing
    int trigNum = trcalib->GetLeaf("triggerNumber")->GetValue();
    vector<int>::iterator itTarget = find(listRpcNum.begin(), listRpcNum.end(), trigNum);
    if(itTarget == listRpcNum.end()) continue;
    listRpcNum.erase(itTarget);
    
    /// loop over all hits and record the ID's of triggered modules
    int nhit = trcalib->GetLeaf("nHitsRpc")->GetValue();
    set<pair<int,int> > modList;
    for(int h = 0; h < nhit; h++)
    {
      int row = trcalib->GetLeaf("rpcRow")->GetValue(h);
      int col = trcalib->GetLeaf("rpcColumn")->GetValue(h);
      pair<int,int> mId(row,col);
      modList.insert(mId);
    }
    int hall = trcalib->GetLeaf("site")->GetValue();
    //cout << includeRpcArray(modList,hall) << " ";
    //cout << includeRpcTelescope(modList,hall) << endl;
    int hitpattern = 0;
    if(includeRpcArray(modList,hall)) hitpattern += 1;
    if(includeRpcTelescope(modList,hall)) hitpattern += 2;
    numHitPattern[hitpattern]++;
    //cout << modList.size() << endl;
    //int row = trcalib->GetLeaf("rpcLayer")->GetValue(0);
    //cout << nhit << " " << row << endl;
    //if()
    //if(find(listRpcNum.begin(), listRpcNum.end(), ) == listRpcNum.end()) continue;
  }
  
  map<int,int>::iterator ithit = numHitPattern.begin();
  for(; ithit != numHitPattern.end(); ithit++)
    cout << ithit->first << " " << ithit->second << endl;
  
  
  return 0;
}



bool includeRpcArray(set<pair<int,int> >& mods, int hall)
{
  int maxrow = 6;
  if(hall == 4) maxrow = 9;
  
  set<pair<int,int> >::iterator it = mods.begin();
  for(; it != mods.end(); it++)
  {
    int row = it->first;
    if(row >= 1 && row <= maxrow) return true;
  }
  return false;
}



bool includeRpcTelescope(set<pair<int,int> >& mods, int hall)
{
  int maxrow = 7;
  if(hall == 4) maxrow = 10;
  
  set<pair<int,int> >::iterator it = mods.begin();
  for(; it != mods.end(); it++)
  {
    int row = it->first;
    if(row == 0 || row == maxrow) return true;
  }
  return false;
}
