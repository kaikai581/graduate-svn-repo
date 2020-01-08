#ifndef DATASETHALLDATA_HPP
#define DATASETHALLDATA_HPP


#include <string>
#include <vector>
#include "DataModel/DatasetModuleData.hpp"
#include "rpcnamespace.hpp"
class ReadoutData;
class TFile;



class DatasetHallData
{
public:
  /*
   * member variables
   */
  unsigned int runNumber;
  unsigned int fileNumber;
  unsigned int site;

  
  /// size of cluster size cut
  unsigned int clCutSize;

  /// module data of a dataset
  rpc::dsModules     modules;
  /// module data of a dataset AFTER number of tracks cut
  rpc::dsModules     modulesNT;


  /*
   * member functions
   */
  DatasetHallData(std::string);
  DatasetHallData(std::string, unsigned int);
  
  void         fillDatasetModuleVariables();
  unsigned int getRunNumber(std::string);
  unsigned int getFileNumber(std::string);
  
  /// increment n-fold counters and trigger times
  void         incrementDataset(ReadoutData&);
  void         printDatasetHallData();
  void         setClusterCutSize(unsigned int);
  
  /// output the results
  void         write2ROOT(TFile&);
};



#endif
