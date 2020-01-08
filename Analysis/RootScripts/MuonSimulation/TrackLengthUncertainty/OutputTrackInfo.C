/*
 * Script for estimating track length uncertainty of RPC-OWS reconstruction.
 * Based on Jimmy Ngai's script.
 * 
 * 
 * IMPORTANT LESSON:
 * Each execution cycle a GenHeader is generated. However not every generated
 * particle is reconstructed. To find out which generated particle is
 * reconstructed, go to SimHeader and read the "inputHeaders.m_entry" for the
 * corresponding entry in GenHeader.
 * 
 * 
 * To make a file list, one can use the following command:
 * $ find /project/projectdirs/dayabay/scratch/jimmy/MuonMCTest/rec/ -type f|xargs ls > filelist.txt
 * 
 * Usage:
 *        $ root -b -q "OutputTrackInfo.C"
 * 
 * 
 * FINDING:
 * SimHeaders's "inputHeaders.m_entry" always starts from 0 for each new file.
 * Therefore one cannot make a TChain to deal with it at one time. I go for a
 * loop method.
 * 
 * 
 * July 2014 by Shih-Kai.
 */


#include <iostream>
#include <fstream>
#include <map>
#include "TChain.h"
#include "TLeaf.h"
#include "TROOT.h"
#include "TVector3.h"
#include "include/common.h"
#include "include/dybproject.h"
#include "include/TrackLength.h"


using namespace std;


TVector3 pointGlobal2Local(TVector3 p, int hall)
{
  map<int, TVector3> Ogl;
  Ogl[1] = TVector3(-16520, -802110, -7066);
  Ogl[2] = TVector3(471810, 63150, -3686);
  Ogl[4] = TVector3(-410670, 813170, -2216);
  
  map<int, double> rotationAngle;
  rotationAngle[1] = -122.90*TMath::DegToRad();
  rotationAngle[2] = 79.71*TMath::DegToRad();
  rotationAngle[4] = 150.55*TMath::DegToRad();
  TVector3 Op = p-Ogl[hall];
  Op.RotateZ(rotationAngle[hall]);
  
  return Op;
}


TVector3 vectorGlobal2Local(TVector3 v, int hall)
{
  map<int, double> rotationAngle;
  rotationAngle[1] = -122.90*TMath::DegToRad();
  rotationAngle[2] = 79.71*TMath::DegToRad();
  rotationAngle[4] = 150.55*TMath::DegToRad();
  v.RotateZ(rotationAngle[hall]);
  
  return v;
}


void OutputTrackInfo()
{
  /// load NuWa classes
  gROOT->ProcessLine("vector<PerGenParticle*> genPDummy");
  gROOT->ProcessLine("vector<PerGenVertex*> genVDummy");
  gROOT->ProcessLine(".L include/loader.C+");
  gROOT->ProcessLine(".x $ROOTIOTESTROOT/share/load.C");
  
  
  /// make output tree
  double skewd, skewa; /// distance and angle between the two skew lines
  double dcrec, dctru; /// distance from OWS center to the track
  int site;
  ofstream oftru("truetrackinfo.txt");
  ofstream ofrec("rectrackinfo.txt");
  ofstream ofrpcows("rpcows.txt");
  

  ifstream inf("filelist.txt");
  string tmpfn;
  while(inf >> tmpfn)
  {
    TChain chMuonTrue("/Event/Gen/GenHeader");
    TChain chSimHeader("/Event/Sim/SimHeader");
    TChain chCalib("/Event/CalibReadout/CalibReadoutHeader");
    TChain chRpcRec("/Event/Rec/RpcSimple");
    TChain chPoolRec("/Event/Rec/PoolSimple");
    TChain chMuonLoose("/Event/Tag/Physics/MuonLoose");
    chMuonTrue.Add(tmpfn.c_str());
    chSimHeader.Add(tmpfn.c_str());
    chCalib.Add(tmpfn.c_str());
    chRpcRec.Add(tmpfn.c_str());
    chPoolRec.Add(tmpfn.c_str());
    chMuonLoose.Add(tmpfn.c_str());

  
  
    PerGenHeader *perGenHeader = 0;
    chMuonTrue.SetBranchAddress("Gen_GenHeader", &perGenHeader);
    PerSimHeader *perSimHeader = 0;
    chSimHeader.SetBranchAddress("Sim_SimHeader", &perSimHeader);
    
    
    int triggerTimeSecPre = 0, triggerTimeNanoSecPre = 0;
    int triggerTimeSecRpc = 0, triggerTimeNanoSecRpc = 0;
    double x_gen = 0.0, y_gen = 0.0, z_gen = 0.0;
    double theta_gen = 0.0, phi_gen = 0.0, e_gen = 0.0;
    vector<int> goodGenHeaderEntries;
    map<int, MyLine> trackTruth;
    
    /// first loop to determine the GenHeader entries with reconstructed info
    int nEntries = chSimHeader.GetEntries();
    cout << "total sim header entries: " << nEntries << endl;
    for(int entry = 0; entry < nEntries; entry++)
    {
      chSimHeader.GetEntry(entry);
      goodGenHeaderEntries.push_back(chSimHeader.GetLeaf("inputHeaders.m_entry")->GetValue());
    }
    
    /// second loop to extract the muon truch info
    for(unsigned int i = 0; i < goodGenHeaderEntries.size(); i++)
    {
      int entNum = goodGenHeaderEntries[i];
      chMuonTrue.GetEntry(entNum);
      
      x_gen = perGenHeader->event->vertices[0]->position.x();
      y_gen = perGenHeader->event->vertices[0]->position.y();
      z_gen = perGenHeader->event->vertices[0]->position.z();
      theta_gen = perGenHeader->event->particles[0]->momentum.theta();
      phi_gen = perGenHeader->event->particles[0]->momentum.phi();
      e_gen = perGenHeader->event->particles[0]->momentum.e();
      
      trackTruth[entNum].p = TVector3(x_gen, y_gen, z_gen);
      trackTruth[entNum].v = TVector3(perGenHeader->event->particles[0]->momentum.x(), perGenHeader->event->particles[0]->momentum.y(), perGenHeader->event->particles[0]->momentum.z()).Unit();
    }
  
    
    nEntries = chMuonTrue.GetEntries();
    int genHdrPtr = 0;
    bool hitRPC = false, hitOWS = false;
    TVector3 pOWS, pRPC;
    vector<int> myTrackGenHeaderEntries;
    map<int, MyLine> myTrack;
    map<int, TVector3> verticesOWS; /// need RPC & OWS points data
    int currentSite;
    map<int, int> muonSite;
    for(int entry = 0; entry < nEntries; entry++)
    {
      /// if the upper bound explode, quit the loop
      if(genHdrPtr+1 == goodGenHeaderEntries.size()) break;
      
      chPoolRec.GetEntry(entry);
      if((int)chPoolRec.GetLeaf("detector")->GetValue() == 6)
      {
        hitOWS = true;
        pOWS.SetX(chPoolRec.GetLeaf("x")->GetValue()*1000);
        pOWS.SetY(chPoolRec.GetLeaf("y")->GetValue()*1000);
        pOWS.SetZ(chPoolRec.GetLeaf("z")->GetValue()*1000);
        currentSite = chPoolRec.GetLeaf("site")->GetValue();
      }
      
      chRpcRec.GetEntry(entry);
      if((int)chRpcRec.GetLeaf("detector")->GetValue() == 7)
      {
        hitRPC = true;
        pRPC.SetX(chRpcRec.GetLeaf("clusters.x")->GetValue());
        pRPC.SetY(chRpcRec.GetLeaf("clusters.y")->GetValue());
        pRPC.SetZ(chRpcRec.GetLeaf("clusters.z")->GetValue());
      }
      
      /// if entry is just befor the next good GenHeader, wrap up
      if(entry == goodGenHeaderEntries[genHdrPtr+1]-1)
      {
        if(hitRPC && hitOWS)
        {
          int genEnt = goodGenHeaderEntries[genHdrPtr];
          cout << "My track is found for GenHeader entry " << goodGenHeaderEntries[genHdrPtr] << endl;
          myTrackGenHeaderEntries.push_back(genEnt);
          myTrack[genEnt].p = pRPC;
          myTrack[genEnt].v = (pOWS-pRPC).Unit();
          verticesOWS[genEnt] = pOWS;
          muonSite[genEnt] = currentSite;
        }
        else
          cout << "My track is not found for GenHeader entry " << goodGenHeaderEntries[genHdrPtr] << endl;
        
        hitRPC = false;
        hitOWS = false;
        genHdrPtr++;
      }
    }
    
    for(unsigned int i = 0; i < myTrackGenHeaderEntries.size(); i++)
    {
      int entry = myTrackGenHeaderEntries[i];
      trackTruth[entry].p = pointGlobal2Local(trackTruth[entry].p, muonSite[entry]);
      trackTruth[entry].v = vectorGlobal2Local(trackTruth[entry].v, muonSite[entry]);
      
      TVector3 p1 = trackTruth[entry].p;
      TVector3 v1 = trackTruth[entry].v;
      TVector3 p2 = myTrack[entry].p;
      TVector3 v2 = myTrack[entry].v;
      TVector3 n = v1.Cross(v2).Unit();
      TVector3 pows = verticesOWS[entry];
      
      site = muonSite[entry];
      skewd = TMath::Abs(n.Dot(p1-p2));
      skewa = v1.Angle(v2);
      dctru = p1.Mag()*sin(p1.Angle(v1));
      dcrec = p2.Mag()*sin(p2.Angle(v2));
      
      oftru << p1.x() << " " << p1.y() << " " << p1.z() << " ";
      oftru << v1.x() << " " << v1.y() << " " << v1.z() << endl;
      ofrec << p2.x() << " " << p2.y() << " " << p2.z() << " ";
      ofrec << v2.x() << " " << v2.y() << " " << v2.z() << endl;
      if(p2.Mag() > 1)
      {
        ofrpcows << p2.x() << " " << p2.y() << " " << p2.z() << " ";
        ofrpcows << pows.x() << " " << pows.y() << " " << pows.z() << endl;
      }
    }
  } /// end of file loop

  
  oftru.close();
  ofrec.close();
  ofrpcows.close();
  
}
