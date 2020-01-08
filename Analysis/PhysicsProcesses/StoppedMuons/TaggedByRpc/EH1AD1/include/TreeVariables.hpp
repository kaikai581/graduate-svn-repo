#ifndef TREEVARIABLES_HPP
#define TREEVARIABLES_HPP

#include <string>


class TreeVariables
{
public:
  static float         energy;
  static float         nPESum_AD1;
  static bool          prompt;
  static bool          delayed;
  static bool          flasher;
  static int           nHit_IWS;
  static int           nHit_OWS;
  /// number of delayed events in the given time window
  static unsigned int  multiplicity;
  static unsigned long dt;
};



#endif
