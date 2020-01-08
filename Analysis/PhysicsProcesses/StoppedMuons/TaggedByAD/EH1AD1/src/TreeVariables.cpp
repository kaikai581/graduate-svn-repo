#include "TreeVariables.hpp"

using namespace std;

short         TreeVariables::detector(-1);
unsigned int  TreeVariables::triggerNumberMu(0);
float         TreeVariables::energyMu(-1.);
long          TreeVariables::triggerTimeMu(-1);
double        TreeVariables::muTime(-1.);
float         TreeVariables::nPESumMu(0.);
float         TreeVariables::xmu;
float         TreeVariables::ymu;
float         TreeVariables::zmu;
unsigned int  TreeVariables::triggerNumberE(0);
float         TreeVariables::energyE(-1.);
long          TreeVariables::triggerTimeE(-1);
double        TreeVariables::eTime(-1.);
float         TreeVariables::nPESumE(0.);
float         TreeVariables::xe;
float         TreeVariables::ye;
float         TreeVariables::ze;
unsigned long TreeVariables::dt(0);
bool          TreeVariables::isMichel(false);


long                                   TreeVariables::triggerTimeRpc(0);
double                                 TreeVariables::rpcTime(-1.);
int                                    TreeVariables::triggerNumberRpc(0);
unsigned int                           TreeVariables::nModules(0);
vector<unsigned int>                   TreeVariables::rpcRow;
vector<unsigned int>                   TreeVariables::rpcCol;
vector<vector<unsigned int> >          TreeVariables::rpcLayer;
vector<vector<vector<unsigned int> > > TreeVariables::rpcStrip;
vector<bool>                           TreeVariables::rpcFromRot;
