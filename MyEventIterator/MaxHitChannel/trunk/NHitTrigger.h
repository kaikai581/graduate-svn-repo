//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Thu Oct 14 01:06:26 2010 by ROOT version 5.26/00b
// from TTree t/Analysis Tree
//////////////////////////////////////////////////////////

#ifndef NHitTrigger_h
#define NHitTrigger_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include "TString.h"
#include <iostream>
#include <cstdlib>
#include <string>

using namespace std;

class NHitTrigger {
public :
   TTree          *fChain;   //!pointer to the analyzed TTree or TChain
   Int_t           fCurrent; //!current Tree number in a TChain
	//char filename[100];

   // Declaration of leaf types
   Int_t           Run;
   Int_t           Event;
   Int_t           Det;
   Int_t           TrigNum;
   Int_t           TrigSecond;
   Int_t           TrigNanoSec;
   Int_t           TrigType;
   Double_t        SecToPre;
   Int_t           Tag;
   Double_t        FirstHitSum;
   Double_t        Uniformity;
   Int_t           NHit;
   Int_t           NChannel;
   Int_t           NTrigger;
   Double_t        Rate;
   Double_t        CCX;
   Double_t        CCY;
   Double_t        CCZ;
   Double_t        CCT;
   Int_t           CCStatus;
   Double_t        QSum;
   Int_t           QSumStatus;
   Double_t        OpModelX;
   Double_t        OpModelY;
   Double_t        OpModelZ;
   Double_t        OpModelE;
   Int_t           Board[350];   //[NHit]
   Int_t           Channel[350];   //[NHit]
   Int_t           Ring[350];   //[NHit]
   Int_t           Column[350];   //[NHit]
   Double_t        Charge[350];   //[NHit]
   Double_t        Time[350];   //[NHit]
   Int_t           TriggerSec[1];   //[NTrigger]
   Int_t           TriggerNano[1];   //[NTrigger]
   Int_t           TriggerType[1];   //[NTrigger]
   Int_t           Adc[350];   //[NHit]
   Int_t           Tdc[350];   //[NHit]
   Int_t           Cycle[350];   //[NHit]
   Int_t           Range[350];   //[NHit]
   Int_t           PreAdc[350];   //[NHit]
   Int_t           HitCount[350];   //[NHit]
   Int_t           MultiTdc[350];   //[NHit]
   Int_t           TdcByMedian[350];   //[NHit]
   Double_t        AvePed[350];   //[NHit]
   Double_t        QuasiAdc[350];   //[NHit]
   Double_t        TotalQuasiAdc;
   Double_t        MaxQuasiAdc;
   Int_t           MaxQuasiAdcId;
   Double_t        TotalChrg;
   Double_t        MaxChrg;
   Int_t           MaxChrgId;

   // List of branches
   TBranch        *b_Run;   //!
   TBranch        *b_Event;   //!
   TBranch        *b_Det;   //!
   TBranch        *b_TrigNum;   //!
   TBranch        *b_TrigSecond;   //!
   TBranch        *b_TrigNanoSec;   //!
   TBranch        *b_TrigType;   //!
   TBranch        *b_SecToPre;   //!
   TBranch        *b_Tag;   //!
   TBranch        *b_FirstHitSum;   //!
   TBranch        *b_Uniformity;   //!
   TBranch        *b_NHit;   //!
   TBranch        *b_NChannel;   //!
   TBranch        *b_NTrigger;   //!
   TBranch        *b_Rate;   //!
   TBranch        *b_CCX;   //!
   TBranch        *b_CCY;   //!
   TBranch        *b_CCZ;   //!
   TBranch        *b_CCT;   //!
   TBranch        *b_CCStatus;   //!
   TBranch        *b_QSum;   //!
   TBranch        *b_QSumStatus;   //!
   TBranch        *b_OpModelX;   //!
   TBranch        *b_OpModelY;   //!
   TBranch        *b_OpModelZ;   //!
   TBranch        *b_OpModelE;   //!
   TBranch        *b_Board;   //!
   TBranch        *b_Channel;   //!
   TBranch        *b_Ring;   //!
   TBranch        *b_Column;   //!
   TBranch        *b_Charge;   //!
   TBranch        *b_Time;   //!
   TBranch        *b_TriggerSec;   //!
   TBranch        *b_TriggerNano;   //!
   TBranch        *b_TriggerType;   //!
   TBranch        *b_Adc;   //!
   TBranch        *b_Tdc;   //!
   TBranch        *b_Cycle;   //!
   TBranch        *b_Range;   //!
   TBranch        *b_PreAdc;   //!
   TBranch        *b_HitCount;   //!
   TBranch        *b_MultiTdc;   //!
   TBranch        *b_TdcByMedian;   //!
   TBranch        *b_AvePed;   //!
   TBranch        *b_QuasiAdc;   //!
   TBranch        *b_TotalQuasiAdc;   //!
   TBranch        *b_MaxQuasiAdc;   //!
   TBranch        *b_MaxQuasiAdcId;   //!
   TBranch        *b_TotalChrg;   //!
   TBranch        *b_MaxChrg;   //!
   TBranch        *b_MaxChrgId;   //!

   NHitTrigger(char *fn, TTree *tree=0);
   virtual ~NHitTrigger();
   virtual Int_t    Cut(Long64_t entry);
   virtual Int_t    GetEntry(Long64_t entry);
   virtual Long64_t LoadTree(Long64_t entry);
   virtual void     Init(TTree *tree);
   virtual void     Loop();
   virtual Bool_t   Notify();
   virtual void     Show(Long64_t entry = -1);
};

#endif

#ifdef NHitTrigger_cxx
NHitTrigger::NHitTrigger(char *fn, TTree *tree)
{
// if parameter tree is not specified (or zero), connect the file
// used to generate this class and read the Tree.
   if (tree == 0) {
      TFile *f = (TFile*)gROOT->GetListOfFiles()->FindObject(fn);
      if (!f) {
			cout << "LFile not found!" << endl;
			exit(1);
      }
      tree = (TTree*)gDirectory->Get("t");
   }
   Init(tree);
}

NHitTrigger::~NHitTrigger()
{
   if (!fChain) return;
   delete fChain->GetCurrentFile();
}

Int_t NHitTrigger::GetEntry(Long64_t entry)
{
// Read contents of entry.
   if (!fChain) return 0;
   return fChain->GetEntry(entry);
}
Long64_t NHitTrigger::LoadTree(Long64_t entry)
{
// Set the environment to read one entry
   if (!fChain) return -5;
   Long64_t centry = fChain->LoadTree(entry);
   if (centry < 0) return centry;
   if (!fChain->InheritsFrom(TChain::Class()))  return centry;
   TChain *chain = (TChain*)fChain;
   if (chain->GetTreeNumber() != fCurrent) {
      fCurrent = chain->GetTreeNumber();
      Notify();
   }
   return centry;
}

void NHitTrigger::Init(TTree *tree)
{
   // The Init() function is called when the selector needs to initialize
   // a new tree or chain. Typically here the branch addresses and branch
   // pointers of the tree will be set.
   // It is normally not necessary to make changes to the generated
   // code, but the routine can be extended by the user if needed.
   // Init() will be called many times when running on PROOF
   // (once per file to be processed).

   // Set branch addresses and branch pointers
   if (!tree) return;
   fChain = tree;
   fCurrent = -1;
   fChain->SetMakeClass(1);

   fChain->SetBranchAddress("Run", &Run, &b_Run);
   fChain->SetBranchAddress("Event", &Event, &b_Event);
   fChain->SetBranchAddress("Det", &Det, &b_Det);
   fChain->SetBranchAddress("TrigNum", &TrigNum, &b_TrigNum);
   fChain->SetBranchAddress("TrigSecond", &TrigSecond, &b_TrigSecond);
   fChain->SetBranchAddress("TrigNanoSec", &TrigNanoSec, &b_TrigNanoSec);
   fChain->SetBranchAddress("TrigType", &TrigType, &b_TrigType);
   fChain->SetBranchAddress("SecToPre", &SecToPre, &b_SecToPre);
   fChain->SetBranchAddress("Tag", &Tag, &b_Tag);
   fChain->SetBranchAddress("FirstHitSum", &FirstHitSum, &b_FirstHitSum);
   fChain->SetBranchAddress("Uniformity", &Uniformity, &b_Uniformity);
   fChain->SetBranchAddress("NHit", &NHit, &b_NHit);
   fChain->SetBranchAddress("NChannel", &NChannel, &b_NChannel);
   fChain->SetBranchAddress("NTrigger", &NTrigger, &b_NTrigger);
   fChain->SetBranchAddress("Rate", &Rate, &b_Rate);
   fChain->SetBranchAddress("CCX", &CCX, &b_CCX);
   fChain->SetBranchAddress("CCY", &CCY, &b_CCY);
   fChain->SetBranchAddress("CCZ", &CCZ, &b_CCZ);
   fChain->SetBranchAddress("CCT", &CCT, &b_CCT);
   fChain->SetBranchAddress("CCStatus", &CCStatus, &b_CCStatus);
   fChain->SetBranchAddress("QSum", &QSum, &b_QSum);
   fChain->SetBranchAddress("QSumStatus", &QSumStatus, &b_QSumStatus);
   fChain->SetBranchAddress("OpModelX", &OpModelX, &b_OpModelX);
   fChain->SetBranchAddress("OpModelY", &OpModelY, &b_OpModelY);
   fChain->SetBranchAddress("OpModelZ", &OpModelZ, &b_OpModelZ);
   fChain->SetBranchAddress("OpModelE", &OpModelE, &b_OpModelE);
   fChain->SetBranchAddress("Board", Board, &b_Board);
   fChain->SetBranchAddress("Channel", Channel, &b_Channel);
   fChain->SetBranchAddress("Ring", Ring, &b_Ring);
   fChain->SetBranchAddress("Column", Column, &b_Column);
   fChain->SetBranchAddress("Charge", Charge, &b_Charge);
   fChain->SetBranchAddress("Time", Time, &b_Time);
   fChain->SetBranchAddress("TriggerSec", &TriggerSec, &b_TriggerSec);
   fChain->SetBranchAddress("TriggerNano", &TriggerNano, &b_TriggerNano);
   fChain->SetBranchAddress("TriggerType", &TriggerType, &b_TriggerType);
   fChain->SetBranchAddress("Adc", Adc, &b_Adc);
   fChain->SetBranchAddress("Tdc", Tdc, &b_Tdc);
   fChain->SetBranchAddress("Cycle", Cycle, &b_Cycle);
   fChain->SetBranchAddress("Range", Range, &b_Range);
   fChain->SetBranchAddress("PreAdc", PreAdc, &b_PreAdc);
   fChain->SetBranchAddress("HitCount", HitCount, &b_HitCount);
   fChain->SetBranchAddress("MultiTdc", MultiTdc, &b_MultiTdc);
   fChain->SetBranchAddress("TdcByMedian", TdcByMedian, &b_TdcByMedian);
   fChain->SetBranchAddress("AvePed", AvePed, &b_AvePed);
   fChain->SetBranchAddress("QuasiAdc", QuasiAdc, &b_QuasiAdc);
   fChain->SetBranchAddress("TotalQuasiAdc", &TotalQuasiAdc, &b_TotalQuasiAdc);
   fChain->SetBranchAddress("MaxQuasiAdc", &MaxQuasiAdc, &b_MaxQuasiAdc);
   fChain->SetBranchAddress("MaxQuasiAdcId", &MaxQuasiAdcId, &b_MaxQuasiAdcId);
   fChain->SetBranchAddress("TotalChrg", &TotalChrg, &b_TotalChrg);
   fChain->SetBranchAddress("MaxChrg", &MaxChrg, &b_MaxChrg);
   fChain->SetBranchAddress("MaxChrgId", &MaxChrgId, &b_MaxChrgId);
   Notify();
}

Bool_t NHitTrigger::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   return kTRUE;
}

void NHitTrigger::Show(Long64_t entry)
{
// Print contents of entry.
// If entry is not specified, print current entry
   if (!fChain) return;
   fChain->Show(entry);
}
Int_t NHitTrigger::Cut(Long64_t entry)
{
// This function may be called from Loop.
// returns  1 if entry is accepted.
// returns -1 otherwise.
   return 1;
}
#endif // #ifdef NHitTrigger_cxx
