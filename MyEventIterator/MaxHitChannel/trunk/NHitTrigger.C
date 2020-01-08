#define NHitTrigger_cxx
#include "NHitTrigger.h"
#include <TH2.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <algorithm>
#include <vector>
#include <iostream>
#include <string>
#include <cmath>
//#include <fstream>

#define TIMEPERBIN 1.5625

#define NPMT 192
extern string filename;
extern int tbin;

void NHitTrigger::Loop()
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


//********start user defined variables************************
	//filename manapulation
	char *timelen;

	int windowlen = tbin*TIMEPERBIN;
	timelen = new char[(int)log10(10.)+3];
	sprintf(timelen,"%dns",windowlen);
	string ofname("maxhit");

	string inputpathname(filename);
	size_t found=inputpathname.find_last_of("/");
	if(found==inputpathname.npos)
		found = 0;
	else
		found++;
	inputpathname=inputpathname.substr(found,inputpathname.size()-1);
	inputpathname=inputpathname.substr(0,inputpathname.find_last_of("."));
	ofname.append(timelen);
	delete [] timelen;
	ofname.append(inputpathname).append(".root");
	cout << ofname << " is being generated." << endl;
	//end of filename manipulation
	TFile outf(ofname.c_str(),"recreate");
	TString hdesc;
	//TString hname;
	//hname.Form("h%s",inputpathname.c_str());
	hdesc.Form("maximum number of channels hit in %d ns", windowlen);
	//TH1F h1(hname,hdesc,192,0,192);
	TH1F h1("h",hdesc,192,0,192);
	int i, j, curTime;
	int ltime, rtime;
	int hitChannel[192], totHitCh, maxHitCh, maxHitStartTime;
	int chi, chj;
//********end user defined variables**************************
	
/*********************************************************
Start extracting data from the root file
*********************************************************/
   for (Long64_t jentry=0; jentry<nentries;jentry++) {

		//test mode
		//if(jentry>100)
			//break;
		//end of test mode

      Long64_t ientry = LoadTree(jentry);
      if (ientry < 0) break;
      nb = fChain->GetEntry(jentry);   nbytes += nb;

	
	//variable time window

		maxHitCh = 0;
		maxHitStartTime = -1;
		//open time window and scan; if # hit channel is exactly nPMT threshold, success.
		for(i = 0; i < NHit; i++)
		{
			totHitCh = 0;

			//clear designation
			for(j = 0; j < 192; j++)
				hitChannel[j] = 0;

			rtime = Tdc[i];
			//if(Ring[i]<=0||Ring[i]>=9||Column[i]<=0||Column[i]>=25)
				//continue;
			chi=24*(Ring[i]-1)+Column[i]-1;
			if(chi<0||chi>=192)
			{
				//cout << chi << endl;
				continue;
			}
			hitChannel[chi]=1;
			ltime = rtime-tbin;

			for(j = 0; j < 192; j++)
			{
				curTime=Tdc[j];
				//cout << curTime << endl;
				if(curTime>=ltime&&curTime<=rtime)
				{
					chj=24*(Ring[j]-1)+Column[j]-1;
					if(chj<0||chj>=192)
						continue;
					hitChannel[chj]=1;
				}
			}
			for(j = 0; j < 192; j++)
				totHitCh+=hitChannel[j];

			if(totHitCh>maxHitCh)
			{
				maxHitCh = totHitCh;
				maxHitStartTime = rtime;
			}
			//cout << totHitCh << endl;
		}

		h1.Fill(maxHitCh);
      // if (Cut(ientry) < 0) continue;
   }
	h1.Write();
}
