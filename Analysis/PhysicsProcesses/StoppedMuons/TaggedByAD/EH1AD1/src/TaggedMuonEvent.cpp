#include <iostream>
#include "TaggedMuonEvent.hpp"
#include "TLeaf.h"
#include "TTree.h"



using namespace std;




TaggedMuonEvent::TaggedMuonEvent(TTree* tsp, int entry)
{
  fillVariables(tsp, entry);
}




void TaggedMuonEvent::fillVariables(TTree* tsp, int entry)
{
  if(entry >= tsp->GetEntries())
  {
    cerr << "TaggedMuonEvent::fillVariables entry out of range" << endl;
    return;
  }
  
  tsp->GetEntry(entry);

  for(int i = 0; i < 4; i++)
  {
    string tnad = Form("triggerNumber_AD%d", i+1);
    triggerNumberAD[i] = (int)tsp->GetLeaf(tnad.c_str())->GetValue();
  }
  
  hitRPC = (int)tsp->GetLeaf("hitRPC")->GetValue();
  triggerNumberRPC = (int)tsp->GetLeaf("triggerNumber_RPC")->GetValue();
}
