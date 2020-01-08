#include <iostream>
#include "CsNVars.hpp"
#include "EnhancedTreeVars.hpp"
#include "EnhancedVars.hpp"
#include "PerCalibReadoutEvent/PerCalibReadoutHeader.h"
#include "RawVars.hpp"
#include "structopt.hpp"
#include "TTree.h"


using namespace std;


//EnhancedVars processEvent(PerCalibReadoutHeader* rh, vector<CsNVars*> cnv)
EnhancedVars processEvent(PerCalibReadoutHeader* rh, vector<CsNVars>& cnv)
{
  option_t opt;
  
  EnhancedVars   ev(rh);
  for(unsigned int i = 0; i < cnv.size(); i++)
  {
    cnv[i].filterClusters(ev, i+1);
  }
  

  if(opt.dbgFlag)
  {
    cout << ev.getTitle() << ":" << endl;
    ev.printRpcRow();
    ev.printRpcColumn();
    ev.printRpcLayer();
    ev.printRpcStrip();
    ev.printRpcFromRot();
    ev.printRearranged();
    ev.printClusters();
    ev.printClusterCMS();
    ev.printClusterDistance();
    ev.printTracks();
    ev.printForcedTracks();
    ev.printSpurios();
    cout << endl;
    
    for(unsigned int i = 0; i < cnv.size(); i++)
    {
      //cout << cnv[i]->getTitle() << ":" << endl;
      //cnv[i]->printClusters();
      //cnv[i]->printForcedTracks();
      cout << cnv[i].getTitle() << ":" << endl;
      cnv[i].printClusters();
      cnv[i].printForcedTracks();
    }
    for(int i = 0; i < 80; i++) cout << "-";
    cout << endl;
  }
  
  return ev;
}
