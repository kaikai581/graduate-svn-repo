#define MoreInfo_cxx
#include "MoreInfo.h"
#include <TH2.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <algorithm>
#include <vector>
#include <iostream>
#include <string>

void MoreInfo::Loop()
{
//   In a ROOT session, you can do:
//      Root > .L MoreInfo.C
//      Root > MoreInfo t
//      Root > t.GetEntry(12); // Fill t data members with entry number 12
//      Root > t.Show();       // Show values of entry 12
//      Root > t.Show(16);     // Read and show values of entry 16
//      Root > t.Loop();       // Loop on all entries
//

//     This is the loop skeleton where:
//    jentry is the global entry number in the chain
//    ientry is the entry number in the current Tree
//  Note that the argument to GetEntry must be:
//    jentry for TChain::GetEntry
//    ientry for TTree::GetEntry and TBranch::GetEntry
//
//       To read only selected branches, Insert statements like:
// METHOD1:
//    fChain->SetBranchStatus("*",0);  // disable all branches
//    fChain->SetBranchStatus("branchname",1);  // activate branchname
// METHOD2: replace line
//    fChain->GetEntry(jentry);       //read all branches
//by  b_branchname->GetEntry(ientry); //read only this branch
   if (fChain == 0) return;

   Long64_t nentries = fChain->GetEntriesFast();

   Long64_t nbytes = 0, nb = 0;

	TFile *f1 = new TFile("moreinfo.root","recreate");
	TH1F *hInterTime[4];
	TString histName;
	
	int ii, jj;
	for(ii = 0; ii < 4; ii++)
	{
		histName = Form("hInterTime%d",ii);
		hInterTime[ii] = new TH1F(histName, "The inter arrival time of two events", 200, 0, .01);
	}
	
	vector<double> timeSeries;

/*********************************************************
Start extracting data from the root file
*********************************************************/
   for (Long64_t jentry=0; jentry<nentries;jentry++) {
      Long64_t ientry = LoadTree(jentry);
      if (ientry < 0) break;
      nb = fChain->GetEntry(jentry);   nbytes += nb;
		timeSeries.push_back(TrigSecond+TrigNanoSec*1e-9);
      // if (Cut(ientry) < 0) continue;
   }

/**********************************************************
Fill histograms with interarrival time:t1-t0,t2-t0,etc.
These histograms obey gamma distribution function.
c.f. Nucl. Phys. B, 370 (1992) 432
**********************************************************/
	for(ii = 0; (unsigned)ii < timeSeries.size(); ii++)
		for(jj = 1; jj < 5; jj++)
			if((unsigned)(ii+jj)<timeSeries.size())
				hInterTime[jj-1]->Fill(timeSeries[ii+jj]-timeSeries[ii]);
	for(jj = 0; jj < 4; jj++)
		hInterTime[jj]->Write();
	for(jj = 0; jj < 4; jj++)
		delete hInterTime[jj];

	delete f1;
}
