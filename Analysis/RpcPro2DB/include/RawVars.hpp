/*
 * Base variables are variables directly copied from production root files.
 */


#ifndef RAWVARS_HPP
#define RAWVARS_HPP

#include <vector>
#include "PerCalibReadoutEvent/PerCalibReadoutHeader.h"
#include "rpcnamespace.hpp"


class RawVars
{
public:

  unsigned int site;
  unsigned int detector;
  unsigned int triggerNumber;
  unsigned int triggerType;
  unsigned int triggerTimeSec;
  unsigned int triggerTimeNanoSec;
  unsigned int nHitsRpc;
  
  rpc::mFromRot mRpcFromRot;
  
  /*
   * If an event has no fired strips with id 1 or 8, it is defined as interior.
   * Such kind of fiducial cut on RPC modules is made to avoid difficulties
   * resulting from stagger arrangement of modules.
   * If all events in a readout are interior, allHitsInterior is set true.
   * Otherwise, false.
   */
  static bool         allHitsInterior;
  /// allHitsInterior2: every hit is more than 2 strips away from the edge.
  static bool         allHitsInterior2;
  
  
  unsigned int                getSite() {return site;}
  unsigned int                getDetector() {return detector;}
  unsigned int                getTriggerNumber() {return triggerNumber;}
  unsigned int                getTriggerType() {return triggerType;}
  unsigned int                getTriggerTimeSec() {return triggerTimeSec;}
  unsigned int                getTriggerTimeNanoSec() {return triggerTimeNanoSec;}
  unsigned int                getNHitsRpc() {return nHitsRpc;}
  std::vector< unsigned int > getRpcRow() {return rpcRow;}
  std::vector< unsigned int > getRpcColumn() {return rpcColumn;}
  std::vector< unsigned int > getRpcLayer() {return rpcLayer;}
  std::vector< unsigned int > getStrip() {return rpcStrip;}
  std::vector< bool >         getRpcFromRot() {return rpcFromRot;}
  
  
  
  std::string getTitle() {return varTitle;};
  
  void printRpcRow();
  void printRpcColumn();
  void printRpcLayer();
  void printRpcStrip();
  void printRpcFromRot();
  void printRearranged();


protected:
  RawVars(){};
  RawVars(PerCalibReadoutHeader*);
  
  rpc::mEvent   mRpcReadout; // variable used for storing rearranged information
  
  std::string   varTitle;
  
private:

  std::vector< unsigned int > rpcRow;
  std::vector< unsigned int > rpcColumn;
  std::vector< unsigned int > rpcLayer;
  std::vector< unsigned int > rpcStrip;
  std::vector< bool > 	      rpcFromRot;
  
  void rearrangeVars();
};


#endif
