/*
 * 
 * This program will extract RPC events from production files and store them in
 * a new file with a tree name rawrpc. At the same time it will calculate some
 * pertinent variables and store them in a tree named enrpc, meaning enhanced
 * RPC. Finally it will discard any clusters with more than 2 strips and make a
 * new tree called cs2rpc, meaning "cluster size 2".
 * 
 */


#include "structopt.hpp"
#include "tclap/CmdLine.h"
#include "utilities.hpp"


using namespace std;


void processFile(string);


int main(int argc, char** argv)
{
  /// parsing command line arguments
  TCLAP::CmdLine cmd("Command description message", ' ', "0.9");
  
  //TCLAP::ValueArg<string> ipArg("i", "input",
  //"Input file path", false,
  //"/eliza16/dayabay/data/exp/dayabay/2012/p12b/Neutrino/0101",
  //"string");
  
  TCLAP::SwitchArg dbArg("d", "database", "Write to database", false);
  
  TCLAP::SwitchArg dbgArg("g", "debug", "Print debug info", false);
  
  TCLAP::ValueArg<string> ipArg("i", "input",
  "Input file path", false, "input", "string");
  
  TCLAP::ValueArg<int> nEvtArg("n", "number",
  "Number of events", false, -1, "integer");
  
  TCLAP::ValueArg<string> opArg("o", "output",
  "Output file path", false, "output", "string");
  
  TCLAP::ValueArg<int> printEvtArg("p", "print",
  "Print debug info for the event specified by its enrty number", false, -1,
  "integer");

  TCLAP::ValueArg<int> scanSizeArg("s", "size",
  "Maximum size of cluster size scan", false, 8, "integer");
  
  TCLAP::ValueArg<string> dbOpArg("u", "dboutput",
  "Database output file path", false, "output", "string");

  
  TCLAP::SwitchArg overwriteArg("v", "overwrite", "Overwrite the input root file", false);

  
  
  cmd.add(ipArg);
  cmd.add(opArg);
  cmd.add(dbOpArg);
  cmd.add(nEvtArg);
  cmd.add(printEvtArg);
  cmd.add(scanSizeArg);
  cmd.add(overwriteArg);
  cmd.add(dbgArg);
  cmd.add(dbArg);
  cmd.parse(argc, argv);
  
  option_t opt;
  opt.inputPath = trimTrailingSlash(ipArg.getValue());
  opt.outputPath = opArg.getValue();
  opt.dbOutputPath = dbOpArg.getValue();
  opt.nEvt = nEvtArg.getValue();
  opt.printEvt = printEvtArg.getValue();
  opt.scanSize = scanSizeArg.getValue();
  opt.owFlag = overwriteArg.getValue();
  opt.dbgFlag = dbgArg.getValue();
  opt.dbFlag = dbArg.getValue();
    
  
  /*
   * store all file names to be processed
   */
  vector<string> vifpn;
  if(isRootFile(opt.inputPath))
    vifpn.push_back(opt.inputPath);
  else
    getInputFilePathnames(vifpn, opt.inputPath);

  
  
  
  if(opt.owFlag)
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
  
  
  for(unsigned int i = 0; i < vifpn.size(); i++) processFile(vifpn[i]);
  
  
  return 0;
}
