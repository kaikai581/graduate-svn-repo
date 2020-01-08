#ifndef STRUCTOPT_HPP
#define STRUCTOPT_HPP

#include <string>


class option_t{
public:
  static std::string inputPath;
  static std::string outputPath;
  static std::string dbOutputPath;
  static int nEvt;
  static int printEvt;
  static int scanSize;
  static bool owFlag;
  static bool dbgFlag;
  static bool dbFlag;
};



#endif
