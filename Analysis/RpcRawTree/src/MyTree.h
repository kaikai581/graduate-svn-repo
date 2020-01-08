#ifndef MYTREE_H
#define MYTREE_H
#include "TTree.h"
#include <vector>

// be able to accommodate EH3
// 32 strips/module.
// 81 modules at most
#define MAXROSTRIPS 2592
#define MAXROMODULES 81
#define MAXSTRLEN 200

/*/// my first try of vector in TTree
#ifdef __MAKECINT__
#pragma link C++ class vector<int>+;
#pragma link C++ class vector<vector<int> >+;
#endif*/

class MyTree
{
public:
  int roSite;
  int roDetector;
  int roTriggerNumber;
  bool isForcedTrigger;
  double triggerSpan;
  char triggerType[MAXSTRLEN];
  unsigned int eventNumber;
  unsigned int localTriggerNumber;
  unsigned int runNumber;
  unsigned int event;
  bool hasTriggers;
  int eventType;
  unsigned int nReadoutPanels;
  unsigned int nHitPanels; // number of modules with hits
  int firedLayerNum[MAXROMODULES];
  bool fromRot[MAXROMODULES];
  
  /// For each RPC trigger multiple modules could be readout regardless
  /// having hits or not
  /// hitRow & hitCol only save modules with hits
  std::vector<int>* hitRow;
  std::vector<int>* hitCol;
  std::vector<int>* nLayers;
  std::vector<bool>* forceTrigger;
  std::vector<std::vector<int> >* stripId;
  int roTriggerTimeSec;
  int roTriggerTimeNanoSec;
  int nRoStrips;
  int nRoModules;
  int roRow[MAXROSTRIPS];
  int roColumn[MAXROSTRIPS];
  int roLayer[MAXROSTRIPS];
  int roStrip[MAXROSTRIPS];
  int nRoLayers[MAXROSTRIPS];

  int nStripsPerModule[MAXROMODULES];
	
  MyTree(){};
  ~MyTree(){};
};

#endif
