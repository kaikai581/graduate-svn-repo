#ifndef RPCRAWTREE_H
#define RPCRAWTREE_H

#include <string>
#include "MyTree.h"
#include "GaudiAlg/GaudiAlgorithm.h"
#include "Event/ReadoutHeader.h"
#include "Event/CalibReadoutHeader.h"
#include "TFile.h"

using namespace std;
using namespace DayaBay;

class RpcRawTreeAlg : public GaudiAlgorithm
{
public:
	RpcRawTreeAlg(const string& name, ISvcLocator* pSvcLocator);
	virtual ~RpcRawTreeAlg();
	
	virtual StatusCode initialize();
	virtual StatusCode execute();
	virtual StatusCode finalize();
	
private:
	int m_nRo;
	int m_xStgModCorn, m_yStgModCorn;
	double m_preTime2f, m_curTime2f;
	double m_preTime3f, m_curTime3f;
	double m_preTime4f, m_curTime4f;
	double m_preTime234f, m_curTime234f;
	double m_preTime34f, m_curTime34f;
	bool m_firstRPCEvt2f; // used for calculating dt
	bool m_firstRPCEvt3f;
	bool m_firstRPCEvt4f;
	bool m_firstRPCEvt234f;
	bool m_firstRPCEvt34f;
	int m_maxRoLayersPerModule;
	MyTree* m_treeStruct;
	TFile* m_f;
	TTree* m_t;
  
  // configurable members
  string m_outputFileName;

	StatusCode writeReadoutHeader(const ReadoutHeader*);
  StatusCode writeCalibReadoutHeader(const CalibReadoutHeader*
		pCalibReadoutHdr);
//	StatusCode qmReconAlg(const CalibReadoutHeader* pCalibReadoutHdr);
//	void getRpcRecPerf();
	void initializeTree();
//	void getStgModCornCoor(int row, int col); // get stagger module corner coordinate
};

#endif
