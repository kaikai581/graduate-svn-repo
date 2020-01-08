/*
 * This program calculates RPC layer efficiency by fitting intertrigger time
 * of 3 fold and 4 fold separately. See UH group meeting for more details.
*/


#include <string>
#include <tclap/CmdLine.h>
#include "structopt.hpp"
#include "utilities.hpp"


using namespace std;


void eventLoop(option_t);


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
  
  TCLAP::ValueArg<int> hallArg("a", "hall",
  "Hall number", false, 1, "integer");
  
  TCLAP::ValueArg<int> rowArg("r", "row",
  "Row number", false, 3, "integer");
  
  TCLAP::ValueArg<int> colArg("c", "column",
  "Column number", false, 4, "integer");
  
  TCLAP::ValueArg<int> nEvtArg("n", "number",
  "Number of events", false, 0, "integer");
  
  TCLAP::ValueArg<string> strCutArg("u", "cut",
  "Cut", false, "", "string");
  
  TCLAP::SwitchArg dbgArg("g", "debug", "Print debug info", false);
  
  cmd.add(ipArg);
  cmd.add(opArg);
  cmd.add(hallArg);
  cmd.add(rowArg);
  cmd.add(colArg);
  cmd.add(nEvtArg);
  cmd.add(strCutArg);
  cmd.add(dbgArg);
  cmd.parse(argc, argv);
  
  
  option_t opt;
  opt.inputPath = trimTrailingSlash(ipArg.getValue());
  opt.outputPath = opArg.getValue();
  opt.hall = hallArg.getValue();
  opt.row = rowArg.getValue();
  opt.col = colArg.getValue();
  opt.nEvt = nEvtArg.getValue();
  opt.strCut = strCutArg.getValue();
  opt.dbgFlag = dbgArg.getValue();
  
  
  eventLoop(opt);
  
  return 0;
}
