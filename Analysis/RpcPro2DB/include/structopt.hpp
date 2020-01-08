#ifndef STRUCTOPT_HPP
#define STRUCTOPT_HPP

#include <string>


class option_t{
public:
  static std::string inputPath;
  static std::string outputPath;
  static int nEvt;
  static int printEvt;
  static bool owFlag;
  static bool dbgFlag;
};



#endif
