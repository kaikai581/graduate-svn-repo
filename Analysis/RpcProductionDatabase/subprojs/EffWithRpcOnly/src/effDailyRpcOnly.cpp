/*
 * This program calculate RPC efficiency with RPC hits only. Input files are
 * refined root files generated with "RpcProductionDatabase" program.
*/


#include <string>
#include <tclap/CmdLine.h>


using namespace std;


void eventLoop(string, string, string, int, int, bool);


int main(int argc, char** argv)
{
  /// parsing command line arguments
  TCLAP::CmdLine cmd("Command description message", ' ', "0.9");
  
  TCLAP::ValueArg<string> ipArg("i", "input",
  "Input file path", false,
  "/home/sklin/Data/sklin/Data/DayaBayLocalData/RootFiles/RpcProductionTree/2012/p12b/Neutrino/0101/rpctree.Neutrino.0021358.Physics.EH1-Merged.P12B-P._0002.root",
  "string");
  
  TCLAP::ValueArg<string> opArg("o", "output",
  "Output file path", false, "./", "string");
  
  TCLAP::ValueArg<string> hallArg("a", "hall",
  "Hall number", false, "1", "string");
  
  TCLAP::ValueArg<int> rowArg("r", "row",
  "Row number", false, 3, "integer");
  
  TCLAP::ValueArg<int> colArg("c", "column",
  "Column number", false, 4, "integer");
  
  TCLAP::SwitchArg dbgArg("g", "debug", "Print debug info", false);
  
  cmd.add(ipArg);
  cmd.add(opArg);
  cmd.add(hallArg);
  cmd.add(rowArg);
  cmd.add(colArg);
  cmd.add(dbgArg);
  cmd.parse(argc, argv);
  
  string inputPath = ipArg.getValue();
  string outputPath = opArg.getValue();
  string hall = hallArg.getValue();
  int row = rowArg.getValue();
  int col = colArg.getValue();
  bool dbgFlag = dbgArg.getValue();
  
  eventLoop(inputPath, outputPath, hall, row, col, dbgFlag);
  
  return 0;
}
