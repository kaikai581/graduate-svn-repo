#include "NHitTrigger.h"
#include "TFile.h"
#include <iostream>
//#include <fstream>

using namespace std;
int tbin;
string filename;

int main(int argc, char *argv[])
{
	//parameter parser
	if(argc == 1)
	{
		cout << "Please input a root file and specify time window in nanosecond!" << endl;
		return -1;
	}
	if(argc == 2)
	{
		cout << "Default time window of 100ns is used!" << endl;
		tbin=64;
	}
	if(argc == 3)
		tbin=(int)(atof(argv[2])/1.5625);
	if(argc >= 4)
	{
		cout << "Too many parameters!" << endl;
		return -1;
	}

	filename=argv[1];
	TFile f1(argv[1]);
	NHitTrigger *treeLoop = new NHitTrigger(argv[1],0);
	treeLoop->Loop();

	return 0;
}
