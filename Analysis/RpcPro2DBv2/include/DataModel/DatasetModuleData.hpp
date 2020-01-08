#ifndef DATASETMODULEDATA_HPP
#define DATASETMODULEDATA_HPP



#include <vector>
#include "rpcnamespace.hpp"
class ReadoutData;



class DatasetModuleData
{
public:
  /// member variables
  
  /*
   * counter for 3/4-fold events
   * Index 0 is 4-fold counts. Index i is 3-fold with layer i absent.
   * "CS" suffix means AFTER cluster size cut.
   */
  std::vector<unsigned int>    nFoldCountsCS;
  /// Under Dr. Lau's instruction, count only the interior events.
  /// Here interior by 1/2 strip along the edges.
  std::vector<unsigned int>    nFoldCountsCSInt1;
  std::vector<unsigned int>    nFoldCountsCSInt2;
  
  
  /// calculated efficiency with cluster size cut
  /// Index 0 is module efficiency.
  /// Index i is ith layer efficiency.
  /// Suffix "CS" means Cluster Size cut.
  std::vector<rpc::efficiency> effCS;
  /// Again, suffix means #strips cut from the rim of a module.
  std::vector<rpc::efficiency> effCSInt1;
  std::vector<rpc::efficiency> effCSInt2;
  
  
  /// trigger time stamp
  std::vector<unsigned int>    triggerTimeSec;
  std::vector<unsigned int>    triggerTimeNanoSec;
  /// trigger time in second
  std::vector<double>          triggerTimeCS;
  
  /// trigger rate and its error
  std::pair<double, double>    triggerRateCS;
  
  
  /// member functions
  DatasetModuleData();

  void         fillModuleEfficiency();
  void         fillTriggerRate();
  void         incrementNFold(ReadoutData&);
  //void         incrementTriggerTime(double);
  void         incrementTriggerTime(unsigned int, unsigned int);
  void         printDatasetModuleData();

private:
  void         fillLayerEfficiencies();
};




#endif
