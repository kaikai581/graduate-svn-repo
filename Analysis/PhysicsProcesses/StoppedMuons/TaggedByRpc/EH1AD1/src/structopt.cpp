#include <string>
#include "structopt.hpp"

using namespace std;

std::string option_t::inputPath("");
std::string option_t::outputPath("");
std::string option_t::dbOutputPath("");
int option_t::nEvt(0);
int option_t::printEvt(0);
int option_t::scanSize(0);
bool option_t::owFlag(false);
bool option_t::dbgFlag(false);
bool option_t::dbFlag(false);
