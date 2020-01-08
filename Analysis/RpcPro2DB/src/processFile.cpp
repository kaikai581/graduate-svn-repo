#include <iostream>
#include "CsNVars.hpp"
#include "EnhancedVars.hpp"
#include "EnhancedTreeVars.hpp"
#include "PerCalibReadoutEvent/PerCalibReadoutHeader.h"
#include "structopt.hpp"
#include "TEntryList.h"
#include "TFile.h"
#include "TTree.h"
#include "utilities.hpp"


#define NSCAN 7

using namespace std;


EnhancedVars processEvent(PerCalibReadoutHeader*, vector<CsNVars>&);


void processFile(string ifpn)
{
  
  option_t opt;
  
  unsigned int runNumber = atoi(getRunNumber(ifpn).c_str());
  unsigned int fileNumber = atoi(getFileNumber(ifpn).c_str());
  
  /// exclude non-physics runs
  if(!runNumber) return;
  cout << "processing run " << runNumber << " file " << fileNumber << endl;
  
  
  /// open the TFile with the option set by owerwrite flag
  string openOpt;
  if(opt.owFlag) openOpt = "update";
  else       openOpt = "read";
  
  
  
  TFile fif(ifpn.c_str(), openOpt.c_str());
  TTree *treeCalib = (TTree*)fif.Get("/Event/CalibReadout/CalibReadoutHeader");
  
  if(!treeCalib)
  {
    cout << "spallation tree not found" << endl;
    return;
  }
  
  
  /// look at only RPC events, i.e. nHitsRpc>=1
  treeCalib->Draw(">>elist", "nHitsRpc>=1", "entrylist");
  TEntryList *elist = (TEntryList*)gDirectory->Get("elist");
  
  /// dataset level storage
  PerCalibReadoutHeader* rh = 0;
  treeCalib->SetBranchAddress("CalibReadout_CalibReadoutHeader", &rh);
  
  
  /// allocate output
  TFile fof(Form("%s", getOutputPathName(ifpn, opt.outputPath, string("rpctrees")).c_str()), "recreate");
  TTree ent("enhanced", "an enhanced RPC tree");
  EnhancedTreeVars etvars(ent);
  
  vector<TTree*> csnt(NSCAN);
  vector<EnhancedTreeVars*> csntreevars(NSCAN);
  for(unsigned int i = 0; i < NSCAN; i++)
  {
    csnt[i] = new TTree(Form("cs%d", i+1), Form("RPC tree with cluster size > %d removed", i+1));
    csntreevars[i] = new EnhancedTreeVars(*csnt[i]);
  }

  
  /// start event loop
  Long64_t listEntries = elist->GetN();
  for(int entry = 0; entry < listEntries; entry++)
  {
    
    if(opt.nEvt>0 && entry>=opt.nEvt)
      break;
    
    treeCalib->GetEntry(elist->GetEntry(entry));
    

    vector<CsNVars> csnvars(7);

    
    EnhancedVars ev = processEvent(rh, csnvars);
    ev.eventId = entry;
    ev.runNumber = runNumber;
    ev.fileNumber = fileNumber;
    

    etvars.fillTreeVars(ev);
    ent.Fill();
    for(unsigned int i = 0; i < NSCAN; i++)
    {
      csntreevars[i]->fillTreeVars(csnvars[i]);
      csnt[i]->Fill();
    }
    
  }
  
  ent.Write();
  for(int i = 0; i < NSCAN; i++)
    csnt[i]->Write();
  
  
  
  for(int i = 0; i < NSCAN; i++)
  {
    /// NOTE: If the trees are deleted after close of TFile,
    /// segmentation faults will result.
    delete csnt[i];
    delete csntreevars[i];
  }
  fof.Close();
}
