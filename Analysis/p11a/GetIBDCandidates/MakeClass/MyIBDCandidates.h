#ifndef MYIBDCANDIDATES_H
#define MYIBDCANDIDATES_H

#include "CoincidenceTree.h"

class MyIBDCandidates : public CoincidenceTree
{
public:
	MyIBDCandidates(TTree *tree=0);
	~MyIBDCandidates();
};

#endif
