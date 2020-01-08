/*******************************************************************************
First try of getting data from Daya Bay raw data.
October 2010 @ Daya Bay by Shih-Kai Lin
*******************************************************************************/

#ifndef GETDATAFROMRAW_H
#define GETDATAFROMRAW_H

#include "GaudiAlg/GaudiAlgorithm.h"
#include "Event/ReadoutHeader.h"

using namespace std;
using namespace DayaBay;

class GetDataFromRaw : public GaudiAlgorithm
{
public:
	GetDataFromRaw(const std::string& name, ISvcLocator* pSvcLocator);
	virtual ~GetDataFromRaw();
	
	virtual StatusCode initialize();
	virtual StatusCode execute();
	virtual StatusCode finalize();
	
private:
	StatusCode processReadoutHeader(const ReadoutHeader* pReadoutHdr);
};

#endif