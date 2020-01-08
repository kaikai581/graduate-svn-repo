#include <string>
#include <tclap/CmdLine.h>
#include "EnrichedTreeVars.hpp"
#include "utilities.hpp"


using namespace std;


void addBranch2File(string, string, int, int, bool, bool);


int main(int argc, char** argv)
{
  
  /// parsing command line arguments
  TCLAP::CmdLine cmd("Command description message", ' ', "0.9");
  
  TCLAP::ValueArg<string> ipArg("i", "input",
  "Input file path", false,
  "/mnt/data6/scratch/sklin/MyData/DayaBay/RpcProductionTree/2012/p12b/Neutrino/0101",
  "string");
  
  TCLAP::ValueArg<string> opArg("o", "output",
  "Output file path", false, "./", "string");
  
  TCLAP::ValueArg<int> nEvtArg("n", "number",
  "Number of events", false, -1, "integer");
  
  TCLAP::ValueArg<int> printEvtArg("p", "print",
  "Print debug info for the event specified by its enrty number", false, -1,
  "integer");
  
  TCLAP::SwitchArg overwriteArg("v", "overwrite", "Overwrite the input root file", false);
  
  TCLAP::SwitchArg dbgArg("g", "debug", "Print debug info", false);
  
  
  cmd.add(ipArg);
  cmd.add(opArg);
  cmd.add(nEvtArg);
  cmd.add(printEvtArg);
  cmd.add(overwriteArg);
  cmd.add(dbgArg);
  cmd.parse(argc, argv);
  string inputPath = trimTrailingSlash(ipArg.getValue());
  string outputPath = opArg.getValue();
  int nEvt = nEvtArg.getValue();
  int printEvt = printEvtArg.getValue();
  bool owFlag = overwriteArg.getValue();
  bool dbgFlag = dbgArg.getValue();
  
  
  /*
   * store all file names to be processed
   */
  vector<string> vifpn;
  if(isRootFile(inputPath))
    vifpn.push_back(inputPath);
  else
    getInputFilePathnames(vifpn, inputPath);
  
  
  
  if(owFlag)
  {
    cerr << "overwrite function is not implemented yet" << endl;
    return -1;
  }
  /// start file loop
  /*
   * Load high dimentional vector containers.
   * Note: this loader has to be placed before TFile::Get()
   */
  loadDictionary();
  
  for(unsigned int i = 0; i < vifpn.size(); i++)
    addBranch2File(vifpn[i], outputPath, nEvt, printEvt, owFlag, dbgFlag);
  
  
  return 0;
}
