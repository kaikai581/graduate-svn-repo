#ifndef PRINTMUONRECSIMPLEALG_HPP
#define PRINTMUONRECSIMPLEALG_HPP


#include "GaudiAlg/GaudiAlgorithm.h"

class PrintMuonRecSimpleAlg : public GaudiAlgorithm
{
public:
  /// Constructor has to be in this form
  PrintMuonRecSimpleAlg(const std::string&, ISvcLocator*);
  virtual ~PrintMuonRecSimpleAlg(){};
  
  /// Three mandatory member functions of any algorithm
  StatusCode initialize();
  StatusCode execute();
  StatusCode finalize();
};


#endif
