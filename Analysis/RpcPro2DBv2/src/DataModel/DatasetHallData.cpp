#include <iostream>
#include "DataModel/DatasetHallData.hpp"
#include "DataModel/ReadoutData.hpp"
#include "TFile.h"


using namespace std;





DatasetHallData::DatasetHallData(string ifpn)
{
  runNumber = getRunNumber(ifpn);
  fileNumber = getFileNumber(ifpn);
  
  /// set size of cluster size cut
  setClusterCutSize(8);
}






DatasetHallData::DatasetHallData(string ifpn, unsigned int clsize)
{
  runNumber = getRunNumber(ifpn);
  fileNumber = getFileNumber(ifpn);
  
  /// set size of cluster size cut
  setClusterCutSize(clsize);
}






void DatasetHallData::incrementDataset(ReadoutData& ro)
{

  site = ro.site;
  
  for(rpc::readout::iterator it = ro.modules.begin(); it != ro.modules.end(); it++)
  {
    /// check if this module's data already exist
    rpc::dsModules::iterator itmod = modules.find(it->first);
    
    /// nonexistent: allocate space for N-fold counter and initialize
    if(itmod == modules.end())
      modules[it->first] = DatasetModuleData();

    /// increment n-fold counters
    if(it->second.getUnfiredLayersOffline().size() == 0) // 4-fold
    {
      modules[it->first].nFoldCountsCS[0]++;
      if(ro.allHitsInterior1)
        modules[it->first].nFoldCountsCSInt1[0]++;
      if(ro.allHitsInterior2)
        modules[it->first].nFoldCountsCSInt2[0]++;
    }
    else if(it->second.getUnfiredLayersOffline().size() == 1) // 3-fold
    {
      modules[it->first].nFoldCountsCS[it->second.getUnfiredLayersOffline()[0]]++;
      if(ro.allHitsInterior1)
        modules[it->first].nFoldCountsCSInt1[it->second.getUnfiredLayersOffline()[0]]++;
      if(ro.allHitsInterior2)
        modules[it->first].nFoldCountsCSInt2[it->second.getUnfiredLayersOffline()[0]]++;
    }

    /// increment module trigger time
    //double triggerTime = ro.triggerTimeSec + 1e-9*ro.triggerTimeNanoSec;
    //modules[it->first].incrementTriggerTime(triggerTime);
    modules[it->first].incrementTriggerTime(ro.triggerTimeSec, ro.triggerTimeNanoSec);
  }
  
  /// fill data AFTER number of tracks cut
  for(rpc::readout::iterator it = ro.modulesNT.begin(); it != ro.modulesNT.end(); it++)
  {
    /// check if this module's data already exist
    rpc::dsModules::iterator itmod = modulesNT.find(it->first);
    
    /// nonexistent: allocate space for N-fold counter and initialize
    if(itmod == modulesNT.end())
      modulesNT[it->first] = DatasetModuleData();

    /// increment n-fold counters
    if(it->second.getUnfiredLayersOffline().size() == 0) // 4-fold
    {
      modulesNT[it->first].nFoldCountsCS[0]++;
      if(ro.allHitsInterior1)
        modulesNT[it->first].nFoldCountsCSInt1[0]++;
      if(ro.allHitsInterior2)
        modulesNT[it->first].nFoldCountsCSInt2[0]++;
    }
    else if(it->second.getUnfiredLayersOffline().size() == 1) // 3-fold
    {
      modulesNT[it->first].nFoldCountsCS[it->second.getUnfiredLayersOffline()[0]]++;
      if(ro.allHitsInterior1)
        modulesNT[it->first].nFoldCountsCSInt1[it->second.getUnfiredLayersOffline()[0]]++;
      if(ro.allHitsInterior2)
        modulesNT[it->first].nFoldCountsCSInt2[it->second.getUnfiredLayersOffline()[0]]++;
    }

    /// increment module trigger time
    //double triggerTime = ro.triggerTimeSec + 1e-9*ro.triggerTimeNanoSec;
    //modulesNT[it->first].incrementTriggerTime(triggerTime);
    modulesNT[it->first].incrementTriggerTime(ro.triggerTimeSec, ro.triggerTimeNanoSec);
  }
  
}






void DatasetHallData::fillDatasetModuleVariables()
{
  for(rpc::dsModules::iterator it = modules.begin(); it != modules.end(); it++)
  {
    it->second.fillModuleEfficiency();
    it->second.fillTriggerRate();
  }

  for(rpc::dsModules::iterator it = modulesNT.begin(); it != modulesNT.end(); it++)
  {
    it->second.fillModuleEfficiency();
    it->second.fillTriggerRate();
  }
}






unsigned int DatasetHallData::getFileNumber(string ifpn)
{
  if(ifpn.find("_") == string::npos)
    return atoi(string("").c_str());
  return atoi(ifpn.substr(ifpn.find("_")+1, 4).c_str());
}





unsigned int DatasetHallData::getRunNumber(string ifpn)
{
  for(int i = 0; i < 2; i++)
  {
    if(ifpn.find(".") == string::npos)
      return atoi(string("").c_str());
    ifpn = ifpn.substr(ifpn.find(".")+1, ifpn.length()-ifpn.find("."));
  }
  return atoi(ifpn.substr(0, 7).c_str());
}





void DatasetHallData::printDatasetHallData()
{
  for(rpc::dsModules::iterator it = modules.begin(); it != modules.end(); it++)
  {
    cout << "module (" << it->first.first << ",";
    cout << it->first.second << ")" << endl;

    it->second.printDatasetModuleData();
    
    cout << "\t\t\t AFTER NUMBER OF TRACKS CUT" << endl;
    if(modulesNT.find(it->first) != modulesNT.end())
      modulesNT[it->first].printDatasetModuleData();
    else
      cout << "\t REMOVED BY CUTS" << endl;
  }
}





void DatasetHallData::setClusterCutSize(unsigned int clsize)
{
  clCutSize = clsize;
}





void DatasetHallData::write2ROOT(TFile& fof)
{
  int nrows, ncols = 9;
  
  if(site == 1 || site == 2) nrows = 8;
  else                       nrows = 11;
  
  string hname = Form("effCS%d", clCutSize);
  
  TH2F heff(hname.c_str(), hname.c_str(), ncols, 1, ncols+1, nrows, 0, nrows);
  
  for(rpc::dsModules::iterator it = modulesNT.begin(); it != modulesNT.end(); it++)
  {
    heff.SetBinContent(it->first.second, it->first.first, modulesNT[it->first].effCSInt2[0]);
  }
}
