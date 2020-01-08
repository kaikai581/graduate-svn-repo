#ifndef READOUTDATA_HPP
#define READOUTDATA_HPP


#include "DataModel/rpcnamespace.hpp"
#include "DataModel/ModuleData.hpp"
class PerCalibReadoutHeader;



class ReadoutData
{
public:
  /*
   * member variables
   */
  
  unsigned int site;
  unsigned int detector;
  unsigned int triggerNumber;
  unsigned int triggerType;
  unsigned int triggerTimeSec;
  unsigned int triggerTimeNanoSec;
  rpc::readout modules;
  
  /// container for data after number of tracks cut
  rpc::readout modulesNT;
  
  /// size of cluster cut:
  /// Clusters with size <= clCutSize are kept.
  unsigned int clCutSize;

  /*
   * If an event has no fired strips with id 1 or 8, it is defined as interior.
   * Such kind of fiducial cut on RPC modules is made to avoid difficulties
   * resulting from staggered arrangement of modules.
   * If all events in a readout are interior, allHitsInteriorN is set true.
   * Otherwise, false.
   * allHitsInteriorN means removing N strips along the edges of a RPC module.
   */
  bool         allHitsInterior1;
  /// allHitsInterior2: every hit is more than 2 strips away from the edge.
  bool         allHitsInterior2;



  /*
   * member functions
   */
  /// A cluster size parameter can be passed into the constructor
  /// which in turn is passed into LayerData::clusterize in order to
  /// achieve the cluster size cut.
  ReadoutData(PerCalibReadoutHeader*);
  ReadoutData(PerCalibReadoutHeader*, unsigned int);
  void fillVars(PerCalibReadoutHeader*, unsigned int);
  void printModules();
};




#endif
