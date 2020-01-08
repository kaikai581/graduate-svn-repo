// this script is based on Dan's script dybTreeGetLeafUnfriendly.C
// which one can find at
// http://dayabay.ihep.ac.cn/tracs/dybsvn/browser/dybgaudi/trunk/Tutorial/
// Quickstart/share/dybTreeGetLeafUnfriendly.C
//
//   Usage:
//   root [0] .L MyIBDCandidates.C+
//   root [1] MyIBDCandidates("recon*.root")

#include <iostream>
#include "TChain.h"
#include "TLeaf.h"
#include "TH2F.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TVirtualPad.h"
#include "TBranchElement.h"
#include "TFile.h"
#include <vector>

std::vector<float>& getLeafVectorF(const char* leafName, TTree* tree){
  void* objPtr = (dynamic_cast<TBranchElement*>(tree->GetBranch(leafName)))->GetObject();
  return *((std::vector<float>*)(objPtr));  
}

std::vector<int>& getLeafVectorI(const char* leafName, TTree* tree){
  void* objPtr = (dynamic_cast<TBranchElement*>(tree->GetBranch(leafName)))->GetObject();
  return *((std::vector<int>*)(objPtr));  
}

void InitSimpleIBDTree(TTree*);

void MyIBDCandidates(const char* filename)
{
	// save TEntryList
	TFile f("~/output.root","recreate");
	
  // Set root style
  gStyle->SetPalette(1);

  // Load coincidence tree
  TChain adCoincT("/Event/Data/Physics/CoincidenceLoose");
  int status = adCoincT.Add(filename);
  if(status==0){
    std::cout << "Bad filename: " << filename << std::endl;
    return;
  }

  // Load 'unfriendly' calibrated statistics tree
  TChain calibStatsT("/Event/Data/CalibStats");
  status = calibStatsT.Add(filename);
  if(status==0){
    std::cout << "Bad filename: " << filename << std::endl;
    return;
  }
  
  // Disable pre-existing index in the calib stats trees
  //  (Another reason ROOT is frustrating; we must manually do this)
  calibStatsT.GetEntries();
  Long64_t* firstEntry = calibStatsT.GetTreeOffset();
  for(int treeIdx=0; treeIdx<calibStatsT.GetNtrees(); treeIdx++){
    calibStatsT.LoadTree(firstEntry[treeIdx]);
    calibStatsT.GetTree()->SetTreeIndex(0);
  }

  // Build a new look-up index for the 'unfriendly' tree
  //  (Trigger number and detector id uniquely identify an entry)
  calibStatsT.BuildIndex("triggerNumber","context.mDetId");

  // Prepare histograms
  TH2F* chargeVsEnergyAD1H = new TH2F("chargeVsEnergyAD1H",
				      "Photoelectrons per MeV vs. Energy, AD#1",
				      150,0,15,
				      150,0,300);
  chargeVsEnergyAD1H->GetXaxis()->SetTitle("energy [MeV]");
  chargeVsEnergyAD1H->GetYaxis()->SetTitle("nominal charge / energy [p.e. MeV^{-1}]");
  TH2F* chargeVsEnergyAD2H = new TH2F("chargeVsEnergyAD2H",
				      "Photoelectrons per MeV vs. Energy, AD#2",
				      150,0,15,
				      150,0,300);
  chargeVsEnergyAD2H->GetXaxis()->SetTitle("energy [MeV]");
  chargeVsEnergyAD2H->GetYaxis()->SetTitle("nominal charge / energy [p.e. MeV^{-1}]");

  // Process each coincidence set
  int maxEntries=adCoincT.GetEntries();
  for(int entry=0;entry<maxEntries;entry++){

    // Get next coincidence set
    adCoincT.GetEntry(entry);

    // Get multiplet data
    int multiplicity = (int) adCoincT.GetLeaf("multiplicity")->GetValue();
    int detector = (int) adCoincT.GetLeaf("context.mDetId")->GetValue();
    std::vector<int>& triggerNumberV = getLeafVectorI("triggerNumber",&adCoincT);
    std::vector<int>& energyStatusV = getLeafVectorI("energyStatus",&adCoincT);
    std::vector<float>& energyV = getLeafVectorF("e",&adCoincT);

    // Loop over AD events in multiplet
    for(int multIdx=0; multIdx<multiplicity; multIdx++){

      // Get data for each AD trigger in the multiplet
      int triggerNumber = triggerNumberV[multIdx];
      int energyStatus = energyStatusV[multIdx];
      float energy = energyV[multIdx];

      // Look up corresponding entry in calib stats
      status = calibStatsT.GetEntryWithIndex(triggerNumber, detector);
      if(status<=0){
	std::cout << "Failed to find calib stats for trigger number " 
		  << triggerNumber << " and detector ID " << detector 
		  << std::endl;
	continue;
      }
      // Get data from matching calib stats entry
      double nominalCharge = calibStatsT.GetLeaf("NominalCharge")->GetValue();

      // Fill histograms
      if(energyStatus==1 && energy>0){ // Reconstruction was successful
	if(detector==1){ 
	  // AD#1
	  chargeVsEnergyAD1H->Fill(energy,nominalCharge/energy);
	}else if(detector==2){
	  // AD#2
	  chargeVsEnergyAD2H->Fill(energy,nominalCharge/energy);
	}
      }

    }  // End loop over AD triggers in the multiplet
  } // End loop over AD coincidence multiplets

  // Draw histograms
  TCanvas* c1 = new TCanvas;
  c1->Divide(1,2);

  TVirtualPad* pad1 = c1->cd(1);
  pad1->SetLogz();
  chargeVsEnergyAD1H->Draw("colz");

  TVirtualPad* pad2 = c1->cd(2);
  pad2->SetLogz();
  chargeVsEnergyAD2H->Draw("colz");

  return;
}


void InitSimpleIBDTree(TTree *t)
{
	float promptE;
	float delayedE;
}
