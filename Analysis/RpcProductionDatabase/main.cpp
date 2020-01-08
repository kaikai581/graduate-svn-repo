#include <string>
#include "commonfunctions.h"
#include "loop2ROOT.h"
#include "tclap/CmdLine.h"
#include "TROOT.h"


int main(int argc, char** argv)
{
  /// load CalibReadoutHeader data model
  /// without this line, ROOT will complain about
  /// "no dictionary for class SOME_CLASS"
  //gROOT->ProcessLine(".x $ROOTIOTESTROOT/share/load.C");
  globalSettings();
  
  /// parse command line arguments with TCLAP
  TCLAP::CmdLine cmd("Command description message", ' ', "0.9");
  TCLAP::ValueArg<string> ifpathnameArg("i", "input",
    "Input file pathname", true, "", "string");
  TCLAP::ValueArg<string> ofpathnameArg("o", "output",
    "Output file path or pathname", false, "", "string");
  TCLAP::SwitchArg mysqlSwitch("d", "database",
    "Write to ROOT or MySQL. If -d is specified, output will be written to MySQL database. Otherwise ROOT.", false);
  TCLAP::ValueArg<int> nEvtArg("n", "number",
  "Number of events", false, -1, "integer");
  TCLAP::SwitchArg dbgArg("g", "debug", "Print debug info", false);
  cmd.add(ifpathnameArg);
  cmd.add(ofpathnameArg);
  cmd.add(nEvtArg);
  cmd.add(mysqlSwitch);
  cmd.add(dbgArg);
  cmd.parse(argc, argv);
  
  string ifpn = ifpathnameArg.getValue();
  string ofpn = getOutputPathName(ifpn, ofpathnameArg.getValue());
  int nEvt = nEvtArg.getValue();
  bool dbgFlag = dbgArg.getValue();
  if(ofpn == "")
  {
    cout << "Cannot open output file. ";
    cout << "Please check path and filename." << endl;
    return -1;
  }
  else
  {
    cout << "Results are written to:" << endl;
    cout << "  " << ofpn << endl;
  }
  
  
  /// start event loop and output
  if(!fExists(ifpn.c_str()))
  {
    cout << "Cannot open input file" << endl;
    return -1;
  }
  if(!mysqlSwitch.getValue())
    loop2ROOT(ifpn, ofpn, nEvt, dbgFlag);
  
  return 0;
}
