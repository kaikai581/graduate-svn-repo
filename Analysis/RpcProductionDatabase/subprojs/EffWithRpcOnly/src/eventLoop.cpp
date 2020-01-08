#include <iostream>
#include "TChain.h"
#include "TEntryList.h"
#include "TROOT.h"

using namespace std;

void eventLoop(string ifp, string ofp, string hall, int row, int col, bool dbgFlag)
{
  TChain rpcChain("rpcTree");
  rpcChain.Add(ifp.c_str());
  
  string moduleSelection = Form("mRpcRow==%d&&mRpcColumn==%d", row, col);
  
  /// make cut on the tree chain
  rpcChain.Draw(">>elist", moduleSelection.c_str(), "entrylist");
  TEntryList *elist = (TEntryList*)gDirectory->Get("elist");
  Long64_t listEntries = elist->GetN();
  Int_t treenum = 0;
  rpcChain.SetEntryList(elist);
  
  /// print number of events information
  if(dbgFlag)
  {
    cout << "Number of events before module cut:" << rpcChain.GetEntries() << endl;
    cout << "Number of events after module cut:" << listEntries << endl;
  }
  
  /// setup container variables
  vector<unsigned int>* mRpcRow = new vector<unsigned int>;
  vector<unsigned int>* mRpcColumn = new vector<unsigned int>;
  rpcChain.SetBranchAddress("mRpcRow", &mRpcRow);
  rpcChain.SetBranchAddress("mRpcColumn", &mRpcColumn);
  
  
  
  /// loop over events passing the cut
  for (Long64_t el = 0; el < listEntries; el++)
  {
    Long64_t treeEntry = elist->GetEntryAndTree(el, treenum);
    Long64_t chainEntry = treeEntry + rpcChain.GetTreeOffset()[treenum];

    rpcChain.GetEntry(chainEntry);
    
    
    
    /// print debug information
    if(dbgFlag)
    {
      cout << "RPC row: ";
      for(unsigned int i = 0; i < mRpcRow->size(); i++)
        cout << mRpcRow->at(i) << " ";
      cout << endl;
      
      cout << "RPC col: ";
      for(unsigned int i = 0; i < mRpcColumn->size(); i++)
        cout << mRpcColumn->at(i) << " ";
      cout << endl;
    }
    cout << endl;
  }

}
