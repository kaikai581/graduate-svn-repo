#include <iostream>
#include "MyIBDCandidates.h"

using namespace std;

MyIBDCandidates :: MyIBDCandidates(TTree *tree) : CoincidenceTree(tree)
{
	cout << "MyIBDCandidates instance created" << endl;
}

MyIBDCandidates :: ~MyIBDCandidates()
{
	cout << "MyIBDCandidates instance destroyed" << endl;
}
