#include <iostream>
#include <string>
#include "EnrichedTreeVars.hpp"
#include "TFile.h"
#include "TH1F.h"
#include "TTree.h"
#include "utilities.hpp"

using namespace std;


void write5StripsHists(TFile&, EnrichedTreeVars&);
void writeClusterSizeHists(TFile&, EnrichedTreeVars&);

void addBranch2File(string ifpn, string ofp, int nEvt, int printEvt,
                    bool owFlag, bool dbgFlag)
{
  
  int runNumber = atoi(getRunNumber(ifpn).c_str());
  int fileNumber = atoi(getFileNumber(ifpn).c_str());
  cout << "processing run " << runNumber << " file " << fileNumber << endl;
  
  /// open the TFile with the option set by owerwrite flag
  string openOpt;
  if(owFlag) openOpt = "update";
  else       openOpt = "read";
  
  /// clone old trees to new trees
  TFile fif(ifpn.c_str(), openOpt.c_str());
  TTree* tOld = (TTree*)fif.Get("rpcTree");
  TFile fnew(Form("%s", getOutputPathName(ifpn, ofp).c_str()), "recreate");
  TTree *tNew = tOld->CloneTree();
  
  
  /// assign variable containers
  EnrichedTreeVars etv;
  tOld->SetBranchAddress("triggerTimeSec", &etv.m_triggerTimeSec);
  tOld->SetBranchAddress("triggerTimeNanoSec", &etv.m_triggerTimeNanoSec);
  tOld->SetBranchAddress("mRpcFromRot", &etv.m_mRpcFromRot);
  tOld->SetBranchAddress("mRpcRow", &etv.m_mRpcRow);
  tOld->SetBranchAddress("mRpcColumn", &etv.m_mRpcColumn);
  tOld->SetBranchAddress("mRpcLayer", &etv.m_mRpcLayer);
  tOld->SetBranchAddress("mRpcStrip", &etv.m_mRpcStrip);
  tOld->SetBranchAddress("nModules", &etv.m_nModules);
  tOld->SetBranchAddress("mNLayers", &etv.m_mNLayers);
  tOld->SetBranchAddress("mNStrips", &etv.m_mNStrips);
  tOld->SetBranchAddress("nStrips", &etv.m_nStrips);
  
  int eventId;
  TBranch *b1 = tNew->Branch("eventId", &eventId, "eventId/I");
  TBranch *b2 = tNew->Branch("runNumber", &runNumber, "runNumber/I");
  TBranch *b3 = tNew->Branch("fileNumber", &fileNumber, "fileNumber/I");
  TBranch *b4 = tNew->Branch("lStripCms", &etv.m_lStripCms);
  TBranch *b5 = tNew->Branch("mXCmsDist", &etv.m_mXCmsDist);
  TBranch *b6 = tNew->Branch("mYCmsDist", &etv.m_mYCmsDist);
  TBranch *b7 = tNew->Branch("mNTracksX", &etv.m_mNTracksX);
  TBranch *b8 = tNew->Branch("mNTracksY", &etv.m_mNTracksY);
  TBranch *b9 = tNew->Branch("mNClusters", &etv.m_mNClusters);
  TBranch *b10 = tNew->Branch("mMaxClusterSizeX", &etv.m_mMaxClusterSizeX);
  TBranch *b11 = tNew->Branch("mMaxClusterSizeY", &etv.m_mMaxClusterSizeY);
  TBranch *b12 = tNew->Branch("mTrackClusterIdxX1", &etv.m_mTrackClusterIdxX1);
  TBranch *b13 = tNew->Branch("mTrackClusterIdxY2", &etv.m_mTrackClusterIdxY2);
  TBranch *b14 = tNew->Branch("mTrackClusterIdxY3", &etv.m_mTrackClusterIdxY3);
  TBranch *b15 = tNew->Branch("mTrackClusterIdxX4", &etv.m_mTrackClusterIdxX4);
  TBranch *b16 = tNew->Branch("mDx", &etv.m_mDx);
  TBranch *b17 = tNew->Branch("mDy", &etv.m_mDy);
  TBranch *b18 = tNew->Branch("mNDx", &etv.m_mNDx);
  TBranch *b19 = tNew->Branch("mNDy", &etv.m_mNDy);
  TBranch *b20 = tNew->Branch("mDxMuon", &etv.m_mDxMuon);
  TBranch *b21 = tNew->Branch("mDyMuon", &etv.m_mDyMuon);
  TBranch *b22 = tNew->Branch("dSingleMuon", &etv.m_dSingleMuon);
  TBranch *b23 = tNew->Branch("singleStripId32", &etv.m_singleStripId32);
  TBranch *b24 = tNew->Branch("mNSpuriousX", &etv.m_mNSpuriousX);
  TBranch *b25 = tNew->Branch("mNSpuriousY", &etv.m_mNSpuriousY);
  
  
  
  Long64_t nentries = tOld->GetEntries();
  for (Long64_t i=0;i<nentries;i++)
  {
    if(nEvt > 0 && i >= nEvt)
      break;
    tOld->GetEntry(i);
    eventId = i;
    etv.getHitLayersAndReverseIndex();
    etv.getStripCms();
    etv.getCmsDist();
    etv.getClusterHits();
    etv.getTracks();
    etv.getDistance();
    //tNew->Fill();
    b1->Fill();
    b2->Fill();
    b3->Fill();
    b4->Fill();
    b5->Fill();
    b6->Fill();
    b7->Fill();
    b8->Fill();
    b9->Fill();
    b10->Fill();
    b11->Fill();
    b12->Fill();
    b13->Fill();
    b14->Fill();
    b15->Fill();
    b16->Fill();
    b17->Fill();
    b18->Fill();
    b19->Fill();
    b20->Fill();
    b21->Fill();
    b22->Fill();
    b23->Fill();
    b24->Fill();
    b25->Fill();
    
    
    /// print debug information
    if(dbgFlag || i==printEvt)
    {
      unsigned int nModules = etv.m_nModules;
      cout << "number of modules in this trigger: " << nModules << endl;
      for(unsigned int m = 0; m < nModules; m++)
      {
        cout << "module ID: (" << etv.m_mRpcRow->at(m);
        cout << "," << etv.m_mRpcColumn->at(m) << ")" << endl;
        
        
        cout << "\tfired layer ID:";
        for(unsigned int l = 0; l < etv.m_mNLayers->at(m); l++)
          cout << " " << etv.m_mRpcLayer->at(m)[l];
        cout << endl;
        
        cout << "\tfired strip ID: ";
        for(unsigned int l = 0; l < etv.m_mNLayers->at(m); l++)
        {
          cout << "(";
          for(unsigned int s = 0; s < etv.m_nStrips->at(m)[l]; s++)
          {
            cout << etv.m_mRpcStrip->at(m)[l][s];
            if(s != etv.m_nStrips->at(m)[l]-1)
              cout << ",";
          }
          cout << ") ";
        }
        cout << endl;
        
        cout << "\tcms distance for x and y: ";
        cout << etv.m_mXCmsDist->at(m) << " " << etv.m_mYCmsDist->at(m) << endl;
        
        cout << "\tcluster hits: ";
        for(unsigned int l = 0; l < etv.m_mNLayers->at(m); l++)
        {
          cout << "(";
          for(unsigned int c = 0; c < etv.m_mClusterHits->at(m)[l].size(); c++)
          {
            cout << "(";
            for(unsigned int s = 0; s < etv.m_mClusterHits->at(m)[l][c].size(); s++)
            {
              cout << etv.m_mClusterHits->at(m)[l][c][s];
              if(s!=etv.m_mClusterHits->at(m)[l][c].size()-1)
                cout << ",";
              else
                cout << ")";
            }
            if(c!=etv.m_mClusterHits->at(m)[l].size()-1)
              cout << ",";
            else cout << ") ";
          }
        }
        cout << endl;
        
        
        cout << "\tnumber of x and y tracks: " << etv.m_mNTracksX->at(m) << " ";
        cout << etv.m_mNTracksY->at(m) << endl;
        
        
        cout << "\toverlapping x clusters: ";
        if(etv.m_mNTracksX->at(m))
        {
          cout << "(";
          for(unsigned int tt = 0; tt < etv.m_mNTracksX->at(m); tt++)
          {
            cout << "(";
            for(unsigned int s = 0; s < etv.m_mClusterHits->at(m)[etv.m_layer2Idx[m][0]][etv.m_mTrackClusterIdxX1->at(m)[tt]].size(); s++)
            {
              cout << etv.m_mClusterHits->at(m)[etv.m_layer2Idx[m][0]][etv.m_mTrackClusterIdxX1->at(m)[tt]][s];
              cout << ((s==etv.m_mClusterHits->at(m)[etv.m_layer2Idx[m][0]][etv.m_mTrackClusterIdxX1->at(m)[tt]].size()-1)?")":",");
            }
            cout << "(";
            for(unsigned int s = 0; s < etv.m_mClusterHits->at(m)[etv.m_layer2Idx[m][3]][etv.m_mTrackClusterIdxX4->at(m)[tt]].size(); s++)
            {
              cout << etv.m_mClusterHits->at(m)[etv.m_layer2Idx[m][3]][etv.m_mTrackClusterIdxX4->at(m)[tt]][s];
              cout << ((s==etv.m_mClusterHits->at(m)[etv.m_layer2Idx[m][3]][etv.m_mTrackClusterIdxX4->at(m)[tt]].size()-1)?")":",");
            }
          }
          cout << ")";
        }
        cout << endl;
        
        
        cout << "\toverlapping y clusters: ";
        if(etv.m_mNTracksY->at(m))
        {
          cout << "(";
          for(unsigned int tt = 0; tt < etv.m_mNTracksY->at(m); tt++)
          {
            cout << "(";
            for(unsigned int s = 0; s < etv.m_mClusterHits->at(m)[etv.m_layer2Idx[m][1]][etv.m_mTrackClusterIdxY2->at(m)[tt]].size(); s++)
            {
              cout << etv.m_mClusterHits->at(m)[etv.m_layer2Idx[m][1]][etv.m_mTrackClusterIdxY2->at(m)[tt]][s];
              cout << ((s==etv.m_mClusterHits->at(m)[etv.m_layer2Idx[m][1]][etv.m_mTrackClusterIdxY2->at(m)[tt]].size()-1)?")":",");
            }
            cout << "(";
            for(unsigned int s = 0; s < etv.m_mClusterHits->at(m)[etv.m_layer2Idx[m][2]][etv.m_mTrackClusterIdxY3->at(m)[tt]].size(); s++)
            {
              cout << etv.m_mClusterHits->at(m)[etv.m_layer2Idx[m][2]][etv.m_mTrackClusterIdxY3->at(m)[tt]][s];
              cout << ((s==etv.m_mClusterHits->at(m)[etv.m_layer2Idx[m][2]][etv.m_mTrackClusterIdxY3->at(m)[tt]].size()-1)?")":",");
            }
          }
          cout << ")";
        }
        cout << endl;
        
        
        
        
        cout << "\tnumber of x and y spurious tracks: ";
        cout << etv.m_mNSpuriousX->at(m) << " ";
        cout << etv.m_mNSpuriousY->at(m) << endl;
        
        
        cout << "\tspurious x clusters: ";
        if(etv.m_mNSpuriousX->at(m))
        {
          cout << "(";
          for(unsigned int tt = 0; tt < etv.m_mNSpuriousX->at(m); tt++)
          {
            cout << "(";
            for(unsigned int s = 0; s < etv.m_mClusterHits->at(m)[etv.m_layer2Idx[m][0]][etv.m_mSpuriousClusterIdxX1->at(m)[tt]].size(); s++)
            {
              cout << etv.m_mClusterHits->at(m)[etv.m_layer2Idx[m][0]][etv.m_mSpuriousClusterIdxX1->at(m)[tt]][s];
              cout << ((s==etv.m_mClusterHits->at(m)[etv.m_layer2Idx[m][0]][etv.m_mSpuriousClusterIdxX1->at(m)[tt]].size()-1)?")":",");
            }
            cout << "(";
            for(unsigned int s = 0; s < etv.m_mClusterHits->at(m)[etv.m_layer2Idx[m][3]][etv.m_mSpuriousClusterIdxX4->at(m)[tt]].size(); s++)
            {
              cout << etv.m_mClusterHits->at(m)[etv.m_layer2Idx[m][3]][etv.m_mSpuriousClusterIdxX4->at(m)[tt]][s];
              cout << ((s==etv.m_mClusterHits->at(m)[etv.m_layer2Idx[m][3]][etv.m_mSpuriousClusterIdxX4->at(m)[tt]].size()-1)?")":",");
            }
          }
          cout << ")";
        }
        cout << endl;
        


        cout << "\tspurious y clusters: ";
        if(etv.m_mNSpuriousY->at(m))
        {
          cout << "(";
          for(unsigned int tt = 0; tt < etv.m_mNSpuriousY->at(m); tt++)
          {
            cout << "(";
            for(unsigned int s = 0; s < etv.m_mClusterHits->at(m)[etv.m_layer2Idx[m][1]][etv.m_mSpuriousClusterIdxY2->at(m)[tt]].size(); s++)
            {
              cout << etv.m_mClusterHits->at(m)[etv.m_layer2Idx[m][1]][etv.m_mSpuriousClusterIdxY2->at(m)[tt]][s];
              cout << ((s==etv.m_mClusterHits->at(m)[etv.m_layer2Idx[m][1]][etv.m_mSpuriousClusterIdxY2->at(m)[tt]].size()-1)?")":",");
            }
            cout << "(";
            for(unsigned int s = 0; s < etv.m_mClusterHits->at(m)[etv.m_layer2Idx[m][2]][etv.m_mSpuriousClusterIdxY3->at(m)[tt]].size(); s++)
            {
              cout << etv.m_mClusterHits->at(m)[etv.m_layer2Idx[m][2]][etv.m_mSpuriousClusterIdxY3->at(m)[tt]][s];
              cout << ((s==etv.m_mClusterHits->at(m)[etv.m_layer2Idx[m][2]][etv.m_mSpuriousClusterIdxY3->at(m)[tt]].size()-1)?")":",");
            }
          }
          cout << ")";
        }
        cout << endl;

        
        
        cout << "\tnumber of dx and dy calculated: " << etv.m_mNDx->at(m);
        cout << " " << etv.m_mNDy->at(m) << endl;
        if(etv.m_mNDx->at(m) > 0)
        {
          cout << "\tdx: ";
          for(unsigned int dd = 0; dd < etv.m_mNDx->at(m); dd++)
            cout << etv.m_mDx->at(m)[dd] << " ";
          cout << endl;
        }
        if(etv.m_mNDy->at(m) > 0)
        {
          cout << "\tdy: ";
          for(unsigned int dd = 0; dd < etv.m_mNDy->at(m); dd++)
            cout << etv.m_mDy->at(m)[dd] << " ";
          cout << endl;
        }
        
      }
      cout << endl;
    }
    
    
  }
  

  tNew->Write();
  write5StripsHists(fnew, etv);
  writeClusterSizeHists(fnew, etv);
  fnew.Close();
  fif.Close();
}




void write5StripsHists(TFile& f, EnrichedTreeVars& v)
{
  f.cd("/");
  if(!f.GetDirectory("Singles2MuonDist")) f.mkdir("Singles2MuonDist");
  f.cd("Singles2MuonDist");
  
  map<pair<unsigned int, unsigned int>, vector<unsigned int> >::iterator it;
  for(it = v.m_dSingleMuonH2.begin(); it != v.m_dSingleMuonH2.end(); it++)
  {
    string hName = Form("R%dC%d", it->first.first, it->first.second);
    TH1I h(hName.c_str(), hName.c_str(), 8, 0, 8);
    for(unsigned int s = 0; s < it->second.size(); s++)
      h.Fill(it->second[s]);
    h.Write();
  }
  
  /// write signed distance
  map<pair<unsigned int, unsigned int>, vector<int> >::iterator it2;
  for(it2 = v.m_dSingleMuonSigned.begin(); it2 != v.m_dSingleMuonSigned.end(); it2++)
  {
    string hName = Form("R%dC%dSigned", it2->first.first, it2->first.second);
    TH1I h(hName.c_str(), hName.c_str(), 16, -8, 8);
    for(unsigned int s = 0; s < it2->second.size(); s++)
      h.Fill(it2->second[s]);
    h.Write();
  }
}



void writeClusterSizeHists(TFile& f, EnrichedTreeVars& v)
{
  f.cd("/");
  if(!f.GetDirectory("ClusterSize")) f.mkdir("ClusterSize");
  f.cd("ClusterSize");
  
  
  map<pair<unsigned int, unsigned int>, vector<unsigned int> >::iterator it;
  for(it = v.m_clusterSizeDataSet.begin(); it != v.m_clusterSizeDataSet.end(); it++)
  {
    string hName = Form("R%dC%d", it->first.first, it->first.second);
    TH1I h(hName.c_str(), hName.c_str(), 8, 0, 8);
    for(unsigned int s = 0; s < it->second.size(); s++)
      h.Fill(it->second[s]);
    h.Write();
  }
}
