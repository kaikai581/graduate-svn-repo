/*
 * This program calculates RPC rate by fitting muon interarrival time.
 * Input files are refined root files generated with "RpcProductionDatabase"
 * program.
*/


#include <fstream>
#include <string>
#include <tclap/CmdLine.h>
#include "TROOT.h"



using namespace std;


bool checkPath(string);
void getInputFilenames(vector<string>&, string, string);
void rateSetBySet(const vector<string>&, string, string, int, int, string,
                  unsigned int, bool);
void effSetBySet(const vector<string>&, string, string, int, int, unsigned int,
                 bool);
void getCorrectedRate(const vector<string>&, string, string, int, int);


int main(int argc, char** argv)
{
  /// parsing command line arguments
  TCLAP::CmdLine cmd("Command description message", ' ', "0.9");

  /// input path
  TCLAP::ValueArg<string> ipArg("i", "input", "Input file path", false,
                                "DataFiles/2012/p12b/Neutrino/0101", "string");


  TCLAP::ValueArg<string> opArg("o", "output",
  "Output file path", false, "./", "string");

  TCLAP::ValueArg<string> hallArg("a", "hall",
  "Hall number", false, "1", "string");

  TCLAP::ValueArg<int> rowArg("r", "row",
  "Row number", false, 3, "integer");

  TCLAP::ValueArg<int> colArg("c", "column",
  "Column number", false, 4, "integer");

  TCLAP::ValueArg<string> varArg("v", "variable",
  "dt variable name", false, "dtInSec4Fold", "string");

  TCLAP::ValueArg<unsigned int> nSetArg("n", "nset",
  "Number of dataset to be processed", false, 0, "integer");

  TCLAP::SwitchArg dbgArg("g", "debug", "Print debug info", false);

  TCLAP::SwitchArg divideArg("d", "divide", "Divide rate with efficiency",
                             false);

  TCLAP::SwitchArg effArg("e", "efficiency", "Switch on efficiency and turn off rate",
                          false);

  cmd.add(ipArg);
  cmd.add(opArg);
  cmd.add(hallArg);
  cmd.add(rowArg);
  cmd.add(colArg);
  cmd.add(varArg);
  cmd.add(nSetArg);
  cmd.add(dbgArg);
  cmd.add(divideArg);
  cmd.add(effArg);
  cmd.parse(argc, argv);

  string inputPath = ipArg.getValue();
  string outputPath = opArg.getValue();
  string hall = hallArg.getValue();
  int row = rowArg.getValue();
  int col = colArg.getValue();
  string dtVarName = varArg.getValue();
  int nSet = nSetArg.getValue();
  bool dbgFlag = dbgArg.getValue();
  bool effSwitch = effArg.getValue();
  bool divSwidth = divideArg.getValue();


  /// input path cannot end with a slash
  if(inputPath.rfind("/") == inputPath.length()-1)
    inputPath = inputPath.substr(0, inputPath.length()-1);


  /// check if there is any file under the given path
  if(!checkPath(inputPath))
  {
    cerr << "No root files found under given path" << endl;
    return -1;
  }


  vector<string> vIfn;
  getInputFilenames(vIfn, inputPath, hall);
  if(!divSwidth)
  {
    if(!effSwitch)
      rateSetBySet(vIfn, outputPath, hall, row, col, dtVarName, nSet, dbgFlag);
    else
      effSetBySet(vIfn, outputPath, hall, row, col, nSet, dbgFlag);
  }

  getCorrectedRate(vIfn, outputPath, hall, row, col);


  return 0;
}



bool checkPath(string ip)
{
  gROOT->ProcessLine(Form(".! ls %s/*.root 2>/dev/null|wc -l>nfiles",
                          ip.c_str()));
  ifstream fnfiles("nfiles");
  int nfiles;
  fnfiles >> nfiles;
  fnfiles.close();
  gROOT->ProcessLine(".! rm -rf nfiles");
  if(nfiles) return true;
  return false;
}



void getInputFilenames(vector<string>& vIfn, string ip, string hall)
{
  gROOT->ProcessLine(Form(".! ls %s/*EH%s*.root 2>/dev/null>files", ip.c_str(),
                          hall.c_str()));
  
  ifstream ffiles("files");
  string ifn;
  while(ffiles >> ifn)
  {
    //string ifpn = ip + "/" + ifn;
    vIfn.push_back(ifn);
  }
  ffiles.close();
  gROOT->ProcessLine(".! rm -rf files");
}
