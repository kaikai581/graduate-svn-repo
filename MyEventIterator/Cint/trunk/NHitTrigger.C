#define NHitTrigger_cxx
#include "NHitTrigger.h"
#include <TH2.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <algorithm>
#include <vector>
#include <iostream>
#include <string>

#define NPMT 192

void NHitTrigger::Loop(int windowBin, int shiftbin)
{
//   In a ROOT session, you can do:
//      Root > .L NHitTrigger.C
//      Root > NHitTrigger t
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


	int tWin;
	TString outfn;
	TString hName;
	TString hDesc;
	outfn.Form("nhitscan_%d_%s",shiftbin,filename);

	TFile *f1 = new TFile(outfn,"recreate");

	TH1F **hNHit = new TH1F*[windowBin];
	
	int pmtHit[NPMT], i;
	int nHitPMT;
	
	for(tWin=0; tWin<windowBin; tWin++)
	{
		hName.Form("hHit%d",(tWin+1)*2);
		//hDesc.Form("number of PMTs fired during %f nano second centered at TDC=%d",1.5625*(tWin+1)*2,1000+shiftbin+100);
		hDesc="";
		hNHit[tWin] = new TH1F(hName,hDesc,192,0,192);
		hNHit[tWin]->GetXaxis()->SetTitle("number of hit channels");
		//cout << shiftbin << endl;
	}

/*********************************************************
Start extracting data from the root file
*********************************************************/
   for (Long64_t jentry=0; jentry<nentries;jentry++) {
      Long64_t ientry = LoadTree(jentry);
      if (ientry < 0) break;
      nb = fChain->GetEntry(jentry);   nbytes += nb;

	
	//variable time window
		for(tWin=0;tWin<windowBin;tWin++)
		{
			for(i = 0; i < NPMT; i++)
				pmtHit[i] = 0;
			nHitPMT = 0;
			for(i = 0; i < NHit; i++)
				if(Tdc[i]<=1000+tWin+1+shiftbin&&Tdc[i]>=1000-tWin+shiftbin)
					pmtHit[(Ring[i]-1)*24+Column[i]-1]++;
			for(i = 0; i < NPMT; i++)
				if(pmtHit[i])
					nHitPMT++;
			hNHit[tWin]->Fill(nHitPMT);
		}
      // if (Cut(ientry) < 0) continue;
   }

	for(tWin=0;tWin<windowBin;tWin++)
	{
		hNHit[tWin]->Write();
		delete hNHit[tWin];
	}

	delete [] hNHit;

	delete f1;
}
