#include <iostream>
#include "Database/MySQLDB.hpp"
#include "Database/RpcCalibDB.hpp"
#include "DataModel/DatasetHallData.hpp"
#include "DataModel/ReadoutData.hpp"
#include "PerCalibReadoutEvent/PerCalibReadoutHeader.h"
#include "structopt.hpp"
#include "TEntryList.h"
#include "TFile.h"
#include "TTree.h"
#include "utilities.hpp"


using namespace std;



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
  else           openOpt = "read";
  
  
  
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

  
  //DatasetHallData dsd(ifpn, 3);
  /// do a cluster size cut scan
  
  const int ss = opt.scanSize;
  
  vector<DatasetHallData*> dsd;
  for(int i = 1; i <= ss; i++) dsd.push_back(new DatasetHallData(ifpn, i));
  
  /// MySQL database
  //MySQLDB mysqldb;
  //mysqldb.connect();

  /// start event loop
  Long64_t listEntries = elist->GetN();
  for(int entry = 0; entry < listEntries; entry++)
  {
    
    if(opt.nEvt>0 && entry>=opt.nEvt)
      break;
    
    treeCalib->GetEntry(elist->GetEntry(entry));
    
    //ReadoutData ro(rh, 3);
    
    /// print debug information
    //if(opt.dbgFlag) ro.printModules();
    
    /// fill in dataset information
    //dsd.incrementDataset(ro);
    vector<ReadoutData*> ro;
    for(int i = 0; i < ss; i++)
    {
      ro.push_back(new ReadoutData(rh, i+1));
      if(opt.dbgFlag) ro[i]->printModules();
      dsd[i]->incrementDataset(*ro[i]);
    }
    
    for(int i = 0; i < ss; i++) delete ro[i];
  }
  
  
  /// calculate module efficiency for a dataset
  //dsd.fillDatasetModuleVariables();
  /// print debug information
  //dsd.printDatasetHallData();
  
  //RpcCalibDB calibDB(dsd);
  vector<RpcCalibDB*> calibDB;
  
  for(int i = 0; i < ss; i++)
  {
    dsd[i]->fillDatasetModuleVariables();
    if(opt.dbgFlag) dsd[i]->printDatasetHallData();
    calibDB.push_back(new RpcCalibDB(*dsd[i]));
  }
  
  for(int i = 0; i < ss; i++)
  {
    delete dsd[i];
    delete calibDB[i];
  }

  fof.Close();
}
