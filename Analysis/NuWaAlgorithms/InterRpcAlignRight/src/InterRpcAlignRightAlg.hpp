/*
 * This algorithm is to section time axis into slices with RPC triggers.
 * Look at the AD events in between.
 * 
 * 2012 Fall by Shih-Kai.
*/


#ifndef INTERRPCALIGNRIGHT_HPP
#define INTERRPCALIGNRIGHT_HPP


#include "GaudiAlg/GaudiAlgorithm.h"
#include "TreeVariables.hpp"
#include "TFile.h"
#include "TTree.h"


class InterRpcAlignRightAlg : public GaudiAlgorithm
{
public:
  /// Constructor has to be in this form
  InterRpcAlignRightAlg(const std::string&, ISvcLocator*);
  virtual ~InterRpcAlignRightAlg(){};

  /// Three mandatory member functions of any algorithm
  StatusCode initialize();
  StatusCode execute();
  StatusCode finalize();

private:
  /// ROOT variables
  TFile*                     m_rootfile;
  TTree*                     m_tree;
  TTree*                     m_trCoin;
  std::string                m_rootfilename;
  
  std::vector<TreeVariables> m_tv;
  std::vector<TreeVariables> m_coinEvts;
  TreeVariables              m_tvContiner;
  CoincidenceTree            m_tvCoin;
  bool                       flasherTest();
};

#endif
