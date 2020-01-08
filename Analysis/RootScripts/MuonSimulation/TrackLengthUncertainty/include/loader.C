/* loader.C */
#include "TVector3.h"
#include <map>

/// My Line
struct MyLine
{
  TVector3 p;
  TVector3 v;
};

#ifdef __MAKECINT__
#pragma link C++ class std::map<int, MyLine>;
#pragma link C++ class std::map<int, MyLine>::iterator;
#pragma link C++ class std::map<int, TVector3>;
#endif
