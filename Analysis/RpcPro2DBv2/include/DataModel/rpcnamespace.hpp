#ifndef RPCNAMESPACE_HPP
#define RPCNAMESPACE_HPP


#include <map>
#include <vector>
class ModuleData;
class DatasetModuleData;


namespace rpc
{
  typedef std::pair<unsigned int, unsigned int> moduleId;
  typedef std::map<rpc::moduleId, ModuleData>   readout;
  
  /// Prefix "ds" means dataset-wise.
  
  /// efficiency for a readout
  /// The first value is the calculated efficiency.
  /// The second value is the associated error.
  /// Index 0 is module efficiency. Index i is ith layer efficiency.
  typedef std::pair<double,double>                          efficiency;
  typedef std::map<rpc::moduleId, DatasetModuleData>        dsModules;
  
  
  typedef std::vector<std::pair<unsigned int, unsigned int> > mTrackPairIndices;
  /// reconstructed muon positions in a module
  typedef std::vector<std::pair<double, double> >             mReconPoints;
}


#endif
