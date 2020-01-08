/*
 * Measure neutron yield for two different regions.
 * One corresponds to thick mountain, thus high energy muons.
 * The other corresponds to thin mountain, thus low energy muons.
 * 
 * From BCW's technote, shower muons are muons with charge > 300000 PEs.
*/

#include <fstream>
#include <iostream>
#include <sstream>
#include "TCanvas.h"
#include "TChain.h"
#include "TFile.h"
#include "TH1F.h"
#include "TLeaf.h"

#define MAXNINTEREVT 10000

void RhoDistLoopEvent(int det=1, string infname = "EH1DataPath.txt")
{
  vector<string> folders;
  ifstream infpath(infname.c_str());
  string tmpline;
  while(infpath >> tmpline) folders.push_back(tmpline.c_str());
  
  TChain* ch = new TChain("RPC-OWS/mun");
  for(unsigned int i = 0; i < folders.size(); i++)
  {
    string files = folders[i];
    ch->Add(files.c_str());
  }
  
  double tottrlen = 0;
  int nNeutron = 0;
  int nMuon = 0;
  /// containers for shower muons
  double tottrlen_shmu = 0;
  int nNeutron_shmu = 0;
  int nMuon_shmu = 0;
  
  double emu;
  double dt[MAXNINTEREVT], e[MAXNINTEREVT], dlNeu[MAXNINTEREVT];
  bool inCyl[MAXNINTEREVT];
  ch->SetBranchAddress(Form("eAd%d",det), &emu);
  ch->SetBranchAddress(Form("dtIntEvtAd%d",det), &dt);
  ch->SetBranchAddress(Form("eIntEvtAd%d",det), &e);
  ch->SetBranchAddress(Form("dlIntEvt%d",det), &dlNeu);
  ch->SetBranchAddress(Form("inCyl%d",det), &inCyl);
  
  int nent = ch->GetEntries();
  stringstream outfn;
  outfn << infname << ".shower.root";
  TFile* outf = new TFile(outfn.str().c_str(),"RECREATE");
  TH1F* hshower = new TH1F(Form("ad%d_shower",det),Form("AD%d neutron radial distribution by shower muons", det), 50, 0, 1000);
  TH1F* hnonshower = new TH1F(Form("ad%d_nonshower",det),Form("AD%d neutron radial distribution by nonshower muons", det), 50, 0, 1000);
  for(int ent = 0; ent < nent; ent++)
  {
    ch->GetEntry(ent);
    
    /// if passes IAV, add up the track length
    if(!(int)ch->GetLeaf(Form("inscribable%d",det))->GetValue()) continue;
    
    double curlen = ch->GetLeaf(Form("tCyl%dUp",det))->GetValue()-ch->GetLeaf(Form("tCyl%dBot",det))->GetValue();
    tottrlen += curlen;
    nMuon++;
    if(emu > 1500)
    {
      tottrlen_shmu += curlen;
      nMuon_shmu++;
    }
    
    int nneu = (int)ch->GetLeaf(Form("nIntEvtAd%d",det))->GetValue();
    for(int i = 0; i < nneu; i++)
    {
      if(dt[i] > 20e-6 && dt[i] < 1000e-6 && e[i] > 6 && e[i] < 12 && inCyl[i])
      {
        nNeutron++;
        if(emu > 1500)
        {
          nNeutron_shmu++;
          hshower->Fill(dlNeu[i]);
        }
        else hnonshower->Fill(dlNeu[i]);
      }
    }
  }
  
  hshower->Write();
  hnonshower->Write();
  outf->Close();
}
