/*
 * Try to use stopping power to define showering muon.
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

void StoppingPower(int det=1, string infname = "EH1DataPath.txt")
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
  
  TFile* fdedx = new TFile("dedx.root", "recreate");
  TH1F* hdedxall = new TH1F("hdedxall", "", 200,0,10);
  TH1F* hdedxwithn = new TH1F("hdedxwithn", "", 200,0,10);
  
  double tottrlen = 0;
  int nNeutron = 0;
  int nMuon = 0;
  /// containers for shower muons
  double tottrlen_shmu = 0;
  int nNeutron_shmu = 0;
  int nMuon_shmu = 0;
  
  double emu, dlOav;
  double dt[MAXNINTEREVT], e[MAXNINTEREVT];
  bool inCyl[MAXNINTEREVT];
  ch->SetBranchAddress(Form("eAd%d",det), &emu);
  ch->SetBranchAddress(Form("dlOav%d",det), &dlOav);
  ch->SetBranchAddress(Form("dtIntEvtAd%d",det), &dt);
  ch->SetBranchAddress(Form("eIntEvtAd%d",det), &e);
  ch->SetBranchAddress(Form("inCyl%d",det), &inCyl);
  
  int nent = ch->GetEntries();
  for(int ent = 0; ent < nent; ent++)
  {
    ch->GetEntry(ent);
    
    /// if passes IAV, add up the track length
    if(!(int)ch->GetLeaf(Form("inscribable%d",det))->GetValue()) continue;
    
    //double curlen = ch->GetLeaf(Form("tCyl%dUp",det))->GetValue()-ch->GetLeaf(Form("tCyl%dBot",det))->GetValue();
    //tottrlen += curlen;
    //nMuon++;
    //if(emu > 1500)
    //{
      //tottrlen_shmu += curlen;
      //nMuon_shmu++;
    //}
    
    bool hasGdNeutron = false;
    int nneu = (int)ch->GetLeaf(Form("nIntEvtAd%d",det))->GetValue();
    for(int i = 0; i < nneu; i++)
    {
      if(dt[i] > 20e-6 && dt[i] < 1000e-6 && e[i] > 6 && e[i] < 12 && inCyl[i])
      {
        nNeutron++;
        hasGdNeutron = true;
      }
    }
    
    double dedx = emu/dlOav*10;
    if(hasGdNeutron) hdedxwithn->Fill(dedx);
    else if(nneu == 0) hdedxall->Fill(dedx);
  }
  
  hdedxwithn->Write();
  hdedxall->Write();
  fdedx->Close();
  //cout << tottrlen << endl;
  //cout << nNeutron << endl;
  //cout << nMuon << endl;
  //cout << tottrlen_shmu << endl;
  //cout << nNeutron_shmu << endl;
  //cout << nMuon_shmu << endl;
  
  //stringstream outfn;
  //outfn << infname << ".out";
  //ofstream outf(outfn.str().c_str());
  //outf << "tottrlen " << tottrlen << endl;
  //outf << "nNeutron " << nNeutron << endl;
  //outf << "nMuon " << nMuon << endl;
  //outf << "tottrlen_shmu " << tottrlen_shmu << endl;
  //outf << "nNeutron_shmu " << nNeutron_shmu << endl;
  //outf << "nMuon_shmu " << nMuon_shmu << endl;
  //outf.close();
}
