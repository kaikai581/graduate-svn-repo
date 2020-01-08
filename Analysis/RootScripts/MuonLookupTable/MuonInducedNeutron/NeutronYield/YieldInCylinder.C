/*
 * Measure neutron yield for two different regions.
 * One corresponds to thick mountain, thus high energy muons.
 * The other corresponds to thin mountain, thus low energy muons.
*/

#include <iostream>
#include "TCanvas.h"
#include "TChain.h"
#include "TLeaf.h"

void YieldInCylinder(int det=1)
{
  vector<string> folders;
  folders.push_back("/disk1/data/MuonLookupTable/MuonInducedNeutron/2011/p12e/Neutrino/1224/");
  folders.push_back("/disk1/data/MuonLookupTable/MuonInducedNeutron/2011/p12e/Neutrino/1225/");
  folders.push_back("/disk1/data/MuonLookupTable/MuonInducedNeutron/2011/p12e/Neutrino/1226/");
  folders.push_back("/disk1/data/MuonLookupTable/MuonInducedNeutron/2011/p12e/Neutrino/1227/");
  
  TChain* ch = new TChain("RPC-OWS/mun");
  //for(unsigned int i = 0; i < folders.size(); i++)
  //{
    //string files = folders[i] + "*EH1*.root";
    //ch->Add(files.c_str());
  //}
  ch->Add("/disk1/data/MuonLookupTable/MuonInducedNeutron/2011/p12e/Neutrino/1226/neutron.Neutrino.0021232.Physics.EH3-Merged.P12E-P._0044.root");
  
  double tottrlen[3] = {0};
  int nNeutron[3] = {0};
  int nMuon[3] = {0};
  
  double dt[1000], e[1000];
  bool inCyl[1000];
  ch->SetBranchAddress(Form("dtIntEvtAd%d",det), &dt);
  ch->SetBranchAddress(Form("eIntEvtAd%d",det), &e);
  ch->SetBranchAddress(Form("inCyl%d",det), &inCyl);
  
  int nent = ch->GetEntries();
  for(int ent = 0; ent < nent; ent++)
  {
    ch->GetEntry(ent);
    
    /// if passes IAV, add up the track length
    if(!(int)ch->GetLeaf(Form("inscribable%d",det))->GetValue()) continue;
    
    double theta = ch->GetLeaf("theta")->GetValue();
    double phi = ch->GetLeaf("phi")->GetValue();
    double phi2 = (phi+(1-(phi>0)+(phi<0))*3.1415)*180./3.1415;
    
    int region = -1;
    
    if(cos(theta)>.5&&phi2>240.&&phi2<360.) region = 0;
    if(cos(theta)<.5&&phi2>120.&&phi2<240.) region = 1;
    if(cos(theta)<.5&&phi2>0.&&phi2<120.) region = 2;
    
    tottrlen[region] += ch->GetLeaf(Form("tCyl%dUp",det))->GetValue()-ch->GetLeaf(Form("tCyl%dBot",det))->GetValue();
    nMuon[region]++;
    
    int nneu = (int)ch->GetLeaf(Form("nIntEvtAd%d",det))->GetValue();
    for(int i = 0; i < nneu; i++)
    {
      if(dt[i] > 20e-6 && dt[i] < 1000e-6 && e[i] > 6 && e[i] < 12 && inCyl[i])
      {
        nNeutron[region]++;
      }
    }
  }
  
  cout << tottrlen[0] << " " << tottrlen[1] << " " << tottrlen[2] << endl;
  cout << nNeutron[0] << " " << nNeutron[1] << " " << nNeutron[2] << endl;
  cout << nMuon[0] << " " << nMuon[1] << " " << nMuon[2] << endl;
}
