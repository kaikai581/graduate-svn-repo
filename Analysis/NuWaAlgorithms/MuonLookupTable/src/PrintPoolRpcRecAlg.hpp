#ifndef PRINTPOOLRPCRECALG_HPP
#define PRINTPOOLRPCRECALG_HPP


#include "GaudiAlg/GaudiAlgorithm.h"

class PrintPoolRpcRecAlg : public GaudiAlgorithm
{
public:
  /// Constructor has to be in this form
  PrintPoolRpcRecAlg(const std::string&, ISvcLocator*);
  virtual ~PrintPoolRpcRecAlg(){};
  
  /// Three mandatory member functions of any algorithm
  StatusCode initialize();
  StatusCode execute();
  StatusCode finalize();
};


#endif
