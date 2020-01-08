#ifndef STRUCTOPT_HPP
#define STRUCTOPT_HPP

#include <string>


struct option_t{
  std::string inputPath;
  std::string outputPath;
  int hall;
  int row;
  int col;
  int nEvt;
  std::string strCut;
  bool dbgFlag;
};



#endif
