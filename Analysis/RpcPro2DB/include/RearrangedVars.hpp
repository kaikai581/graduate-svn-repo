#ifndef REARRANGEDVARS_HPP
#define REARRANGEDVARS_HPP

#include "RawVars.hpp"
#include "rpcnamespace.hpp"

class RearrangedVars : public RawVars
{
public:
  static unsigned int eventId;
  static unsigned int runNumber;
  static unsigned int fileNumber;
};


#endif
