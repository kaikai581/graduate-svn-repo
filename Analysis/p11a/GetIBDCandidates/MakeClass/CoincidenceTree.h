//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Sat Dec  3 05:27:36 2011 by ROOT version 5.30/05
// from TTree CoincidenceLoose/Tree at /Event/Data/Physics/CoincidenceLoose holding Data_Physics_CoincidenceLoose
// found on file: recon.NoTag.0017274.Physics.EH1-Merged.P11A-P._0016.root
//////////////////////////////////////////////////////////

#ifndef CoincidenceTree_h
#define CoincidenceTree_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
   const Int_t kMaxinputHeaders = 15;

class CoincidenceTree {
public :
   TTree          *fChain;   //!pointer to the analyzed TTree or TChain
   Int_t           fCurrent; //!current Tree number in a TChain

   // Declaration of leaf types
 //PerHeaderObject *Data_Physics_CoincidenceLoose;
   Int_t           clID;
   Int_t           earliest_mSec;
   Int_t           earliest_mNanoSec;
   Int_t           latest_mSec;
   Int_t           latest_mNanoSec;
   Int_t           context_mSite;
   Int_t           context_mSimFlag;
   Int_t           context_mTimeStamp_mSec;
   Int_t           context_mTimeStamp_mNanoSec;
   Int_t           context_mDetId;
   Int_t           execNumber;
   UInt_t          jobId_m_data[4];
   vector<unsigned long> randomState;
   Int_t           inputHeaders_;
   Int_t           inputHeaders_m_entry[kMaxinputHeaders];   //[inputHeaders_]
   string          inputHeaders_m_path[kMaxinputHeaders];
   vector<int>     *I;
   vector<int>     *J;
   vector<float>   *calib_ELast_ADShower_ms;
   vector<float>   *calib_EarlyCharge;
   vector<float>   *calib_Kurtosis;
   vector<float>   *calib_LateCharge;
   vector<float>   *calib_MaxQ;
   vector<float>   *calib_MiddleTimeRMS;
   vector<float>   *calib_NominalCharge;
   vector<float>   *calib_PeakRMS;
   vector<float>   *calib_Quadrant;
   vector<float>   *calib_dtLastAD1_ms;
   vector<float>   *calib_dtLastAD2_ms;
   vector<float>   *calib_dtLastAD3_ms;
   vector<float>   *calib_dtLastAD4_ms;
   vector<float>   *calib_dtLastIWS_ms;
   vector<float>   *calib_dtLastOWS_ms;
   vector<float>   *calib_dtLastRPC_ms;
   vector<float>   *calib_dtLast_ADMuon_ms;
   vector<float>   *calib_dtLast_ADShower_ms;
   vector<float>   *calib_dtNextAD1_ms;
   vector<float>   *calib_dtNextAD2_ms;
   vector<float>   *calib_dtNextAD3_ms;
   vector<float>   *calib_dtNextAD4_ms;
   vector<float>   *calib_dtNextIWS_ms;
   vector<float>   *calib_dtNextOWS_ms;
   vector<float>   *calib_dtNextRPC_ms;
   vector<int>     *calib_nHit;
   vector<float>   *calib_nPERMS;
   vector<float>   *calib_nPESum;
   vector<float>   *calib_nPulseRMS;
   vector<float>   *calib_nPulseSum;
   vector<float>   *calib_tEarliest;
   vector<float>   *calib_tLatest;
   vector<float>   *calib_tMean;
   vector<int>     *dt_ns;
   vector<float>   *e;
   vector<int>     *energyStatus;
   Int_t           fileNo;
   Int_t           multiplicity;
   vector<int>     *positionStatus;
   Int_t           runNo;
   vector<int>     *t_ns;
   vector<int>     *t_s;
   vector<int>     *triggerNumber;
   vector<int>     *triggerType;
   vector<float>   *x;
   vector<float>   *y;
   vector<float>   *z;

   // List of branches
   TBranch        *b_Data_Physics_CoincidenceLoose_clID;   //!
   TBranch        *b_Data_Physics_CoincidenceLoose_earliest_mSec;   //!
   TBranch        *b_Data_Physics_CoincidenceLoose_earliest_mNanoSec;   //!
   TBranch        *b_Data_Physics_CoincidenceLoose_latest_mSec;   //!
   TBranch        *b_Data_Physics_CoincidenceLoose_latest_mNanoSec;   //!
   TBranch        *b_Data_Physics_CoincidenceLoose_context_mSite;   //!
   TBranch        *b_Data_Physics_CoincidenceLoose_context_mSimFlag;   //!
   TBranch        *b_Data_Physics_CoincidenceLoose_context_mTimeStamp_mSec;   //!
   TBranch        *b_Data_Physics_CoincidenceLoose_context_mTimeStamp_mNanoSec;   //!
   TBranch        *b_Data_Physics_CoincidenceLoose_context_mDetId;   //!
   TBranch        *b_Data_Physics_CoincidenceLoose_execNumber;   //!
   TBranch        *b_Data_Physics_CoincidenceLoose_jobId_m_data;   //!
   TBranch        *b_Data_Physics_CoincidenceLoose_randomState;   //!
   TBranch        *b_Data_Physics_CoincidenceLoose_inputHeaders_;   //!
   TBranch        *b_inputHeaders_m_entry;   //!
   TBranch        *b_inputHeaders_m_path;   //!
   TBranch        *b_I;   //!
   TBranch        *b_J;   //!
   TBranch        *b_calib_ELast_ADShower_ms;   //!
   TBranch        *b_calib_EarlyCharge;   //!
   TBranch        *b_calib_Kurtosis;   //!
   TBranch        *b_calib_LateCharge;   //!
   TBranch        *b_calib_MaxQ;   //!
   TBranch        *b_calib_MiddleTimeRMS;   //!
   TBranch        *b_calib_NominalCharge;   //!
   TBranch        *b_calib_PeakRMS;   //!
   TBranch        *b_calib_Quadrant;   //!
   TBranch        *b_calib_dtLastAD1_ms;   //!
   TBranch        *b_calib_dtLastAD2_ms;   //!
   TBranch        *b_calib_dtLastAD3_ms;   //!
   TBranch        *b_calib_dtLastAD4_ms;   //!
   TBranch        *b_calib_dtLastIWS_ms;   //!
   TBranch        *b_calib_dtLastOWS_ms;   //!
   TBranch        *b_calib_dtLastRPC_ms;   //!
   TBranch        *b_calib_dtLast_ADMuon_ms;   //!
   TBranch        *b_calib_dtLast_ADShower_ms;   //!
   TBranch        *b_calib_dtNextAD1_ms;   //!
   TBranch        *b_calib_dtNextAD2_ms;   //!
   TBranch        *b_calib_dtNextAD3_ms;   //!
   TBranch        *b_calib_dtNextAD4_ms;   //!
   TBranch        *b_calib_dtNextIWS_ms;   //!
   TBranch        *b_calib_dtNextOWS_ms;   //!
   TBranch        *b_calib_dtNextRPC_ms;   //!
   TBranch        *b_calib_nHit;   //!
   TBranch        *b_calib_nPERMS;   //!
   TBranch        *b_calib_nPESum;   //!
   TBranch        *b_calib_nPulseRMS;   //!
   TBranch        *b_calib_nPulseSum;   //!
   TBranch        *b_calib_tEarliest;   //!
   TBranch        *b_calib_tLatest;   //!
   TBranch        *b_calib_tMean;   //!
   TBranch        *b_dt_ns;   //!
   TBranch        *b_e;   //!
   TBranch        *b_energyStatus;   //!
   TBranch        *b_fileNo;   //!
   TBranch        *b_multiplicity;   //!
   TBranch        *b_positionStatus;   //!
   TBranch        *b_runNo;   //!
   TBranch        *b_t_ns;   //!
   TBranch        *b_t_s;   //!
   TBranch        *b_triggerNumber;   //!
   TBranch        *b_triggerType;   //!
   TBranch        *b_x;   //!
   TBranch        *b_y;   //!
   TBranch        *b_z;   //!

   CoincidenceTree(TTree *tree=0);
   virtual ~CoincidenceTree();
   virtual Int_t    Cut(Long64_t entry);
   virtual Int_t    GetEntry(Long64_t entry);
   virtual Long64_t LoadTree(Long64_t entry);
   virtual void     Init(TTree *tree);
   virtual void     Loop();
   virtual Bool_t   Notify();
   virtual void     Show(Long64_t entry = -1);
};

#endif

#ifdef CoincidenceTree_cxx
// initialize fChain to 0 to avoid crash on destruction
CoincidenceTree::CoincidenceTree(TTree *tree) : fChain(0)
{
// if parameter tree is not specified (or zero), connect the file
// used to generate this class and read the Tree.
   if (tree == 0) {
   
#ifdef SINGLE_TREE
      // The following code should be used if you want this class to access
      // a single tree instead of a chain
      TFile *f = (TFile*)gROOT->GetListOfFiles()->FindObject("recon.NoTag.0017274.Physics.EH1-Merged.P11A-P._0016.root");
		if(f)
		{
			if (!f || !f->IsOpen()) {
         f = new TFile("recon.NoTag.0017274.Physics.EH1-Merged.P11A-P._0016.root");
      }
      f->GetObject("recon.NoTag.0017274.Physics.EH1-Merged.P11A-P._0016.root:/Event/Data/Physics/CoincidenceLoose",tree);
		}

#else // SINGLE_TREE

      // The following code should be used if you want this class to access a chain
      // of trees.
      TChain * chain = new TChain("recon.NoTag.0017274.Physics.EH1-Merged.P11A-P._0*.root:/Event/Data/Physics/CoincidenceLoose","");
      tree = chain;
#endif // SINGLE_TREE
   }
   if(tree)
		Init(tree);
}

CoincidenceTree::~CoincidenceTree()
{
   if (!fChain) return;
   delete fChain->GetCurrentFile();
}

Int_t CoincidenceTree::GetEntry(Long64_t entry)
{
// Read contents of entry.
   if (!fChain) return 0;
   return fChain->GetEntry(entry);
}
Long64_t CoincidenceTree::LoadTree(Long64_t entry)
{
// Set the environment to read one entry
   if (!fChain) return -5;
   Long64_t centry = fChain->LoadTree(entry);
   if (centry < 0) return centry;
   if (fChain->GetTreeNumber() != fCurrent) {
      fCurrent = fChain->GetTreeNumber();
      Notify();
   }
   return centry;
}

void CoincidenceTree::Init(TTree *tree)
{
   // The Init() function is called when the selector needs to initialize
   // a new tree or chain. Typically here the branch addresses and branch
   // pointers of the tree will be set.
   // It is normally not necessary to make changes to the generated
   // code, but the routine can be extended by the user if needed.
   // Init() will be called many times when running on PROOF
   // (once per file to be processed).

   // Set object pointer
   I = 0;
   J = 0;
   calib_ELast_ADShower_ms = 0;
   calib_EarlyCharge = 0;
   calib_Kurtosis = 0;
   calib_LateCharge = 0;
   calib_MaxQ = 0;
   calib_MiddleTimeRMS = 0;
   calib_NominalCharge = 0;
   calib_PeakRMS = 0;
   calib_Quadrant = 0;
   calib_dtLastAD1_ms = 0;
   calib_dtLastAD2_ms = 0;
   calib_dtLastAD3_ms = 0;
   calib_dtLastAD4_ms = 0;
   calib_dtLastIWS_ms = 0;
   calib_dtLastOWS_ms = 0;
   calib_dtLastRPC_ms = 0;
   calib_dtLast_ADMuon_ms = 0;
   calib_dtLast_ADShower_ms = 0;
   calib_dtNextAD1_ms = 0;
   calib_dtNextAD2_ms = 0;
   calib_dtNextAD3_ms = 0;
   calib_dtNextAD4_ms = 0;
   calib_dtNextIWS_ms = 0;
   calib_dtNextOWS_ms = 0;
   calib_dtNextRPC_ms = 0;
   calib_nHit = 0;
   calib_nPERMS = 0;
   calib_nPESum = 0;
   calib_nPulseRMS = 0;
   calib_nPulseSum = 0;
   calib_tEarliest = 0;
   calib_tLatest = 0;
   calib_tMean = 0;
   dt_ns = 0;
   e = 0;
   energyStatus = 0;
   positionStatus = 0;
   t_ns = 0;
   t_s = 0;
   triggerNumber = 0;
   triggerType = 0;
   x = 0;
   y = 0;
   z = 0;
   // Set branch addresses and branch pointers
   if (!tree) return;
   fChain = tree;
   fCurrent = -1;
   fChain->SetMakeClass(1);

   fChain->SetBranchAddress("clID", &clID, &b_Data_Physics_CoincidenceLoose_clID);
   fChain->SetBranchAddress("earliest.mSec", &earliest_mSec, &b_Data_Physics_CoincidenceLoose_earliest_mSec);
   fChain->SetBranchAddress("earliest.mNanoSec", &earliest_mNanoSec, &b_Data_Physics_CoincidenceLoose_earliest_mNanoSec);
   fChain->SetBranchAddress("latest.mSec", &latest_mSec, &b_Data_Physics_CoincidenceLoose_latest_mSec);
   fChain->SetBranchAddress("latest.mNanoSec", &latest_mNanoSec, &b_Data_Physics_CoincidenceLoose_latest_mNanoSec);
   fChain->SetBranchAddress("context.mSite", &context_mSite, &b_Data_Physics_CoincidenceLoose_context_mSite);
   fChain->SetBranchAddress("context.mSimFlag", &context_mSimFlag, &b_Data_Physics_CoincidenceLoose_context_mSimFlag);
   fChain->SetBranchAddress("context.mTimeStamp.mSec", &context_mTimeStamp_mSec, &b_Data_Physics_CoincidenceLoose_context_mTimeStamp_mSec);
   fChain->SetBranchAddress("context.mTimeStamp.mNanoSec", &context_mTimeStamp_mNanoSec, &b_Data_Physics_CoincidenceLoose_context_mTimeStamp_mNanoSec);
   fChain->SetBranchAddress("context.mDetId", &context_mDetId, &b_Data_Physics_CoincidenceLoose_context_mDetId);
   fChain->SetBranchAddress("execNumber", &execNumber, &b_Data_Physics_CoincidenceLoose_execNumber);
   fChain->SetBranchAddress("jobId.m_data[4]", jobId_m_data, &b_Data_Physics_CoincidenceLoose_jobId_m_data);
   fChain->SetBranchAddress("randomState", &randomState, &b_Data_Physics_CoincidenceLoose_randomState);
   fChain->SetBranchAddress("inputHeaders", &inputHeaders_, &b_Data_Physics_CoincidenceLoose_inputHeaders_);
   fChain->SetBranchAddress("inputHeaders.m_entry", inputHeaders_m_entry, &b_inputHeaders_m_entry);
   fChain->SetBranchAddress("inputHeaders.m_path", inputHeaders_m_path, &b_inputHeaders_m_path);
   fChain->SetBranchAddress("I", &I, &b_I);
   fChain->SetBranchAddress("J", &J, &b_J);
   fChain->SetBranchAddress("calib_ELast_ADShower_ms", &calib_ELast_ADShower_ms, &b_calib_ELast_ADShower_ms);
   fChain->SetBranchAddress("calib_EarlyCharge", &calib_EarlyCharge, &b_calib_EarlyCharge);
   fChain->SetBranchAddress("calib_Kurtosis", &calib_Kurtosis, &b_calib_Kurtosis);
   fChain->SetBranchAddress("calib_LateCharge", &calib_LateCharge, &b_calib_LateCharge);
   fChain->SetBranchAddress("calib_MaxQ", &calib_MaxQ, &b_calib_MaxQ);
   fChain->SetBranchAddress("calib_MiddleTimeRMS", &calib_MiddleTimeRMS, &b_calib_MiddleTimeRMS);
   fChain->SetBranchAddress("calib_NominalCharge", &calib_NominalCharge, &b_calib_NominalCharge);
   fChain->SetBranchAddress("calib_PeakRMS", &calib_PeakRMS, &b_calib_PeakRMS);
   fChain->SetBranchAddress("calib_Quadrant", &calib_Quadrant, &b_calib_Quadrant);
   fChain->SetBranchAddress("calib_dtLastAD1_ms", &calib_dtLastAD1_ms, &b_calib_dtLastAD1_ms);
   fChain->SetBranchAddress("calib_dtLastAD2_ms", &calib_dtLastAD2_ms, &b_calib_dtLastAD2_ms);
   fChain->SetBranchAddress("calib_dtLastAD3_ms", &calib_dtLastAD3_ms, &b_calib_dtLastAD3_ms);
   fChain->SetBranchAddress("calib_dtLastAD4_ms", &calib_dtLastAD4_ms, &b_calib_dtLastAD4_ms);
   fChain->SetBranchAddress("calib_dtLastIWS_ms", &calib_dtLastIWS_ms, &b_calib_dtLastIWS_ms);
   fChain->SetBranchAddress("calib_dtLastOWS_ms", &calib_dtLastOWS_ms, &b_calib_dtLastOWS_ms);
   fChain->SetBranchAddress("calib_dtLastRPC_ms", &calib_dtLastRPC_ms, &b_calib_dtLastRPC_ms);
   fChain->SetBranchAddress("calib_dtLast_ADMuon_ms", &calib_dtLast_ADMuon_ms, &b_calib_dtLast_ADMuon_ms);
   fChain->SetBranchAddress("calib_dtLast_ADShower_ms", &calib_dtLast_ADShower_ms, &b_calib_dtLast_ADShower_ms);
   fChain->SetBranchAddress("calib_dtNextAD1_ms", &calib_dtNextAD1_ms, &b_calib_dtNextAD1_ms);
   fChain->SetBranchAddress("calib_dtNextAD2_ms", &calib_dtNextAD2_ms, &b_calib_dtNextAD2_ms);
   fChain->SetBranchAddress("calib_dtNextAD3_ms", &calib_dtNextAD3_ms, &b_calib_dtNextAD3_ms);
   fChain->SetBranchAddress("calib_dtNextAD4_ms", &calib_dtNextAD4_ms, &b_calib_dtNextAD4_ms);
   fChain->SetBranchAddress("calib_dtNextIWS_ms", &calib_dtNextIWS_ms, &b_calib_dtNextIWS_ms);
   fChain->SetBranchAddress("calib_dtNextOWS_ms", &calib_dtNextOWS_ms, &b_calib_dtNextOWS_ms);
   fChain->SetBranchAddress("calib_dtNextRPC_ms", &calib_dtNextRPC_ms, &b_calib_dtNextRPC_ms);
   fChain->SetBranchAddress("calib_nHit", &calib_nHit, &b_calib_nHit);
   fChain->SetBranchAddress("calib_nPERMS", &calib_nPERMS, &b_calib_nPERMS);
   fChain->SetBranchAddress("calib_nPESum", &calib_nPESum, &b_calib_nPESum);
   fChain->SetBranchAddress("calib_nPulseRMS", &calib_nPulseRMS, &b_calib_nPulseRMS);
   fChain->SetBranchAddress("calib_nPulseSum", &calib_nPulseSum, &b_calib_nPulseSum);
   fChain->SetBranchAddress("calib_tEarliest", &calib_tEarliest, &b_calib_tEarliest);
   fChain->SetBranchAddress("calib_tLatest", &calib_tLatest, &b_calib_tLatest);
   fChain->SetBranchAddress("calib_tMean", &calib_tMean, &b_calib_tMean);
   fChain->SetBranchAddress("dt_ns", &dt_ns, &b_dt_ns);
   fChain->SetBranchAddress("e", &e, &b_e);
   fChain->SetBranchAddress("energyStatus", &energyStatus, &b_energyStatus);
   fChain->SetBranchAddress("fileNo", &fileNo, &b_fileNo);
   fChain->SetBranchAddress("multiplicity", &multiplicity, &b_multiplicity);
   fChain->SetBranchAddress("positionStatus", &positionStatus, &b_positionStatus);
   fChain->SetBranchAddress("runNo", &runNo, &b_runNo);
   fChain->SetBranchAddress("t_ns", &t_ns, &b_t_ns);
   fChain->SetBranchAddress("t_s", &t_s, &b_t_s);
   fChain->SetBranchAddress("triggerNumber", &triggerNumber, &b_triggerNumber);
   fChain->SetBranchAddress("triggerType", &triggerType, &b_triggerType);
   fChain->SetBranchAddress("x", &x, &b_x);
   fChain->SetBranchAddress("y", &y, &b_y);
   fChain->SetBranchAddress("z", &z, &b_z);
   Notify();
}

Bool_t CoincidenceTree::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   return kTRUE;
}

void CoincidenceTree::Show(Long64_t entry)
{
// Print contents of entry.
// If entry is not specified, print current entry
   if (!fChain) return;
   fChain->Show(entry);
}
Int_t CoincidenceTree::Cut(Long64_t entry)
{
// This function may be called from Loop.
// returns  1 if entry is accepted.
// returns -1 otherwise.
   return 1;
}
#endif // #ifdef CoincidenceTree_cxx
