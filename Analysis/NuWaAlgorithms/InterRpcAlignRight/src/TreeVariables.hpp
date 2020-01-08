#ifndef TREEVARIABLES_HPP
#define TREEVARIABLES_HPP

#include "Context/TimeStamp.h"

class TreeVariables
{
public:
  TreeVariables(){};
  ~TreeVariables(){};

  int          site;
  int          triggerNumber;
  TimeStamp    triggerTime;
  long         dt;
  double       e;
  unsigned int detector;
  bool         isFlasher;
  bool         hitIWS;
  bool         hitOWS;
  unsigned int triggerType;
};


class CoincidenceTree
{
public:
  int          site;
  int          detector;
  int          pTimeSec;
  int          pTimeNanoSec;
  int          dTimeSec;
  int          dTimeNanoSec;
  double       pE;
  double       dE;
  double       dt;
  int          pTriggerNumber;
  int          dTriggerNumber;
};


#endif
