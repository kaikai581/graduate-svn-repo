#ifndef TREEVARIABLES_HPP
#define TREEVARIABLES_HPP

#include <string>
#include <vector>


class TreeVariables
{
public:
  static short         detector;
  static unsigned int  triggerNumberMu;
  static float         energyMu;
  static long          triggerTimeMu;
  static double        muTime;
  static float         nPESumMu;
  static float         xmu;
  static float         ymu;
  static float         zmu;
  static unsigned int  triggerNumberE;
  static float         energyE;
  static long          triggerTimeE;
  static double        eTime;
  static float         nPESumE;
  static float         xe;
  static float         ye;
  static float         ze;
  static unsigned long dt;
  static bool          isMichel;
  
  static long                      triggerTimeRpc;
  static double                    rpcTime;
  static int                       triggerNumberRpc;
  static unsigned int              nModules;
  static std::vector<unsigned int> rpcRow;
  static std::vector<unsigned int> rpcCol;
  static std::vector<std::vector<unsigned int> > rpcLayer;
  static std::vector<std::vector<std::vector<unsigned int> > > rpcStrip;
  static std::vector<bool> rpcFromRot;
};



#endif
