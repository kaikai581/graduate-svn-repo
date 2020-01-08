/*************************************************************
Simulation:
Suppose the random process is poluted by a periodic signal.
October 2010 @ Daya Bay
*************************************************************/

#include "TFile.h"
#include "TRandom3.h"
#include "TH1F.h"
#include "TString.h"
#include <vector>
#include <algorithm>
#define NEVENT 54363
#define NRANDOM 41729
#define TIMEINTERVAL 54.69

using namespace std;

int main()
{
	TFile f1("convolve.root","recreate");
	vector<double> timeSeries;
	const double nperiodic = NEVENT-NRANDOM;
	int i, j;
	const double dt = TIMEINTERVAL/nperiodic;
	TRandom3 r1(0);
	TH1F *hInterTime[4];

	TString histName;
	
	for(i = 0; i < 4; i++)
	{
		histName.Form("hInterTime%d",i);
		hInterTime[i] = new TH1F(histName, "The inter arrival time of two events", 200, 0, .01);
	}
		

	for(i = 0; i<nperiodic; i++)
		timeSeries.push_back(dt*i);
	for(i = 0; i < NRANDOM; i++)
		timeSeries.push_back(r1.Uniform(TIMEINTERVAL));

	sort(timeSeries.begin(),timeSeries.end());

	for(i = 0; (unsigned)i<timeSeries.size(); i++)
		for(j = 1; j < 5; j++)
			if((unsigned)(i+j)<timeSeries.size())
				hInterTime[j-1]->Fill(timeSeries[i+j]-timeSeries[i]);
	for(i = 0; i < 4; i++)
		hInterTime[i]->Write();

	for(i = 0; i < 4; i++)
		delete hInterTime[i];

	return 0;
}
